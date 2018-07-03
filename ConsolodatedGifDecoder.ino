/*
    Animated GIFs Display Code for SmartMatrix and 32x32 RGB LED Panels

    This file contains code to enumerate and select animated GIF files by name

    Written by: Craig A. Lindley
*/

//#include "FilenameFunctions.h"    //defines USE_SPIFFS as reqd

#if defined (ARDUINO)
//#ifdef USE_SPIFFS
#if defined(ESP8266)
#include "FS.h"
//#define USE_SPIFFS_DIR
#elif defined(ESP32)
//#include <SPIFFS.h>
//#define SD SPIFFS
#else
#error USE_SPIFFS only valid on Expressif controllers
#endif
#else
//#include <SD.h>
#endif
//#elif defined (SPARK)
//#include "sd-card-library-photon-compat/sd-card-library-photon-compat.h"
//#endif

//File file;

int numberOfFiles;

bool fileSeekCallback(unsigned long position) {
//#ifdef USE_SPIFFS
//    return file.seek(position, SeekSet);
//#else
    return file.seek(position);
//#endif
}

unsigned long filePositionCallback(void) {
    return file.position();
}

int fileReadCallback(void) {
    return file.read();
}

int fileReadBlockCallback(void * buffer, int numberOfBytes) {
    return file.read((uint8_t*)buffer, numberOfBytes); //.kbv
}

//int initSdCard(int chipSelectPin) {
//    // initialize the SD card at full speed
//    pinMode(chipSelectPin, OUTPUT);
//#ifdef USE_SPIFFS
//    if (!SPIFFS.begin())
//        return -1;
//#else
//    if (!SD.begin(chipSelectPin))
//        return -1;
//#endif
//    return 0;
//}

bool isAnimationFile(const char filename []) {
    if (filename[0] == '_')
        return false;

    if (filename[0] == '~')
        return false;

    if (filename[0] == '.')
        return false;

    String filenameString = String(filename);   //.kbv STM32 and ESP need separate statements
    filenameString.toUpperCase();
    if (filenameString.endsWith(".GIF") != 1)
        return false;

    return true;
}

// Enumerate and possibly display the animated GIF filenames in GIFS directory
int enumerateGIFFiles(const char *directoryName, bool displayFilenames) {

    char *filename;
    numberOfFiles = 0;
//#ifdef USE_SPIFFS_DIR
//    File file;
//    Dir directory = SPIFFS.openDir(directoryName);
//    //    if (!directory == 0) return -1;
//
//    while (directory.next()) {
//        file = directory.openFile("r");
//        if (!file) break;
//#else
    File file;
    File directory = SD.open(directoryName);
    if (!directory) {
        return -1;
    }

    while (file = directory.openNextFile()) {
//#endif
        filename = (char*)file.name();
        if (isAnimationFile(filename)) {
            numberOfFiles++;
            if (displayFilenames) {
                Serial.print(numberOfFiles);
                Serial.print(":");
                Serial.print(filename);
                Serial.print("    size:");
                Serial.println(file.size());
            }
        }
        else Serial.println(filename);
        file.close();
    }

    //    file.close();
//#ifdef USE_SPIFFS
//#else
    directory.close();
//#endif

    return numberOfFiles;
}

// Get the full path/filename of the GIF file with specified index
void getGIFFilenameByIndex(const char *directoryName, int index, char *pnBuffer) {

    char* filename;

    // Make sure index is in range
    if ((index < 0) || (index >= numberOfFiles))
        return;

//#ifdef USE_SPIFFS_DIR
//    Dir directory = SPIFFS.openDir(directoryName);
//    //    if (!directory) return;
//
//    while (directory.next() && (index >= 0)) {
//        file = directory.openFile("r");
//        if (!file) break;
//#else
    File directory = SD.open(directoryName);
    if (!directory) {
        return;
    }

    while ((index >= 0)) {
        file = directory.openNextFile();
        if (!file) break;
//#endif

        filename = (char*)file.name();  //.kbv
        if (isAnimationFile(filename)) {
            index--;

            // Copy the directory name into the pathname buffer
            strcpy(pnBuffer, directoryName);
//#if defined(ESP32) || defined(USE_SPIFFS)
//            pnBuffer[0] = 0;
//#else
            int len = strlen(pnBuffer);
            if (len == 0 || pnBuffer[len - 1] != '/') strcat(pnBuffer, "/");
//#endif
            // Append the filename to the pathname
            strcat(pnBuffer, filename);
        }

        file.close();
    }

    file.close();
//#ifdef USE_SPIFFS
//#else
    directory.close();
//#endif
}

int openGifFilenameByIndex(const char *directoryName, int index) {
    char pathname[40];

    getGIFFilenameByIndex(directoryName, index, pathname);

    Serial.print("Pathname: ");
    Serial.println(pathname);

    if (file)
        file.close();

    // Attempt to open the file for reading
//#ifdef USE_SPIFFS
//    file = SPIFFS.open(pathname, "r");
//#else
    file = SD.open(pathname);
//#endif
    if (!file) {
        Serial.println("Error opening GIF file");
        return -1;
    }

    return 0;
}


// Return a random animated gif path/filename from the specified directory
void chooseRandomGIFFilename(const char *directoryName, char *pnBuffer) {

    int index = random(numberOfFiles);
    getGIFFilenameByIndex(directoryName, index, pnBuffer);
}


