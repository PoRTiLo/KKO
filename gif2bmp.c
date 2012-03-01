/**
 * @file gif2bmp.c
 * @Synopsis  
 * @author Bc. Jaroslav Sendler <xsendl00ATstud.fit.vutbr.cz>
 * @version 0.1
 * @date 2012-02-23
 */
#include "gif2bmp.h"
#include <iostream>
#include <inttypes.h>
#include <sstream>
using namespace std;

tGIF2BMP gif2Bmp;

int32_t gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

	pullGif(openGif(inputFile));
	return 0;
}



/**
 * @brief   Otevre obrazek a nacte binarni data do pole.
 * @param   
 * @return  
 */
vector<int32_t> openGif(FILE *inputFile) {

	// zjisteni velikosti souboru
	fseek(inputFile, 0L, SEEK_END);																								// nastaveni ukazatele v souboru na konec
	gif2Bmp.gifSize = ftell(inputFile);																							// ulozeni velikosti souboru do struktury
	fseek(inputFile, 0L, SEEK_SET);																								// nastaveni ukazatele v souboru n azacatek

	// nacteni obrazku do vectoru
	vector<int32_t> data;																											// kontejner obsahujici nactena data z obrazku
	int32_t ch = 0;
	while((ch = fgetc(inputFile))!=EOF) {																						// nacitam po znacich dokud neni konec souboru
		data.push_back(ch);
	}

	return data;
}

