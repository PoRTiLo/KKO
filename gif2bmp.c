/**
 * @file gif2bmp.c
 * @Synopsis  
 * @author Bc. Jaroslav Sendler <xsendl00ATstud.fit.vutbr.cz>
 * @version 0.1
 * @date 2012-02-23
 */
#include "gif2bmp.h"
#include <iostream>
using namespace std;

tGIF2BMP gif2Bmp;

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

	fprintf(stderr, "funguje");
	pullGif(openGif(inputFile));
	return 0;
}



/**
 * @brief   Otevre obrazek a nacte binarni data do pole.
 * @param   
 * @return  
 */
vector<int> openGif(FILE *inputFile) {

	fseek(inputFile, 0L, SEEK_END);
	fprintf(stderr, "velikost soubor:%ld\n",ftell(inputFile));
	gif2Bmp.gifSize = ftell(inputFile);
	fseek(inputFile, 0L, SEEK_SET);

	vector<int> data;
	int ch = 0;
	while((ch = fgetc(inputFile))!=EOF) {
		data.push_back(ch);
	}

	return data;
}

tGIF pullGif(vector<int> data) {
	tGIF gif;
	gif.header = pullHead(data);
	data.erase(data.begin(), data.begin() + sizeof(gif.header));
	gif.logDescription = pullLogDesc(data);
	data.erase(data.begin(), data.begin() + sizeof(gif.logDescription) - sizeof(gif.logDescription.packedFields) + 1);
	if(gif.logDescription.packedFields.pixelBits != 1) {// ||  gif.logDescription.packedFields.pixelBits); // zpracovani globallni tabulky barev, pokud existuje
		gif.globalColor = pullGlobalColor(data, gif.logDescription.packedFields.pixelBits);
		data.erase(data.begin(), data.begin() + ((1 << gif.logDescription.packedFields.pixelBits))*3);
	}
	fprintf(stderr, "data 1 = %x\n",data[0]);
	fprintf(stderr, "data 1 = %x\n",data[1]);
// vyhledani urciteho bloku
	int pozicion = 0;
	while(pozicion < gif2Bmp.gifSize) {
		if(data[pozicion] == 0x21) {
			pozicion++;
			if(data[pozicion] == 0xF9) {
				gif.graphicControlExt = pullGraphicControlExt(data);
				data.erase(data.begin(), data.begin() + (sizeof(gif.graphicControlExt) - sizeof(gif.graphicControlExt.packedFields) ));
				pozicion = 0;
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
		
			gif.imageDescription = pullImageDescription(data);
			data.erase(data.begin(), data.begin() + (sizeof(gif.imageDescription) - sizeof(gif.imageDescription.packedFields) -1));
			if(gif.imageDescription.packedFields.color == 1) {																// pokud 1(true) je pritomna lokalni tabulka barev
				fprintf(stderr,"obsahuje lokalni tabulku barev\n");
			}
			// ctu jiz data
			int sizeLZW = data[pozicion++];																						// velikost minamilniho kodu LZW
			int sizeBlock = data[pozicion++];																					// velikost bloku dat
			vector<int> subBlock;
			bool endBlock = false;
			int i = 1;
			while(endBlock != true) {
				if(i <= sizeBlock) {
					//fprintf(stderr, "i:%d data v hexa=%x\n", i ,data[pozicion]);
					subBlock.push_back(data[pozicion++]);
					i++;
				}
				else if(data[pozicion] == 0x00) {
					fprintf(stderr, "konec bloku dat\n");
					endBlock = true;
				}
				else {//nacitam novy blok, zjisti jeho velikost a realokace pole
					fprintf(stderr, "Realokace i:%d data v hexa=%x\n", i ,data[pozicion]);
					sizeBlock = sizeBlock + data[pozicion++];
					fprintf(stderr, "sizeBlock=%d\n", sizeBlock);
				}
			}	
		}
		else {
		}
	}
	
	return gif;
}

/**
 * Naplneni hlavicky daty
 */
tHEADER pullHead(vector<int> data) {
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
	return header;
}

tLOSCDES pullLogDesc(vector<int> data) {
	tLOSCDES logDescription;																										// struktura obsahujici popisovac logicke obrazovky
	logDescription.screenWidth = hexToDec(data[0], data[1]);																// sirka obrazku v pixelech
	logDescription.screenHeight = hexToDec(data[2], data[3]);															// vyska obrazku v pixelech
	logDescription.packedFields.globalColorMap = (data[4] & 128) >> 7;												// 7 bit, 1 =tab. existuje, 0= neexistuje
	logDescription.packedFields.colorResolutionBits = ((data[4] & 112)  >> 4) + 1;								// 4-6bit, barevna rozlisovaci hodnpta +1
	logDescription.packedFields.reserved = (data[4] & 8) >> 3;															// 3bit, navesti trideni tabulky barev
	logDescription.packedFields.pixelBits = (data[4] & 7) + 1;															// 0-2bit, velikost globalni tabulky +1	
	logDescription.backgroundColorIndex = data[5];																			// index do tabulky barev, pozadi obrazku a ramecek
	logDescription.pixelAspectRatio = (data[6] + 15) / 64;																// pomer stran obrazku

	return logDescription;
}

vector<tGlobalColor> pullGlobalColor(vector<int> data, char pixelBits) {
// nacteni dat do globalni tabulky barev
	int countIndexTable = 1 << pixelBits;
	vector<tGlobalColor> aGColor;
	tGlobalColor globalColor;
	int pozicion = 0;
	for(int i = 0; i < countIndexTable; i++) {
		globalColor.red = data[pozicion++];
		globalColor.green = data[pozicion++];
		globalColor.blue = data[pozicion++];
		aGColor.push_back( globalColor);
	}

	return aGColor;
}

tGraphicControlExt pullGraphicControlExt(vector<int> data) {
	tGraphicControlExt graphicExt;
	graphicExt.extIntroducer = data[0];
	graphicExt.graphicControlLabel = data[1];
	graphicExt.blockSize = data[2];																		//0x04;;
	graphicExt.packedFields.colorFlag = data[3] & 1;											
	graphicExt.packedFields.input = (data[3] & 2)  >> 1;								
	graphicExt.packedFields.disposalMethod = (data[3] & 28) >> 2;				
	graphicExt.packedFields.reserved = (data[3] & 224) >> 5;						
	graphicExt.delayTime = hexToDec(data[4], data[5]);
	graphicExt.transparentColorIndex = data[6];
	graphicExt.blockTerminator = data[7];

	return graphicExt;
}
tImageDescription pullImageDescription(vector<int> data) {
	fprintf(stderr, "tady jsem u data = %x\n",data[0]);
	tImageDescription imageDesc;
	imageDesc.imageSeparator = data[0];
	imageDesc.imageLeftPos = hexToDec(data[1], data[2]);
	imageDesc.imageTopPos = hexToDec(data[3], data[4]);
	imageDesc.imageWIdth = hexToDec(data[5], data[6]);
	imageDesc.imageHeight = hexToDec(data[7], data[8]);
	imageDesc.packedFields.color = (data[9] & 1);
	imageDesc.packedFields.interleaved = (data[9] & 2) >> 1;
	imageDesc.packedFields.sort = (data[9] & 4) >> 2;
	imageDesc.packedFields.reserved = (data[9] & 18) >> 3;
	imageDesc.packedFields.size = (data[9] & 224) >> 5;
//printTPackedImg(imageDesc.packedFields);
	return imageDesc;
}

void sortData() {
	int i,j;
	vector<int> table1;
	vector<int> table2;
	int imageHeight = 16;
	for(i = 0; i <imageHeight; i++) {
		table1.push_back(i);
	}
	j = 0;
	for(i = 0; i < imageHeight; i += 8, j++) {
		table2[i] = table1[j];
	}
	for(i = 4; i < imageHeight; i += 8, j++) {
		table2[i] = table1[j];
	}
	for(i = 2; i < imageHeight; i += 4, j++) {
		table2[i] = table1[j];
	}
	for(i = 1; i < imageHeight; i += 2, j++) {
		table2[i] = table1[j];
	}
}
void printTGlobalColor(tGlobalColor color) {
//	for(int i = 0; i<countIndexTable; i++) {
//		fprintf(stderr,"red=%x, green=%x, blue=%x\n",color[i].red, color[i].green, color[i].blue);
//	}
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

