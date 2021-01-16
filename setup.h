#ifndef setup_ino
#define setup_ino Jan 1, 2021
#include "utilities.h"

void setupConnection(void){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupmDNS(void){
  if (MDNS.begin(hostname)) {
    Serial.print(F("MDNS responder started http://"));
    Serial.print(hostname);  Serial.println(F(".local"));
  }
}

void setupBiLED(void){
	  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
	  digitalWrite(LED_BUILTIN, 0);             // Off

}
void setupSerial(void){
	  delay(100);
	  Serial.begin(74880); //115200);
	  Serial.print(F(copyright)); Serial.print(F(" ")); Serial.println(F(compiledate));
}

void setupServer(void){
	  server.onNotFound(handleNotFound);
	  server.on(F("/")         , 	rootPage);
	  server.on(F("/test")     , 	testPage);
	  server.on(F("/stationsL"), 	stationPage8x8);    // large screen
	  server.on(F("/stationsW"), 	stationPage4x16);   // small screen ipad or phone rotated 90 deg
	  server.on(F("/stationsN"), 	stationPage2x32);   // small screen phone
	  server.on(F("/reboot")   ,	rebootPage);
	  server.on(F("/sliderpage"),	slider_page);
	  server.on(F("/wxID"), 		iDLED);  // For html get function that calls flash LEDs

	  server.begin();
	  Serial.println("HTTP server started");



}

//void setupClient(){
//
//}

/*

                                            setupAirportString()
                            to prevent memory fragmentation create the airport string only once

                            thoughts:
                              - Break up the new airport string (without NULL stations) to download a few smaller segments that should use less memory.
                              - Either use a structure for the substrings or an index array to store the last index of the last station in the airport array
                                this should limit the time used for String search which I think is causing corrupted data.
                            globals needed
                               - Total number of substrings (integer for cycle) might be possible to use a flag or null pointer in the substrings array
                               - Number of substrings array for the substring pointers into the airports string (without NULLS)
                               - Number of substrings array with index into the airport array for each sub string remember airport array contains NULLs


*/

/*
 *                                          setupBigBlock()
 *                            Idea here is to block out a large chunk of memory to use instead of dynamic Strings that may cause memory
 *                              fragmentation resulting traps when the SSL connection does not having enough continuous free space to build
 *                              it's buffers
 *
 *                            The airport substrings and the WX data structures also use in this space the WX Raw Text field will use up
 *                            the remaining block up to the end of the block after which point to the static Out Of Memory message
 *
 */

 // Has to be at least 5 chars * the number of Stations
 // in good weather add 72 chars for each WX text field (good weather)
 // 100 active stations needs = 500 + 7200 total of 7,200 Bytes
 // WX download connect needs  30000 continuous bytes

 // #define bigBlockSize (1024 * 7)  // 7 K
 // #define bigBlockSize (512 * 15)  // 7.5 k Some out of memory 4 today
 // #define bigBlockSize (256 * 31)  // 7.75 k
 // #define bigBlockSize (128 * 63)  // 7.875 k
  #define bigBlockSize (1024 * 8)   // 8K works in good weather with 100 active stations OK with 64 active
 // #define bigBlockSize (512 * 17)   // 8.5K somewhat OK frequent download fragmentation & errors occasional reboots
 // #define bigBlockSize (256 * 35)   // 8.75K Max with 104 stations
 // #define bigBlockSize (128 * 71)   // 8.875K Max with 96 stations
 // #define bigBlockSize (1024 * 10)


static char bigBlock[((sizeof(char) * bigBlockSize) +2)];     // add additional byte for terminator if the last block of data just happens to == bigBlockSize
static int firstAvailable = 0;                                // Tracks the next unused location in the bigBlockSize array
void setupBigBlock(){
  // bigBlock = new char[((sizeof(char) * bigBlockSize) +1)]; // better to allocate memory outside of function that way the block is defined first
  for(int i = 0; i < bigBlockSize; i++){
    bigBlock[i] = '\0';
  }
  firstAvailable = 0;
}


//String airportString = "";
char* airportString = NULL;             // working c-string airports array without the NULL's
// Thoughts as this string does not change unless airports are modified requiring program re-upload
// so after being created might use a check for first run after upload only then create and store this string and pointers into ee-ROM
// delete this now temporary working string and work only from ee-ROM memory this saves #airports * 5 of valuable ram space (500 bytes for 100 stations)
// + a few bytes for the pointers

//// const static int numOfAirportsGet  = 25; // NUM_AIRPORTS; // Number of airports to download per loop  MOVED to .h
//int actualNumAirports  = NUM_AIRPORTS;  // Total without NULL locations
//int airportStringsSize = 0;             // total number of char's needed
//int noOfAirportStrings = 1;             // if dividing up for download how many substrings needed / used
//char** airportStrings;                  // pointers to substrings
//int* airportIndex;                      // Last location in airports[] for each airport substring limit station's
//// string compare loop I.E. from airportindex[0] to airportindex[1]
/*
 *
 *              setupAirportString()
 *
 *
 *
 */
void setupAirportString() {
  if (airportString != NULL) {
    // Serial.println(F("airportString Already setup skipping recreation to prevent memory fragmentation"));
    return;
  }

  // First find the number of stations and compute amount of memory needed to be allocated
  actualNumAirports = 0;    // Actual number of active WX stations without the NULL's
  for (int i = 0; i < NUM_AIRPORTS; i++) {
    if (strncmp_P("NULL", airports[i], 4) != 0)
      actualNumAirports++;
    mtrsf[i].mtrlighting = false;   // reset lightning
    mtrsf[i].mtrstat = NOTUSED;
    mtrsf[i].mtrspeed = 0;          // wind in knots
    mtrsf[i].mtrgusts = 0;          // reset wind gusts
    //  mtrsf[i].mtrtime[0] = '\0'; // Observation Time Note: Replaced with rawText
    mtrsf[i].rawText = ooMem;       // Default pointer to static "Out Of Memory"
}

  // These are created once for life never deleted
  airportStringsSize = ((sizeof(char) * (actualNumAirports) ) * 5);   // integer use to create, sanity checks and prevent overflows length includes ',' or the null ie "KJXN,"
  airportString = &bigBlock[firstAvailable++];                        // This is the string of WX stations to be sent to WX_Weather.com for the get (downloads)
  firstAvailable += airportStringsSize;
  airportString[0] = '\0';                                            //

  // Using pointers create several null terminated sub-strings from airportString
  noOfAirportStrings = (int) ( ( (double) actualNumAirports / (double) numOfAirportsGet) + 0.9);  // number of sub strings needed round up for the last possibly partial line
  airportStrings = new char*[sizeof(char*) * noOfAirportStrings];     // For the pointers into the airportString replace the last ',' with a null creating shorter c_strings
  airportIndex = new int[noOfAirportStrings+1];                       // index into the main airports string to limit number of string compares when processing data downloaded using the sub
  airportIndex[0] = 0;                                                // stings. Make +1 because we are starting with 0 and the last location = the end
  int sindex[actualNumAirports];                                      // temp to track and convert string index to substring index
  int sidxi = 0;                                                      // iterator for sindex


  Serial.println(F("Creating a new airportString"));
  for (int i = 0; i < NUM_AIRPORTS; i++) {
    if (strncmp_P("NULL", airports[i], 4) != 0 ) {
      strcat_P(airportString, airports[i]);
      strcat(airportString, ",");   // Separate stations
      sindex[sidxi++] = i;          // sequential index to LEDS without NULL stations
      mtrsf[i].mtrstat = UNKWN;
    } else {                        // Added to report null stations V.W>
      mtrsf[i].mtrstat = NOTUSED;   //  Default meteorology status 99 is for NULL locations a tag to skip progressing in stations html page
    }
  }
  Serial.print(F("AirportString = \n{")); Serial.print(airportString); Serial.println("}");

  // Finished creating working c-string.
  // now break into several null terminated sub c-strings
  // link the index between the end of substring and airports limiting string compares to this subset when finding LED ID from the downloaded data
  airportStrings[0] = airportString; // noOfAirportStrings = 0 Always need the first c-string / substring
  // remove the last ','
  if( airportString[strlen(airportString) -1] == ','){
    airportString[strlen(airportString) -1] = '\0';  // terminate end even if partial
  } // else Serial.println("Something is verry wrong airportString not ',' terminated");

  for (int i = 1; i < noOfAirportStrings; i++) {                      // the end of first is the start of next sub-string
    airportStrings[i] = airportString + ((numOfAirportsGet * 5) * i); // remember first string / substring started at 0
    if (airportString[(numOfAirportsGet * 5  * i) - 1] == ',') {      // Should be the last char in substring
      airportString[(numOfAirportsGet * 5 * i) - 1] = '\0';           // replace with the null terminate making it a substring
      airportIndex[i] = sindex[i * numOfAirportsGet];
      Serial.print(F("Last in substring = ")); Serial.println((airportStrings[i]-5));
    } // else { OhNo somthing is very wrong }
  }   // Last one may be shorter then the rest
  airportIndex[noOfAirportStrings] = sindex[actualNumAirports-1];
  Serial.print(F(" Last index number = ")); Serial.println(airportIndex[noOfAirportStrings]);
  /*
   *            NUM_AIRPORTS 15
   *            airports[][5]
   *                    0  "KBEH", // 1
   *                    1  "KLWA", // 2
   *                    2  "NULL", // 3
   *                    3  "KAZO", // 4
   *                    4  "KBTL", // 5
   *                    5  "KRMY", // 6
   *                    6  "NULL", // 7
   *                    7  "KHAI", // 8
   *                    8  "KIRS", // 9
   *                    9  "KOEB", // 10
   *                    0  "KJYM", // 11
   *                   11  "NULL", // 12
   *                   12  "KADG", // 13
   *                   13  "KUSE", // 14
   *                   14  "KTOL", // 15
   *
   *
   *            airportString "KBEH,KLWA,KAZO,KBTL,KRMY,KHAI,KIRS,KOEB,KJYM,KADG,KUSE,KTOL"
   *                           *                       *                        *
   *                            ->0                     ->26                     ->51
   *
   *            actualNumAirports = 12
   *
   *            airportStringsSize = 60
   *
   *            numOfAirportsGet = 5
   *
   *            noOfAirportStrings = 3
   *
   *            airportStrings[]
   *                          0 ->0
   *                          1 ->26
   *                          2 ->51
   *
   *            sindex[]
   *                   0  0        //  KBEH
   *                   1  1        //  KLWA
   *                   2  3        //  KAZO
   *                   3  4        //  KBTL
   *                   4  5        //  KRMY
   *                   5  7        //  KHAI
   *                   6  8        //  KIRS
   *                   7  9        //  KOEB
   *                   8  10       //  KJYM
   *                   9  12       //  KADG
   *                  10  13       //  KUSE
   *                  11  14       //  KTOL
   *
   *
   *            airportIndex[]
   *                         0  5
   *                         1  12
   *                         2  14
   *
   */
  Serial.print(F("Creating a new airportString ")); Serial.print(strlen( airportString));
  Serial.print(F(" size ")); Serial.print(airportStringsSize);
  Serial.print(F(" noOf ")); Serial.print(noOfAirportStrings);
  Serial.print(F(" sizeof ")); Serial.println(sizeof(airportStrings));
  for (int i = 0; i < noOfAirportStrings; i++) {
    Serial.print(i);
    Serial.print(F(">>"));
    Serial.print(airportStrings[i]);
    Serial.print(F("<< "));
    Serial.println(airportIndex[i+1]);    // started with 0 with one additional location
  }
  return;
}


#endif
