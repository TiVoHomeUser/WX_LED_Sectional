
#define copyright "&#169; Sep 2025 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define compiledate __DATE__
/*
 * 	https://github.com/TiVoHomeUser/WX_LED_Sectional
 * 	Original inspired from https://led-sectional.kyleharmon.com  https://github.com/WKHarmon/led-sectional
 *
 */
 
/*
 * 		NOTE:
 * 		Due to memory (ram) fragmentation and available amount *DO NOT* use Strings. Anything that will not change put in Flash Prom see F() macro
 * 		and PROGMEM. Remember there is a Maximum of 4MB of SRAM and the WX Download needs 30,000 continuous bytes.
 *
 * 		Libraries needed are
 * 		 ESP8266mDNS 		(part of the esp8266 core for Arduino environment)
 * 		 ESP8266WebServer 	(bundled with the version of Arduino IDE)
 * 		 ESP8266WiFi		(Board Manager. Type "ESP8266" in the text box to search and install the ESP8266 software for Arduino IDE.)
 * 		  - FastLED by Daniel Garica
 *
 *      Optional:
 *    	 for autoconnect
 *    	  - WiFiManager by tzapu,tablatronix
 *    
 *   	 for TSL2561 light sensor
 *    	  - Adafruit TSL2561 by Adafruit
 *
 */
/*
 * 2024/06/12
 * 		Removed months and seconds from Up time display rootpage.h and utilites.h
 * 		added error count display on rootpage when there are connection error(s)
 */
#include "Arduino.h"

#include "user_settings.h"

#include "WX_LED_Sectional.h"

const char* ssid = STASSID;
const char* password = STAPSK;
const char* hostname = MYHOSTNAME;

ESP8266WebServer server(80);

#define NO_EVENT    0
#define MY_ID       1
#define MY_TEST     2
#define MY_WXUPDATE 3
#define MY_LED_REFRESH	4

byte my_Event = MY_TEST;
const unsigned int loop_interval = WX_REFRESH_INTERVAL * 60;    // How often to run WX update converted in seconds
unsigned int loop_time = loop_interval;                         // Force WX update first loop
// Not really 'c' header files break up this .ino file into smaller sections still accessible by the Arduino IDE
#include "utilities.h"

#if INFO_PAGE
	#include "infoPage.h"
#endif

#include "LEDString.h"
#include "notFoundPage.h"
#include "testPage.h"
#include "rootPage.h"
#include "stationPage.h"
#include "setup.h"
#include "metars.h"



void setup(void) {
	delay(100);			// some boards are unstable
	setupBuiltInLED();
	setupSerial();
	setupBigBlock();
	setupAirportString();
	setupConnection();
	setupmDNS();
	setupServer();
	setupLEDString();
}

/*
 *
 * 								Main Loop
 *
 */

void loop(void) {
  static unsigned int testTime = loop_time;    // delay for test() leds
  server.handleClient();
  MDNS.update();

  if ( timeElapsed() ) {                  	// Do every second
     switch(my_Event){
       case MY_ID:{							// LED ID call to flash LED on or off map if from console
             idLED();
       }
       break;

       case MY_TEST:{						// Cycle colors all LEDs
#if WX_DEBUG
    	   loop_time = loop_interval - 5;	// force update
    	   my_Event = NO_EVENT;
#else
           // To prevent Software WD timeout test is called from loop 5 times
           if(testTime <= loop_time ){
             test();
             testTime = loop_time + 5;         // 5 seconds before next test
           }
#endif
       }
       break;

       case MY_WXUPDATE:{
    	   server.close();                     // Stop clients from locking up the 4 available connections
         loop_time = loopLEDSectional();     // Return allows loop time to be adjusted for WX GET failures
         testTime = loop_time;               // When loop_time overflows the next test may be delayed
         my_Event = MY_LED_REFRESH;
         server.begin();                     // Restart Service
       }
       break;

       case MY_LED_REFRESH:{
    	   FastLED.show();
    	   my_Event = NO_EVENT;
       }
       break;

       default: {
           toggleBuiltInLED();
           adjustBrightness();                 // Call for light sensor
           if (DO_LIGHTNING && lightningLeds.size() > 0) {
             loop_time++;                      // cuts refresh time between WX updates in half in bad weather
             doLighting();
           }
           if (loop_time > loop_interval){     // In default: to prevent conflicts with ID and Test
             my_Event = MY_WXUPDATE;
           }
       }

     } // switch(my_Event)
     loop_time++;   // count the seconds ( 1 loop = about 1 second )
  }
  // Test for serial data entry. So far only the Flash LED event
  if (Serial.available() > 0) {
	  idLED(getCommand()); // process any input from terminal
  }
}
