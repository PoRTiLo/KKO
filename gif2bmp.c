/**
 *       @file  gif2bmp.c
 *      @brief  V jazyce C/C++ naimplementovana knihovna pro prevod souboru 
 *              grafickeho formatu GIF (Graphics Interchange Format) na soubor 
 *              grafickeho formatu BMP (Microsoft Windows Bitmap). Pro soubor ve 
 *              formatu GIF se predpoklada jeho zakodovani pomoci metody LZW. 
 *              Vystupni soubor ve formatu BMP je ulozen v nezakodovane forme.
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

#include "gif2bmp.h"


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Rozhrani mezi programme a knihovnou pro prevod z GIF do BMP.
 *
 * @Param gif2bmp Struktura obsahujici informace o vstupnim a vystupnim soubrou.
 * @Param inputFile Vstupbni soubor.
 * @Param outputFile Vystupni soubor.
 *
 * @Returns   0 prevod se povdel, jinak -1
 */
/* ----------------------------------------------------------------------------*/
int32_t gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

	tGIF gif;
	tBMP bmp;
	//createBmp(pullGif(openGif(inputFile)), outputFile);
	int64_t sizeGif = 0;
	vector<int32_t> vec(openGif(inputFile, sizeGif));
	gif = pullGif(vec, sizeGif);
	if(gif.ok == -1) {
		return -1;
	}
	//tisk dat do bmp souboru
	bmp = createBmp(gif, outputFile);
	if(bmp.ok == -1) {
		return -1;
	}
	gif2bmp->bmpSize = bmp.header.size;
	gif2bmp->gifSize = sizeGif;
	return 0;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Otevre vstupni obrazek a nacte vsechna data
 *
 * @Param inputFile Vstupni soubor znehoz budou ctena data.
 * @Param size Velikost vstupniho souboru.
 *
 * @Returns   Nactena data z obrazku.
 */
/* ----------------------------------------------------------------------------*/
vector<int32_t> openGif(FILE *inputFile, int64_t &size) {
	// zjisteni velikosti souboru
	fseek(inputFile, 0L, SEEK_END);																								// nastaveni ukazatele v souboru na konec
	size = ftell(inputFile);																										// ulozeni velikosti souboru do struktury
	fseek(inputFile, 0L, SEEK_SET);																								// nastaveni ukazatele v souboru n azacatek

	// nacteni obrazku do vectoru
	vector<int32_t> data;																											// kontejner obsahujici nactena data z obrazku
	int32_t ch = 0;
	while((ch = fgetc(inputFile)) != EOF) {																					// nacitam po znacich dokud neni konec souboru
		data.push_back(ch);
	}

	return data;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  VYtvori strukturu tBMP z indexu do globalni tabulky barev z 
 *            obrazku gif. Nasledne vytvoreny obrazek vytsikne do souboru 
 *            outputFile.
 *
 * @Param gif Plna strktura tGIF, pripravena na prevod na BMP.
 * @Param outputFile Ukazatel na vystupni soubor.
 *
 * @Returns   Naplnena struktura tBMP.
 */
/* ----------------------------------------------------------------------------*/
tBMP createBmp(tGIF gif, FILE *outputFile) {
	//TODO:kontrola zda ze povedlo zapsat
	tBMP bmp = pullBmp(gif);
	uint32_t pom32[1];
	uint16_t pom16[1];
	pom16[0] = (bmp.header.fileType);
	fwrite(pom16, sizeof(uint16_t),1, outputFile);
	pom32[0] = (bmp.header.size);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom16[0] = 0x0000;
	fwrite(pom16, sizeof(uint16_t),1, outputFile);
	pom16[0] = 0x0000;
	fwrite(pom16, sizeof(uint16_t),1, outputFile);
	pom32[0] = bmp.header.dataOffset;
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.headerSize);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.width);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.height);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom16[0] = (bmp.infoHeader.numOfPlanes);
	fwrite(pom16, sizeof(uint16_t),1, outputFile);
	pom16[0] = (bmp.infoHeader.bitsPerPixel);
	fwrite(pom16, sizeof(uint16_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.compressMethod);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.sizeBitmap);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.horzResolution);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.vertResolution);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.numColorUsed);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);
	pom32[0] = (bmp.infoHeader.significantColors);
	fwrite(pom32, sizeof(uint32_t),1, outputFile);

	uint8_t pom[1];
	for(uint32_t i = 0; i< bmp.data.size();i++) {
		pom[0]= bmp.data[i];
		fwrite(pom, sizeof(uint8_t),1, outputFile);
	}
	return bmp;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Vytvari surova zdrojova data pro obrazek formnatu bmp. Nejprve 
 *            pripadne prevede na neprokladany format, priradi indexu v 
 *            tabulce barvy, nasledne prevede data do poradi kdyz se zacina 
 *            v levem dolnim rohu.
 *
 * @Param gif Struktura obrazku gif, obsahujici globalni tabulku barev a data.
 *
 * @Returns   Pretransformovany vektor dat, ktery se muzu rovnou tisknout 
 *            do souboru.
 */
