/******************************************************************************
*									      *
*	File:     hex2dec.c						      *
*	Author:   Alison Chaiken <alchaiken@gmail.com>          	      *
*	Created:  Wed Jul 14 18:40:04 PDT 2010				      *
*	Contents: Convert hex-formatted numbers to decimal format and         *
*       and print the results.	Accepts input on command-line or from         *
*       stdin.                                                                *
*									      *
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

void usage(int bad_digit){
	printf("hex2dec: Illegal digit %c: use A-F, a-f and 0-9 with \
		leading 'Ox'.\n", bad_digit);
}

void process_token (char *str_token)
{
	int i, digits, startdigit, coeff, thisdigit;
	long unsigned outnum = 0;

	digits = strlen(str_token);
	if ((str_token[0] == 48) && (str_token[1] == 120))
		startdigit = 2;		/* skip over 0x prefix */
	else if ((str_token[0] >= 48) && (str_token[1] <= 57))
		startdigit = 0;
	else usage(str_token[0]);
	for (i = startdigit; i < digits; i++) {
		thisdigit = str_token[i];

		/* in ASCII, zero is 48, nine is 57 */
		if ((thisdigit >= 48) && (thisdigit <= 57))
		coeff = thisdigit - 48;
		/* A is 65, F is 70 */
		else if ((thisdigit >= 65) && (thisdigit <= 70))
		coeff = thisdigit - 55;
		/* a is 97, f is 102 */
		else if ((thisdigit >= 97) && (thisdigit <= 102))
		coeff = thisdigit - 87;
		else {
			usage(thisdigit);
			exit(1);
		}
		outnum += coeff * (long unsigned) pow(16, (digits - i - 1));
	}
	printf("%lu ", outnum);
	return;
}

void process_line(char *thisline)
{
	char *next_token;

	next_token = strtok(thisline," ");
	do {
		process_token(next_token);
	} while ((next_token = strtok(NULL," ")) != NULL) ;

}

int main(int argc, char *argv[])
{

	int i;
	
	char			instring[MAXSTRING], *stringp;

	if (argc > 1) { /* args to be converted on command line */
		strcpy(instring,argv[1]);
		for (i=2;i<argc;i++){
			strcat(instring," ");
			strcat(instring,argv[i]);
		}	
		process_line(instring);
	}

	else {
		/* args from stdin */
		while (fgets(instring,MAXSTRING,stdin) != NULL){
			stringp = instring;
			/* line from stdin ends in '\n' */
			/* while ((*stringp++ = getchar())!= '\n') */
			while (*stringp++ != '\n') 
				;
			/* make last character a null instead of '\n' */
			*--stringp='\0';
			process_line(instring);
		}
	}
	printf("\n");
	
	exit(0);
}
