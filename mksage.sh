#!/usr/bin/env bash

#*************************************************************************************************
#
#	Copyright (c) 2018-2024 Thomas Kahl
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in all
#	copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#	SOFTWARE.
#
#*************************************************************************************************
# pg2hda - Compute higher-dimensional automata modeling concurrent systems given by program graphs
# File mksage.sh
#
# This bash script computes the homology language of a system for the use in SageMath from files  
# containing the system information and the homology generators.
#*************************************************************************************************

if [ -z "$TMPDIR" ] ; then TMPDIR=/tmp ; fi
tmpdir=$(mktemp -d $TMPDIR/.tmpXXXXXX)

printf "Lambda.<" 
b=0
c=0
d=0
while read -a line 
do 
	if [ "$b" -eq "0" ]
	then 
		if [ "${line[1]}" = "actions" ] || [ "${line[1]}" = "action" ]
		then
			b=${line[0]} 
			c=1 
		fi	
	else
		if [ "${#line[@]}" != "0" ] && [ "$c" -eq "1" ]
		then
			if [ "$d" -eq "0" ]
			then 
				printf "${line[0]}" 
				d=1
			else 
				printf ",${line[0]}" 
			fi		
			c=0
		elif [ "${#line[@]}" = "0" ]  && [ "$c" -eq "0" ]
		then
			((b--))	
			c=1		 
		fi				  
	fi
done < $1

printf "> = ExteriorAlgebra(GF(2))\n" 
printf "ngens = Lambda.ngens()\n"
printf "B = Family(Lambda.gens())\n"

cat $2 > $tmpdir/chainHgen
printf "\n" >> $tmpdir/chainHgen	

dim=-1
while read -a line 
do 
    if [ "${line[0]:0:3}" = "[H_" ]
    then 
        dim=$((dim+1))      
    fi
done < $tmpdir/chainHgen

for ((i=1; i<=dim; i++))
do	
	printf "k0 = -1\n" >> $tmpdir/sagefile
	printf "S = []\n" >> $tmpdir/sagefile		
	for ((j=1; j<=i; j++))
	do
		for ((l=1; l<j; l++))
		do	
			printf "\t" >> $tmpdir/sagefile
		done
		printf "for k$j in IntegerRange(k$((j-1))+1,ngens):\n"  >> $tmpdir/sagefile					
	done
	for ((l=1; l<=i; l++))
	do	
		printf "\t"  >> $tmpdir/sagefile
	done
	printf "x = Lambda.one()\n" >> $tmpdir/sagefile
	for ((j=1; j<=i; j++))
	do
		for ((l=1; l<=i; l++))
		do	
			printf "\t"  >> $tmpdir/sagefile
		done
		printf "x = x*B.unrank(k$j)\n" >> $tmpdir/sagefile 					
	done	
	for ((l=1; l<=i; l++))
	do	
		printf "\t"  >> $tmpdir/sagefile
	done
	printf "S.append(x)\n" >> $tmpdir/sagefile 							
	printf "B$i = Family(S)\n" >> $tmpdir/sagefile
	printf "V$i = VectorSpace(GF(2),B$i.cardinality())\n" >> $tmpdir/sagefile
	printf "W$i = Family(V$i.basis())\n" >> $tmpdir/sagefile
	printf "def v$i(x):
\tSUM = V$i.zero()
\tF = Family(x.support())
\tfor y in F:
\t\tx = Lambda.one()\n" >> $tmpdir/sagefile	
	for ((j=0; j<i; j++))
	do
		printf "\t\tx = x*B[y[$j]]\n" >> $tmpdir/sagefile	
	done
	printf "\t\tSUM = SUM + W$i.unrank(B$i.rank(x))
\treturn SUM\n" >> $tmpdir/sagefile
	printf "G$i = [" >> $tmpdir/sagefile
	b=0
	d=0
	while read -a line 
	do 
		q=1
		if [ "$b" -eq "0" ]
		then 
			if [ "${line[0]}" = "[H_$i]" ]
			then
				b=1 
			fi	
		else
			q=0	
			if [ "${#line[@]}" = "0" ]
			then
				if [ "$b" -eq "1" ]
				then 
					q=1			
				fi
				break
			fi
			if [ "$d" -gt "0" ]
			then				
				printf "," >> $tmpdir/sagefile
			fi
			((d++))	
			printf "v$i(" >> $tmpdir/sagefile			
			c=${#line[@]} 	
			for ((j = 0; j < c; j++))
			do 
				printf "${line[j]#*:}" >> $tmpdir/sagefile			
			done 
			printf ")" >> $tmpdir/sagefile 
			((b++))				  			
		fi
	done < $tmpdir/chainHgen
	printf "]\n" >> $tmpdir/sagefile
	printf "M$i = matrix(GF(2),G$i,sparse=True)\n" >> $tmpdir/sagefile
	printf "N$i = M$i.image()\n" >> $tmpdir/sagefile
	printf "def l$i(vec):
\tSUM = Lambda.zero()
\tfor i in range(V$i.dimension()):
\t\tSUM = SUM + vec[i]*B$i[i]        
\treturn SUM\n" >> $tmpdir/sagefile
	printf "HL$i = []\n" >> $tmpdir/sagefile
	printf "for i in range(N$i.rank()):
\tHL$i.append(l$i(N$i.basis()[i]))\n" >> $tmpdir/sagefile
	echo -e "HL${i}_matrix = N${i}.matrix()\n" >> $tmpdir/sagefile
	if [ "$q" -eq "0" ]
	then 
		cat $tmpdir/sagefile		
	fi
	cat /dev/null > $tmpdir/sagefile
done
printf "print(\"Homology language loaded.\")\n"
printf "print(\"The exterior algebra under consideration is Lambda. To view its generators, type 'Lambda.gens()'.\")\n"
printf "print(\"For each n between 1 and the dimension of the homology language, HLn is a list containing a basis of the homology language in degree n.\")\n"
printf "print(\"For an element x of Lambda of such a degree n, vn(x) is the coordinate vector of x with respect to the lexicographically ordered basis of monomials of degree n.\")\n"
printf "print(\"HLn_matrix is a matrix whose lines are the coordinate vectors of the elements of HLn.\")\n"
printf "print(\"An element x of degree n is in the homology language if and only if the command 'HLn_matrix.dense_matrix().solve_left(vn(x))' returns a solution.\")\n"
rm -r $tmpdir
exit 0