tGIF pullGif(vector<int32_t> data) {

	tGIF gif;																															// struktura obsahujici vsechny informace o obrazku
	gif.header = pullHead(data);																									// naplneni struktury hlavicky 
	data.erase(data.begin(), data.begin() + sizeof(gif.header));														// odebrani dat obsazenych v hlavicce
	gif.logDescription = pullLogDesc(data);																					// naplneni struktury Lokalni popisova cobrazku
	data.erase(data.begin(), data.begin() + sizeof(gif.logDescription) - sizeof(gif.logDescription.packedFields) + 1);	// mazani pouzitych dat
// pokud existuje globalni tabulka barev, zpracuj ji
	if(gif.logDescription.packedFields.pixelBits != 1) {																	// zpracovani globallni tabulky barev, pokud existuje
		gif.globalColor = pullGlobalColor(data, gif.logDescription.packedFields.pixelBits);						// naplneni struktury globalni tabulky
		data.erase(data.begin(), data.begin() + ((1 << gif.logDescription.packedFields.pixelBits))*3);		// mazani pouzityhc dat
	}

// vyhledani urciteho bloku, bud samotnych bunarnich dat obrazku nebo nektereho rozsireni
	uint32_t pozicion = 0;																											// pozice prochazeni v datech
	vector<int32_t> subBlock;																										// vector obsahujici binarni data v LZW kode
	while(pozicion < gif2Bmp.gifSize) {
// rozsireni
		if(data[pozicion] == 0x21) {
			pozicion++;
			if(data[pozicion] == 0xF9) {																							// rozsireni rizeni grafiky
				gif.graphicControlExt = pullGraphicControlExt(data);
				data.erase(data.begin(), data.begin() + (sizeof(gif.graphicControlExt) - sizeof(gif.graphicControlExt.packedFields) ));	// mazani pouzitych dat
				pozicion = 0;																											// nastaveni pozice na zacatek
			}
			else if(data[pozicion] == 0xFE) {																					// rozsireni komentare
			}
			else if(data[pozicion] == 0xFF) {																					// rozsireni pro aplikaci
			}
			else if(data[pozicion] == 0x01) {																					// rozsireni standardniho textu
			}
			else {

			}
		}
		else if(data[pozicion] == 0x2c) {																						// binarni data kodovana v LZW
		
			gif.imageDescription = pullImageDescription(data);																// naplneni struktury lokalni popisovac predlohy
			data.erase(data.begin(), data.begin() + (sizeof(gif.imageDescription) - sizeof(gif.imageDescription.packedFields) -1));	//mazani
// pokud je pritomna lokalni tabulka barev, zpracuj ji
			if(gif.imageDescription.packedFields.color == 1) {																// pokud 1(true) je pritomna lokalni tabulka barev
				fprintf(stderr,"obsahuje lokalni tabulku barev\n");														// TODO: zpracova lokalni tabulku barev
			}
// cteni binarnich dat v LZW kodu
			gif.sizeLZW = data[pozicion++];																							// velikost minamilniho kodu LZW
			int32_t sizeBlock = data[pozicion++];																				// velikost bloku dat
			bool endBlock = false;																									// ukonceni cteni, podle delky a kodu 0x00
			int16_t i = 1;																												// aktualne nacteny znak
			while(endBlock != true) {
				if(i <= sizeBlock) {																									// ctu jeden blok
					subBlock.push_back(data[pozicion++]);																		// vlozeni dat do subBloku
					i++;
				}
				else if(data[pozicion] == 0x00) {																				// nacteni konce toku binarnich dat
					endBlock = true;																									// ukonceni
				}
				else {																													//nacitam novy blok, zjisti jeho velikost a realokace pole
					sizeBlock = sizeBlock + data[pozicion++];																	// pricteni k velikosti velikost dalsiho bloku
				}
			}
			break;																														// TODO: dalsi rozsireni, ted natvrdo ukoncim po precteni
		}
		else {
		}
	}
	decodeLZW(subBlock, gif);
	return gif;
}
void decodeLZW(vector<int32_t> subBlock, tGIF gif) {

	// inicializace slovniku
	vector<vector<int16_t> > dict;																								// TODO: asi string
	int16_t dictSize = (1 << gif.logDescription.packedFields.pixelBits)+2;											// inicializace slovniku, velikost - pocet barev + CC + EOI
	for(uint32_t i = 0; i < dictSize; i++) {
		dict.push_back(vector<int16_t>());
	}
	for(uint32_t i = 0; i < dictSize; i++) {																					// zapis hodnot od 0-velikost tabulky, CC + EOI 
		dict[i].push_back(i);
	}

	// inicializace promennych
	uint32_t pozicion = 0;																											// pozice prave cteneho kodu
	uint16_t mask[] = {1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095};										// maska slouzici k ziskani spravneho cisla
	uint8_t lzwBit = gif.sizeLZW + 1;
	uint8_t up = 0;																													// posunuti prvniho cislo o up bitu
	uint8_t down = gif.sizeLZW;																									// posunuti druheho cisla o dowb bitu, na zacatek velikost LZW kodu
	uint16_t pom = 0;																													// pomocn epromenne, 1. nacteny kod
	uint16_t pom1 = 0;																												// pomnocna promenna, 2.nacteny kod
	uint16_t result = 0;																												// kod do slovniku, pom + pom1, posunute o n bity

	
	fprintf(stderr, "zacnu decodovat\n");


// ------------------- dekodovani prvniho slova, mel by byt ve slovniku nebo to je hodnota inicializace slovniku ---------------------------------------------------------------

	fprintf(stderr, "pozice:%d nacteny kod=0x%x, ", pozicion ,subBlock[pozicion]);
	pom = subBlock[pozicion++] >> up;																							// posunuti doprava o n bitu
	fprintf(stderr, "posun o=%d, prvni posun=%d\n", up , pom);
	pom1 = subBlock[pozicion]&mask[up];																							// prvnich n bitu z druheho cisla
	fprintf(stderr, "pozice:%d nacteny kod=0x%x, pouzita maska%d, ", pozicion,  pom1, mask[up]);
	pom1 = pom1 << down;																												// posunuti o n bitu druheho cisla, prefix pom
	fprintf(stderr, "posun o=%d, druhy posun=%d\n",down, (pom1));
	result = pom1+pom;																												// vysledna hodnota, spojeni
	fprintf(stderr, "vysledek=%d\n---------\n", result);
	down--;up++;																														// posun o jiny pocet bitu

	// pokud je to znak CC, tak inicializace slovniku a nacteni dalsiho znaku
	uint16_t CC = dict.size() - 2;
	if(result == dict[CC][0]) {																						//inicilaizace slovniku, slovnik je jiz nachystan z drivejska
		fprintf(stderr, "pozice:%d nacteny kod=0x%x, ", pozicion ,subBlock[pozicion]);
		pom = subBlock[pozicion++] >> up;																						// posunuti doprava o n bitu
		fprintf(stderr, "posun o=%d, prvni posun=%d\n", up , pom);
		pom1 = subBlock[pozicion]&mask[up];																						// prvnich n bitu z druheho cisla
		fprintf(stderr, "pozice:%d nacteny kod=0x%x, pouzita maska%d, ", pozicion,  pom1, mask[up]);
		pom1 = pom1 << down;																											// posunuti o n bitu druheho cisla, prefix pom
		fprintf(stderr, "posun o=%d, druhy posun=%d\n",down, (pom1));
		result = pom1+pom;																											// vysledna hodnota, spojeni
		fprintf(stderr, "vysledek=%d\n---------\n", result);
		down--;up++;																													// posun o jiny pocet bitu
	}
	// zpracuji prvni znak, ktery pujde do slovniku
	vector<int16_t> output;																											// vystupni vektor
	vector<int16_t> c;																												// vektor nactenehych znaku
	vector<int16_t> old;																												// vektor znaku predeslych

	//TODO:mozno predelat, vzdy tam bude jen jeden index, netreba prohledavat
	for(uint16_t k = 0; k < dict[result].size(); k++) {																	// nahrani indexu daneho indexu 
		c.push_back(dict[result][k]);
	}
	for(uint16_t k = 0; k < c.size(); k++) {																					// zaloha aktualne cteneho indexu indexu
		old.push_back(c[k]);
	}
	for(uint16_t k = 0; k < c.size(); k++) {																					// tisk 1. indexu do globalni tabulky barev
		output.push_back(c[k]);
	}

// ------------------- dekodovani zbytkuslov, bezi dokud nejsou prectena vsechan data ---------------------------------------------------------------
 	int32_t i = 0;
 	int16_t pos = dictSize;																											// posledni index slovniku
	while(pozicion != subBlock.size()) {																						// dokud neprectu cely blok
			fprintf(stderr, "oooooooooo \n");
		if(dict.size() == 4095) {																									// pokud je velikost slovniku 4095
			fprintf(stderr, "xxxxxxxxxx REINICIALZICAE slovniku:je = %lu velky\n",dict.size());					// TODO:doresit
			lzwBit = gif.sizeLZW;																									// 
			exit(0);
		}
		if(lzwBit == 13) {																											// kontrola zda pocet nacitanych bitu neni vetsi nez 13, max12
			fprintf(stderr, "xxxxxxxxxx REINICIALIZACE slovniku:pocet ctebych bitu je = %d\n", lzwBit);		// TODO:doresit, spojit inicializace a kontrolu s tim nahore
			lzwBit = gif.sizeLZW;
			exit(0);
		}
		if(down == 0) {																												// precetl jsem 1bit z prvniho slova a vsechny bity s 2, posun na dalis
			down = lzwBit-1;																											// nova velikost down, v zavislosti na poctu nacitanych bitu (8,9,10, 11)
			up = 0;																														// nastaveni pocatecni velikost up 
//			fprintf(stderr, "oooooooooo ZMENA posunovacich konstant na: up=%d,, down=%d\n", up, down);
			pozicion++;																													// posun na dalsi slovo
		}
		// zpracovani a nacteni indexu do slovniku
		pom = 0;																															// nulovani 1.cteneho slova
		pom1 = 0;																														// nulovani 2.cteneho slova
//		fprintf(stderr, "pozice:%d nacteny kod=0x%x, ", pozicion ,subBlock[pozicion]);
		pom = subBlock[pozicion++] >> up;																						// posunuti doprava o n bitu
//		fprintf(stderr, "posun o=%d, prvni posun=%d\n", up , pom);
		pom1 = subBlock[pozicion]&mask[up];																						// prvnich n bitu z druheho cisla
//		fprintf(stderr, "pozice:%d nacteny kod=0x%x, pouzita maska%d, ", pozicion,  pom1, mask[up]);
		pom1 = pom1 << down;																											// posunuti o n bitu druheho cisla, prefix pom
//		fprintf(stderr, "posun o=%d, druhy posun=%d\n",down, (pom1));
		result = pom1+pom;																											// vysledna hodnota, spojeni
//		fprintf(stderr, "vysledek=%d\n---------\n", result);

		if(result == dict[CC][0]) {																								// nacten kod na reinicializaci slovniku
			fprintf(stderr, "oooooooooo ZMENA posunovacich konstant na: up=%d,, down=%d\n", up, down);
			fprintf(stderr, "xxxxxxxxxx REINICIALIZACE slovniku:prisel kod 0x%x, %d\n",result,CC);
			lzwBit++;																													// zvetseni nacitanych poctu bitu
//			dict.clear();
//			for(uint32_t i = 0; i < dictSize; i++) {
//				dict.push_back(vector<int16_t>());
//			}
//			for(uint32_t i = 0; i < dictSize; i++) {																					// zapis hodnot od 0-velikost tabulky, CC + EOI 
//				dict[i].push_back(i);
//			}
//			pos = dictSize;
//			exit(0);																														// TODO:reinicializace, doresit
		}
		else {																															// porovnej s daty ve slovniku a zapis danou hodnotu
//TODO:poradi jestli to co je pod timto delat i kdtz je reinicializace slovnuki!!!!!
	//	fprintf(stderr, "dict end%d\n", dict[dict.size()-1][0]);
	//	fprintf(stderr, "dict %lu\n", dict.size()-1);
//	exit(0);
		//if(result < dict[dict.size()-1][0]) {																				// je ve slovniku
		if(result < (dict.size()-1)) {																							// je ve slovniku, ve slovniku jiz existuje index s touto hodnotou
			//TODO:poresit co kdz dostanu moce velak cisla, to by se nemelo asik stat
			dict.push_back(vector<int16_t>());																					// pridani hodnoty do slovniku
			for( uint16_t p = 0; p < old.size(); p++) {																		// kopirovani predeslych znaku
				dict[pos].push_back( old[p]);
			}
			dict[pos].push_back( dict[result][0]);																				// pridani aktualniho znaku
//		fprintf(stderr,"index v tabulce R=%x, G=%x, B=%x\n,",gif.globalColor[pom1+pom].red, gif.globalColor[pom+pom1].green, gif.globalColor[pom+pom1].blue);
//		fprintf(stderr,"index v tabulce R=%x, G=%x, B=%x\n,",gif.globalColor[pom1+pom-1].red, gif.globalColor[pom+pom1-1].green, gif.globalColor[pom+pom1-1].blue);
			//exit(0);
		}
		else {																															// neni ve slovniku
			dict.push_back(vector<int16_t>());																					// pridani prcku do slovniku
			for(uint16_t p = 0; p < old.size(); p++) {
				dict[pos].push_back( old[p]);																						// kopirovami hopdnot z predesle casti kalendare
				//fprintf(stderr, "%d, ",old[p]);
			}
			dict[pos].push_back( old[old.size()-1]);																			//. pridani posledni hodoty na nove vzniklemisto
		}
		//for(int l = 0; l < dict[pos].size(); l++) {
		//	fprintf(stderr, " %dl..................slovnik:%d\n",dict[(dict.size()-1)][l],l);
		//}

		c.clear();																														// mazani vstupu
		for(uint16_t k = 0; k < dict[result].size(); k++) {																// nacitani hodnot z kalendare
			c.push_back(dict[result][k]);
		}
		old.clear();																													// mazani starych hodnot
		for(uint16_t k = 0; k < c.size(); k++) {																				// vytvoreni zalohy aktualniho vektoru hopdnot
			old.push_back(c[k]);
		}
		for(uint16_t k = 0; k < c.size(); k++) {																				// vytisteni hodnot indexu barvy na vystup
			output.push_back(c[k]);
		}
//		fprintf(stderr,"index v tabulce R=%x, G=%x, B=%x\n,",gif.globalColor[pom1+pom].red, gif.globalColor[pom+pom1].green, gif.globalColor[pom+pom1].blue);
//		fprintf(stderr,"index v tabulce R=%x, G=%x, B=%x\n,",gif.globalColor[pom1+pom-1].red, gif.globalColor[pom+pom1-1].green, gif.globalColor[pom+pom1-1].blue);
		i++;
		//if(i >80) {
		//	exit(0);
		//	}
		}
		down--;up++;pos++;																											// zmena posunovaich hodnot, posun ve slovniku o jedno dopredu
	}
		fprintf(stderr,"jsem na konci kodovani\n");
}

