#ifndef LEDString_ino
#define LEDString_ino "Jan 2, 2021"

// All-ready disabled for WS8211 LED string JIC for trying different versions of FastLED
#define FASTLED_ALLOW_INTERRUPTS 0 // Fix for fastled conflict with WiFi won't compile 

#include <FastLED.h>

// FastLED conflects  with WiFi with the current versions of esp8266 board manager
// Here is where I am working to disable WiFi when updating LEDS.
inline void FastLED_show(){
//  WiFi.disconnect();
//  server.close();
  FastLED.show();
//  server.begin();
//WiFi.begin();
//  WiFi.reconnect();
}

/* This section only applies if you have an ambient light sensor connected */
#if USE_LIGHT_SENSOR
#if LIGHT_SENSOR_TSL2561
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Wire.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
#else
// ─────────────────────────────────────────────────────────────────────────────
//  Analog light sensor pin
//
//  ESP8266: A0 is the only valid ADC pin.
//  ESP32-C3: GPIO1 (ADC1_CH1) is a safe default — not shared with USB, SPI,
//            or the on-board WS2812. Change to match your wiring.
//
//  ADC resolution:
//    ESP8266 = 10-bit → 0–1023   (ADC_MAX 1023)
//    ESP32   = 12-bit → 0–4095   (ADC_MAX 4095)
//  Both are normalised to 0–1023 before the brightness calculation so the
//  math is identical on both platforms.
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
  #define LIGHTSENSORPIN 1      // GPIO1 / ADC1_CH1 — change to your wiring
  #define ADC_MAX        4095
#else
  #define LIGHTSENSORPIN A0     // Only valid ADC pin on ESP8266
  #define ADC_MAX        1023
#endif
#endif  //  LIGHT_SENSOR_TSL2561
#endif  //  USE_LIGHT_SENSOR


#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

// Define the array of leds
static CRGB leds[NUM_OF_LEDS];

/*
 * 				Setup for Led String and Light sensor
 * 
 */
