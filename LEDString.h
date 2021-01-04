#ifndef LEDString_ino
#define LEDString_ino "Jan 2, 2021"

#include <FastLED.h>


/* This section only applies if you have an ambient light sensor connected */
#if USE_LIGHT_SENSOR
/* The sketch will automatically scale the light between MIN_BRIGHTNESS and
  MAX_BRIGHTNESS on the ambient light values between MIN_LIGHT and MAX_LIGHT
  Set MIN_BRIGHTNESS and MAX_BRIGHTNESS to the same value to achieve a simple on/off effect. */

#if LIGHT_SENSOR_TSL2561
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Wire.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
#else
#define LIGHTSENSORPIN A0 // A0 is the only valid pin for an analog light sensor
#endif  //  LIGHT_SENSOR_TSL2561


#endif  //  USE_LIGHT_SENSOR

//// Set LIGHT_SENSOR_TSL2561 to true if you're using a TSL2561 digital light sensor.
//// Kits shipped after March 1, 2019 have a digital light sensor. Setting this to false assumes an analog light sensor.
#define LIGHT_SENSOR_TSL2561 false

//#define DATA_PIN    5 //6 //14 // Kits shipped after March 1, 2019 should use 14. Earlier kits us 5.    NodeMCU ESP 12-E modules
#define DATA_PIN    14 // Kits shipped after March 1, 2019 should use 14. Earlier kits us 5.            LILON WEMOS D1 mini Lite
// ******************* NOTE Pin 14 causes compile error for NodeMCU board ********************

#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

#define LIGHTSENSORPIN A0 // A0 is the only valid pin for an analog light sensor

// Define the array of leds
static CRGB leds[NUM_AIRPORTS];

/*
 * 				Setup for Led String and Light sensor
 * 
 */
