/*
  DMX RGB Controller with Strobe
  Created by Rory McMahon
  SPI RGB Strip controller
  Conceptinetics DMX Shield
*/
  
#include <Conceptinetics.h>  
#include "RGBdriver.h"

//
// The slave device will use a block of 10 channels counting from
// its start address.
//
// If the start address is for example 56, then the channels kept
// by the dmx_slave object is channel 56-66
//
#define DMX_SLAVE_CHANNELS   10 

//
// Pin number to change read or write mode on the shield
// Uncomment the following line if you choose to control 
// read and write via a pin
//
// On the CTC-DRA-13-1 shield this will always be pin 2,
// if you are using other shields you should look it up 
// yourself
//
///// #define RXEN_PIN                2


// Configure a DMX slave controller
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS );

// If you are using an IO pin to control the shields RXEN
// the use the following line instead
///// DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS , RXEN_PIN );

// The DMX start channel for this device
const int dmxaddr = 1;

// Define the pins for each colour (PWM pins 2-7)
const int redPin = 2;
const int greenPin = 3;
const int bluePin = 4;

// Define the DMX channels for each colour intensity
const int redDMXch = 1;
const int greenDMXch = 2;
const int blueDMXch = 3;
const int intensityDMXch = 4;
const int strobeDMXch = 5;

// Default levels for each colour
volatile int redLevel = 0;
volatile int greenLevel = 0;
volatile int blueLevel = 0;

// Defaults for Strobe function
int strobeOn = 0;
unsigned long nextStrobe = millis();

//pins definitions for RGB the driver
#define CLK 3        
#define DIO 2
RGBdriver Driver(CLK,DIO);
void setup()  
{
  // Enable DMX slave interface and start recording DMX data
  dmx_slave.enable();
  
  // Set the DMX start address
  dmx_slave.setStartAddress(dmxaddr);
  
  // Set the PWM pins to output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

// Loop function
void loop()  
{ 
  //Function: SetColor();
  //Parameters: red:0~255
  //            green:0~255
  //            blue:0~255
  
  // If the over all intensity is greater than 0
  if (dmx_slave.getChannelValue(intensityDMXch) > 0) {
    // Set the global intensity
    int globalIntensity = dmx_slave.getChannelValue(intensityDMXch);

    // Move the previous levels to another param
    int prevRedLevel = redLevel;
    int prevGreenLevel = greenLevel;
    int prevBlueLevel = blueLevel;
    
    // Get the levels for each colour
    redLevel = dmx_slave.getChannelValue(redDMXch);
    greenLevel = dmx_slave.getChannelValue(greenDMXch);
    blueLevel = dmx_slave.getChannelValue(blueDMXch);
    
    // Adjust the intensities per the global
    if (redLevel > globalIntensity) {
      redLevel = globalIntensity;
    }
    if (greenLevel > globalIntensity) {
      greenLevel = globalIntensity;
    }
    if (blueLevel > globalIntensity) {
      blueLevel = globalIntensity;
    }
    
    // If strobe is active
    if (dmx_slave.getChannelValue(strobeDMXch) > 0) {
      // Set the strobe rate
      unsigned long strobeRate = dmx_slave.getChannelValue(strobeDMXch);
      
      // If the strobe is in the ON position
      if (strobeOn > 0) {
        if ((unsigned long) (millis() - nextStrobe) >= strobeRate) {
          // begin control of the RGB strip
          Driver.begin();
          // Set the colour values
          Driver.SetColor(0, 0, 0);
          Driver.end();
          nextStrobe = millis() + strobeRate;
          strobeOn = 0;
        }
      }
      
      // If the strobe is in the OFF position
      if (strobeOn < 1) {
        if ((unsigned long) (millis() - nextStrobe) >= strobeRate) {
          // begin control of the RGB strip
          Driver.begin();
          // Set the colour values
          Driver.SetColor(redLevel, greenLevel, blueLevel);
          Driver.end();
          nextStrobe = millis() + strobeRate;
          strobeOn = 0;
        }
      }
    }else{
      // Standard fade functions
      
      // Check if the red level has changed
      if ((redLevel != prevRedLevel) || (greenLevel != prevGreenLevel) || (blueLevel != blueLevel)) {
        // set the red level if it's changed
        Driver.begin();
        Driver.SetColor(redLevel, greenLevel, blueLevel);
        Driver.end();
      }
    }
  }
}
