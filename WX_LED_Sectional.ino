
#define copyright "&#169; Jan 2021 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define compiledate __DATE__
/*
 *  Based from https://led-sectional.kyleharmon.com
 *  https://github.com/WKHarmon/led-sectional
 *
 */
/*
 * 		NOTE:
 * 		Due to memory (ram) fragmentation and available amount *DO NOT* use Strings. Anything that will not change put in Flash Prom see F() macro
 * 		and PROGMEM. Remember there is a Maximum of 4MB of SRAM and the WX Download needs 30,000 continuous bytes.
 *
 * 		Libraries needed are
 * 		 ESP8266mDNS
 * 		 ESP8266WebServer
 * 		 ESP8266WiFi
 * 		 FastLED
 * 		 SPI
 *
 * 		 I added EEPROM for debugging
 *
 */
#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "user_settings.h"
#include "WX_LED_Sectional.h"

const char* ssid = STASSID;
const char* password = STAPSK;
const char* hostname = MYHOSTNAME;

ESP8266WebServer server(80);
//ESP8266WiFiClass WiFic;

//const int led = 13;
#define NO_EVENT    0
#define MY_ID       1
#define MY_TEST     2
#define MY_WXUPDATE 3
byte my_Event = MY_TEST;
const unsigned int loop_interval = WX_REFRESH_INTERVAL * 60;    // How often to run WX update converted in seconds
unsigned int loop_time = loop_interval;                         // Force WX update first loop


#include "utilities.h"
#include "LEDString.h"
#include "notFoundPage.h"
#include "testPage.h"
#include "rootPage.h"
#include "stationPage.h"
#include "infoPage.h"
#include "setup.h"
#include "metars.h"



void setup(void) {
	delay(100);			// some boards are unstable
	setupBiLED();
	setupSerial();
	//readEeprom();
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

  if ( timeElapsed() ) {                  // Do every second
     switch(my_Event){
       case MY_ID:{
             idLED();
       }
       break;

       case MY_TEST:{
           // To prevent Software WD timeout test is called from loop 5 times
           if(testTime <= loop_time ){
             test();
             testTime = loop_time + 5;         // 5 seconds before next test
           }
       }
       break;

       case MY_WXUPDATE:{
    	   server.close();                     // Stop clients from locking up the 4 available connections
Serial.print(uptime());
showFree(true);
           loop_time = loopLEDSectional();     // Return allows loop time to be adjusted for WX GET failures
           server.begin();                     // Restart Service
           testTime = loop_time;               // When loop_time overflows the next test may be delayed
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
