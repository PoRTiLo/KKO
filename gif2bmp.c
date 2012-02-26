/**
 * @file gif2bmp.c
 * @Synopsis  
 * @author Bc. Jaroslav Sendler <xsendl00ATstud.fit.vutbr.cz>
 * @version 0.1
 * @date 2012-02-23
 */
#include "gif2bmp.h"

//using namespace std;

tGIF2BMP gif2Bmp;

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
	fprintf(stderr, "velikost soubor:%ld\n",ftell(inputFile));
	gif2Bmp.gifSize = ftell(inputFile);
	fseek(inputFile, 0L, SEEK_SET);

	int *data;
	data = (int *) malloc((ftell(inputFile)+1) * (sizeof(int)));
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
	return 0;
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
	fprintf(stderr, "count=%d\n\n\n",countIndexTable);
	int sizeTable = 3 * countIndexTable;
	tGlobalColorT *aGColor;
	fprintf(stderr,"konec\n");
	aGColor = (tGlobalColorT *) malloc((countIndexTable+1) * (sizeof(tGlobalColorT )));
	//aGColor = (tGlobalColorT *) malloc(100 * 3);

	fprintf(stderr,"velikost samo%ld\n",sizeof(tGlobalColorT));
	fprintf(stderr,"velikost nova%ld\n",sizeof(aGColor));
	if(aGColor == NULL) {
		exit(0);
	}

	fprintf(stderr, "index=%d, size=%d\n",countIndexTable, sizeTable);
	//map<int, tGlobalColorT> mapGColor;
	fprintf(stderr,"konec\n");
	tGlobalColorT globalColor;
	fprintf(stderr,"konec\n");
	int pozicion = 13;
fprintf(stderr, "datapozicion=%d\n",pozicion);
	for(int i = 0; i < countIndexTable; i++) {
//fprintf(stderr, "datapozicion=%d\n",data[pozicion]);
		globalColor.red = data[pozicion++];
//fprintf(stderr, "datapozicion=%d\n",data[pozicion]);
		globalColor.green = data[pozicion++];
//fprintf(stderr, "datapozicion=%d\n",data[pozicion]);
		globalColor.blue = data[pozicion++];
//		mapGColor[i] = globalColor;
		aGColor[i] = globalColor;
//fprintf(stderr, "datapozicion=%d\n",pozicion);
	}
fprintf(stderr, "datapozicion=%d\n",pozicion);
	fprintf(stderr,"zacatek bloku %x\n",data[pozicion]);
	//printGlobalColorTable(mapGColor);

	fprintf(stderr,"konec\n");
