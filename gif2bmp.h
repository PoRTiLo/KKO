#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>

typedef unsigned char BYTE;	// 1 byte
typedef unsigned short WORD;	// 2 byte
typedef unsigned long DWORD;	// 4 byte


typedef struct {
	int64_t bmpSize;
	int64_t long gifSize;
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
	tHEADER header;
	tLOSCDES logDescription;
} tGIF;

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} tGlobalColorT;

typedef struct {
	BYTE imageSeparator;
	WORD imageLeftPos;
	WORD imageTopPos;
	WORD imageWIdth;
	WORD imageHeight;
	BYTE packedFields;
} tImageDescription;

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} tLocalColorT;

typedef struct {
	BYTE lzwSize;
	int imageData;
} tImageData;

typedef struct {
	BYTE extIntroducer;
	BYTE graphicControlLabel;
	BYTE blockSize;
	BYTE packedFields;
	WORD delayTime;
	BYTE transparentColorIndex;
	BYTE blockTerminator;
} tGraphicControlExt;

typedef struct {
	BYTE extIntroducer;
	BYTE commentLabel;
	char commentData;						// data TODO:retezec znaku-dodelta
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
	char plainTextData;					// data TODO:retezec dodelat
	BYTE blockTerminator;
} tPlainTextExt;

typedef struct {
	BYTE extIntroducer;
	BYTE extLabel;
	BYTE blockSize;
	BYTE applicationIdentifier[8];
	BYTE applicationAuthCode[3];
	char applicationData;					// TODO:data
	BYTE blockTerminator;
} tApplicatiopnExt;

typedef struct {
	BYTE gifTrailer;
} tTrailer;

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);
int openGif(FILE *inputFile);
tGIF pullHead(int pole[]);
char isGif(BYTE type[], BYTE version[]);
unsigned short hexToDec(BYTE num1, BYTE num2);
void printTPacked(tPACKED packedFields); 
void printTLoscdes(tLOSCDES logDescription);
void printGlobalColorTable(std::map<int,tGlobalColorT> colorTable); 
