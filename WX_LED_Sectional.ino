/*
 *
 *      TODO select and test LED pins and Dimmer pins
 *          AutoConnect SSID correct missing WL_WRONG_PASSWORD | WL_NO_SSID_AVAIL was it really needed?
 */
#define copyright "&#169; March 2026 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define compiledate __DATE__
#define VERSION "RC1_2.0"
/*
 * 	https://github.com/TiVoHomeUser/WX_LED_Sectional
 * 	Original inspired from https://led-sectional.kyleharmon.com  https://github.com/WKHarmon/led-sectional
 *
 */
 
/*
 * 		NOTE:
 * 		Due to memory (ram) fragmentation and available amount *DO NOT* use Strings. Anything that will not change put in Flash Prom see F() macro
 * 		and PROGMEM. Remember there is a Maximum of 4MB of SRAM and the WX Download needs 30,000 continuous bytes.
 *    Boards Manager 
 *      ESP8266 boards
 *        esp8266 by ESP8266 Community Max version 2.7.4
 *      ESP32
 *        esp32 by Espressif Systems   version 3.3.7
 *
 * 		Libraries
 *      ESP8266
 * 		    FastLED by Daniel Garcia     Max version 3.7.0
 *      ESP32
 * 		    FastLED by Daniel Garcia     version 3.10.3
 *
 * 		Both
 * 		  ESP8266mDNS 		(part of the esp8266 core for Arduino environment)
 * 		  ESP8266WebServer 	(bundled with the version of Arduino IDE)
 * 		  ESP8266WiFi		(Board Manager. Type "ESP8266" in the text box to search and install the ESP8266 software for Arduino IDE.)
 *
 *    Optional:
 *      for autoconnect
 *    	  - WiFiManager by tzapu,tablatronix
 *    
 *      for TSL2561 light sensor
 *    	  - Adafruit TSL2561 by Adafruit
 *
 */
/*
 * 2024/06/12
 *      Removed months and seconds from Up time display rootpage.h and utilities.h
 *      added error count display on rootpage when there are connection error(s)
 *
 * 2025/10/0
 *      Soft boot using "Magic Number" to skip LED test
 *
 * 2026/02/21
 *      BearSSL::WiFiClientSecure client changed to static to prevent memory fragmentation
 *
 * 2026/02/28
 *      Removed all vector types (lightningLeds), Refresh now static WXClient refresh every few hours,
 *      Softboot for heap fragmentation
 *
 * 2026/03/12
 *      Addition of connect totals
 *      Clean up removing not needed comments and removed DEBUG DEBUG1 compile time configurations
 * 
 * 2026/03/14
 *      Use hostname instead of hardcoded name in info and station page to match title in root page
 *
 * 2026/03/xx
 *      Convert to work on ESP32 development board using compile time toggles.
 *      Added platform.h abstraction layer for ESP32-C3 support.
 *      ESP8266 behavior is unchanged. Select target board in Arduino IDE.
 *
 * 2026/03/25
 *  Due to conflects with LED string added 
 *    X  WiFi.setSleepMode() where WiFi is setup
 *    Over-ride FastLED.Show with FastLED_Show to block WiFi interferance.
 */

#include "Arduino.h"

#include "user_settings.h"   // Must come first — defines HTML, AUTOCONNECT, NUM_OF_LEDS etc.
#include "platform.h"        // Must come second — pulls in the right WiFi/SSL/WebServer headers
                              // and defines WebServerClass, WxSSLClient, platform_* functions.
#include "WX_LED_Sectional.h"

char* ssid = STASSID;
char* password = STAPSK;
const char* hostname = MYHOSTNAME;


#if HTML
WebServerClass server(80);   // WebServerClass resolves to ESP8266WebServer or WebServer
#endif

#define NO_EVENT    0
#define MY_ID       1
#define MY_TEST     2
#define MY_WXUPDATE 3
#define MY_LED_REFRESH	4

static byte my_Event = MY_TEST;
const unsigned int loop_interval = WX_REFRESH_INTERVAL * 60;    // How often to run WX update converted in seconds
unsigned int loop_time = loop_interval;                         // Force WX update first loop
// Not really 'c' header files break up this .ino file into smaller sections still accessible by the Arduino IDE
#include "utilities.h"
#include "LEDString.h"
#include "setup.h"
#include "metars.h"

void setup(void) {
	delay(100);			// some boards are unstable
	setupSerial();
  
  if(true == set_softboot(false, &lightOffset, &boot_reason)){
		my_Event = MY_WXUPDATE;   // Start with update skip LED test
    Serial.println(F(" SOFT BOOT "));
  } else {
    Serial.println(F(" HARD BOOT "));
  }

	setupBuiltInLED();
	setupBigBlock();
	setupAirportString();
	setupConnection();

#if HTML
	setupServer();
	setupmDNS();
#endif
	setupLEDString();
}

/*
 *
 * 								Main Loop
 *
 */

void loop(void) {
  static unsigned int testTime = loop_time;    // delay for test() leds

#if HTML
  server.handleClient();
  platform_MDNS_update();
#endif

  if ( timeElapsed() ) {                  	// Do every second
     switch(my_Event){
       case MY_ID:{							// LED ID call to flash LED on or off map if from console
             idLED();
       }
       break;

       case MY_TEST:{						// Cycle colors all LEDs
           // To prevent Software WD timeout test is called from loop 5 times
           if(testTime <= loop_time ){
             test();
             testTime = loop_time + 5;         // 5 seconds before next test
           }
        }
       break;

       case MY_WXUPDATE:{
#if HTML
         server.close();                    // Stop clients from locking up the 4 available connections
#endif
        loop_time = loopLEDSectional();    // Return allows loop time to be adjusted for WX GET failures
loop_time = WX_REFRESH_INTERVAL;
         testTime = loop_time;              // When loop_time overflows the next test may be delayed
         my_Event = MY_LED_REFRESH;
#if HTML
         server.begin();                    // Restart Service
#endif
       }
       break;

        case MY_LED_REFRESH:{
    	    FastLED_show();
    	    my_Event = NO_EVENT;
        }
        break;

       default: {
          toggleBuiltInLED();
          adjustBrightness();              // updates brightness scalar AND calls FastLED.show()
          if (DO_LIGHTNING && lightningLedsCount > 0) {  
            loop_time++;                   // cuts refresh time between WX updates in half in bad weather
            doLighting();                  // flashes lightning LEDs, manages its own show() calls
          }
          if (loop_time > loop_interval){
           my_Event = MY_WXUPDATE;
          }
       }

     } // switch(my_Event)
     loop_time++;   // count the seconds ( 1 loop = about 1 second )
  }
  // Test for serial data entry. Flash LED event and reboot
  if (Serial.available() > 0) {
	   idLED(getCommand()); // process any input from terminal
  }
}