void setupLEDString(void){
	FastLED.addLeds<LED_TYPE, LED_STR_DATA_PIN, COLOR_ORDER>(leds, NUM_OF_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.clear(true);

   //pinMode(LED_STR_DATA_PIN, OUTPUT);

	#if USE_LIGHT_SENSOR
	#if LIGHT_SENSOR_TSL2561
	  Wire.begin(D2, D1);
	  if (!tsl.begin()) {
	    Serial.println(F("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!"));
	  } else {
	    tsl.enableAutoRange(true);
	    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
	  }
	#else
	  pinMode(LIGHTSENSORPIN, INPUT);
	#endif
	#endif
}

/*
 * 				AdjustBrightness()
 * 		Working with two sensors the Digital TSL2561 or Analog
 * 		I tested with an Analog sensor from the package of 10 Gowoops ZZ20161810
 *
 *  FastLED.show() is called at the end so brightness changes from the slider
 *  and the ambient light sensor are applied to the strip in real time (once
 *  per second from the default: case in loop()).
 *
 *  ESP32 note: analogRead() on a floating (unwired) pin returns random values.
 *  If USE_LIGHT_SENSOR is true but no sensor is wired, set USE_LIGHT_SENSOR
 *  to false in user_settings.h, or wire a fixed resistor divider to the pin.
 */
static int16_t brightness;		 // needs to work with '-' numbers
void adjustBrightness() {
    float reading;

	#if USE_LIGHT_SENSOR
		#if LIGHT_SENSOR_TSL2561
      	  sensors_event_t event;
      	  if(tsl.getEvent(&event)) {
      		  reading = (event.light);
      	  } else {
      		  reading = 512;				// Fake-it for defective or missing sensor
      	  }
   	   	#else	// Analog sensor
       	   reading = ADC_MAX - analogRead(LIGHTSENSORPIN);  // Invert: dark=high, bright=low
       	   // sensor values (ESP8266 with no sensor wired reads ~8, stable):
       	   //   dark   = ADC_MAX
       	   //   bright = ~60 (iPhone flashlight)
       	   //   no sensor on ESP8266 = ~8
#if defined(ESP32)
           reading = reading / 4;   // Normalise 0–4095 → 0–1023 to match ESP8266 range
#endif
		#endif	// TSL2561
    #else	// No light sensor — fake it so the slider adjustment still works
        reading = 512;  // Half way
    #endif // USE_LIGHT_SENSOR

    brightness =  (reading / 4);    // 0–1023 → 0–255
    brightness = brightness + lightOffset;
    if(brightness < 20) brightness = 20;
    if(brightness > 255) brightness = 255;

    FastLED.setBrightness(brightness);
    FastLED_show();   // apply brightness to strip once per second (called from default: in loop())
}  // adjustBrightness()

/*
 *                                          idLED()
 *                                  Flashes the single LED ledNo in the string
 */
static uint8_t ledNo;
static CRGB lastLED;
 #define NO_OF_TIMES 25
void idLED(){
  static byte flashLEDCnt = NO_OF_TIMES;
  static boolean flashLEDOn;
  if(flashLEDCnt > 0){
       if(flashLEDOn){
         leds[ledNo] = CRGB::Black;
       }else{
         leds[ledNo] = CRGB::White;
       }
       flashLEDOn = !flashLEDOn;
  }else {
    leds[ledNo] = lastLED;
    my_Event = NO_EVENT;
    flashLEDCnt = NO_OF_TIMES;
  }
       flashLEDCnt--;
       delay(10);
       FastLED_show();
}

void idLED(uint8_t led_number) {
  if (led_number < NUM_OF_LEDS){
    Serial.print(F("Flashing LED #"));
    Serial.println(led_number);
    ledNo = led_number;
    lastLED = leds[ledNo];
    my_Event = MY_ID;
    idLED();
  }
}

/*
 *                                                CRGBtoHex
 *                    Convert from CRGB to a hex string suitable for HTML color attribute
 */
static char hexval[8];
char* CRGBtoHex(CRGB c){
  hexval[0] = '#';
  hexval[1] = '\0';
  strncat(hexval, byteToHex(c.r), 2);
  strncat(hexval, byteToHex(c.g), 2);
  strncat(hexval, byteToHex(c.b), 2);
  return hexval;
}


/*
 *                                        Test
 *                      turn on every led red blue green and white
 */
    static byte testcycle = 5;
    void resetTestCycle() { testcycle = 5; }
void test(void){
      // Guarantee WS2811 reset pulse — hold data line low for 300µs
    // before every frame. FastLED does not do this automatically
    // between calls separated by more than 50µs.
    digitalWrite(LED_STR_DATA_PIN, LOW);
    delayMicroseconds(300);

    CRGB testColor;
    switch(testcycle){
        case 5:{
          delay(100);
		      Serial.print(F(" Red "));
          testColor = CRGB::Red;
          testcycle--;
        }
        break;
        case 4:{
          Serial.print(F(" Blue "));
          testColor = CRGB::Blue;
          testcycle--;
        }
        break;
        case 3:{
          Serial.print(F(" Green "));
          testColor = CRGB::Green;
          testcycle--;
        }
        break;
        case 2:{
          Serial.print(F(" White "));
          testColor = CRGB::White;
        	testcycle--;
        }
        break;
        case 1:{
          Serial.print(F(" Black "));
          testColor = CRGB::Black;
          testcycle = 5;
          my_Event = NO_EVENT;
          loop_time = loop_interval - 5;  // Force update in 5 seconds
        }
        break;
        default: {
          testcycle = 5;
           my_Event = NO_EVENT;
        }
      }
    Serial.print(CRGBtoHex(leds[0]));
    for( int i=0; i < NUM_OF_LEDS; i++ ){
      leds[i] = testColor;
    }
    if(testcycle == 5 ) Serial.println();
    FastLED_show();
    delay(100);
    Serial.flush(); 
}

/*
                                  doLighting()
              Process lightning — flash the LED(s) about once a second.
*/
static CRGB lightningColors[MAX_LIGHTNING];   // fixed-size static — replaces VLA

void doLighting() {
  if (DO_LIGHTNING && lightningLedsCount > 0) {
    for (unsigned short int i = 0; i < lightningLedsCount; ++i) {
      lightningColors[i]      = leds[lightningLeds[i]];  // save
      leds[lightningLeds[i]]  = CRGB::White;              // flash white
    }
    delay(25); my_yield();
    FastLED_show();
    delay(75); my_yield();
    for (unsigned short int i = 0; i < lightningLedsCount; ++i) {
      leds[lightningLeds[i]] = lightningColors[i];        // restore
    }
    FastLED_show();
  }
}   // doLighting()



#endif
