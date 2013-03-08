/*
 * mood.pde:
 *      Gordon Henderson, February 2013, <projects@drogon.net>
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

#include <math.h>
#include <genieArduino.h>

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#ifndef	TRUE
#define	TRUE	(1==1)
#define	FALSE	(!TRUE)
#endif

#define	RED_LED		 9
#define	GREEN_LED	10
#define	BLUE_LED	11


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

  /**/ if (value == 0)
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

  genieWriteObj (GENIE_OBJ_SLIDER, 0, 0) ;
  genieWriteObj (GENIE_OBJ_SLIDER, 1, 0) ;
  genieWriteObj (GENIE_OBJ_SLIDER, 2, 0) ;
}




/*
 * handleGenieEvent:
 *	Take a reply off the display and call the appropriate handler for it.
 *********************************************************************************
 */

void handleGenieEvent (struct genieReplyStruct *reply)
{
  int r, g, b ;

  if (reply->cmd != GENIE_REPORT_EVENT)
  {
    digitalWrite (13, 1) ;
    return ;
  }

  /**/ if (reply->object == GENIE_OBJ_WINBUTTON)
  {
    /**/ if (reply->index == 0)		// Button 0 - all Max
    {
      setLEDs (255, 255, 255) ;

      genieWriteObj (GENIE_OBJ_SLIDER, 0, 255) ;
      genieWriteObj (GENIE_OBJ_SLIDER, 1, 255) ;
      genieWriteObj (GENIE_OBJ_SLIDER, 2, 255) ;
    }
    else if (reply->index == 1)		// Button 1 is all off
      resetDisplay () ;
    else if (reply->index == 2)		// Button 2 is random
    {
      r = random () % 256 ;
      g = random () % 256 ;
      b = random () % 256 ;
      setLEDs (r, g, b) ;

      genieWriteObj (GENIE_OBJ_SLIDER, 0, r) ;
      genieWriteObj (GENIE_OBJ_SLIDER, 1, g) ;
      genieWriteObj (GENIE_OBJ_SLIDER, 2, b) ;
    }
    else
      digitalWrite (13, 1) ;
  }
  else if (reply->object == GENIE_OBJ_SLIDER)
  {
    /**/ if (reply->index == 0)
      analogWrite (RED_LED,   logLed (reply->data)) ;
    else if (reply->index == 1)
      analogWrite (GREEN_LED, logLed (reply->data)) ;
    else if (reply->index == 2)
      analogWrite (BLUE_LED,  logLed (reply->data)) ;
    else
      digitalWrite (13, 1) ;
  }
  else
    digitalWrite (13, 1) ;
}


/*
 *********************************************************************************
 * main:
 *	Run our little demo
 *********************************************************************************
 */

void setup ()
{
  pinMode      (RED_LED,   OUTPUT) ;
  pinMode      (GREEN_LED, OUTPUT) ;
  pinMode      (BLUE_LED,  OUTPUT) ;

  pinMode      (13, OUTPUT) ;
  digitalWrite (13, 0) ;

  genieSetup   (115200) ;
}


void loop (void)
{
  int i ;
  struct genieReplyStruct reply ;

// Select form 0 

  genieWriteObj (GENIE_OBJ_FORM, 0, 0) ;
  resetDisplay () ;

/* Test
  for (;;)
  {
    for (i = 0 ; i < 256 ; ++i)
    {
      analogWrite (RED_LED,   logLed (i)) ;
      analogWrite (GREEN_LED, logLed (i)) ;
      analogWrite (BLUE_LED,  logLed (i)) ;
      delay (10) ;
    }
    delay (1000) ;
  }
*/

// Big loop - just wait for events from the display now

  for (;;)
  {
    genieGetReply    (&reply) ;
    handleGenieEvent (&reply) ;
  }
}
