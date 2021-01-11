//AD9833 generator + some timing control 
//for playing basic monophonic melodies

#include <MD_AD9833.h>
#include <SPI.h>
#include "notes.h"

// Pins for SPI comm with the AD9833 IC
#define FSYNC 10  ///< SPI Load pin number (FSYNC in AD9833 usage)
#define CLK   13  ///< SPI Clock pin number
#define DATA  11  ///< SPI Data pin number

MD_AD9833 AD(FSYNC);  // Hardware SPI

//setup our BPM for playback speed
int tempo = 180;

//our melody array -- each entry contains note (pitch) and it's length
const int melody[] = {
  NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,8, //1
  NOTE_G5,4, REST,4, NOTE_G4,8, REST,4, 
  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 3
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,
  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4,
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,
};

void playNote(int note, int duration){
  AD.setFrequency(MD_AD9833::CHAN_0, note);
  delay(duration*0.8);
  AD.setFrequency(MD_AD9833::CHAN_0, 0);
  delay(duration*0.2);
}

void setup() {

  //Initialize AD generator
  AD.begin();
  AD.setFrequency(MD_AD9833::CHAN_0, 0);
  AD.setActiveFrequency(MD_AD9833::CHAN_0);
  AD.setMode(MD_AD9833::MODE_SQUARE1);

  //each note contains two entries
  int notesCnt = (sizeof(melody) / sizeof(melody[0]));
  
  //in this simple case let's say we're playing at 4/4,
  //thus whole note length is BPM -> whole note duration in ms
  int wholeNote = (60000 * 4) / tempo;

  //play our recording
  for(int i=0; i<notesCnt; i+=2){
    int divider = melody[i+1];
    int duration = 0;
    if(divider > 0){
      duration = wholeNote / divider;
    }else if(divider < 0){
      duration = wholeNote / abs(divider);
      duration *= 1.5;
    }

    playNote(melody[i], duration);
  }

  //silence on end
  playNote(REST, 0);
}

void loop() {
  //nothing to do after playing tones
}
