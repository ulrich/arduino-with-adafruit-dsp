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
// Connect CLK, MISO and MOSI to hardware SPI pins. 
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

// create breakout-example object!
// Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

int volume = 20;

void logger(char message[]) {
  Serial.println(F(""));
}

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  // initialise the music player
  if (!musicPlayer.begin()) {
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));

  // initialise the SD card
  SD.begin(CARDCS);
  
  // set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);

  // timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // if DREQ is on an interrupt pin (on uno, #2 or #3) we can do background audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  // play one file, don't return until complete
  //Serial.println(F("Playing track 001"));
  //musicPlayer.playFullFile("track1.mp3");

  printDirectory(SD.open("/"), 0);

  // play another file in the background, REQUIRES interrupts!
  Serial.println(F("Playing track1.mp3 without blocking"));
  musicPlayer.startPlayingFile("TRACK2.MP3");
  Serial.println(F("Not blocking"));
}

// update or decrease volume if we get an 'u' or 'd' char on the serial console, 
void updateVolume(char command) {
  if ('u' == command) {
      volume--;
  } else if ('d' == command) {
    volume++;
  } else {
    return;
  }
  Serial.print("Volume=");
  Serial.println(volume);

  musicPlayer.setVolume(volume, volume);
}

// stop or pause read if we get an 's' or 'p' char on the serial console, 
void updateState(char command) {
  if ('s' == command) {
    musicPlayer.stopPlaying();
  }
  if ('p' == command) {
    if (!musicPlayer.paused()) {
      Serial.println("Paused");
      musicPlayer.pausePlaying(true);
    } else { 
      Serial.println("Resumed");
      musicPlayer.pausePlaying(false);
    }
  }
}

void loop() {
  // File is playing in the background
  if (musicPlayer.stopped()) {
    Serial.println("Done playing music");
    while (1);
  }
  if (Serial.available()) {
    char c = Serial.read();

    updateVolume(c);

    updateState(c);
  }
  delay(100);
}

/// file listing helper
void printDirectory(File dir, int numTabs) {
  char tracks[100];

  while(true) {
    File entry = dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs+1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

