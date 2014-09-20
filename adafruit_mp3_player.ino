/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/**
  Simple MP3 Player with basic commands.
  
  @author: Ulrich VACHON (2014)
*/
// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
// #define CLK             13     // SPI Clock, shared with SD card
// #define MISO            12     // Input data, from VS1053/SD card
// #define MOSI            11     // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI . 
// See http://arduino.cc/en/Reference/SPI "Connections"

// these are the pins used for the breakout example
#define BREAKOUT_RESET     9      // VS1053 reset pin (output)
#define BREAKOUT_CS        10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS       8      // VS1053 Data/command select pin (output)

// these are the pins used for the music maker shield
#define SHIELD_CS          7      // VS1053 chip select pin (output)
#define SHIELD_DCS         6      // VS1053 Data/command select pin (output)

// these are common pins between breakout and shield
#define CARDCS             4      // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ               3      // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

int volume = 20;

int fileIndex = 0;
int fileNumber = 0;

char* fileList[10];

// update or decrease volume if we get an 'u' or 'd' char on the serial console
void updateVolume(char command) {
  if ('u' == command) {
      volume--;
  } else if ('d' == command) {
    volume++;
  } else {
    return;
  }
  Serial.print("[DEBUG] - Volume=");
  Serial.println(volume);

  musicPlayer.setVolume(volume, volume);
}

// stop or pause read if we get an 's' or 'p' char on the serial console
void updateState(char command) {
  if (('b' == command) && (fileIndex > 0)) {
    musicPlayer.startPlayingFile(fileList[--fileIndex]);
    Serial.print("[DEBUG] - Back command, fileIndex=");
    Serial.println(fileIndex);
    Serial.print("[DEBUG] - Reading=");
    Serial.println(fileList[fileIndex]);
  }
  if (('f' == command) && (fileIndex < fileNumber - 1)) {
    musicPlayer.startPlayingFile(fileList[++fileIndex]);
    Serial.print("[DEBUG] - Forward command, fileIndex=");
    Serial.println(fileIndex);
    Serial.print("[DEBUG] - Reading=");
    Serial.println(fileList[fileIndex]);
  }
  if ('s' == command) {
    musicPlayer.stopPlaying();
    Serial.println("[DEBUG] - Stopped");
  }
  if ('p' == command) {
    if (!musicPlayer.paused()) {
      musicPlayer.pausePlaying(true);
      Serial.println("[DEBUG] - Paused");
    } else {
      musicPlayer.pausePlaying(false);
      Serial.println("[DEBUG] - Resumed");
    }
  }
}

void setup() {
  Serial.begin(9600);

  Serial.println("[DEBUG] - MP3 files Player based on Adafruit VS1053 shield");

  if (!musicPlayer.begin()) {
     Serial.println(F("[ERROR] - Couldn't find VS1053, do you have the right pins defined?"));
     while(1);
  }
  Serial.println(F("[DEBUG] - VS1053 found"));

  // initialise the SD card
  SD.begin(CARDCS);

  // timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // if DREQ is on an interrupt pin (on uno, #2 or #3) we can do background audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  // set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);

  createFileList();

  Serial.print("[DEBUG] - Number of file=");
  Serial.println(fileNumber);

  if (!fileList[fileIndex]) {
     Serial.println(F("[DEBUG] - File not found in directory"));
     while(1);
  }
  Serial.print("[DEBUG] - Reading=");
  Serial.println(fileList[fileIndex]);

  musicPlayer.startPlayingFile(fileList[fileIndex]);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    updateVolume(c);

    updateState(c);
  }
  delay(100);
}

// file listing helper
void createFileList() {
  File root = SD.open("/");

  for (int i = 0; i < 100; i++) {
    File entry = root.openNextFile();

    if (!entry) {
      break;
    }
    fileList[i] = strdup(entry.name());

    Serial.print("[DEBUG] - Found file=");
    Serial.println(fileList[i]);

    fileNumber++;

    entry.close();
  }
}

