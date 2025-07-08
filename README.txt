README file for pg2hda

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
SYNOPSIS

pg2hda is a tool to compute higher-dimensional automata modeling concurrent systems given by program graphs. 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
VERSION

2.5 (July 8, 2025) 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
AUTHOR

Thomas Kahl - Centro de Matemática, Universidade do Minho, Campus de Gualtar, 4710-057 Braga, Portugal - kahl@math.uminho.pt

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
COPYRIGHT

Copyright (c) 2018-2025 Thomas Kahl

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
LICENSE

pg2hda is distributed under the MIT License (see accompanying file COPYING.txt). 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
INCLUDED FILES

The package consists of the file README.txt (this file), the file COPYING.txt, the bash scripts mksage.sh and hlang.sh, a makefile (for GNU make), and three folders named examples, include, and src, which contain the following files:

	* examples:
		
		dining_philosophers.pml
		grocery_store.pml
		peterson.pml			
		
	* include:

		cube.h 
		def.h 
		hda.h
		io.h 
		list.h
 		pgraph.h 
		vector.h 

	* src:

		cube.c
		hda.c
		io.c
		list.c
		main.c
 		pgraph.c
 		pml2pg.l
 		pml2pg.y 
		vector.c
			
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
INSTALLATION

On Linux or under Cygwin on Windows (possibly also on macOS), run GNU make in the extraction directory to create the pg2hda executable in a folder named bin. 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
USAGE

From the command line, run

	path-to-pg2hda-executable input-file [options].
	
The input file must be written in Promela. Only a limited version of Promela is supported, see the included examples. The program computes an HDA model of the concurrent system described in the input file and writes it to the terminal. The available options are the following:
	
	-c	Instead of the HDA model, print its chain complex over Z_2 in the format used by P. Pilarczyk's original CHomP software.

	-i	Print only information on the input program graphs.

	-s	Print just a summary of the results of the computation.
	
	-t	Print the HDA model in a spreadsheet format (tsv).		
	
If more than one of these options is selected, only the last one is considered. If no option is chosen, the results of the computation are displayed in a verbose manner. In this case, the output format for cubes is the following:

	cube degree.number:		global state of the start vertex (locations of the processes, values of the variables)		labels of the edges starting in this state
	
Using the option --old, the program may be run as in the first versions. This possibility is likely to disappear in the future. 	
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXAMPLES

Suppose that pg2hda has been installed to a Linux system and that the working directory is PG2HDA-2.4 (the extraction directory).

1. An HDA model of the system given by Peterson's mutual exclusion algorithm is obtained by executing the following command:

	./bin/pg2hda examples/peterson.pml -t > examples/peterson.tsv
	
The output is redirected to the file peterson.tsv, which is created in the examples folder.	

2. The command

	./bin/pg2hda examples/dining_philosophers.pml -c > examples/chain.chn

yields the chain complex over Z_2 of the HDA model of the dining philosophers system, formatted as required for P. Pilarczyk's original CHomP software. The output is redirected to the file chain.chn, which is created in the examples folder. The Z_2-homology of the HDA may then be computed using CHomP by running the following command:

	path-to-homchain -p2 examples/chain.chn
	
In order to compute the Z_2-homology language of the HDA and to analyze it with SageMath, execute the following commands:

	./bin/pg2hda examples/dining_philosophers.pml -i > system_info_file
	./bin/pg2hda examples/dining_philosophers.pml -c > chain_complex_file	
	path-to-homchain -p2 chain_complex_file -g homology_generators_file
	./mksage.sh system_info_file homology_generators_file > HL.sage
	
If the directory containing homchain is in your PATH variable, you may instead simply run the following command:

	./hlang.sh examples/dining_philosophers.pml > HL.sage

The file HL.sage may then be loaded into SageMath. 
			
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
BUGS

Bug reports may be sent to kahl@math.uminho.pt.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