/* ----------------------------------------------------------------------------*/
vector<uint16_t> bmpData(tGIF gif) {
	if(gif.imageDescription.packedFields.int32_terleaved == 1) {														// 1 pokud je obrazek prokladan
		gif.data = sortData(gif);																									// prevod z prokladaneho formatu na normlani posloupnost
	}
	vector<uint16_t> data;
	for(uint32_t i = 0; i < gif.data.size(); i++) {																			// tvorba dat, prevod indexu do tabulky barev na barvy
		data.push_back(gif.globalColor[gif.data[i]].blue);																	// pridani modre barvy
		data.push_back(gif.globalColor[gif.data[i]].green);																// pridani zelene barvy
		data.push_back(gif.globalColor[gif.data[i]].red);																	// pridani cervene barvy
	}

	// data v bmp jsou razena od leveho dolniho rohu
	int64_t start = data.size() - (gif.logDescription.screenWidth*3) ;												// pocatek radku dat
	int64_t end = data.size();																										// konec radku dat
	uint8_t aligned = 4 - ((gif.logDescription.screenWidth*3) % 4);													// zarovani radku dat na delitelne 4
	vector<uint16_t> bmpData;																										// spravne serazena data
	// transformace poradi dat
	for(;start>=0;) {
		for(; start < end; start++) {																								// tvorba spravneho poradi surovych dat
			bmpData.push_back(data[start]);
		}
		for(uint8_t i = 0; i < aligned; i++) {																					// doplneni nul do zarovnani delitelne 4
			bmpData.push_back(0x00);
		}
		end = end - (gif.logDescription.screenWidth*3);																		// nastaveni noveho konce radku
		start = end - (gif.logDescription.screenWidth*3);																	// nastaveni noveho pocatku radku
	}

	return bmpData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------- Plneni struktur --------------------//
// ////////////////////////////////////////////////////////////////////////////////////////////////////////

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktury pro obrazek BMP.
 *            Naplneni hlavicky, info hlavicky a dat. Je pouzita 24bitova 
 *            hloubka barev.
 *
 * @Param gif 
 *
 * @Returns   Struktura obrazku bmp. Je plna dat, pripravena na tisk.
 */
/* ----------------------------------------------------------------------------*/
tBMP pullBmp(tGIF gif) {
	tBMP bmp;
	bmp.ok = 0;
	bmp.data = bmpData(gif);																										// transformace indexu do globalni tabulky barev na data pro bmp
	tBmpHeader header;
	header.fileType = 0x4D42;																										// zacatek souboru BM
	header.size = 14 + 40 + bmp.data.size();																					// velikost souboru, hlavicka 14, info hlavicka 40 a data
	header.reserved1 = 0x00000000;																								// rezervovane misto
	header.reserved2 = 0x00000000;																								// rezervovane misto
	header.dataOffset = 0x28;																										// pozice kde zacinaji uzitecna data
	tBmpInfoHeader infoHeader;																										// druha hlavicka
	infoHeader.headerSize = 0x28;																									// velikost hlavicky
	infoHeader.width  = gif.logDescription.screenWidth;																	// sirka obrazku
	infoHeader.height = gif.logDescription.screenHeight;																	// vyska obrazku
	infoHeader.numOfPlanes = 0x01;																								//
	infoHeader.bitsPerPixel = 0x18;																								// pocet bitu na pixel, 24, REDx8, greenx3, bluex3
	infoHeader.compressMethod = 0x00;																							// kompresni metoda, bez komprese
	infoHeader.sizeBitmap = bmp.data.size();																					// velikost surovych dat
	infoHeader.horzResolution = 0x0B13;																							//
	infoHeader.vertResolution = 0x0B13;																							//
	infoHeader.numColorUsed = 0x00;																								//
	infoHeader.significantColors = 0x00;																						//
	bmp.header = header;
	bmp.infoHeader = infoHeader;
	return bmp;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktury tGIF reprezentujici cely obrazek gif. 
 *            Vyziva nekoliak fci pro pleneni jednotlivych struktur. 
 *            Rozebere a zhodnoti zda se jedna o obrazek atd..
 *
 * @Param data Data nactena ze souboru
 *
 * @Returns   Naplnena struktura tGIF daty
 */
/* ----------------------------------------------------------------------------*/
tGIF pullGif(vector<int32_t> data, int64_t size) {
	tGIF gif;																															// struktura obsahujici vsechny informace o obrazku
	gif.size = size;
	gif.header = pullHead(data);																									// naplneni struktury hlavicky 
	gif.ok = 0;
	if(isGif(gif.header.signature, gif.header.version ) == -1) {														// kontrola zdali je spravny soubor
		gif.ok = -1;
		return gif;
	}
	data.erase(data.begin(), data.begin() + sizeof(gif.header));														// odebrani dat obsazenych v hlavicce
	gif.logDescription = pullLogDesc(data);																					// naplneni struktury Lokalni popisova cobrazku
	data.erase(data.begin(), data.begin() + sizeof(gif.logDescription) - sizeof(gif.logDescription.packedFields) + 1);	// mazani pouzitych dat
// pokud existuje globalni tabulka barev, zpracuj ji
	if(gif.logDescription.packedFields.globalColorMap == 1) {															// zpracovani globallni tabulky barev, pokud existuje
		gif.globalColor = pullGlobalColor(data, gif.logDescription.packedFields.pixelBits);						// naplneni struktury globalni tabulky
		data.erase(data.begin(), data.begin() + ((1 << gif.logDescription.packedFields.pixelBits))*3);		// mazani pouzityhc dat
	}
	gif.graphicControlExt.extIntroducer = 0x00;
// vyhledani urciteho bloku, bud samotnych bunarnich dat obrazku nebo nektereho rozsireni
	uint32_t pozicion = 0;																											// pozice prochazeni v datech
	vector<uint16_t> subBlock;																										// vector obsahujici binarni data v LZW kode
	while(pozicion < gif.size) {
// rozsireni
		if(data[pozicion] == 0x21) {
			pozicion++;
			if(data[pozicion] == 0xF9) {																							// rozsireni rizeni grafiky
				fprintf(stderr,"rozsireni F9\n");														// TODO: zpracova lokalni tabulku barev
				gif.graphicControlExt = pullGraphicControlExt(data);
				data.erase(data.begin(), data.begin() + (sizeof(gif.graphicControlExt) - sizeof(gif.graphicControlExt.packedFields) ));	// mazani pouzitych dat
				pozicion = 0;																											// nastaveni pozice na zacatek
			}
			else if(data[pozicion] == 0xFE) {																					// rozsireni komentare
				fprintf(stderr,"rozsireni FE\n");														// TODO: zpracova lokalni tabulku barev
				exit(0);
			}
			else if(data[pozicion] == 0xFF) {																					// rozsireni pro aplikaci
				fprintf(stderr,"rozsireni FF\n");														// TODO: zpracova lokalni tabulku barev
				exit(0);
			}
			else if(data[pozicion] == 0x01) {																					// rozsireni standardniho textu
				fprintf(stderr,"rozsirebui 01\n");														// TODO: zpracova lokalni tabulku barev
				exit(0);
			}
			else {

				fprintf(stderr,"odivne nic, zadne rozsireni\n");														// TODO: zpracova lokalni tabulku barev
					exit(0);
			}
		}
		//TODO:poresit kdz jsou obrazky z vice bloku, asi maji i vice lokalnich popisovacu, nevim
		else if(data[pozicion] == 0x2c) {																						// binarni data kodovana v LZW
			gif.imageDescription = pullImageDescription(data);																// naplneni struktury lokalni popisovac predlohy
			data.erase(data.begin(), data.begin() + (sizeof(gif.imageDescription) - sizeof(gif.imageDescription.packedFields) -1));	//mazani
// pokud je pritomna lokalni tabulka barev, zpracuj ji
			if(gif.imageDescription.packedFields.color == 1) {																// pokud 1(true) je pritomna lokalni tabulka barev
				fprintf(stdout,"obsahuje lokalni tabulku barev\n");														// TODO: zpracova lokalni tabulku barev
			}
// cteni binarnich dat v LZW kodu
			gif.sizeLZW = data[pozicion++];																						// velikost minamilniho kodu LZW
			uint32_t sizeBlock = data[pozicion++];																				// velikost bloku dat
			bool endBlock = false;																									// ukonceni cteni, podle delky a kodu 0x00
			uint32_t i = 1;																												// aktualne nacteny znak
			while(endBlock != true) {
				if(i <= sizeBlock) {																									// ctu jeden blok
					subBlock.push_back(data[pozicion++]);																		// vlozeni dat do subBloku
					i++;
				}
				else if(data[pozicion] == 0x00) {																				// nacteni konce toku binarnich dat
					endBlock = true;																									// ukonceni
				}
				else {																													// nacitam novy blok, zjisti jeho velikost a realokace pole
					sizeBlock = sizeBlock + data[pozicion++];																	// pricteni k velikosti velikost dalsiho bloku
				}
			}
			break;																														// TODO: dalsi rozsireni, ted natvrdo ukoncim po precteni
		}
		else {
		}
	}
	// prevod cisel na posloupnost 1 a 0 v retezci 
	string binary;
	for(uint32_t i = 0; i < subBlock.size(); i++) {
		binary.insert(0, dec2bin(subBlock[i]));
	}
	bin2dec( binary.substr(binary.size()-9,binary.size()), 9);
	gif.data = decodeLZW(binary, gif);
	return gif;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Implementuje algoritmus na rozkodovani LZW.
 *
 * @Param subBlock posloupnost 1 a 0 reprezentujici bity obrazkovych dat.
 * @Param gif Naplnena struktura tGIF.
 *
 * @Returns   Rozkodovana posloupnost indexu do globalni tabulky barev.
 */
/* ----------------------------------------------------------------------------*/
vector<uint16_t> decodeLZW(string subBlock, tGIF gif) {

	// inicializace slovniku
	vector<vector<int16_t> > dict;
	uint16_t dictSize = (1 << gif.logDescription.packedFields.pixelBits)+2;											// inicializace slovniku, velikost - pocet barev + CC + EOI
	if(dictSize == 4) {																												//TODO:poresit proc to tak je
		dictSize += 2;
	}
	for(uint32_t i = 0; i < dictSize; i++) {
		dict.push_back(vector<int16_t>());
	}
	for(uint32_t i = 0; i < dictSize; i++) {																					// zapis hodnot od 0-velikost tabulky, CC + EOI 
		dict[i].push_back(i);
	}
	// inicializace promennych
	uint8_t lzwBit = gif.sizeLZW + 1;																							// pocet ctenych bitu
	uint16_t result = 0;																												// kod do slovniku, pom + pom1, posunute o n bity
// ------------------- dekodovani prvniho slova, mel by byt ve slovniku nebo to je hodnota inicializace slovniku ---------------------------------------------------------------
	result = bin2dec( subBlock.substr(subBlock.size()-lzwBit, subBlock.size()), lzwBit);						// ziskani prvniho slova
	subBlock.erase(subBlock.size()-lzwBit, subBlock.size());																// smazani prectenych bitu

	// pokud je to znak CC, tak inicializace slovniku a nacteni dalsiho znaku
	uint16_t CC = dict.size() - 2;																								// pozice hodnoty CC ve slovniku
	uint16_t EOI = dict.size() - 1;																								// pozice hodnoty EOI ve slovniku
	if(result == dict[CC][0]) {																									// inicilaizace slovniku, slovnik je jiz nachystan z drivejska
		result = bin2dec( subBlock.substr(subBlock.size()-lzwBit, subBlock.size()), lzwBit);
		subBlock.erase(subBlock.size()-lzwBit, subBlock.size());
	}
	// zpracuji prvni znak, ktery pujde do slovniku
	vector<uint16_t> output;																										// vystupni vektor
	vector<uint16_t> old;																											// vektor znaku predeslych
	uint8_t last = 0;																													// posldne nacteny bit
	if(result > dict.size()) {
		fprintf(stdout, "DIVNE PORESIT, jako prvni prislo moc velke cislo, ma byt mensi nez velikost slovniku, coy je %u",dict.size()); //TODO
		exit(0);
	}
	old.push_back(dict[result][0]);																								// uchovani nactenych dat daneho indexu slovniku
	output.push_back(dict[result][0]);																							// stream vystupu, index do palety barev

// ------------------- dekodovani zbytkuslov, bezi dokud nejsou prectena vsechan data ---------------------------------------------------------------
 	uint32_t pos = dictSize;																										// posledni index slovniku
	uint8_t maskI = lzwBit;																											// pozice v masce
	uint16_t mask[] = {0,1,3,7,15,31,63,127,255,511, 1023, 2047, 4095, 8191};										// maska slouzici k ziskani spravneho cisla
	uint16_t sizeMax = mask[maskI];																								// maximalni velikost aktualniho slovniku
	while(0 != subBlock.size()) {																									// dokud neprectu cely blok
		//fprintf(stdout, "krok:%d, vstup:%d, [%d]:  \n",pos-dictSize+1, result, pos-1);
		if(lzwBit >= 13) {																											// kontrola zda pocet nacitanych bitu neni vetsi nez 13, max12
			lzwBit = 12;																												// snizeni bitu na maximalni hodnotu, 12
		}

		result = bin2dec( subBlock.substr(subBlock.size()-lzwBit, subBlock.size()), lzwBit);					// nacteni dalsiho kodoveho slova
		subBlock.erase(subBlock.size()-lzwBit, subBlock.size());
		if(result == EOI) {																											// ukoncovaci znak
			break;
		}
		if(result == dict[CC][0]) {																								// nacten kod na reinicializaci slovniku
			fprintf(stdout, "xxxxxxxxxx REINICIALIZACE slovniku:prisel kod 0x%x, %d, LZWbit%d\n",result,CC, lzwBit);
	//------------nastaveni na zaatek, vsechno smazat, inicalizovat slovnik, atd...
			lzwBit = gif.sizeLZW + 1;																								// nastaveni poctu ctenych bitu na pocatecni hpdnotu
			maskI = lzwBit;
			sizeMax = mask[maskI];																									// maxiamlni velikost slovniku
 			pos = dictSize;																											// posledni index slovniku
			// inicializace slovniku
			dict.clear();																												// smazani stareho slovniku
			dictSize = (1 << gif.logDescription.packedFields.pixelBits)+2;												// inicializace slovniku, velikost - pocet barev + CC + EOI
			for(uint32_t i = 0; i < dictSize; i++) {
				dict.push_back(vector<int16_t>());
			}
			for(uint32_t i = 0; i < dictSize; i++) {																			// zapis hodnot od 0-velikost tabulky, CC + EOI 
				dict[i].push_back(i);
			}

			result = bin2dec( subBlock.substr(subBlock.size()-lzwBit, subBlock.size()), lzwBit);				// nacteni dalsiho kodoveho slova
			subBlock.erase(subBlock.size()-lzwBit, subBlock.size());

	// pokud je to znak CC, tak inicializace slovniku a nacteni dalsiho znaku
			if(result == dict[CC][0]) {																							// inicilaizace slovniku, slovnik je jiz nachystan z drivejska
				result = bin2dec( subBlock.substr(subBlock.size()-lzwBit, subBlock.size()), lzwBit);
				subBlock.erase(subBlock.size()-lzwBit, subBlock.size());
			}
	// zpracuji prvni znak, ktery pujde do slovniku
			last = 0;																													// posledne nacteny bit je prazdny
			if(result > dict.size()) {
				fprintf(stdout, "DIVNE PORESIT, jako prvni prislo moc velke cislo, ma byt mensi nez velikost slovniku, coy je %u",dict.size());
				exit(0);
			}
			old.clear();
			old.push_back(dict[result][0]);
			output.push_back(dict[result][0]);
		}
		else {																															// porovnej s daty ve slovniku a zapis danou hodnotu
		if(result < (dict.size())) {																								// je ve slovniku, ve slovniku jiz existuje index s touto hodnotou
			// tisk hodnot ze slovniku z indexu result
			for(uint16_t k = 0; k < dict[result].size(); k++) {															// vytisteni hodnot indexu barvy na vystup
				output.push_back(dict[result][k]);
			}
			// prvni hodnota ze slovniku na indexu[result][0]
			last = dict[result][0];
			dict.push_back(vector<int16_t>());																					// pridani hodnoty do slovniku
			for(uint16_t k = 0; k < old.size(); k++) {																		//minuly vystup
				dict[pos].push_back(old[k]);
			}
			dict[pos].push_back(last);																								//pridani posledniho prvku, z nynejsihy vystupu
			old.clear();																												// smazani vyzsuptu
			// uchovani predelseho vystupu
			for(uint16_t k = 0; k < dict[result].size(); k++) {
				old.push_back(dict[result][k]);
			}
		}
		else {																															// neni ve slovniku
			last = old[0];
			dict.push_back(vector<int16_t>());																					// pridani prcku do slovniku
			for(uint16_t k = 0; k < old.size(); k++) {																		// vytisteni hodnot indexu barvy na vystup
				output.push_back(old[k]);
				dict[pos].push_back(old[k]);																						// kopirovami hopdnot z predesle casti kalendare
			}
			dict[pos].push_back(last);																								// kopirovami hopdnot z predesle casti kalendare
			output.push_back(last);
			old.push_back(last);
		}
		if(pos == sizeMax) {																											// slovnik dosal sve dosavadni maximalni velikosti
			maskI++;																														// posun pozice o jedno v masce velikosti slovniku
			sizeMax = mask[maskI];																									// zvetseni velikosti slovniku
			lzwBit++;																													// zvteseni poctu nacitanych bitu
			fprintf(stdout, "xxxxxxxxxx REINICIALZICAE LZW, zvetseni bitu na = %u velky\n",lzwBit);
		}
		pos++;																															// zmena posunovaich hodnot, posun ve slovniku o jedno dopredu
		}
	}

	return output;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktuty reprezentujici hlavikcu obrazku GIF.
 *
 * @Param data Vektor obsahujicic data nactena z obrazku.
 *
 * @Returns   Naplnena struktura tHEASER.
 */
/* ----------------------------------------------------------------------------*/
tHEADER pullHead(vector<int32_t> data) {
	tHEADER header;																													// struktura obsahujici hlavicku gifu
	// naplneni hlavicky
	header.signature[0] = data[0];																								// 0x47 G
	header.signature[1] = data[1];																								// 0x49 I
	header.signature[2] = data[2];																								// 0x46 F
	header.version[0] = data[3];																									// 0x38 8
	header.version[1] = data[4];																									// 0x39 9 nebo 0x37 7
	header.version[2] = data[5];																									// 0x61 a

	return header;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Naplneni struktury reprezentujici Popisovac logicke obrazovky.
 *
 * @Param data Vektor obsahujici data nactena z obrazku.
 *
 * @Returns   Naplnena struktura tLOSCDES
 */
/* ----------------------------------------------------------------------------*/
tLOSCDES pullLogDesc(vector<int32_t> data) {
	tLOSCDES logDescription;																										// struktura obsahujici popisovac logicke obrazovky
	logDescription.screenWidth = hexToDec(data[0], data[1]);																// sirka obrazku v pixelech
	logDescription.screenHeight = hexToDec(data[2], data[3]);															// vyska obrazku v pixelech
		fprintf(stderr, " end :%d, %d........0x%x,,,0x%x\n", logDescription.screenWidth, logDescription.screenHeight, data[0], data[1]);
	logDescription.packedFields.globalColorMap = (data[4] & 128) >> 7;												// 7 bit, 1 =tab. existuje, 0= neexistuje
	logDescription.packedFields.colorResolutionBits = ((data[4] & 112)  >> 4) + 1;								// 4-6bit, barevna rozlisovaci hodnpta +1
	logDescription.packedFields.reserved = (data[4] & 8) >> 3;															// 3bit, navesti trideni tabulky barev
	logDescription.packedFields.pixelBits = (data[4] & 7) + 1;															// 0-2bit, velikost globalni tabulky +1	
	logDescription.backgroundColorIndex = data[5];																			// index do tabulky barev, pozadi obrazku a ramecek
	logDescription.pixelAspectRatio = (data[6] + 15) / 64;																// pomer stran obrazku

	return logDescription;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktury Globalni tabulky barev.
 *
 * @Param data Vektor obsahujici data nactena z obrazku.
 * @Param pixelBits Velikost globalni tabulky barev.
 *
 * @Returns   Vektor obsahujici struktury tGlobalColor.
 */
/* ----------------------------------------------------------------------------*/
vector<tGlobalColor> pullGlobalColor(vector<int32_t> data, char pixelBits) {
// nacteni dat do globalni tabulky barev
	uint16_t countIndexTable = 1 << pixelBits;																				// velikost globalni tabulkz barev max 255
	vector<tGlobalColor> aGColor;																									// vektor struktur RGB
	tGlobalColor globalColor = {0,0,0};																							// pomocna struktura RGB
	uint16_t pozicion = 0;
	for(uint16_t i = 0; i < countIndexTable; i++) {
		globalColor.red = data[pozicion++];																						// cervena barva
		globalColor.green = data[pozicion++];																					// zelena barva
		globalColor.blue = data[pozicion++];																					// modra barva	
		aGColor.push_back(globalColor);																							// vlozeni struktury do vektoru
	}

	return aGColor;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktury Rozsireni rizeni grafiky.
 *
 * @Param data Vektor obsahujici data nactena z obrazku.
 *
 * @Returns   Naplnena struktura pgraphicExt.
 */
/* ----------------------------------------------------------------------------*/
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

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Naplneni struktury reprezentujici lokalni popisovac predlohy.
 *
 * @Param data Vektor obsahujici vstupni data.
 *
 * @Returns    Naplnena struktura tImageDescription daty z obrazku.
 */
/* ----------------------------------------------------------------------------*/
tImageDescription pullImageDescription(vector<int32_t> data) {
	tImageDescription imageDesc;																									// struktura lokalniho popisovace obrazku
	imageDesc.imageSeparator = data[0];																							// oddelovac 0x2c
	imageDesc.imageLeftPos = hexToDec(data[1], data[2]);																	// leva pozice obrazku
	imageDesc.imageTopPos = hexToDec(data[3], data[4]);																	// horni pozice obrazku
	imageDesc.imageWidth = hexToDec(data[5], data[6]);																		// sirka bloku obrazku
	imageDesc.imageHeight = hexToDec(data[7], data[8]);																	// vyska bloku obrazku
	imageDesc.packedFields.color = (data[9] & 128) >> 7;																	// 1 pokud je lokalni tabulka barev prtiomna (8bit)
	imageDesc.packedFields.int32_terleaved = (data[9] & 64) >> 6;														// 1 pokud je obrazek prokladan (7bit)
	imageDesc.packedFields.sort = (data[9] & 32) >> 5;																		// 1 pokud jsou barvy truideny podle dulezitosti TODO:podivat se
	imageDesc.packedFields.reserved = (data[9] & 24) >> 3;																// vyhrazeno
	imageDesc.packedFields.size = (data[9] & 7);																				// velikost polozek lokalni tabulky barev
	return imageDesc;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------- Tisk struktur ----------------------//
// ////////////////////////////////////////////////////////////////////////////////////////////////////////

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tisken globalni tabulku barev,
 *
 * @Param color Vektor struktur obsahujici globalni tabulku barev.
 */
/* ----------------------------------------------------------------------------*/
void printTGlobalColor(vector<tGlobalColor> color) {
	fprintf(stderr, "\n------------------------------ Globalni tabulka barev -----------------------------------\n\n");
	for(uint16_t i = 0; i < color.size(); i++) {
		fprintf(stderr,"index:%d red=0x%x, green=0x%x, blue=0x%x\n",i, color[i].red, color[i].green, color[i].blue);
	}
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tsik strukturu packed z rozsireni.
 *
 * @Param packedFields Struktura packed obsahujic i info o obrazku.
 */
/* ----------------------------------------------------------------------------*/
void printTPacked(tPACKED packedFields) {
	fprintf(stderr, "7bit=%d, 4-6bit=%d, 3bit=%d, 0-2bit=%d\n", packedFields.globalColorMap, 
			packedFields.colorResolutionBits, packedFields.reserved, packedFields.pixelBits
			);
	string str = "NE";
	if(packedFields.globalColorMap == 1) {
		str = "ANO";
	}
	fprintf(stderr, "Pritomnost globalni tabulky : %s\nBarevna rozlisovaci hodnota na jedne pixel : %d\n \
			Rezervovano : %d\nVelikost globalni tabulky : %d\n", 	str.c_str(), 
			packedFields.colorResolutionBits, packedFields.reserved,	1 << packedFields.pixelBits
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tsik strukturu packed z rozsireni.
 *
 * @Param packedFields Struktura packed obsahujic i info o obrazku.
 */
/* ----------------------------------------------------------------------------*/
void printTPackedEx(tPACKEDEX packedFields) {
	fprintf(stderr, "5-7bit=%d, 2-4bit=%d, 1bit=%d, 0bit=%d\n", packedFields.reserved, 
			packedFields.disposalMethod,packedFields.input,	packedFields.colorFlag
			);
	string str = "NE";
	if(packedFields.colorFlag == 1) {
		str = "ANO";
	}
	string str1 = "NE";
	if(packedFields.input == 1) {
		str1 = "ANO";
	}
	fprintf(stderr, "Vyhrazeno : %d\nManipulace s garfiou : 0x%x\nOcekavan uzivateluv vstup : %s\n \
			Obsahuje polozka 'index pruhlednosti' index do palety barev : %s\n", 
			packedFields.reserved, packedFields.disposalMethod,str1.c_str(),	str.c_str()
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne popis obrazku struktury tPackedImg.
 *
 * @Param packedFields Data struktury, info o lokalni tabulce barev atd..
 */
/* ----------------------------------------------------------------------------*/
void printTPackedImg(tPACKEDIMG packedFields) {
	string str = "NE";
	if(packedFields.color == 1) {																									// obsahuje lokalni tabulku barev?
		str = "ANO";
	}
	string str1 = "NE";
	if(packedFields.int32_terleaved == 1) {																					// jsou data prokladana?
		str1 = "ANO";
	}
	string str2 = "NE";
	if(packedFields.sort == 1) {																									// je tabulka barev tridena?
		str2 = "ANO";
	}
	fprintf(stderr, "Pocet bitu na oplozku v loaklni tabulca barev : %d\nVyhrazeno : %d\n \
			Tabulak barev tridena podle dulezitosti : %s\nProkladani : %s\nPritomna lokalni tabulka barev : %s\n", 
			packedFields.size, packedFields.reserved,	str2.c_str(),str1.c_str(),	str.c_str()
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tisken strukturu obsahujici logicky popis obrazku.
 *
 * @Param logDescription Logicky popis obrazku - struktura.
 */
/* ----------------------------------------------------------------------------*/
void printTLoscdes(tLOSCDES logDescription) {
	fprintf(stderr, "\n------------------------------ Popisovac logicke obrazovky ------------------------------\n\n");
	fprintf(stderr, "Rozmery obrazku - sirka : %dpx, vyska : %dpx\n",	logDescription.screenWidth, logDescription.screenHeight);
	printTPacked(logDescription.packedFields);
	fprintf(stderr,"Index do barvy pozadi : %d\nPomer stran : %d\n",	logDescription.backgroundColorIndex,
																						 	logDescription.pixelAspectRatio
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne popis obrazku na STDOUT.
 *
 * @Param des Struktura obsahujici popis obrazku.
 */
/* ----------------------------------------------------------------------------*/
void printTImageDescription(tImageDescription des) {
	fprintf(stderr, "\n------------------------------ Lokalni popisovac obrazku --------------------------------\n\n");
	fprintf(stderr, "Identifikator : 0x%x\nSourdanice  - X  : %d, sourdanice Y : %d\nRozmery - sirka : %dpx, vyska : %dpx\n",	
				des.imageSeparator, des.imageLeftPos, des.imageTopPos, des.imageWidth, des.imageHeight
			);
	printTPackedImg(des.packedFields);
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne rozsireni grafiky na STDOUT.
 *
 * @Param ext Popisovac grafickeho rozsireni.
 */
/* ----------------------------------------------------------------------------*/
void printTGraphicControlExt(tGraphicControlExt ext) {
	fprintf(stderr, "\n------------------------------ Blok Rozsireni rizeni grafiky ----------------------------\n\n");
	fprintf(stderr, "Identifikace rozsireni : 0x%x, typ : 0x%x, velikost : 0x%x\n",	
			ext.extIntroducer, ext.graphicControlLabel, ext.blockSize
			);
	printTPackedEx(ext.packedFields);
	fprintf(stderr, "Prodleva v setinach sekundy : %d\nIndex pruhlednosti barev : %d\nTerminator, ukonceni : 0x%x\n",	
			ext.delayTime,	ext.transparentColorIndex,	ext.blockTerminator
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne hlavicku souboru GIF. Tedy bud GIF87a nebo GIF89a.
 *
 * @Param head Struktura  obsahujici prvnich 6 bitu souboru gif.
 */
/* ----------------------------------------------------------------------------*/
void printTHeader(tHEADER head) {
	fprintf(stderr, "\n------------------------------ Hlavicka -------------------------------------------------\n\n");
	fprintf(stderr, "Verze obrazku : %c%c%c%c%c%c\n",	head.signature[0], head.signature[1], head.signature[2],
																	head.version[0], head.version[1], head.version[2]		
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne obsah rozsireni komentare na STDOUT.
 *
 * @Param ext Struktura komenater obrazku.
 */
/* ----------------------------------------------------------------------------*/
void printTCommentExt(tCommentExt ext) {
	fprintf(stderr, "\n------------------------------Struktura tCommentExt ---------------------------------\n\n");
	fprintf(stderr, "extIntroducer=0x%x, commentLabel=0x%x, commentData=%s, blockTerminator=0x%x\n",
			ext.extIntroducer, ext.commentLabel, ext.commentData.c_str(),	ext.blockTerminator
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne obsah struktury popisujici textove rozsireni na STDOUT.
 *
 * @Param ext Struktura rozsireneho textu.
 */
/* ----------------------------------------------------------------------------*/
void printTPlainTextExt(tPlainTextExt ext) {
	fprintf(stderr, "\n------------------------------Struktura tPlainTextExt ---------------------------------\n\n");
	fprintf(stderr, "extIntroducer=0x%x, extLabel=0x%x, blockSize=%d, textGridLeftPos=%d, textGridTopPos=%d,	\
			textGridWidth=%d, textGridHeight=%d, characterCellWidth=%d, characterCellHeight=%d, textForColorIndex=%d, \
			textBackgroundColorIndex=%d, plainTextData=%s, blockTerminator=0x%x\n",	ext.extIntroducer,
			ext.plainTextLabel, ext.blockSize, 	ext.textGridLeftPos,	ext.textGridTopPos,ext.textGridWidth,
			ext.textGridHeight, ext.characterCellWidth, ext.characterCellHeight,	ext.textForegroundColorIndex,
			ext.textBackgroundColorIndex,	ext.plainTextData.c_str(),	ext.blockTerminator
			);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Tiskne na standardni chybovy vystup obsah struktury tGIF.
 *
 * @Param gif Struktura tGIF jehoooz data se tisknou.
 */
/* ----------------------------------------------------------------------------*/
void printGif(tGIF gif) {
	printTHeader(gif.header);
	printTLoscdes(gif.logDescription);
	if(gif.logDescription.packedFields.globalColorMap == 1) {
		printTGlobalColor(gif.globalColor);
	}
	if(gif.graphicControlExt.extIntroducer == 0x21) {
		printTGraphicControlExt(gif.graphicControlExt);
	}
	printTImageDescription(gif.imageDescription);
	fprintf(stderr, "\n------------------------------ Obecne informace k LZW -----------------------------------\n\n");
	fprintf(stderr, "Delka LZW kodu : %d\n", gif.sizeLZW);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------- Pomocne funkce ---------------------//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevod cisla z formatu bigendiab na littleEndian.
 *
 * @Param num1 Prvni cislo.
 * @Param num2 Druhe cislo.
 *
 * @Returns   Jedno cislo spojujici obe vstupni cisla.
 */
/* ----------------------------------------------------------------------------*/
unsigned short hexToDec(BYTE num1, BYTE num2) {
	fprintf(stderr, "velikost n1 = 0x%x, n2=0x%x\n", num1, num2);
	if(num2 != 0) {																													// pokud druhe cislo neni nulove
	fprintf(stderr, "n = %d\n", num1+(num2<<8));
		return (num1 + (num2 << 8));																								// posunuti druheho cisla o 8 mist
	}
	else {
	fprintf(stderr, "n = %d\n", num1);
		return (num1);																										// secteni dvou cisel
	}
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Kontroluje zda vstupni soubor je typu gif.
 *
 * @Param signature[] Pole bajtu obsahujici retezec GIF.
 * @Param version[] Pole bajtu, ktere by melo obsahovat verzi gifu.
 *
 * @Returns   Cislo typu gifu nebo 0 pokud obrazek neni gif.
 */
/* ----------------------------------------------------------------------------*/
int8_t isGif(BYTE signature[], BYTE version[] ) {
// kontrola zda je obrazek gif
	if(signature[0] != 0x47 || signature[1] != 0x49 || signature[2] != 0x46) {										// neni gif
		fprintf(stderr, "\nCHYBA: Obrazek neni typu gif nebo je poskozen.");
		return -1;
	}
// kontrola obrazek verze
	if(version[0] == 0x38 && version[1] == 0x39 && version[2] == 0x61) {												// typ 89a
		return 89;
	}
	else if(version[0] == 0x38 && version[1] == 0x37 && version[2] == 0x61) {										// typ 87a
		return 87;
	}
	else {																																// je to gif lae jine verze?
		fprintf(stderr, "\nCHYBA: Obrazek neni verze 87a ani 89a nebo je poskozen.");
		return -1;
	}
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevod retezce nul a jednicek na cislo v dekadickem tvaru, 
 *            vezme se pocet znaku podle hodnoty bit.
 *
 * @Param str Vstupni retezec 1 a 0, ktery se bude prevadet na cislo.
 * @Param bit Pocet znaku, ktere se budou prevadet na cisla.
 *
 * @Returns   Cislo v dekadickem tvaru.
 */
/* ----------------------------------------------------------------------------*/
uint32_t bin2dec(string str, uint8_t bit) {
	uint16_t mask[] = {1,2,4,8,16,32,64,128,256,512,1024, 2048};
	uint16_t num = 0;
	string pom;
	uint8_t pos = bit-1;
	for(uint8_t i = 0; i <= bit; i++, pos--) {
		pom = str.substr(i,1);
		if(pom.compare("1") == 0) {
			num = num + mask[pos];
		}
	}
	return num;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevadi prokladane ulozeni dat na normalni. Nejprve jsou ulozeny
 *            vsechny sude radky a pak liche. Kazdy nasledujici pruchod 
 *            obrazkem uklada vice radku nez pruchod predchozi.
 *
 * @Param gif Obrazek gif, obsahujici prokladana data, sirku a vysku obrazku.
 *
 * @Returns   Preformatovana prokladana data na normalni posloupnost.
 */
/* ----------------------------------------------------------------------------*/
vector<uint16_t> sortData(tGIF gif) {
	uint32_t i,j;
	vector<uint16_t> table2;
	uint32_t width = gif.logDescription.screenWidth;
	uint32_t size = gif.logDescription.screenHeight * gif.logDescription.screenWidth;
	for(i = 0; i < size; i++) {
		table2.push_back(0);
	}
	j = 0;
	uint32_t v,p;
	for(i = 0; i < size; i += (8*width), j += width) {																		// interlace pass 1
		v = i;
		p = j;
		for(uint32_t k = 0; k < width; k++, v++, p++) {
			table2[v] = gif.data[p];
		}
	}
	for(i = (4*width); i < size; i += (8*width), j+=width) {																// interlace pass 2
		v = i;
		p = j;
		for(uint32_t k = 0; k < width; k++, v++, p++) {
			table2[v] = gif.data[p];
		}
	}
	for(i = (2*width); i < size; i += (4*width), j+= width) {															// interlace pass 3
		v = i;
		p = j;
		for(uint32_t k = 0; k < width; k++, v++, p++) {
			table2[v] = gif.data[p];
		}
	}
	for(i = (width*1); i < size; i += (2*width), j+= width) {															// interlace pass 4
		v = i;
		p = j;
		for(uint32_t k = 0; k < width; k++, v++, p++) {
			table2[v] = gif.data[p];
		}
	}
	return table2;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevadi vektor cisel na retezec cisel.
 *
 * @Param vect Vektor cisel.
 *
 * @Returns   Retezec cisel.
 */
/* ----------------------------------------------------------------------------*/
string vector2Str(vector<uint16_t> vect) {
	string str = "";
	for(uint32_t i = 0; i < vect.size(); i++) {
		str += int2Str(vect[i]) + " ";
	}
	return str;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevadi cislo na retzec.
 *
 * @Param num Cislo pro prevod na retezec.
 *
 * @Returns   Csilo prevedene na retezec.
 */
/* ----------------------------------------------------------------------------*/
string int2Str(uint16_t num) {
	stringstream ss;
	ss << num;																															// prevod cisla na retezec
	return ss.str();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Prevod cisla na retezec v binarni podobe.
 *
 * @Param num Cis;ov dekadickem formatu.
 *
 * @Returns   Retezec obsahujici cislo v binarni podobe
 */
/* ----------------------------------------------------------------------------*/
string dec2bin(uint16_t num) {
	string str;
	uint8_t i = 0;
	uint8_t pom = 0;
	bool tr = true;
	uint8_t count = 0;
	while(tr) {
		pom = num>>1;
		i = num-(pom<<1);
		if(i == 0) {
		str.insert(0,"0" );
		}
		else {
		str.insert(0,"1" );
		}
		if(pom ==0) {
			tr = false;
		}
		num = pom;
		count++;
	}
	for(;count<8;count++) {
		str.insert(0,"0" );
	}
	return str;
}

