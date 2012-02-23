/**
 * @file gif2bmp.c
 * @Synopsis  
 * @author Bc. Jaroslav Sendler <xsendl00ATstud.fit.vutbr.cz>
 * @version 0.1
 * @date 2012-02-23
 */
#include "gif2bmp.h"


int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

	fprintf(stderr, "funguje");
	openGif(inputFile);
	return 0;
}



int openGif(FILE *inputFile) {
	int data[20000];
	for(int i = 0;i<2000;i++){
		data[i]='\0';
	}
	int ch = 0;
	int i = 0;
	while((ch = fgetc(inputFile))!=EOF) {
	//	printf("%c",ch);
		data[i]=ch;
		i++;
	}
	//printf("....%d\n",i);

	// naplneni struktury tGIF daty
	tGIF gif = pullHead(data);
}


/**
 * Naplneni hlavicky daty
 */
tGIF pullHead(int data[]) {

	tHEADER header;
	// naplneni hlavicky
	header.signature[0] = data[0];
	header.signature[1] = data[1];
	header.signature[2] = data[2];
	header.version[0] = data[3];
	header.version[1] = data[4];
	header.version[2] = data[5];

	if(isGif(header.signature, header.version ) == 0) {																	// kontrola zdali je spravny soubor
		fprintf(stderr, "NENI OBRAZEK< DODELAT, uzavreni vsech souboru a vypis hlasky\n");
//		return 0;
	}

	tLOSCDES logDescription;

	logDescription.screenWidth = hexToDec(data[6], data[7]);
	logDescription.screenHeight = hexToDec(data[8], data[9]);
	logDescription.packedFields.globalColorMap = (data[10] & 128) >> 7;												// 7 bit, 1 =tab. existuje, 0= neexistuje
	logDescription.packedFields.colorResolutionBits = ((data[10] & 112)  >> 4) + 1;								// 4-6bit, barevna rozlisovaci hodnpta +1
	logDescription.packedFields.reserved = (data[10] & 8) >> 3;															// 3bit, navesti trideni tabulky barev
	logDescription.packedFields.pixelBits = (data[10] & 7) + 1;															// 0-2bit, velikost globalni tbulky +1	
//	fprintf(stderr,"///////////////%d\n\n",data[10]&7);
	printTPacked(logDescription.packedFields);
	logDescription.backgroundColorIndex = data[11];
	logDescription.pixelAspectRatio = (data[12] + 15) / 14;																	// pomer stran obrazku

	printTLoscdes(logDescription);
	tGIF gif;
	gif.header = header;
	gif.logDescription = logDescription;
	return gif;
}

void printTPacked(tPACKED packedFields) {
	fprintf(stderr, "7bit=%d, 4-6bit=%d, 3bit=%d, 0-2bit=%d\n", packedFields.globalColorMap, 
			                                                      packedFields.colorResolutionBits,
																					packedFields.reserved,
																					packedFields.pixelBits
			);
}

void printTLoscdes(tLOSCDES logDescription) {
	fprintf(stderr, "screenWidth=%d, screenHeight=%d\n",	logDescription.screenWidth,
																			logDescription.screenHeight
			);
	printTPacked(logDescription.packedFields);
	fprintf(stderr,"backgroundColorIndex=%d, pixelAspectRatio=%d\n",	logDescription.backgroundColorIndex,
																						 	logDescription.pixelAspectRatio
			);
}


unsigned short hexToDec(BYTE num1, BYTE num2) {
	if(num2 != 0) {
		return (num1 + num2 + 256);
	}
	else {
		return (num1 + num2);
	}
}

char isGif(BYTE signature[], BYTE version[] ) {
	if(signature[0] != 0x47 || signature[1] != 0x49 || signature[2] != 0x46) {								// neni gif
		fprintf(stderr, "CHYBA: Obrazek neni typu gif nebo je poskozen.");
		return 0;
	}

	if(version[0] == 0x38 && version[1] == 0x39 && version[2] == 0x61) {					// typ 89a
		return 89;
	}
	else if(version[0] == 0x38 && version[1] == 0x37 && version[2] == 0x61) {			// typ 87a
		return 87;
	}
	else {
		fprintf(stderr, "CHYBA: Obrazek neni verze 87a ani 89a nebo je poskozen.");
		return 0;
	}
	return 1;
}

