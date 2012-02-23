/**
 * @file gif2bmp.c
 * @Synopsis  
 * @author Bc. Jaroslav Sendler <xsendl00ATstud.fit.vutbr.cz>
 * @version 0.1
 * @date 2012-02-23
 */
#include "gif2bmp.h"

using namespace std;

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

	fprintf(stderr, "funguje");
	openGif(inputFile);
	return 0;
}



/**
 * @brief   Otevre obrazek a nacte binarni data do pole.
 * @param   
 * @return  
 */
int openGif(FILE *inputFile) {

	fseek(inputFile, 0L, SEEK_END);
	fprintf(stderr, "velikost soubor:%d\n",ftell(inputFile));
	fseek(inputFile, 0L, SEEK_SET);

	int *data;
	data = (int *) malloc((ftell(inputFile)+1) * (sizeof(int *)));
	if(data == NULL) {
		exit(0);
	}
	//for(int i = 0;i<2000;i++){
	//	data[i]='\0';
	//}
	int ch = 0;
	int i = 0;
	while((ch = fgetc(inputFile))!=EOF) {
		data[i]=ch;
		i++;
	}

	// naplneni struktury tGIF daty
	tGIF gif = pullHead(data);
}


/**
 * Naplneni hlavicky daty
 */
tGIF pullHead(int data[]) {

	tHEADER header;																													// struktura obsahujici hlavicku gifu
	// naplneni hlavicky
	header.signature[0] = data[0];																								// 0x47 G
	header.signature[1] = data[1];																								// 0x49 I
	header.signature[2] = data[2];																								// 0x46 F
	header.version[0] = data[3];																									// 0x38 8
	header.version[1] = data[4];																									// 0x39 9 nebo 0x37 7
	header.version[2] = data[5];																									// 0x61 a

	if(isGif(header.signature, header.version ) == 0) {																	// kontrola zdali je spravny soubor
		fprintf(stderr, "NENI OBRAZEK< DODELAT, uzavreni vsech souboru a vypis hlasky\n");
		exit(0);
		//return ;0;
	}

	tLOSCDES logDescription;																										// struktura obsahujici popisovac logicke obrazovky

	logDescription.screenWidth = hexToDec(data[6], data[7]);																// sirka obrazku v pixelech
	logDescription.screenHeight = hexToDec(data[8], data[9]);															// vyska obrazku v pixelech
	logDescription.packedFields.globalColorMap = (data[10] & 128) >> 7;												// 7 bit, 1 =tab. existuje, 0= neexistuje
	logDescription.packedFields.colorResolutionBits = ((data[10] & 112)  >> 4) + 1;								// 4-6bit, barevna rozlisovaci hodnpta +1
	logDescription.packedFields.reserved = (data[10] & 8) >> 3;															// 3bit, navesti trideni tabulky barev
	logDescription.packedFields.pixelBits = (data[10] & 7) + 1;															// 0-2bit, velikost globalni tbulky +1	
	logDescription.backgroundColorIndex = data[11];																			// index do tabulky barev, pozadi obrazku a ramecek
	logDescription.pixelAspectRatio = (data[12] + 15) / 64;																// pomer stran obrazku

//-----------------TIKS
	printTLoscdes(logDescription);
//-----------------------------------------------

// nacteni dat do globalni tabulky barev
	int countIndexTable = 1 << logDescription.packedFields.pixelBits;
	int sizeTable = 3 * countIndexTable;
	fprintf(stderr, "index=%d, size=%d\n",countIndexTable, sizeTable);
	map<int, tGlobalColorT> mapGColor;
	tGlobalColorT globalColor;
	int pozicion = 13;
fprintf(stderr, "datapozicion=%d\n",pozicion);
	for(int i = 0; i < countIndexTable; i++) {
		globalColor.red = data[pozicion++];
fprintf(stderr, "datapozicion=%d\n",pozicion);
		globalColor.green = data[pozicion++];
fprintf(stderr, "datapozicion=%d\n",pozicion);
		globalColor.blue = data[pozicion++];
fprintf(stderr, "datapozicion=%d\n",pozicion);
		mapGColor[i] = globalColor;
fprintf(stderr, "datapozicion=%d\n",pozicion);
	}
fprintf(stderr, "datapozicion=%d\n",pozicion);
	fprintf(stderr,"zacatek bloku %x\n",data[pozicion]);
	printGlobalColorTable(mapGColor);

	fprintf(stderr,"konec\n");

// vyhledani urciteho bloku
	fprintf(stderr,"zacatek bloku %x\n",data[pozicion]);
	tGIF gif;
	gif.header = header;
	gif.logDescription = logDescription;
	return gif;
}

/**
 * @brief   Tisk struktury tPACKED.
 * @param   Struktura tPACKED
 * @return
 */
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

void printGlobalColorTable(map<int,tGlobalColorT> colorTable) {
	map<int,tGlobalColorT>::iterator i;
	for(i = colorTable.begin(); i != colorTable.end(); i++) {
		fprintf(stderr,"%d.red=%d, green=%d, blue=%d\n",i->first, i->second.red,i->second.green, i->second.blue);
	}
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