for(int i = 0; i<countIndexTable;i++) {
	fprintf(stderr,"red=%x, green=%x, blue=%x\n",aGColor[i].red, aGColor[i].green, aGColor[i].blue);
}
// vyhledani urciteho bloku
	fprintf(stderr,"zacatek bloku %x\n",data[pozicion]);
	while(pozicion < gif2Bmp.gifSize) {
		if(data[pozicion] == 0x21) {
			pozicion++;
			if(data[pozicion++] == 0xF9) {
				tGraphicControlExt graphicExt;
				graphicExt.extIntroducer = 0x21;
				graphicExt.graphicControlLabel = 0xF9;
				graphicExt.blockSize = data[pozicion++];																		//0x04;;
				graphicExt.packedFields.colorFlag = data[pozicion] & 1;											
				graphicExt.packedFields.input = (data[pozicion] & 2)  >> 1;								
				graphicExt.packedFields.disposalMethod = (data[pozicion] & 28) >> 2;				
				graphicExt.packedFields.reserved = (data[pozicion++] & 224) >> 5;						
				graphicExt.delayTime = hexToDec(data[pozicion++], data[pozicion++]);
fprintf(stderr, "chyba v hexa=%x\n",data[pozicion]);
				graphicExt.transparentColorIndex = data[pozicion++];
fprintf(stderr, "chyba v hexa=%x\n",data[pozicion]);
				graphicExt.blockTerminator = data[pozicion++];
fprintf(stderr, "chyba v hexa=%x\n",data[pozicion]);
			}
			else if(data[pozicion] == 0xFE) {
			}
			else if(data[pozicion] == 0xFF) {
			}
			else if(data[pozicion] == 0x01) {
			}
			else {

			}
		}
		else if(data[pozicion] == 0x2c) {
			fprintf(stderr, "tady jsem u data = %x\n",data[pozicion]);
 			tImageDescription imageDesc;
			imageDesc.imageSeparator = data[pozicion++];
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
			imageDesc.imageLeftPos = hexToDec(data[pozicion++], data[pozicion++]);
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
			imageDesc.imageTopPos = hexToDec(data[pozicion++], data[pozicion++]);
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
			imageDesc.imageWIdth = hexToDec(data[pozicion++], data[pozicion++]);
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
			imageDesc.imageHeight = hexToDec(data[pozicion++], data[pozicion++]);
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
			imageDesc.packedFields.color = (data[pozicion] & 1);
			imageDesc.packedFields.interleaved = (data[pozicion] & 2) >> 1;
			imageDesc.packedFields.sort = (data[pozicion] & 4) >> 2;
			imageDesc.packedFields.reserved = (data[pozicion] & 18) >> 3;
			imageDesc.packedFields.size = (data[pozicion++] & 224) >> 5;
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
printTPackedImg(imageDesc.packedFields);
		
			if(imageDesc.packedFields.color == 1) {																			// pokud 1(true) je pritomna lokalni tabulka barev
			}

			// ctu jiz data
			int sizeLZW = data[pozicion++];																						// velikost minamilniho kodu LZW
			int sizeBlock = data[pozicion++];																					// velikost bloku dat
			int *subBlock;
			subBlock = (int *) malloc((sizeBlock+1) * (sizeof(int)));
			if(subBlock == NULL) {
				exit(0);
			}
			char endBlock ='f';
			int i = 1;
			while(endBlock != 't') {
				if(i <= sizeBlock) {
					fprintf(stderr, "i:%d data v hexa=%x\n", i ,data[pozicion]);
					subBlock[i] = data[pozicion++];
				}
				else {//nacitam novy blok, zjisti jeho velikost a realokace pole
					fprintf(stderr, "Realokace i:%d data v hexa=%x\n", i ,data[pozicion]);
					sizeBlock = sizeBlock + data[pozicion];
					fprintf(stderr, "sizeBlock=%d\n", sizeBlock);
					int * pom = NULL;
					int * pom2;
					int *pom3;
					pom = (int *) malloc( 2*sizeof(int)+1);
					pom2 = (int *)realloc(pom, 10*sizeof(int)+1);
//					subBlock = (int *) realloc((int*)subBlock, sizeof(sizeBlock));
						fprintf(stderr, "CHYBA REALOKACE\n");
//					if((pom == NULL) && (sizeBlock)) {
						fprintf(stderr, "CHYBA REALOKACE\n");
						free(subBlock);
						exit(0);
//					}
//					else {
//						subBlock = pom;
//					}
				}
				i++;
			}	
fprintf(stderr, "chyba? data v hexa=%x\n",data[pozicion]);
		}
		else {
		}
	}
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

void printTPackedEx(tPACKEDEX packedFields) {
	fprintf(stderr, "5-7bit=%d, 2-4bit=%d, 1bit=%d, 0bit=%d\n", packedFields.reserved, 
			                                                      packedFields.disposalMethod,
																					packedFields.input,
																					packedFields.colorFlag
			);
}

void printTPackedImg(tPACKEDIMG packedFields) {
	fprintf(stderr, "5-7bit=%d, 3-4bit=%d, 2bit=%d, 1bit=%d, 0bit=%d\n", packedFields.size, 
			                                 			                     packedFields.reserved,
																								packedFields.sort,
																								packedFields.interleaved,
																								packedFields.color
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

//void printGlobalColorTable(map<int,tGlobalColorT> colorTable) {
//	map<int,tGlobalColorT>::iterator i;
//	for(i = colorTable.begin(); i != colorTable.end(); i++) {
//		fprintf(stderr,"%d.red=%d, green=%d, blue=%d\n",i->first, i->second.red,i->second.green, i->second.blue);
//	}
//}

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

