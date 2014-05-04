/******************************************************************************
*									      *
*	File:     dec2hex.c						      *
*	Author:   Alison Chaiken <alison@hildesheim>			      *
*	Created:  Sat Mar  8 18:59:36 CET 2014				      *
*	Contents: Convert decimal-formatted numbers to hex format and         *
*       and print the results.	Accepts input on command-line or from         *
*       stdin.                                                                *
*									      *
*	Copyright (c) 2014 Alison Chaiken.				      *
*       Distributed under version 3 of the GNU General Public License,        *
*       see http://www.gnu.org/licenses/gpl.html for more info.               *
*									      *
******************************************************************************/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXSTRING 100
#define BASE 16

void usage(void){
	fprintf(stderr, "dec2hex: illegal input.\n");
	exit(-1);
}

void process_token (char *str_token) {
	int digits;
	long long unsigned thistoken = 0, ratio, divisor, outnum = 0;

	digits = strlen(str_token);
	if (!digits) return;

	/* handle '0' input */
	if ((str_token[0] == '0') && (digits == 1))
		digits = 0;

	if ((str_token[0] == '0') && ((str_token[1] == 'x') || (str_token[1]
								== 'X'))) {
		digits -= 2;
		str_token += 2;
	}
	if (digits && ((sscanf(str_token, "%llu", &thistoken) ==
			EOF) || (thistoken == 0)))
		usage();
	while (thistoken && digits) {
		divisor = (long long unsigned) pow(BASE, (digits-1));
		ratio = thistoken/divisor;
		outnum += (long long unsigned) ratio*pow(BASE, (digits - 1));
		thistoken -= (ratio*divisor);
		digits--;
	}
	switch (BASE) {
		case 16:
			printf("0x%llX\n", outnum);
			break;
		case 10:
		case 2:
			printf("%llu\n", outnum);
			break;
		case 8:
			printf("0%llo\n", outnum);
			break;
		default:
			printf("Unsupported format\n");
			exit(-1);
	}

	return;
}


void process_line(char *thisline) {
	char *next_token;

	next_token = strtok(thisline," ");
	do {
		process_token(next_token);
	} while ((next_token = strtok(NULL," ")) != NULL) ;
}

int main(int argc, char *argv[]) {
	int i;
	char			instring[MAXSTRING], *stringp;

	if (argc > 1){ /* args to be converted on command line */
	strcpy(instring,argv[1]);
	for (i=2;i<argc;i++){
		strcat(instring," ");
		strcat(instring,argv[i]);
	}
	process_line(instring);

	}

	else {
		/* args from stdin */
		while (fgets(instring,MAXSTRING,stdin) != NULL) {
			stringp = instring;
			/* line from stdin ends in '\n' */
			/*		while ((*stringp++ = getchar())!= '\n') */
		while (*stringp++ != '\n') 
			;
		/* make last character a null instead of '\n' */
		*--stringp='\0';
	process_line(instring);
		}
	}

	exit(0);

}
