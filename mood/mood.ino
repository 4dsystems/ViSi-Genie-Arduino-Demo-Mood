/*
 * mood.ino:
 *      Original Version by Gordon Henderson, February 2013, <projects@drogon.net>
 *      Copyright (c) 2012-2013 4D Systems PTY Ltd, Sydney, Australia
 ***********************************************************************
 * This file is part of genieArduino:
 *    genieArduino is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    genieArduino is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with genieArduino.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

// UPDATED 16/07/2013 
// This demo controls a tri-coloured LED along with displaying images from the SD card on the display in a slide show.
// Simple demo of 3 sliders and a few push buttons communicating to the Arduino. Features the display of some photos 
// which is done internally of the display configured via the Workshop4 IDE Software.   

#include <math.h>
#include <genieArduino.h>	// MODIFIED new genieArduino library

#define	RED_LED		 9      // RED LED connected to Pin D9
#define	GREEN_LED	10      // GREEN LED connected to Pin D10
#define	BLUE_LED	11      // BLUE LED connected to Pin D11

/*
 *********************************************************************************
 * main:
 *	Run our little demo
 *********************************************************************************
 */

void setup ()
{
  pinMode      (RED_LED,   OUTPUT);
  pinMode      (GREEN_LED, OUTPUT);
  pinMode      (BLUE_LED,  OUTPUT);

  pinMode      (13, OUTPUT);  // For debug purposes, LED on D13 is set if something 'wrong' happens
  digitalWrite (13, 0); // Turn off LED to start with

// This code modifies the code to suit various boards this will be run on.
// For a Uno or Duemilanove whith just 1 serial port (Serial0), debug is not possible. Display is connected to Serial0
// For other boards with more than 1 serial port, debug is on Serial0 and the display is connected to Serial1
// Modify to suit, debug prints are scattered throughout the code which will need to be changed if required.

#define DEBUGCOM
#if defined(__AVR_ATmega328P__)
#undef DEBUGCOM
  genieBegin (GENIE_SERIAL, 115200);  	// 4D display on Serial so no simultaneous console debugging 
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  Serial.begin (9600);					// Serial used for debugging console
  genieBegin (GENIE_SERIAL_1, 115200);	// 4D display on Serial1 (can change to _2 or _3 for other ports)
#endif

#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644P__)
  Serial.begin (9600);					// Serial used for debugging console
  genieBegin (GENIE_SERIAL_1, 115200);	// 4D display on Serial1
#endif

#if defined(__AVR_ATmega32U4__) 		// Leonardo
  Serial.begin (9600);					// Serial used for debugging console
  while(!Serial);						// Wait for Serial to initialize since it is done via USB and virtual COM port
  genieBegin (GENIE_SERIAL_1, 115200);	// 4D display on Serial1
#endif

  genieAttachEventHandler(handleGenieEvent);  // Attach the function handleGenieEvent() to the library, which is called by the library

  delay(3000);				      // Let display settle

  genieWriteObject (GENIE_OBJ_FORM, 0, 0) ;   // Select Form0 of the Display
  resetDisplay () ;                           // 'Reset' the display by setting everything on Form0 to 0
}


void loop (void)
{
  // Big loop - just wait for events from the display now

  genieDoEvents();
  
  // Do other things here if required
}

/*
 * logLed:
 *      Simple attempt to make the scale logarithmic - stops the LEDs
 *      becoming too bright too soon.
 *********************************************************************************
 */

static int logLed (int value)
{
  int result ;
  static double lim = exp (5) ;

  /**/  if (value == 0)
    result =   0 ;
  else if (value == 255)
    result = 255 ;
  else
    result = (int)(exp ((double)value * 5.0 / 255.0) / lim * 255.0) ;

  return result ;
}


/*
 * setLEDs:
 *      Set our PWM LEDs and reflect the slider values too
 *********************************************************************************
 */

static void setLEDs (int r, int g, int b)
{
  analogWrite (RED_LED,   logLed (r)) ;
  analogWrite (GREEN_LED, logLed (g)) ;
  analogWrite (BLUE_LED,  logLed (b)) ;
}


/*
 * resetDisplay:
 *********************************************************************************
 */

static void resetDisplay (void)
{
  setLEDs (0, 0, 0) ;

  genieWriteObject (GENIE_OBJ_SLIDER, 0, 0) ;
  genieWriteObject (GENIE_OBJ_SLIDER, 1, 0) ;
  genieWriteObject (GENIE_OBJ_SLIDER, 2, 0) ;
}


/*
 * handleGenieEvent:
 *	Take a reply off the display and call the appropriate handler for it.
 *********************************************************************************
 */

void handleGenieEvent (void)
{
  int r, g, b ;
  genieFrame reply;
  int slider_val = 0;

  genieDequeueEvent(&reply); // Remove this event from the queue

  if (reply.reportObject.cmd != GENIE_REPORT_EVENT) // If this event is NOT a Reported Message
  {
    digitalWrite (13, 1) ; // Set the LED to show a different message has been received (invalid to this demo)
    return ;
  }

  if (reply.reportObject.object == GENIE_OBJ_WINBUTTON) // If this event is from a WinButton
  {
    if (reply.reportObject.index == 0)		        // Button 0 - all Max
    {
#ifdef	DEBUGCOM
      Serial.println("Button 0 pushed - all MAX");
#endif

      setLEDs (255, 255, 255) ;

      genieWriteObject (GENIE_OBJ_SLIDER, 0, 255) ;
      genieWriteObject (GENIE_OBJ_SLIDER, 1, 255) ;
      genieWriteObject (GENIE_OBJ_SLIDER, 2, 255) ;
    }
    else if (reply.reportObject.index == 1) 		// Button 1 is all off
    {  
#ifdef	DEBUGCOM
      Serial.println("Button 1 pushed - all OFF");
#endif
      resetDisplay () ;
    }
    else if (reply.reportObject.index == 2)		// Button 2 is random
    {
#ifdef	DEBUGCOM
      Serial.println("Button 2 pushed - RANDOM");
#endif

      r = random () % 256 ;
      g = random () % 256 ;
      b = random () % 256 ;
      setLEDs (r, g, b) ;

      genieWriteObject (GENIE_OBJ_SLIDER, 0, r) ;
      genieWriteObject (GENIE_OBJ_SLIDER, 1, g) ;
      genieWriteObject (GENIE_OBJ_SLIDER, 2, b) ;
    }
    else
      digitalWrite (13, 1) ; // If from another button, set the LED as its invalid for this demo
  }
  else if (reply.reportObject.object == GENIE_OBJ_SLIDER)  // If this event is from a Slider
  {
#ifdef	DEBUGCOM
    Serial.print("Slider ");
    Serial.print(reply.reportObject.index,DEC);
    Serial.print(" changed with ");
    Serial.println(reply.reportObject.data_lsb,DEC);
#endif

    if (reply.reportObject.index == 0)                     // Slider 0
      analogWrite (RED_LED,   logLed (reply.reportObject.data_lsb)) ;
    else if (reply.reportObject.index == 1)                // Slider 1
      analogWrite (GREEN_LED, logLed (reply.reportObject.data_lsb)) ;
    else if (reply.reportObject.index == 2)                // Slider 2
      analogWrite (BLUE_LED,  logLed (reply.reportObject.data_lsb)) ;
    else
      digitalWrite (13, 1) ; // If from another slider, set the LED as its invalid for this demo
  }
  else
    digitalWrite (13, 1) ;  // If from another object type, set the LED as its invalid for this demo
}


