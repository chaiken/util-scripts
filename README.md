### Here are some simple C programs that are useful to systems programmers.

1. _cpumask_ calculates hexadecimal cpumasks that are useful with, for example, /usr/bin/taskset from [util-linux](git clone git://git.kernel.org/pub/scm/utils/util-linux/util-linux.git).

2. _hex2dec_ and _dec2hex_ perform the format conversions that should be obvious from their names.   They will read either from stdin or from the command-line, making the following the obvious test:

   	     $ hex2dec 0xFFF | dec2hex<br/>
	     0xFFF

3. _hexsum_ is a bash script that performs addition or substraction on a pair of hex numbers by invoking hex2dec.

4. _watch\_file_ and _watch\_one\_file_ provide a simple method for the user to spy on which files another program is accessing without generating the giant spew of strace.

5. _cdecl_ is a program originally suggested by Kernighan and Ritchie on p. 100 of the Second "ANSI C" Edition of  _The C Programming Language_ in the section "Complicated Declarations."   They say "we will present a pair of programs that convert from valid C to a word description and back again. The word description reads left to right."   I only wrote the program that converts C into English.   K&R present a simpler version of cdecl as an example, while more featureful versions like that presented here are outlined in Exercises 5-18 and 5-20.

     The problem is also discussed by Peter van der Linden in the Programming Challenge  of Chapter 3, p. 85 of _Expert C Programming: Deep C Secrets_, p. 85 of Chapter 3.   The challenge is "Write a program to translate C declarations into English".   I wrote this version without first consulting either of van der Linden's or K&R's versions.   Theirs are more elegant, but I have confirmed that mine reproduces their answers.   Caveat emptor!

    _cdecl_ also works either with command-line or stdin input.   In the latter case, use '-' as an argument after the "cdecl" command:

	     $ echo "char *foo;" | cdecl -<br/>
	     foo is a(n) pointer(s) to char
 
    Processing command line input adds considerable complexity to the program, as does the limited error checking.   I've attempted to support all the features of C declarations described in ANSI C with the exception of those noted in the usage() output.

Alison Chaiken<br/>
alison@she-devel.com