/*
    Animated GIFs Display Code for SmartMatrix and 32x32 RGB LED Panels

    This file contains code to parse animated GIF files

    Written by: Craig A. Lindley

    Copyright (c) 2014 Craig A. Lindley
    Minor modifications by Louis Beaudoin (pixelmatix)

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define GIFDEBUG 2

#if defined (ARDUINO)
#include <Arduino.h>
#elif defined (SPARK)
#include "application.h"
#endif

#include "GifDecoder.h"

#if GIFDEBUG == 2
#define DEBUG_SCREEN_DESCRIPTOR                             1
#define DEBUG_GLOBAL_COLOR_TABLE                            1
#define DEBUG_PROCESSING_PLAIN_TEXT_EXT                     1
#define DEBUG_PROCESSING_GRAPHIC_CONTROL_EXT                1
#define DEBUG_PROCESSING_APP_EXT                            1
#define DEBUG_PROCESSING_COMMENT_EXT                        0
#define DEBUG_PROCESSING_FILE_TERM                          1
#define DEBUG_PROCESSING_TABLE_IMAGE_DESC                   1
#define DEBUG_PROCESSING_TBI_DESC_START                     1
#define DEBUG_PROCESSING_TBI_DESC_INTERLACED                0
#define DEBUG_PROCESSING_TBI_DESC_LOCAL_COLOR_TABLE         1
#define DEBUG_PROCESSING_TBI_DESC_LZWCODESIZE               1
#define DEBUG_PROCESSING_TBI_DESC_DATABLOCKSIZE             0
#define DEBUG_PROCESSING_TBI_DESC_LZWIMAGEDATA_OVERFLOW     1
#define DEBUG_PROCESSING_TBI_DESC_LZWIMAGEDATA_SIZE         1
#define DEBUG_PARSING_DATA                                  1
#define DEBUG_DECOMPRESS_AND_DISPLAY                        1

#define DEBUG_WAIT_FOR_KEY_PRESS                            0

#endif

//#include "GifDecoder.h"


// Error codes
#define ERROR_NONE                 0
#define ERROR_DONE_PARSING         1
#define ERROR_WAITING              2
#define ERROR_FILEOPEN             -1
#define ERROR_FILENOTGIF           -2
#define ERROR_BADGIFFORMAT         -3
#define ERROR_UNKNOWNCONTROLEXT    -4

#define GIFHDRTAGNORM   "GIF87a"  // tag in valid GIF file
#define GIFHDRTAGNORM1  "GIF89a"  // tag in valid GIF file
#define GIFHDRSIZE 6

// Global GIF specific definitions
#define COLORTBLFLAG    0x80
#define INTERLACEFLAG   0x40
//#define TRANSPARENTFLAG 0x01
#define TRANSPARENTFLAG 0x01

#define NO_TRANSPARENT_INDEX -1

// Disposal methods
#define DISPOSAL_NONE       0
#define DISPOSAL_LEAVE      1
#define DISPOSAL_BACKGROUND 2
#define DISPOSAL_RESTORE    3


template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setStartDrawingCallback(callback f) {
    startDrawingCallback = f;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setUpdateScreenCallback(callback f) {
    updateScreenCallback = f;
}

//template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
//void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setDrawPixelCallback(pixel_callback f) {
//    drawPixelCallback = f;
//}

//template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
//void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setDrawLineCallback(line_callback f) {
//    drawLineCallback = f;
//}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setScreenClearCallback(callback f) {
    screenClearCallback = f;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setFileSeekCallback(file_seek_callback f) {
    fileSeekCallback = f;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setFilePositionCallback(file_position_callback f) {
    filePositionCallback = f;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setFileReadCallback(file_read_callback f) {
    fileReadCallback = f;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::setFileReadBlockCallback(file_read_block_callback f) {
    fileReadBlockCallback = f;
}

// Backup the read stream by n bytes
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::backUpStream(int n) {
    fileSeekCallback(filePositionCallback() - n);
}

// Read a file byte
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::readByte() {

    int b = fileReadCallback();
    if (b == -1) {
#if GIFDEBUG == 1
        Serial.println("Read error or EOF occurred");
#endif
    }
    return b;
}

// Read a file word
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::readWord() {

    int b0 = readByte();
    int b1 = readByte();
    return (b1 << 8) | b0;
}

// Read the specified number of bytes into the specified buffer
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::readIntoBuffer(void *buffer, int numberOfBytes) {

    int result = fileReadBlockCallback(buffer, numberOfBytes);
    if (result == -1) {
        Serial.println("Read error or EOF occurred");
    }
#if defined(USE_PALETTE565)
    if (buffer == palette) {
        for (int i = 0; i < 256; i++) {
            uint8_t r = palette[i].red;
            uint8_t g = palette[i].green;
            uint8_t b = palette[i].blue;
            palette565[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
        }
    }
#endif
    return result;
}

// Fill a portion of imageData buffer with a color index
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::fillImageDataRect(uint8_t colorIndex, int x, int y, int width, int height) {

    int yOffset;

    for (int yy = y; yy < height + y; yy++) {
        yOffset = yy * maxGifWidth;
        for (int xx = x; xx < width + x; xx++) {
#if NO_IMAGEDATA < 2
            imageData[yOffset + xx] = colorIndex;
#endif
        }
    }
}

// Fill entire imageData buffer with a color index
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::fillImageData(uint8_t colorIndex) {

#if NO_IMAGEDATA < 2
    memset(imageData, colorIndex, sizeof(imageData));
#endif
}

// Copy image data in rect from a src to a dst
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::copyImageDataRect(uint8_t *dst, uint8_t *src, int x, int y, int width, int height) {

    int yOffset, offset;

    for (int yy = y; yy < height + y; yy++) {
        yOffset = yy * maxGifWidth;
        for (int xx = x; xx < width + x; xx++) {
            offset = yOffset + xx;
            dst[offset] = src[offset];
        }
    }
}

// Make sure the file is a Gif file
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
bool GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseGifHeader() {

    char buffer[10];

    readIntoBuffer(buffer, GIFHDRSIZE);
    if ((strncmp(buffer, GIFHDRTAGNORM,  GIFHDRSIZE) != 0) &&
            (strncmp(buffer, GIFHDRTAGNORM1, GIFHDRSIZE) != 0))  {
        return false;
    }
    else    {
        return true;
    }
}

// Parse the logical screen descriptor
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseLogicalScreenDescriptor() {

    lsdWidth = readWord();
    lsdHeight = readWord();
    lsdPackedField = readByte();
    lsdBackgroundIndex = readByte();
    lsdAspectRatio = readByte();
    frameCount = (cycleNo) ? frameNo : 0;  //.kbv
    cycleNo++;                             //.kbv

#if GIFDEBUG == 1 && DEBUG_SCREEN_DESCRIPTOR == 1
    Serial.print("lsdWidth: ");
    Serial.println(lsdWidth);
    Serial.print("lsdHeight: ");
    Serial.println(lsdHeight);
    Serial.print("lsdPackedField: ");
    Serial.println(lsdPackedField, HEX);
    Serial.print("lsdBackgroundIndex: ");
    Serial.println(lsdBackgroundIndex);
    Serial.print("lsdAspectRatio: ");
    Serial.println(lsdAspectRatio);
#endif
}

// Parse the global color table
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseGlobalColorTable() {

    // Does a global color table exist?
    if (lsdPackedField & COLORTBLFLAG) {

        // A GCT was present determine how many colors it contains
        colorCount = 1 << ((lsdPackedField & 7) + 1);

#if GIFDEBUG == 1 && DEBUG_GLOBAL_COLOR_TABLE == 1
        Serial.print("Global color table with ");
        Serial.print(colorCount);
        Serial.println(" colors present");
#endif
        // Read color values into the palette array
        int colorTableBytes = sizeof(rgb_24) * colorCount;
        readIntoBuffer(palette, colorTableBytes);
    }
}

// Parse plain text extension and dispose of it
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parsePlainTextExtension() {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_PLAIN_TEXT_EXT == 1
    Serial.println("\nProcessing Plain Text Extension");
#endif
    // Read plain text header length
    uint8_t len = readByte();

    // Consume plain text header data
    readIntoBuffer(tempBuffer, len);

    // Consume the plain text data in blocks
    len = readByte();
    while (len != 0) {
        readIntoBuffer(tempBuffer, len);
        len = readByte();
    }
}

// Parse a graphic control extension
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseGraphicControlExtension() {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_GRAPHIC_CONTROL_EXT == 1
    Serial.println("\nProcessing Graphic Control Extension");
#endif
    int len = readByte();   // Check length
    if (len != 4) {
        Serial.println("Bad graphic control extension");
    }

    int packedBits = readByte();
    frameDelay = readWord();
    transparentColorIndex = readByte();

    if ((packedBits & TRANSPARENTFLAG) == 0) {
        // Indicate no transparent index
        transparentColorIndex = NO_TRANSPARENT_INDEX;
    }
    disposalMethod = (packedBits >> 2) & 7;

    if (disposalMethod > 3) {
        disposalMethod = 0;
        Serial.println("Invalid disposal value");
    }

    readByte(); // Toss block end

#if GIFDEBUG == 1 && DEBUG_PROCESSING_GRAPHIC_CONTROL_EXT == 1
    Serial.print("PacketBits: ");
    Serial.println(packedBits, HEX);
    Serial.print("Frame delay: ");
    Serial.println(frameDelay);
    Serial.print("transparentColorIndex: ");
    Serial.println(transparentColorIndex);
    Serial.print("disposalMethod: ");
    Serial.println(disposalMethod);
#endif
}

// Parse application extension
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseApplicationExtension() {

    memset(tempBuffer, 0, sizeof(tempBuffer));

#if GIFDEBUG == 1 && DEBUG_PROCESSING_APP_EXT == 1
    Serial.println("\nProcessing Application Extension");
#endif

    // Read block length
    uint8_t len = readByte();

    // Read app data
    readIntoBuffer(tempBuffer, len);

#if GIFDEBUG == 1 && DEBUG_PROCESSING_APP_EXT == 1
    // Conditionally display the application extension string
    if (strlen(tempBuffer) != 0) {
        Serial.print("Application Extension: ");
        Serial.println(tempBuffer);
    }
#endif

    // Consume any additional app data
    len = readByte();
    while (len != 0) {
        readIntoBuffer(tempBuffer, len);
        len = readByte();
    }
}

// Parse comment extension
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseCommentExtension() {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_COMMENT_EXT == 1
    Serial.println("\nProcessing Comment Extension");
#endif

    // Read block length
    uint8_t len = readByte();
    while (len != 0) {
        // Clear buffer
        memset(tempBuffer, 0, sizeof(tempBuffer));

        // Read len bytes into buffer
        readIntoBuffer(tempBuffer, len);

#if GIFDEBUG == 1 && DEBUG_PROCESSING_COMMENT_EXT == 1
        // Display the comment extension string
        if (strlen(tempBuffer) != 0) {
            Serial.print("Comment Extension: ");
            Serial.println(tempBuffer);
        }
#endif
        // Read the new block length
        len = readByte();
    }
}

// Parse file terminator
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseGIFFileTerminator() {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_FILE_TERM == 1
    Serial.println("\nProcessing file terminator");
#endif

    uint8_t b = readByte();
    if (b != 0x3B) {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_FILE_TERM == 1
        Serial.print("Terminator byte: ");
        Serial.println(b, HEX);
#endif
        Serial.println("Bad GIF file format - Bad terminator");
        return ERROR_BADGIFFORMAT;
    }
    else    {
        return ERROR_NONE;
    }
}

// Parse table based image data
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseTableBasedImage() {

#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_START == 1
    Serial.println("\nProcessing Table Based Image Descriptor");
#endif

#if GIFDEBUG == 1 && DEBUG_PARSING_DATA == 1
    Serial.println("File Position: ");
    Serial.println(filePositionCallback());
    Serial.println("File Size: ");
    //Serial.println(file.size());
#endif

    // Parse image descriptor
    tbiImageX = readWord();
    tbiImageY = readWord();
    tbiWidth = readWord();
    tbiHeight = readWord();
    tbiPackedBits = readByte();

#if GIFDEBUG == 1
    Serial.print("tbiImageX: ");
    Serial.println(tbiImageX);
    Serial.print("tbiImageY: ");
    Serial.println(tbiImageY);
    Serial.print("tbiWidth: ");
    Serial.println(tbiWidth);
    Serial.print("tbiHeight: ");
    Serial.println(tbiHeight);
    Serial.print("PackedBits: ");
    Serial.println(tbiPackedBits, HEX);
#endif

    // Is this image interlaced ?
    tbiInterlaced = ((tbiPackedBits & INTERLACEFLAG) != 0);

#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_INTERLACED == 1
    Serial.print("Image interlaced: ");
    Serial.println((tbiInterlaced != 0) ? "Yes" : "No");
#endif

    // Does this image have a local color table ?
    bool localColorTable =  ((tbiPackedBits & COLORTBLFLAG) != 0);

    if (localColorTable) {
        int colorBits = ((tbiPackedBits & 7) + 1);
        colorCount = 1 << colorBits;

#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_LOCAL_COLOR_TABLE == 1
        Serial.print("Local color table with ");
        Serial.print(colorCount);
        Serial.println(" colors present");
#endif
        // Read colors into palette
        int colorTableBytes = sizeof(rgb_24) * colorCount;
        readIntoBuffer(palette, colorTableBytes);
    }

    // One time initialization of imageData before first frame
    if (keyFrame) {
        frameNo = 0;   //.kbv
        if (transparentColorIndex == NO_TRANSPARENT_INDEX) {
            fillImageData(lsdBackgroundIndex);
        }
        else    {
            fillImageData(transparentColorIndex);
        }
        keyFrame = false;

        rectX = 0;
        rectY = 0;
        rectWidth = maxGifWidth;
        rectHeight = maxGifHeight;
    }
    // Don't clear matrix screen for these disposal methods
    if ((prevDisposalMethod != DISPOSAL_NONE) && (prevDisposalMethod != DISPOSAL_LEAVE)) {
        if (screenClearCallback)
            (*screenClearCallback)();
    }

    // Process previous disposal method
    if (prevDisposalMethod == DISPOSAL_BACKGROUND) {
        // Fill portion of imageData with previous background color
        fillImageDataRect(prevBackgroundIndex, rectX, rectY, rectWidth, rectHeight);
    }
    else if (prevDisposalMethod == DISPOSAL_RESTORE) {
#if NO_IMAGEDATA < 1
        copyImageDataRect(imageData, imageDataBU, rectX, rectY, rectWidth, rectHeight);
#endif
    }

    // Save disposal method for this frame for next time
    prevDisposalMethod = disposalMethod;

    if (disposalMethod != DISPOSAL_NONE) {
        // Save dimensions of this frame
        rectX = tbiImageX;
        rectY = tbiImageY;
        rectWidth = tbiWidth;
        rectHeight = tbiHeight;

        // limit rectangle to the bounds of maxGifWidth*maxGifHeight
        if (rectX + rectWidth > maxGifWidth)
            rectWidth = maxGifWidth - rectX;
        if (rectY + rectHeight > maxGifHeight)
            rectHeight = maxGifHeight - rectY;
        if (rectX >= maxGifWidth || rectY >= maxGifHeight) {
            rectX = rectY = rectWidth = rectHeight = 0;
        }

        if (disposalMethod == DISPOSAL_BACKGROUND) {
            if (transparentColorIndex != NO_TRANSPARENT_INDEX) {
                prevBackgroundIndex = transparentColorIndex;
            }
            else    {
                prevBackgroundIndex = lsdBackgroundIndex;
            }
        }
        else if (disposalMethod == DISPOSAL_RESTORE) {
#if NO_IMAGEDATA < 1
            copyImageDataRect(imageDataBU, imageData, rectX, rectY, rectWidth, rectHeight);
#endif
        }
    }

    // Read the min LZW code size
    lzwCodeSize = readByte();

#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_LZWCODESIZE == 1
    Serial.print("LzwCodeSize: ");
    Serial.println(lzwCodeSize);
    Serial.println("File Position Before: ");
    Serial.println(filePositionCallback());
#endif

    unsigned long filePositionBefore = filePositionCallback();

    // Gather the lzw image data
    // NOTE: the dataBlockSize byte is left in the data as the lzw decoder needs it
    int offset = 0;
    int dataBlockSize = readByte();
    while (dataBlockSize != 0) {
#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_DATABLOCKSIZE == 1
        Serial.print("dataBlockSize: ");
        Serial.println(dataBlockSize);
#endif
        backUpStream(1);
        dataBlockSize++;
        fileSeekCallback(filePositionCallback() + dataBlockSize);

        offset += dataBlockSize;
        dataBlockSize = readByte();
    }

#if GIFDEBUG == 1 && DEBUG_PROCESSING_TBI_DESC_LZWIMAGEDATA_SIZE == 1
    Serial.print("total lzwImageData Size: ");
    Serial.println(offset);
    Serial.println("File Position Test: ");
    Serial.println(filePositionCallback());
#endif

    // this is the position where GIF decoding needs to pick up after decompressing frame
    unsigned long filePositionAfter = filePositionCallback();

    fileSeekCallback(filePositionBefore);

    // Process the animation frame for display

    // Initialize the LZW decoder for this frame
    lzw_decode_init(lzwCodeSize);
    lzw_setTempBuffer((uint8_t*)tempBuffer);

    // Make sure there is at least some delay between frames
    if (frameDelay < 1) {
        frameDelay = 1;
    }

    // Decompress LZW data and display the frame
    decompressAndDisplayFrame(filePositionAfter);

    // Graphic control extension is for a single frame
    transparentColorIndex = NO_TRANSPARENT_INDEX;
    disposalMethod = DISPOSAL_NONE;
}

// Parse gif data
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::parseData() {
    if (nextFrameTime_ms > millis())
        return ERROR_WAITING;

#if GIFDEBUG == 1 && DEBUG_PARSING_DATA == 1
    Serial.println("\nParsing Data Block");
#endif

    bool parsedFrame = false;
    while (!parsedFrame) {

#if GIFDEBUG == 1 && DEBUG_WAIT_FOR_KEY_PRESS == 1
        Serial.println("\nPress Key For Next");
        while (Serial.read() <= 0);
#endif

        // Determine what kind of data to process
        uint8_t b = readByte();

        if (b == 0x2c) {
            // Parse table based image
#if GIFDEBUG == 1 && DEBUG_PARSING_DATA == 1
            Serial.println("\nParsing Table Based");
#endif
            parseTableBasedImage();
            parsedFrame = true;

        }
        else if (b == 0x21) {
            // Parse extension
            b = readByte();

#if GIFDEBUG == 1 && DEBUG_PARSING_DATA == 1
            Serial.println("\nParsing Extension");
#endif

            // Determine which kind of extension to parse
            switch (b) {
                case 0x01:
                    // Plain test extension
                    parsePlainTextExtension();
                    break;
                case 0xf9:
                    // Graphic control extension
                    parseGraphicControlExtension();
                    break;
                case 0xfe:
                    // Comment extension
                    parseCommentExtension();
                    break;
                case 0xff:
                    // Application extension
                    parseApplicationExtension();
                    break;
                default:
                    Serial.print("Unknown control extension: ");
                    Serial.println(b, HEX);
                    return ERROR_UNKNOWNCONTROLEXT;
            }
        }
        else    {
#if GIFDEBUG == 1 && DEBUG_PARSING_DATA == 1
            Serial.println("\nParsing Done");
#endif

            // Push unprocessed byte back into the stream for later processing
            backUpStream(1);

            return ERROR_DONE_PARSING;
        }
    }
    return ERROR_NONE;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::startDecoding(void) {
    // Initialize variables
    keyFrame = true;
    cycleNo = 0;
    cycleTime = 0;
    prevDisposalMethod = DISPOSAL_NONE;
    transparentColorIndex = NO_TRANSPARENT_INDEX;
    nextFrameTime_ms = 0;
    fileSeekCallback(0);

    // Validate the header
    if (! parseGifHeader()) {
        Serial.println("Not a GIF file");
        return ERROR_FILENOTGIF;
    }
    // If we get here we have a gif file to process

    // Parse the logical screen descriptor
    parseLogicalScreenDescriptor();

    // Parse the global color table
    parseGlobalColorTable();

    return ERROR_NONE;
}

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
//int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::decodeFrame(void) {
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::decodeFrame() {
    // Parse gif data
    int result = parseData();
    if (result < ERROR_NONE) {
        Serial.println("Error: ");
        Serial.println(result);
        Serial.println(" occurred during parsing of data");
        return result;
    }

    if (result == ERROR_DONE_PARSING) {
        //startDecoding();
        // Initialize variables like with a new file
        keyFrame = true;
        prevDisposalMethod = DISPOSAL_NONE;
        transparentColorIndex = NO_TRANSPARENT_INDEX;
        nextFrameTime_ms = 0;
        fileSeekCallback(0);

        // parse Gif Header like with a new file
        parseGifHeader();

        // Parse the logical screen descriptor
        parseLogicalScreenDescriptor();

        // Parse the global color table
        parseGlobalColorTable();
    }

    return result;
}

// Decompress LZW data and display animation frame
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::decompressAndDisplayFrame(unsigned long filePositionAfter) {

    // Each pixel of image is 8 bits and is an index into the palette

    // How the image is decoded depends upon whether it is interlaced or not
    // Decode the interlaced LZW data into the image buffer
#if NO_IMAGEDATA < 2
    uint8_t *p = imageData + tbiImageX;
    if (tbiInterlaced) {
        // Decode every 8th line starting at line 0
        for (int line = tbiImageY + 0; line < tbiHeight + tbiImageY; line += 8) {
            lzw_decode(p + (line * maxGifWidth), tbiWidth, min(imageData + (line * maxGifWidth) + maxGifWidth, imageData + sizeof(imageData)));
        }
        // Decode every 8th line starting at line 4
        for (int line = tbiImageY + 4; line < tbiHeight + tbiImageY; line += 8) {
            lzw_decode(p + (line * maxGifWidth), tbiWidth, min(imageData + (line * maxGifWidth) + maxGifWidth, imageData + sizeof(imageData)));
        }
        // Decode every 4th line starting at line 2
        for (int line = tbiImageY + 2; line < tbiHeight + tbiImageY; line += 4) {
            lzw_decode(p + (line * maxGifWidth), tbiWidth, min(imageData + (line * maxGifWidth) + maxGifWidth, imageData + sizeof(imageData)));
        }
        // Decode every 2nd line starting at line 1
        for (int line = tbiImageY + 1; line < tbiHeight + tbiImageY; line += 2) {
            lzw_decode(p + (line * maxGifWidth), tbiWidth, min(imageData + (line * maxGifWidth) + maxGifWidth, imageData + sizeof(imageData)));
        }
    }
    else    {
        // Decode the non interlaced LZW data into the image data buffer
        for (int line = tbiImageY; line < tbiHeight + tbiImageY; line++) {
            lzw_decode(p  + (line * maxGifWidth), tbiWidth, imageData + sizeof(imageData));
        }
    }

#if GIFDEBUG == 1 && DEBUG_DECOMPRESS_AND_DISPLAY == 1
    Serial.println("File Position After: ");
    Serial.println(filePositionCallback());
#endif

#if GIFDEBUG == 1 && DEBUG_WAIT_FOR_KEY_PRESS == 1
    Serial.println("\nPress Key For Next");
    while (Serial.read() <= 0);
#endif

    // LZW doesn't parse through all the data, manually set position
    fileSeekCallback(filePositionAfter);

    // Optional callback can be used to get drawing routines ready
    if (startDrawingCallback)
        (*startDrawingCallback)();

    // Image data is decompressed, now display portion of image affected by frame
    int yOffset, pixel;
    for (int y = tbiImageY; y < tbiHeight + tbiImageY; y++) {
        yOffset = y * maxGifWidth;
        for (int x = tbiImageX; x < tbiWidth + tbiImageX; x++) {
            // Get the next pixel
            pixel = imageData[yOffset + x];

            // Check pixel transparency
            if (pixel == transparentColorIndex) {
                continue;
            }

            // Pixel not transparent so get color from palette and draw the pixel
            if (true) // if (drawPixelCallback)
                //(*drawPixelCallback)(x, y, palette[pixel].red, palette[pixel].green, palette[pixel].blue);
                write_screen_buffer(x,y,tft.color565(palette[pixel].red, palette[pixel].green, palette[pixel].blue));
        }
    }
#else
#define GSZ maxGifWidth
    //#define GSZ 221   //llama fails on 220
    uint8_t imageBuf[GSZ];
    //    memset(imageBuf, 0, GSZ);
    int starts[] = {0, 4, 2, 1, 0};
    int incs[]   = {8, 8, 4, 2, 1};
    frameNo++;
#if GIFDEBUG > 1
    char buf[80];
    unsigned long filePositionBefore = filePositionCallback();
    if (frameNo == 1) {
        sprintf(buf, "Logical Screen [LZW=%d %dx%d P:0x%02X B:%d A:%d F:%dms] frames:%d pass=%d",
                lzwCodeSize, lsdWidth, lsdHeight, lsdPackedField, lsdBackgroundIndex, lsdAspectRatio,
                frameDelay * 10, frameCount, cycleNo);
        Serial.println(buf);
    }
#endif
#if GIFDEBUG > 2
    sprintf(buf, "Frame %2d: [=%6ld P:0x%02X B:%d F:%dms] @ %d,%d %dx%d ms:",
            frameNo, filePositionBefore, tbiPackedBits, transparentColorIndex, frameDelay * 10,
            tbiImageX, tbiImageY, tbiWidth, tbiHeight);
    Serial.print(buf);
    delay(10);    //allow Serial to complete @ 115200 baud
    int32_t t = millis();
#endif
    for (int state = 0; state < 4; state++) {
        if (tbiInterlaced == 0) state = 4; //regular does one pass
        for (int line = starts[state]; line < tbiHeight; line += incs[state]) {
            if (disposalMethod == DISPOSAL_BACKGROUND) memset(imageBuf, prevBackgroundIndex, maxGifWidth);
//            int align = (lsdWidth > maxGifWidth) ? lsdWidth - maxGifWidth : 0;
//            int ofs = tbiImageX - align;
//            uint8_t *dst = (ofs < 0) ? imageBuf : imageBuf + ofs;
//            align = (ofs < 0) ? -ofs : 0;
            int align = 0;
            int len = lzw_decode(imageBuf + tbiImageX, tbiWidth, imageBuf + maxGifWidth - 1, align);
            if (len != tbiWidth) Serial.println(len);
            int xofs = (disposalMethod == DISPOSAL_BACKGROUND) ? 0 : tbiImageX;
            int wid = (disposalMethod == DISPOSAL_BACKGROUND) ? lsdWidth : tbiWidth;
            int skip = (disposalMethod == DISPOSAL_BACKGROUND) ? -1 : transparentColorIndex;;
            if (true) {    //if (drawLineCallback) { 
                //(*drawLineCallback)(xofs, line + tbiImageY, imageBuf + xofs, wid, palette565, skip);
                //drawLineCallback(int16_t x, int16_t y, uint8_t *buf, int16_t w, uint16_t *palette, int16_t skip) 

                int16_t x = xofs;
                int16_t y = line + tbiImageY;
                uint8_t *buf = imageBuf + xofs;
                int16_t w = wid;
                uint16_t *palette = palette565;
                
                uint8_t pixel;
                bool first;
                if (y >= tft.height() || x >= tft.width() ) return;
                if (x + w > tft.width()) w = tft.width() - x;
                if (w <= 0) return;
                int16_t endx = x + w - 1;
                //uint8_t buf565[2*w];  //uint16_t buf565[w];
                uint16_t buf565[w];
                //uint8_t buflow;
                //uint8_t bufhigh;
                for (int i = 0; i < w; ) {
                    int n = 0;
                    while (i < w) {
                        pixel = buf[i++];
                        if (pixel == skip) break;
                        buf565[n++] = palette[pixel];
                    }
                    if (n) {
                         int startx = x + i - n;
                         int xlength = endx - startx;
                         for (int j = 0; j < xlength; j++){
                           //screenState[startx++][y] = buf565[j];
                           write_screen_buffer(startx++,y,buf565[j]);
                         }  
                    }
                }
                
            } else if (true) {  //else if (drawPixelCallback) {
                for (int x = 0; x < wid; x++) {
                    uint8_t pixel = imageBuf[x + xofs];
                    if ((pixel != skip))
                        //(*drawPixelCallback)(x + xofs, line + tbiImageY, palette[pixel].red, palette[pixel].green, palette[pixel].blue);
                        write_screen_buffer(x + xofs,line + tbiImageY,tft.color565(palette[pixel].red, palette[pixel].green, palette[pixel].blue));
                }
            }
        }
    }
    // LZW doesn't parse through all the data, manually set position
    fileSeekCallback(filePositionAfter);
#if GIFDEBUG > 2
    Serial.println(millis() - t);
#endif
#endif
    // Make animation frame visible
    // swapBuffers() call can take up to 1/framerate seconds to return (it waits until a buffer copy is complete)
    // note the time before calling

    // wait until time to display next frame
    while (nextFrameTime_ms > millis());

    // calculate time to display next frame
    nextFrameTime_ms = millis() + (10 * frameDelay);
    cycleTime += 10 * frameDelay;
    if (updateScreenCallback)
        (*updateScreenCallback)();
}


/*
 * Animated GIFs Display Code for SmartMatrix and 32x32 RGB LED Panels
 *
 * This file contains code to decompress the LZW encoded animated GIF data
 *
 * Written by: Craig A. Lindley, Fabrice Bellard and Steven A. Bennett
 * See my book, "Practical Image Processing in C", John Wiley & Sons, Inc.
 *
 * Copyright (c) 2014 Craig A. Lindley
 * Minor modifications by Louis Beaudoin (pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define LZWDEBUG 0

#if defined (ARDUINO)
#include <Arduino.h>
#elif defined (SPARK)
#include "application.h"
#endif

#include "GifDecoder.h"

template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::lzw_setTempBuffer(uint8_t * tempBuffer) {
    temp_buffer = tempBuffer;
}

// Initialize LZW decoder
//   csize initial code size in bits
//   buf input data
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::lzw_decode_init (int csize) {

    // Initialize read buffer variables
    bbuf = 0;
    bbits = 0;
    bs = 0;
    bcnt = 0;

    // Initialize decoder variables
    codesize = csize;
    cursize = codesize + 1;
    curmask = mask[cursize];
    top_slot = 1 << cursize;
    clear_code = 1 << codesize;
    end_code = clear_code + 1;
    slot = newcodes = clear_code + 2;
    oc = fc = -1;
    sp = stack;
}

// this writes to the class variable screen buffer
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
void GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::write_screen_buffer(int x, int y, uint16_t value) {
   screen_buffer[x][y] = value;
}

//this reads from the sclass variable screenbuffer
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::read_screen_buffer(int x, int y) {
   int return_value =  screen_buffer[x][y];
   return return_value;
}

//destructor
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::~GifDecoder() {
   //delete this;

}


//  Get one code of given number of bits from stream
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::lzw_get_code() {

    while (bbits < cursize) {
        if (bcnt == bs) {
            // get number of bytes in next block
            readIntoBuffer(temp_buffer, 1);
            bs = temp_buffer[0];
            readIntoBuffer(temp_buffer, bs);
            bcnt = 0;
        }
        bbuf |= temp_buffer[bcnt] << bbits;
        bbits += 8;
        bcnt++;
    }
    int c = bbuf;
    bbuf >>= cursize;
    bbits -= cursize;
    return c & curmask;
}

// Decode given number of bytes
//   buf 8 bit output buffer
//   len number of pixels to decode
//   returns the number of bytes decoded
// .kbv add optional number of pixels to skip i.e. align
template <int maxGifWidth, int maxGifHeight, int lzwMaxBits>
int GifDecoder<maxGifWidth, maxGifHeight, lzwMaxBits>::lzw_decode(uint8_t *buf, int len, uint8_t *bufend, int align) {
    int l, c, code;

#if LZWDEBUG == 1
    unsigned char debugMessagePrinted = 0;
#endif

    if (end_code < 0) {
        return 0;
    }
    l = len;

    for (;;) {
        while (sp > stack) {
            uint8_t q = *(--sp);     //.kbv pull off stack anyway
            // load buf with data if we're still within bounds
            if (align > 0) align--;
            else if (buf < bufend) {
                *buf++ = q;
            } else {
                // out of bounds, keep incrementing the pointers, but don't use the data
#if LZWDEBUG == 1
                // only print this message once per call to lzw_decode
                if (buf == bufend)
                    Serial.println("****** LZW imageData buffer overrun *******");
#endif
            }
            if ((--l) == 0) {
                return len;
            }
        }
        c = lzw_get_code();
        if (c == end_code) {
            break;

        }
        else if (c == clear_code) {
            cursize = codesize + 1;
            curmask = mask[cursize];
            slot = newcodes;
            top_slot = 1 << cursize;
            fc = oc = -1;

        }
        else    {

            code = c;
            if ((code == slot) && (fc >= 0)) {
                *sp++ = fc;
                code = oc;
            }
            else if (code >= slot) {
                break;
            }
            while (code >= newcodes) {
                *sp++ = suffix[code];
                code = prefix[code];
            }
            *sp++ = code;
            if ((slot < top_slot) && (oc >= 0)) {
                suffix[slot] = code;
                prefix[slot++] = oc;
            }
            fc = code;
            oc = c;
            if (slot >= top_slot) {
                if (cursize < lzwMaxBits) {
                    top_slot <<= 1;
                    curmask = mask[++cursize];
                } else {
#if LZWDEBUG == 1
                    if (!debugMessagePrinted) {
                        debugMessagePrinted = 1;
                        Serial.println("****** cursize >= lzwMaxBits *******");
                    }
#endif
                }

            }
        }
    }
    end_code = -1;
    return len - l;
}

//////////////////////////HOME PAGE FUNCTIONS//////////////////
bool fileSeekCallback_P(unsigned long position) {
    g_seek = position;
    return true;
}

unsigned long filePositionCallback_P(void) {
    return g_seek;
}

int fileReadCallback_P(void) {
    return pgm_read_byte(g_gif + g_seek++);
}

int fileReadBlockCallback_P(void * buffer, int numberOfBytes) {
    memcpy_P(buffer, g_gif + g_seek, numberOfBytes);
    g_seek += numberOfBytes;
    return numberOfBytes; //.kbv
}

void screenClearCallback(void) {
    //    tft.fillRect(0, 0, 128, 128, 0x0000);
}

//bool openGifFilenameByIndex_P(const char *dirname, int index)
//{
//    gif_detail_t *g = &gifs[index];
//    g_gif = g->data;
//    g_seek = 0;
//
//    Serial.print("Flash: ");
//    Serial.print(g->name);
//    Serial.print(" size: ");
//    Serial.println(g->sz);
//
//    return index < num_files;
//}
void updateScreenCallback(void) {
    ;
}

//void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
//    //tft.drawPixel(x, y, tft.color565(red, green, blue));
//    //screenState[x][y] = tft.color565(red, green, blue);
//    //bring back this one
//    //decoder.write_screen_buffer(x,y,tft.color565(red, green, blue));
//}
//
//void drawLineCallback(int16_t x, int16_t y, uint8_t *buf, int16_t w, uint16_t *palette, int16_t skip) {
////    uint8_t pixel;
////    bool first;
////    if (y >= tft.height() || x >= tft.width() ) return;
////    if (x + w > tft.width()) w = tft.width() - x;
////    if (w <= 0) return;
////    int16_t endx = x + w - 1;
////    //uint8_t buf565[2*w];  //uint16_t buf565[w];
////    uint16_t buf565[w];
////    //uint8_t buflow;
////    //uint8_t bufhigh;
////    for (int i = 0; i < w; ) {
////        int n = 0;
////        while (i < w) {
////            pixel = buf[i++];
////            if (pixel == skip) break;
////            buf565[n++] = palette[pixel];
////        }
////        if (n) {
////             int startx = x + i - n;
////             int xlength = endx - startx;
////             for (int j = 0; j < xlength; j++){
////               //screenState[startx++][y] = buf565[j];
////               decoder.write_screen_buffer(startx++,y,buf565[j]);
////             }  
////        }
////    }
//}

void playGIF(char* filename){
    GifDecoder<GIFWIDTH, 128, 12> decoder;
    Serial.print("Free heap during: ");
    Serial.println(ESP.getFreeHeap());
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    //decoder.setDrawPixelCallback(drawPixelCallback);
    //decoder.setDrawLineCallback(drawLineCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
  
    for(int x = 0; x < 128; x++){
        for(int y = 0; y < 128; y++){
          //screenState[x][y] = 0;
          decoder.write_screen_buffer(x ,y, 0);
        }
    }
    File binary_file;
    file = SD.open(filename);
    if(!file){
      Serial.println("File not found!");
      return;
    }

    // work with filename and open a new binary file
    String filename_holder = String(filename);
    int dot_index = filename_holder.indexOf(".");
    String filename_suffix = filename_holder.substring((dot_index+1));
    String filename_prefix = filename_holder.substring(0, dot_index);
    
    if(filename_suffix == "gif"){
      char filename_buffer[80];
      String binary_suffix = ".bvf";
      filename_prefix.concat(binary_suffix);
      String binary_filename = filename_prefix;
      if(SD.exists(binary_filename)){
        //return;
      }
      binary_filename.toCharArray(filename_buffer, sizeof(filename_buffer));
      binary_file = SD.open(filename_buffer, FILE_WRITE);
      Serial.println(filename_buffer);
    }else if(filename_suffix == "jpg"){
      return;  
    }else{
      return;
    }
    
    static unsigned long futureTime, cycle_start;
    int32_t frames = decoder.getFrameCount();
    Serial.print("Frames:  ");
    Serial.println(frames);
    static int index = -1;
    decoder.startDecoding();
    int decodeframe = 0;
    int GIF_position = 0;
    //To get this to work, C:\Users\Brad\Documents\Arduino\hardware\espressif\esp32\cores\esp32\main.cpp increased looptask from 8192 to 65536, essentially an allocated ram increase
    while(decoder.getCycleNo() == 1){
      decodeframe++;
      Serial.print(decodeframe);
      Serial.print("  ");
      decoder.decodeFrame();
      for(int y = 0; y < 128; y++){
        for(int x = 0; x < 128; x++){
//            binary_file.write(highByte(screenState[x][y]));
//            binary_file.write(lowByte(screenState[x][y]));
              binary_file.write(highByte(decoder.read_screen_buffer(x, y)));
              binary_file.write(lowByte(decoder.read_screen_buffer(x, y)));
        }
      }
    } 

    if(binary_file.size()){
      binary_file.close();
      Serial.println("Closing File! Size:");
      Serial.println(binary_file.size());
    }

}


 

