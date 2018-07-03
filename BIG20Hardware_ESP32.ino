// please read credits at the bottom of file

//#include <SD.h>
#ifndef min
#define min(a, b) (((a) <= (b)) ? (a) : (b))
#endif
#include "GifDecoder.h"
//#include "FilenameFunctions.h"    //defines USE_SPIFFS

#define DISPLAY_TIME_SECONDS 80
#define GIFWIDTH 128 //228 fails on COW_PAINT
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library 
File file;

#define TFT_CS     32
#define TFT_RST    27  // you can also connect this to the Arduino reset
#define TFT_DC     33
#define SD_CS 22
#define SPI_SPEED 70000000 //70M stable
#define MODE_BUTTON 32
#define BOARD_POWER 14

int length_counter = 0;

#if defined(USE_SPIFFS)
#define GIF_DIRECTORY "/"     //ESP8266 SPIFFS
#define DISKCOLOUR   CYAN
#else
#define GIF_DIRECTORY "/"
//#define GIF_DIRECTORY "/gifs32"
//#define GIF_DIRECTORY "/gifsdbg"
#define DISKCOLOUR   BLUE
#endif

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

//#include <TFT_eSPI.h>
//TFT_eSPI tft;
//#define TFTBEGIN() tft.begin()
#define PUSHCOLOR(x) tft.pushColor(x)

SPIClass SPI2(HSPI);

File root;

char* defaultFiles[] = {
"/cleric8.bvf", "/rogue6.bvf", "/mage13.bvf", "/fighter17.bvf", "/fire20.bvf"
};

//char* defaultFiles[] = {
//"/cleric8.bvf"
//};

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

int num_files;
const uint8_t *g_gif;
uint32_t g_seek;

// Setup method runs once, when the sketch starts
void setup() {
    pinMode(21, OUTPUT);
    digitalWrite(21, LOW);
    Serial.begin(115200);
    Serial.println("AnimatedGIFs_SD");
     if (!SD.begin(22, SPI2, 1000000000)) {
        Serial.println("No SD card");
    }
    else {
        Serial.println("SD inititaliazed");
        root = SD.open("/");
        printDirectory(root, 0);
    }


    tft.initR(INITR_144GREENTAB);
    tft.fillScreen(BLACK);

    for(int t = 0 ; t < sizeof(defaultFiles)/sizeof(int); t++){
      Serial.println("Checking files");
      if(!SD.exists(defaultFiles[t])){
        Serial.print("Converting: ");
        Serial.println(defaultFiles[t]);
        char filename_buffer[80];
        String filename = String(defaultFiles[t]);
        int dot_index = filename.indexOf(".");
        String filename_suffix = filename.substring((dot_index+1));
        String filename_prefix = filename.substring(0, dot_index);
        String gif_suffix = ".gif";
        filename_prefix.concat(gif_suffix);
        filename_prefix.toCharArray(filename_buffer, sizeof(filename_buffer));
        playGIF(filename_buffer);
      }  
    }
    Serial.println(" ");
    Serial.print("Free heap after: ");
    Serial.println(ESP.getFreeHeap());
}


void loop() {
  Serial.println("playing raw files!");
  
  rawFullSPI("/rogue6.bvf");
//
  rawFullSPI("/fire20.bvf");
//
  rawFullSPI("/fighter17.bvf");
//
  rawFullSPI("/cleric8.bvf");
//
  rawFullSPI("/mage13.bvf");
}


