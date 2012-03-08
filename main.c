/**
 *       @file  main.c
 *      @brief  
 *
 * Detailed description starts here.
 *
 *     @author  Bc. Jaroslav Sendler (xsendl00), xsendl00@stud.fit.vutbr.cz
 *
 *   @internal
 *     Created  03/08/2012
 *     Company  FIT-VUT, Brno
 *   Copyright  Copyright (c) 2012, Bc. Jaroslav Sendler
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

#include "gif2bmp.h"


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Na standardni vystup bude vytistena napovead a nasledne aplikace
 *            ukoncena.
 */
/* ----------------------------------------------------------------------------*/
void print_help() {
	fprintf(stdout,"\n\
		Vstupni parametry\n\n\
	-i <ifile>   název vstupního souboru <ifile>. Pokud parametr nebude zadán, bude se za \n\
	             vstupní soubor považovat standardní vstup (stdin).\n\
	-o <ofile>   název výstupního souboru <ofile>. Pokud parametr nebude zadán, bude se za \n\
	             vstupní soubor považovat standardní výstup (stdout).\n\
	-l <logfile> název souboru výstupní zprávy <logfile>. Pokud parametr nebude zadán, tak \n\
	             výpis zprávy bude ignorován.\n\
	-h           vypíše nápovědu na standardní výstup a ukončí se.\n\n\
");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis   
 *
 * @Param      argc Pocet vstupnich parametru
 * @Param      argv Vstupni parametry
 *
 * @Returns    0 pokud vse v poradku jinak -1
 */
/* ----------------------------------------------------------------------------*/
int main(int argc, char **argv) {

	int c;
	int8_t result = 0;
	bool printLog = false;
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
				printLog = true;
				logFile = optarg;
				break;
			case 'h':
				print_help();
				return 0;
			case '?':
				if(optopt == 'i' || optopt == 'o' || optopt == 'l') {
					fprintf(stderr, "CHYBA: Prepinac -%c vyzaduje argument.\n", optopt);
					return -1;
				}
				else if(isprint(optopt)) {
					fprintf(stderr, "CHYBA: Neznamy prepinac'-%c'.\n", optopt);
					return -1;
				}
				else {
					fprintf(stderr, "CHYBA: Neznamy znak '\\x%x'.\n", optopt);
					return -1;
				}
				break;
			default:
				fprintf(stderr,"CHYBA: Neznama chyba\n");
				break;
		}
	}

	if(outputFile != NULL) {                                                    // otevreni vystupniho obrazku
		output = fopen(outputFile, "wb");
		if(!output) {
			fprintf(stderr, "CHYBA: Nepovedlo se otevrit vystupni obrazek!\n");
			result = -1;
		}
	}

	if(inputFile != NULL) {                                                     // otevreni vstupniho obrazku
		input = fopen(inputFile, "rb");
		if(!input) {
			fprintf(stderr, "CHYBA: Nepovedlo se otevrit vstupni obtrazek!\n");
			result = -1;
		}
	}

//------------------------------ prace s obrazkem -----------------------------//

	if(result != -1) {
		tGIF2BMP gif2bmpSt;
		if((result = gif2bmp(&gif2bmpSt, input, output)) != -1 && printLog == true) {
			if(logFile != NULL) {                                                     // otevreni vstupniho obrazku
				logput = fopen(logFile, "w");
				if(!logput) {
					fprintf(stderr, "CHYBA: Nepovedlo se otevrit vystupni soubor pro log!\n");
					result = -1;
				}
			}
			if(result != -1) {
				fprintf(logput, "login = xsendl00\nuncodedSize = %lld\ncodedSize = %lld\n", gif2bmpSt.gifSize, gif2bmpSt.bmpSize);
			}
		}
	}

//-----------------------------------------------------------------------------//

	fclose(output);                                                             // uzavreni vystupniho obrazku
	fclose(input);                                                              // uzavreni vstupniho obrazku
	if(printLog != false) {
		fclose(logput);                                                          // uzavreni vystupni zpravy
	}

	fprintf(stderr,"Jsme na konci!\n");
	return result;
}