/**
 * Naplneni hlavicky daty
 */
tHEADER pullHead(vector<int32_t> data) {
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
		exit(0);																															//TODO: poresit kdyz neni obrazek
		//return ;0;
	}
	return header;
}

tLOSCDES pullLogDesc(vector<int32_t> data) {
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

vector<tGlobalColor> pullGlobalColor(vector<int32_t> data, char pixelBits) {
// nacteni dat do globalni tabulky barev
	int8_t countIndexTable = 1 << pixelBits;																					// velikost globalni tabulkz barev max 255
	vector<tGlobalColor> aGColor;																									// vektor struktur RGB
	tGlobalColor globalColor;																										// pomocna struktura RGB
	int16_t pozicion = 0;
	for(int8_t i = 0; i < countIndexTable; i++) {
		globalColor.red = data[pozicion++];																						// cervena barva
		globalColor.green = data[pozicion++];																					// zelena barva
		globalColor.blue = data[pozicion++];																					// modra barva	
		aGColor.push_back(globalColor);																							// vlozeni struktury do vektoru
	}

	return aGColor;
}

tGraphicControlExt pullGraphicControlExt(vector<int32_t> data) {
	tGraphicControlExt graphicExt;																								// struktura grafickeho rozsireni
	graphicExt.extIntroducer = data[0];																							// oddelovac 0x21
	graphicExt.graphicControlLabel = data[1];																					// identifikator rozsireni 0xF9
	graphicExt.blockSize = data[2];																								// velikost zbyvajicich poli, vzdy 0x04
	graphicExt.packedFields.colorFlag = data[3] & 1;																		// 1 tak pole ColorIndex obsahuje index barvy pruhlednosti
	graphicExt.packedFields.input = (data[3] & 2)  >> 1;																	// 1 pokud je ocekavan uzivatelsky stisk, aktivita
	graphicExt.packedFields.disposalMethod = (data[3] & 28) >> 2;														// 0x00 neni speicfikovano, 0x01 neimplmentovat, 0x02 a 0x04
	graphicExt.packedFields.reserved = (data[3] & 224) >> 5;																// vyhrazeno, nepouzito, vzdy 0x00
	graphicExt.delayTime = hexToDec(data[4], data[5]);																		// cas v setinach mezi obrazky	
	graphicExt.transparentColorIndex = data[6];																				// index pruhlednosti
	graphicExt.blockTerminator = data[7];																						// 0x00 konec tohoto bloku

	return graphicExt;
}
tImageDescription pullImageDescription(vector<int32_t> data) {
	tImageDescription imageDesc;																									// struktura lokalniho popisovace obrazku
	imageDesc.imageSeparator = data[0];																							// oddelovac 0x2c
	imageDesc.imageLeftPos = hexToDec(data[1], data[2]);																	// leva pozice obrazku
	imageDesc.imageTopPos = hexToDec(data[3], data[4]);																	// horni pozice obrazku
	imageDesc.imageWidth = hexToDec(data[5], data[6]);																		// sirka bloku obrazku
	imageDesc.imageHeight = hexToDec(data[7], data[8]);																	// vyska bloku obrazku
	imageDesc.packedFields.color = (data[9] & 1);																			// 1 pokud je lokalni tabulka barev prtiomna
	imageDesc.packedFields.int32_terleaved = (data[9] & 2) >> 1;														// 1 pokud je obrazek prokladan
	imageDesc.packedFields.sort = (data[9] & 4) >> 2;																		// 1 pokud jsou barvy truideny podle dulezitosti TODO:podivat se
	imageDesc.packedFields.reserved = (data[9] & 18) >> 3;																// vyhrazeno
	imageDesc.packedFields.size = (data[9] & 224) >> 5;																	// velikost polozek lokalni tabulky barev
	return imageDesc;
}

void sortData() {
	int32_t i,j;
	vector<int32_t> table1;
	vector<int32_t> table2;
	int32_t imageHeight = 16;
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
void printTGlobalColor(vector<tGlobalColor> color) {
	fprintf(stderr, "------------------------------Struktura tGlobalColor ---------------------------------\n");
	for(uint8_t i = 0; i < color.size(); i++) {
		fprintf(stderr,"index:%d red=0x%x, green=0x%x, blue=0x%x\n",i, color[i].red, color[i].green, color[i].blue);
	}
}

/**
 * @brief   Tisk struktury tPACKED.
 * @param   Struktura tPACKED
 * @return
 */
void printTPacked(tPACKED packedFields) {
	fprintf(stderr, "------------------------------Struktura tPACKED ---------------------------------\n");
	fprintf(stderr, "7bit=%d, 4-6bit=%d, 3bit=%d, 0-2bit=%d\n", packedFields.globalColorMap, 
			                                                      packedFields.colorResolutionBits,
																					packedFields.reserved,
																					packedFields.pixelBits
			);
}

void printTPackedEx(tPACKEDEX packedFields) {
	fprintf(stderr, "------------------------------Struktura tPACKEDEX ---------------------------------\n");
	fprintf(stderr, "5-7bit=%d, 2-4bit=%d, 1bit=%d, 0bit=%d\n", packedFields.reserved, 
			                                                      packedFields.disposalMethod,
																					packedFields.input,
																					packedFields.colorFlag
			);
}

void printTPackedImg(tPACKEDIMG packedFields) {
	fprintf(stderr, "------------------------------Struktura tPACKEDIMG ---------------------------------\n");
	fprintf(stderr, "5-7bit=%d, 3-4bit=%d, 2bit=%d, 1bit=%d, 0bit=%d\n", packedFields.size, 
			                                 			                     packedFields.reserved,
																								packedFields.sort,
																								packedFields.int32_terleaved,
																								packedFields.color
			);
}

void printTLoscdes(tLOSCDES logDescription) {
	fprintf(stderr, "------------------------------Struktura tLOSCDES ---------------------------------\n");
	fprintf(stderr, "screenWidth=%d, screenHeight=%d\n",	logDescription.screenWidth,
																			logDescription.screenHeight
			);
	printTPacked(logDescription.packedFields);
	fprintf(stderr,"backgroundColorIndex=%d, pixelAspectRatio=%d\n",	logDescription.backgroundColorIndex,
																						 	logDescription.pixelAspectRatio
			);
}

void printTImageDescription(tImageDescription des) {
	fprintf(stderr, "------------------------------Struktura tImageDescription ---------------------------------\n");
	fprintf(stderr, "imageSeparator=0x%x, imageLeftPos=%d, imageTopPos=%d, imageWidth=%d, imageHeight=%d\n",	des.imageSeparator,
																																				des.imageLeftPos,
																																				des.imageTopPos,
																																				des.imageWidth,
																																				des.imageHeight
			);
	printTPackedImg(des.packedFields);
}

void printTGraphicControlExt(tGraphicControlExt ext) {
	fprintf(stderr, "------------------------------Struktura tGraphicControlEx ---------------------------------\n");
	fprintf(stderr, "extIntroducer=0x%x, graphicControlLabel=0x%x, blockSize=0x%x\n",	ext.extIntroducer,
																													ext.graphicControlLabel,
																													ext.blockSize
			);
	printTPackedEx(ext.packedFields);
	fprintf(stderr, "delayTime=%d, transparentColorIndex=%d, blockTerminator0x%x\n",	ext.delayTime,
																												ext.transparentColorIndex,
																												ext.blockTerminator
			);
}

void printTHeader(tHEADER head) {
	fprintf(stderr, "------------------------------Struktura tHEADER ---------------------------------\n");
	fprintf(stderr, "signaturer=%c%c%c, version=%c%c%c\n",	head.signature[0],
																				head.signature[1],
																				head.signature[1],
																				head.version[0],
																				head.version[1],
																				head.version[2]
			);
}

void printTCommentExt(tCommentExt ext) {
	fprintf(stderr, "------------------------------Struktura tCommentExt ---------------------------------\n");
	fprintf(stderr, "extIntroducer=0x%x, commentLabel=0x%x, commentData=%s, blockTerminator=0x%x\n",	ext.extIntroducer,
																																		ext.commentLabel,
																																		ext.commentData.c_str(),
																																		ext.blockTerminator
			);

}

void printTPlainTextExt(tPlainTextExt ext) {
	fprintf(stderr, "------------------------------Struktura tPlainTextExt ---------------------------------\n");
	fprintf(stderr, "extIntroducer=0x%x, extLabel=0x%x, blockSize=%d, textGridLeftPos=%d, textGridTopPos=%d,	textGridWidth=%d, textGridHeight=%d, characterCellWidth=%d, characterCellHeight=%d, textForColorIndex=%d, textBackgroundColorIndex=%d, plainTextData=%s, blockTerminator=0x%x\n",	ext.extIntroducer,
																															ext.plainTextLabel,
																															ext.blockSize,
																															ext.textGridLeftPos,
																															ext.textGridTopPos,
																															ext.textGridWidth,
																															ext.textGridHeight,
																															ext.characterCellWidth,
																															ext.characterCellHeight,
																															ext.textForegroundColorIndex,
																															ext.textBackgroundColorIndex,
																															ext.plainTextData.c_str(),
																															ext.blockTerminator
			);

}

unsigned short hexToDec(BYTE num1, BYTE num2) {
	if(num2 != 0) {																													// pokud druhe cislo neni nulove
		return (num1 + num2 + 256);																								// posunuti druheho cisla o 8 mist
	}
	else {
		return (num1 + num2);																										// secteni dvou cisel
	}
}

//TODO:vyresit kdaz neni gif
char isGif(BYTE signature[], BYTE version[] ) {
// kontrola zda je obrazek gif
	if(signature[0] != 0x47 || signature[1] != 0x49 || signature[2] != 0x46) {										// neni gif
		fprintf(stderr, "CHYBA: Obrazek neni typu gif nebo je poskozen.");
		return 0;
	}
// kontrola obrazek verze
	if(version[0] == 0x38 && version[1] == 0x39 && version[2] == 0x61) {												// typ 89a
		return 89;
	}
	else if(version[0] == 0x38 && version[1] == 0x37 && version[2] == 0x61) {										// typ 87a
		return 87;
	}
	else {																																// je to gif lae jine verze?
		fprintf(stderr, "CHYBA: Obrazek neni verze 87a ani 89a nebo je poskozen.");
		return 0;
	}
	return 1;
}