void setupLEDString(void){
	 FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);

	pinMode(D1, OUTPUT); //Declare Pin mode

	#if USE_LIGHT_SENSOR
	#if LIGHT_SENSOR_TSL2561
	  Wire.begin(D2, D1);
	  if (!tsl.begin()) {
	    /* There was a problem detecting the TSL2561 ... check your connections */
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

static signed short lightOffset = 0; // can be changed using slider on station (button) page
static signed short brightness;
void adjustBrightness() {
    //signed short brightness; // to work with '-' numbers
    float reading;

    #if LIGHT_SENSOR_TSL2561
      sensors_event_t event;
      tsl.getEvent(&event);
      reading = event.light;
    #else
      #if USE_LIGHT_SENSOR
        reading = analogRead(LIGHTSENSORPIN); // dark = 1024 max light = 0
                                              // actual working values using iPhone light
                                              // dark = 1024
                                              // bright = 60
                                              // With No sensor reading = 8 sometimes 7 (using node MCU ESP12E)
      #else
        reading = 512;  // Half way for no sensor
      #endif
    #endif

    brightness =  256 - (reading / 4);    // the sensor values are Max light = 0 (60 for iPhone flashlight) ... Max dark = 1024
                                            // converts values to 256 (212) ... 0 for the LED string
    // sanity check
    brightness = brightness + lightOffset;
    if(brightness < 20) brightness = 20;
    if(brightness > 255) brightness = 255;
    FastLED.setBrightness(brightness);
    FastLED.show();
}  // adjustBrightness()

/*
 *
 *                                                Slider
 *                                  Make a nice html displayed slider
 *
 */
void slider(){
    server.sendContent(F("<h3 align=\"center\" ><form action=\"/sliderpage\" id=\"numform\" oninput=\"x.value=parseInt(a.value)\">\n"
                         "<input type=\"range\" id=\"a\" name=\"a\" value=\""));
    server.sendContent( b2Scs(lightOffset) ); // convert signed short to c-string
    server.sendContent(F("\" max=\"128\" min=\"-128\" >\n"
                         "<br>\n"
                         "<input type=\"submit\">\n"
                         "</form>\n"
                         "<output form=\"numform\" id=\"x\" name=\"x\" for=\"b\"></output><br>\n </h3>"));
}

/*
 *
 *                                                      Slider Page
 *                            Called by a HTML page get for program to procress the slider values
 *                            uses goback to return to the calling page
 *
 *
 */

void slider_page(){
  signed short svalue=0;
  if(server.hasArg("a")){
    Serial.print(F("Found value for 'a' "));
    svalue = server.arg("a").toInt();
    Serial.println(svalue);
    // FastLED.setBrightness(svalue);   // scale  a 0-255 value for how much to scale all leds before writing them out
    lightOffset = svalue;   // Slide value -128 ... 128
  }
  //const static char goBack[] PROGMEM = "<!DOCTYPE html> <script language=\"JavaScript\" type=\"text/javascript\"> setTimeout(\"window.history.go(-1)\",10); </script>";
  server.send(200, "text/html", goBack);    // goBack string stored in PROGMEM seclared in WX_Sectional.h A it is used in 2 functions iDLED and Slider_page
  server.client().stop();
}


/*
 *                                          idLED()
 *                                        call idLED(LEDNo) first
 *                                  Flashes the single LED ledNo in the string
 *                                  each call toggles ledNo white black until flashLEDCnt is 0
 *                                  on the last call flashLEDCnt == 1 restores ledNo to color saved in lastLED
 */
static byte ledNo; // flashLEDCnt = 0;  // number of cycles
static CRGB lastLED; // Prevent memory frag
 #define NO_OF_TIMES 25
void idLED(){
  static byte flashLEDCnt = NO_OF_TIMES;                                           // number of cycles to flash LED on call to ID
  static boolean flashLEDOn;
  if(flashLEDCnt > 1){
       if(flashLEDOn){
         leds[ledNo] = CRGB::Black; //CHSV(0,0,0);
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
       FastLED.show();
}

/*
 *
 *                                       idLED(number) Flash a single LED
 *                                  Setup for and the first entry to idLED()
 *
 *
*/
void idLED(byte led_number) {
  if (led_number < NUM_AIRPORTS){ // && led_number >= 0) {
    Serial.print(F("Flashing LED #"));
    Serial.println(led_number);
    ledNo = led_number; // Save Led Number for subsequent calls to idLED()
    lastLED = leds[ledNo]; // temporarily store original color
    my_Event = MY_ID;
    idLED();  // make first call the rest are done from main loop
  }
}



 /*
 *
 *                                        Test
 *                      turn on every led red blue green and white
 *                      NOTE: Can not spend a lot of time here need to return to main loop B4 SW Timeout
 *                      HWD = 8.2 seconds
 *                      SWD = About 1.5 seconds
 *
 */
void test(void){
    static byte testcycle = 5;        // Test cycle all four test colors used to call test(byte)
    CRGB testColor;                   // Instead of doing all tests in same loop cycle for each color
    switch(testcycle){
        case 5:{
Serial.print("Hello from test ");;
Serial.print(" 5 "); Serial.flush();
          testColor = CRGB::Red;
          testcycle--;
        }
        break;
        case 4:{
          testColor = CRGB::Blue;
Serial.print(" 4 "); Serial.flush();
          testcycle--;
        }
        break;
        case 3:{
          testColor = CRGB::Green;
Serial.print(" 3 "); Serial.flush();
          testcycle--;
        }
        break;
        case 2:{
          testColor = CRGB::White;
Serial.print(" 2 "); Serial.flush();

        	testcycle--;
        }
        break;
        case 1:{
          testColor = CRGB::Black;    //CHSV(0,0,0);
Serial.print(" 1 ");
          testcycle = 5;              // ready for next run
          my_Event = NO_EVENT;
          loop_time = loop_interval - 5;    // force update shortly after test
Serial.println(F("Test Done")); Serial.flush();
        }
        break;
        default:
          testcycle = 5;              // JIC
          my_Event = NO_EVENT;
       }

    int i=0;
    for( i=0; i < NUM_AIRPORTS; i++ ){
      leds[i] = testColor;
    }
    delay(10);
    FastLED.show();
}

/*
 *                                                CRGBtoHex
 *                    Convert from CRGB to a hex string 
 *                    sutibale for HTML Java script color attribute
 *                    
 *                    Function created because I could not find one in the CRGB class in any case the return is formatted 
 *                    with '#' and ready to insert in a color attribute in the HTML page
 *    
 */
static char hexval[8];
char* CRGBtoHex(CRGB c){
  //sprintf(hexval, "#%02X%02X%02X",c.r,c.g,c.b);
  //Serial.print("\nTo Hex  ");
  //Serial.print(hexval);

  hexval[0] = '#';
  hexval[1] = '\0';
  strncat(hexval, byteToHex(c.r), 2);
  strncat(hexval, byteToHex(c.g), 2);
  strncat(hexval, byteToHex(c.b), 2);
  return hexval;
}

/*
                                  doLighting()
              Procress Lighting flash the LED about once a second (every loop)
              Moved outside of LEDSectional allowing increase of the LED lighting flash rate without flooding the WX server.


*/
void doLighting() {
  if (DO_LIGHTNING && lightningLeds.size() > 0) {
    std::vector<CRGB> lightning(lightningLeds.size());
    for (unsigned short int i = 0; i < lightningLeds.size(); ++i) {
      unsigned short int currentLed = lightningLeds[i];
      lightning[i] = leds[currentLed]; // temporarily store original color
      //leds[currentLed] = CRGB::Black; // set to Black before wihite better enhancement when LED is Yellow (high wind gusts)
      //FastLED.show();
      leds[currentLed] = CRGB::White; // set to white briefly
    }
    delay(25); // extra delay seems necessary with light sensor
    FastLED.show();
    delay(75);
    for (unsigned short int i = 0; i < lightningLeds.size(); ++i) {
      unsigned short int currentLed = lightningLeds[i];
      leds[currentLed] = lightning[i]; // restore original color
    }
    FastLED.show();
  }
}   // doLighting()



#endif
