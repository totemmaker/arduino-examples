// AD9833 single tone generator example
//
// Initialises the device to default conditions,
// outputing a single sine wave

#include <MD_AD9833.h>
#include <SPI.h>

// Pins for SPI comm with the AD9833 IC
#define FSYNC 10  // SPI Load pin number (FSYNC in AD9833 usage)
#define CLK   13  // SPI Clock pin number
#define DATA  11	// SPI Data pin number

MD_AD9833	AD(FSYNC);  // Hardware SPI

void setup(void)
{

  //Initialize AD generator
	AD.begin();

  //Set output
  AD.setFrequency(MD_AD9833::CHAN_0, 440);
  AD.setActiveFrequency(MD_AD9833::CHAN_0);
  AD.setMode(MD_AD9833::MODE_SINE);
}

void loop(void)
{
  //nothing to do here
}
