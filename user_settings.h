/*
user_settings.ino
 Created on: Jan 1, 2021
 Last modified March 28 2026
     Author: vwheeler myapps@vicw.net
*/

/*
 * Note:  I have tested with:
 *   D1 mini Lite D2  pin 4
 *   D1 mini      D5  pin 14
 *          ESP8266 2.7.4
 *          FastLED 3.7.0
 * 
 *   NodeMCU 1.0 ESP-12-E using pin D2 (2)    using D5 GPIO 14 won't compile.
 *     working with
 *         ESP8266 2.7.4                    don't work 3.0.0
 *         FastLED 3.3.0,  3.6.0,  3.7.0    don't work 3.9.20, 3.7.8, 3.7.7
 * 
 *   ESP32-C3  using pin 4 GPIO4
 *        esp32 by Espressif 3.3.7
 *        FastLED 3.10.3                    don't work  3.2.0, 2.0.0 python error
 */


#ifndef USER_INFO_INO
#define USER_INFO_INO 1

#define HTML      true      // WX LED Sectional false = LEDs only no Web pages :(
#define AUTOCONNECT true    // Use WiFi Connection manager with fallback web configuration portal instead of hard-coded SSID and Password
                            //    Consumes extra 2 to 3K of valuable ram

#include "credentials.h"    // Contains Hostname and WiFi credentials STASSID and STAPSK I don't want to be public

#define CONNECTION_ERR_RRBOOT 15		// Number of connection errors that will force a reboot

const static int numOfAirportsGet = 100;// Number of airports that are download per loop. Lower this value to make more connections if loosing data
										                    // New page downloads 1000 bytes per station and may need to limit the download buffer size.

#define WX_REFRESH_INTERVAL  12         // Minutes between WX updates

#define WIND_THRESHOLD 25               // Maximum wind speed for green, otherwise the LED turns yellow
#define DO_WINDS  true                  // color LEDs for high winds
#define DO_LIGHTNING true               // Lightning uses more power, but is cool.

#define LIGHT_OFFSET 0				 	      // LED intensity -128 to 127 
										                  // lightOffset can be adjusted using the slider on main page if html enabled.

// Light Sensor settings type and pin(s) are in LEDString.h                                     
#define USE_LIGHT_SENSOR true			    // Set USE_LIGHT_SENSOR to true if you're using any light sensor.
// #define LIGHT_SENSOR_TSL2561 false	// Set LIGHT_SENSOR_TSL2561 to true if you're using a TSL2561 digital light sensor
// 										                //       false assumes an analog light sensor.

// ─────────────────────────────────────────────────────────────────────────────
//  NeoPixel / LED strip data pin
//
//  ESP8266 (D1 Mini):
//    D4 = GPIO2 — original wiring.
//
//  ESP32-C3 SuperMini / DevKitM-1:
//    GPIO8 is shared with the on-board WS2812 RGB LED — avoid it for the strip.
//    Avoid GPIO18/19 (USB on SuperMini)
//    Good choices: GPIO3, 4, 5, 6, or 7.
//    Change LED_DATA_PIN below to match your wiring.

// NODEMCU Esp12-e suggested pind
// D4 (GPIO2): Recommended (default). Drives LEDs well without disrupting boot.
// D1 (GPIO5): Excellent choice for data.
// D2 (GPIO4): Another solid, safe choice. Note: Analog Light Sensor uses D2
// D8 (GPIO15): Known to work well
// ─────────────────────────────────────────────────────────────────────────────
#define LED_STR_DATA_PIN  D4 //14    // Logical pin that the LED string is connected to see pins_arduino.h
                                    //       example: for the esp8266 Mini the pin Labled D2 is GPIO4 use LED_STR_DATA_PIN 4
                                    // D2=4, D3=0, D4=2 I'm confused D4 displays as 2 in info however it works with pin labled D2 in the older versions

#define NUM_OF_LEDS 100             // This is really the number of LEDs not Stations
//
// Total number of stations including NULL's need to equal the Number of LEDs
const static char PROGMEM airports[NUM_OF_LEDS][5] = {
  "KBEH", // 1
  "KLWA", // 2
  "NULL", // 3
  "KAZO", // 4
  "KBTL", // 5
  "KRMY", // 6
  "NULL", // 7
  "KHAI", // 8
  "KIRS", // 9
  "KOEB", // 10
  "KJYM", // 11
  "NULL", // 12
  "KADG", // 13
  "KUSE", // 14
  "KTOL", // 15
  "KDUH", // 16
  "KTDZ", // 17
  "NULL", // 18
  "KPCW", // 19
  "NULL", // 20
  "KTTF", // 21
  "NULL", // 22
  "KARB", // 23
  "KYIP", // 24
  "KDTW", // 25
  "KONZ", // 26
  "KDET", // 27
  "KVLL", // 28
  "KMTC", // 29
  "NULL", // 30
  "KPHN", // 31
  "NULL", // 32
  "KD95", // 33
  "NULL", // 34
  "KPTK", // 35
  "NULL", // 36
  "KFNT", // 37
  "KRNP", // 38
  "NULL", // 39
  "KOZW", // 40
  "KTEW", // 41
  "KJXN", // 42
  "NULL", // 43
  "KFPK", // 44
  "KLAN", // 45
  "NULL", // 46
  "KY70", // 47
  "NULL", // 48
  "KGRR", // 49
  "NULL", // 50
  "KBIV", // 51
  "NULL", // 52
  "KMKG", // 53
  "NULL", // 54
  "KFFX", // 55
  "NULL", // 56
  "KRQB", // 57
  "NULL", // 58
  "NULL", // 59
  "KLDM", // 60
  "KMBL", // 61
  "NULL", // 62
  "KFKS", // 63
  "NULL", // 64
  "NULL", // 65
  "KCAD", // 66
  "NULL", // 67
  "KTVC", // 68
  "NULL", // 69
  "KACB", // 70
  "KCVX", // 71
  "KMGN", // 72
  "KPLN", // 73
  "KMCD", // 74
  "KSLH", // 75
  "NULL", // 76
  "KPZQ", // 77
  "NULL", // 78
  "KAPN", // 79
  "NULL", // 80
  "NULL", // 81
  "KOSC", // 82
  "KBAX", // 83
  "NULL", // 84
  "KCFS", // 85
  "KHYX", // 86
  "KMBS", // 87
  "KIKW", // 88
  "NULL", // 89
  "KAMN", // 90
  "KMOP", // 91
  "NULL", // 92
  "NULL", // 93
  "KY31", // 94
  "NULL", // 95
  "KHTL", // 96
  "NULL", // 97
  "KGOV", // 98
  "NULL", // 99
  "KGLR" // 100
};


#endif


