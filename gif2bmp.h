#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sstream>
#include <iostream>
#include <inttypes.h>

using namespace std;

typedef uint8_t BYTE;	// 1 byte
typedef uint16_t WORD;	// 2 byte
typedef uint32_t DWORD;	// 4 byte


typedef struct {
	int64_t bmpSize;
	int64_t gifSize;
} tGIF2BMP;

typedef struct {
	int8_t data;
} tGIFDATA;

typedef struct {
	BYTE signature[3];
	BYTE version[3];
} tHEADER;

typedef struct {
	BYTE globalColorMap;
	BYTE colorResolutionBits;
	BYTE reserved;
	BYTE pixelBits;
} tPACKED;

typedef struct {
	WORD screenWidth;
	WORD screenHeight;
	tPACKED packedFields;
	BYTE backgroundColorIndex;
	BYTE pixelAspectRatio;
} tLOSCDES;

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} tGlobalColor;

typedef struct {
	BYTE color;
	BYTE int32_terleaved;
	BYTE sort;
	BYTE reserved;
	BYTE size;
} tPACKEDIMG;

typedef struct {
	BYTE imageSeparator;
	WORD imageLeftPos;
	WORD imageTopPos;
	WORD imageWidth;
	WORD imageHeight;
	tPACKEDIMG packedFields;
} tImageDescription;

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} tLocalColorT;

typedef struct {
	BYTE colorFlag;
	BYTE input;
	BYTE reserved;
	BYTE disposalMethod;
} tPACKEDEX;

typedef struct {
	BYTE extIntroducer;
	BYTE graphicControlLabel;
	BYTE blockSize;
	tPACKEDEX packedFields;
	WORD delayTime;
	BYTE transparentColorIndex;
	BYTE blockTerminator;
} tGraphicControlExt;

typedef struct {
	BYTE extIntroducer;
	BYTE commentLabel;
	string commentData;
	BYTE blockTerminator;
} tCommentExt;

typedef struct {
	BYTE extIntroducer;
	BYTE plainTextLabel;
	BYTE blockSize;
	WORD textGridLeftPos;
	WORD textGridTopPos;
	WORD textGridWidth;
	WORD textGridHeight;
	BYTE characterCellWidth;
	BYTE characterCellHeight;
	BYTE textForegroundColorIndex;
	BYTE textBackgroundColorIndex;
	string plainTextData;
	BYTE blockTerminator;
} tPlainTextExt;

typedef struct {
	BYTE extIntroducer;
	BYTE extLabel;
	BYTE blockSize;
	BYTE applicationIdentifier[8];
	BYTE applicationAuthCode[3];
	string applicationData;
	BYTE blockTerminator;
} tApplicatiopnExt;

typedef struct {
	BYTE gifTrailer;
} tTrailer;

typedef struct {
	tHEADER header;
	tLOSCDES logDescription;
	vector<tGlobalColor> globalColor;
	tImageDescription imageDescription;
	tGraphicControlExt graphicControlExt;
	int16_t sizeLZW;
	vector<WORD> data;
} tGIF;

typedef struct {
	WORD fileType;
	DWORD size;
	WORD reserved1;
	WORD reserved2;
	DWORD dataOffset;
} tBmpHeader;

typedef struct {
	DWORD headerSize;
	DWORD width;
	DWORD height;
	WORD numOfPlanes;
	WORD bitsPerPixel;
	DWORD compressMethod;
	DWORD sizeBitmap;
	DWORD horzResolution;
	DWORD vertResolution;
	DWORD numColorUsed;
	DWORD significantColors;
} tBmpInfoHeader;

typedef struct {
	tBmpHeader header;
	tBmpInfoHeader infoHeader;
	vector<WORD> data;
} tBMP;

int32_t gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);
// ------------------ Pomocne funkce ----------------------------------
vector<int32_t> openGif(FILE *inputFile);
char isGif(BYTE type[], BYTE version[]);
unsigned short hexToDec(BYTE num1, BYTE num2);
void sortData();
// ------------------  Funkce plnici struktury ------------------------
vector<tGlobalColor> pullGlobalColor(vector<int32_t> data, char pixelBits);
tLOSCDES pullLogDesc(vector<int32_t> data);
tHEADER pullHead(vector<int32_t> data);
tGIF pullGif(vector<int32_t> data);
tGraphicControlExt pullGraphicControlExt(vector<int32_t> data);
tImageDescription pullImageDescription(vector<int32_t> data);
// ------------------- Pomocne vypisove funkce ------------------------
void printTPackedEx(tPACKEDEX packedFields);
void printTLoscdes(tLOSCDES logDescription);
void printTPacked(tPACKED packedFields); 
void printTPackedImg(tPACKEDIMG packedFields);
void printTGlobalColor(vector<tGlobalColor> color);
void printTImageDescription(tImageDescription des);
void printTGraphicControlExt(tGraphicControlExt ext);
void printTHeader(tHEADER head);
void printTCommentExt(tCommentExt ext);
void printTPlainTextExt(tPlainTextExt ext);
vector<WORD> decodeLZW(string subBlock, tGIF gif);
string dec2bin(WORD num);
DWORD bin2dec(string, BYTE bit);
string int2Str(WORD num);
string vector2Str(vector<WORD> vect);
void printGif(tGIF gif);
BYTE createBmp(tGIF gif, FILE *outputFile);
tBMP pullBmp(tGIF gif);
vector<WORD> bmpData(tGIF gif);
