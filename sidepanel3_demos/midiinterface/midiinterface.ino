//Rudimentary synth, accepting serial MIDI data in, and 
//generating resulting square wave on AD9833 output.
//Has the ability to work in both directions.
//Needs an accompanying software on a computer side for midi routing, see README

#include <MD_AD9833.h>
#include <SPI.h>

// Pins for SPI comm with the AD9833 IC
#define FSYNC 10  ///< SPI Load pin number (FSYNC in AD9833 usage)
#define CLK   13  ///< SPI Clock pin number
#define DATA  11  ///< SPI Data pin number

MD_AD9833 AD(FSYNC);  // Hardware SPI

//generate frequency from note
void playNote(int note){
  //generate silence if note is 0
  if(note == 0){
    AD.setFrequency(MD_AD9833::CHAN_0, 0);
    return;
  }

  //the frequency calculation formula is taken from
  //https://en.wikipedia.org/wiki/MIDI_tuning_standard 
  int freq = 440 * pow (2.0, ((float)(note-69)/12.0));
  AD.setFrequency(MD_AD9833::CHAN_0, freq);
}

//send a note up a midi channel
void sendNote(int channel, int note, int velocity){
  Serial.write(channel);
  Serial.write(note);
  Serial.write(velocity);
}

void setup() {

  Serial.begin(115200);

  //Initialize AD generator
  AD.begin();
  AD.setFrequency(MD_AD9833::CHAN_0, 0);
  AD.setActiveFrequency(MD_AD9833::CHAN_0);
  AD.setMode(MD_AD9833::MODE_SQUARE1);


  //generate startup sound
  delay(300);
  playNote(60);
  sendNote(144, 60, 50);  
  delay(300);
  playNote(0);
  sendNote(144, 64, 50);

  //send note off command
  sendNote(128, 60, 50);
  sendNote(128, 64, 50);
}

void loop() {

  //midi chunks consists of three bytes in a row.
  if(Serial.available() >= 3){
    int channelData = Serial.read();
    int pitch = Serial.read() ;
    int velocity = Serial.read();

    //channel and note are packed in first byte
    int channel = channelData & 0x0F;
    int mode = channelData & 0xF0;
    if(mode == 0x80){
      playNote(0);
      return;
    }
    //note is in the second byte
    //we discard velocity as we don't have any modulation abilities here
    playNote(pitch);
  }
}
