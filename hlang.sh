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
# File hlang.sh
#
# This bash script calls pg2hda, homchain (from P. Pilarczyk's original CHomP) and mksage.sh to 
# compute the homology language of a shared-variable system described in Promela for the use in 
# SageMath. 
#*************************************************************************************************

if [ -z "$TMPDIR" ] ; then TMPDIR=/tmp ; fi
tmpdir=$(mktemp -d $TMPDIR/.tmpXXXXXX)

# if the script is not run in the extraction directory, the next two commands have to be changed accordingly

./bin/pg2hda "$@" -i > $tmpdir/system_info_file
./bin/pg2hda "$@" -c > $tmpdir/chain_complex_file

# if the directory containing homchain is not in the PATH variable, the next command has to be changed accordingly

homchain -p2 $tmpdir/chain_complex_file -g $tmpdir/homology_generators_file >> /dev/null

# if the script is not run in the extraction directory, the next command has to be changed accordingly

./mksage.sh $tmpdir/system_info_file $tmpdir/homology_generators_file

rm -r $tmpdir
exit 0
