#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

#include "gif2bmp.h"

#define LOGIN xsendl00

/**
 * Napoveda.
 **/
void print_help() {
	fprintf(stdout,"help");
}

/**
 * Tisk vystupni zpravy.
 **/
void print_message( const char * message ) {
fprintf(stderr, "%s\n",message);
}

int main(int argc, char **argv) {

	int c;
	char printLog = 'f';
	const char *argument = "ho:l:i:";

	const char *outputFile = NULL;
	const char *inputFile = NULL;
	const char *logFile = NULL;

	FILE *input = stdout;
	FILE *output = stdin;
	FILE *logput = stdout;

	while((c = getopt(argc, argv, argument)) != -1) {
		switch(c) {
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 'l':
				printLog = 't';
				logFile = optarg;
				break;
			case 'h':
				print_help();
				return 1;
			case '?':
				if(optopt == 'i' || optopt == 'o' || optopt == 'l') {
					fprintf(stderr, "Prepinac -%c vyzaduje argument.\n", optopt);
				}
				else if(isprint(optopt)) {
					fprintf(stderr, "Neznamy prepinac'-%c'.\n", optopt);
				}
				else {
					fprintf(stderr, "Neznamy znak '\\x%x'.\n", optopt);
				}
				return 1;
			default:
				fprintf(stderr,"chyba");
				break;
		}
	}

	if(outputFile != NULL) {                                                    // otevreni vystupniho obrazku
		output = fopen(outputFile, "wb");
		if(!output) {
			fprintf(stderr, "Nepovedlo se otevrit vstupni obrazek!\n");
			return 1;
		}
	}

	if(inputFile != NULL) {                                                     // otevreni vstupniho obrazku
		input = fopen(inputFile, "rb");
		if(!input) {
			fprintf(stderr, "Nepovedlo se otevrit vystupni obtrazek!\n");
		}
	}


//------------------------------ prace s obrazkem -----------------------------//

	tGIF2BMP *gif2bmpSt;
	gif2bmp(gif2bmpSt, input, output);

//-----------------------------------------------------------------------------//

	fclose(output);                                                             // uzavreni vystupniho obrazku
	fclose(input);                                                              // uzavreni vstupniho obrazku
	if(printLog != 'f') {
		fclose(logput);                                                          // uzavreni vystupni zpravy
	}

	fprintf(stderr,"Jsme na konci!\n");
	return 0;
}
