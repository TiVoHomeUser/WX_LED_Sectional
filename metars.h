#ifndef metars_ino
#define metars_ino "Jan 2, 2021"

void doColor(char* identifier, unsigned short int led, int wind, int gusts, char* condition, char* wxstring, const char* rawText){ //, boolean last) {
  showFree( false );
#if WX_DEBUG
  Serial.print(F("\t")); Serial.print(identifier);
  Serial.print(F(": "));
  Serial.print(condition);
  Serial.print(F(" "));
  Serial.print(wind);
  if (gusts > 0) {
    Serial.print(F(" G"));
    Serial.print(gusts);
    Serial.print(F("kts\t"));
  } else {
    Serial.print(F("kts    \t"));
  }
  Serial.print(F(" LED "));
  Serial.print(led);
  Serial.print(F(" WX: "));
  Serial.print(wxstring);
  Serial.print(F(" Raw: "));
  Serial.println(rawText);
#endif
  if (strstr(wxstring, "TS") != NULL) {
    Serial.println(F("... found lightning!"));
    lightningLeds.push_back(led);
    mtrsf[led].mtrlighting = true;         // For Button to display lighting
  }

  // 1 VFR, 2 MVFR, 3 LIFR, 4 IFR 0 unknown  maybe use leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
  // TODO ********** NOTE: AIM defines 1 LIFR, 2 IFR, 3 MVFR, 4 VFR also can add WIND CIG FG Example "IFR FG" "LIFR CIG" "VFR WIND" *******
  if (strcmp(condition, "LIFR") == 0) { // || identifier == "LIFR") {
    leds[led] = CRGB::Magenta;
    mtrsf[led].mtrstat = 3;
  }
  else if (strcmp(condition, "IFR") == 0) {
    leds[led] = CRGB::Red;
    mtrsf[led].mtrstat = 4;
  }
  else if (strcmp(condition, "MVFR") == 0) {
    leds[led] = CRGB::Blue;
    mtrsf[led].mtrstat = 2;
  }
  else if (strcmp(condition, "VFR") == 0) {
    mtrsf[led].mtrstat = 1;
    if ((wind > WIND_THRESHOLD || gusts > WIND_THRESHOLD) && DO_WINDS) {
      leds[led] = CRGB::Yellow;
    } else {
      leds[led] = CRGB::Green;
    }
  } else {
    leds[led] = CRGB::Black;
    mtrsf[led].mtrstat = 0;   // 99 for airports defined as NULL 0 will still display for un-reporting stations
  }
  mtrsf[led].mtrspeed = wind;
  mtrsf[led].mtrgusts = gusts;
  mtrsf[led].rawText = rawText;
  total_C_StringLen += strlen(rawText);
  FastLED.show(); // Kid of cool update display as each down-loaded
  // Note: causes the ones not loaded to go black and may be annoying
} // doColor()

// in good weather currentLine length = 220 I was getting much larger values when weather was bad b4 isAscii check was added
// found 238
// -> *** GT 220 currentLine.length() = 238
// ->       <raw_text>KMKG 110121Z 20017G27KT 3SM R32/4000VP6000FT -TSRA BR SCT018 BKN032CB OVC060 16/14 A2963 RMK AO2 PK WND 26036/0057 WSHFT 0047 VIS 2 RWY 14 LTG DSNT ALQDS RAB00 FRQ LTGICCCCG SW-N TS SW-N MOV NE P0043 T01610139</raw_text>
//

/*
                                  getMetars()


*/
#define DONE        0
#define AIRPORT     1
#define CONDITION   2
#define WIND        3
#define GUST        4
#define WX          5
#define RAWTEXT     6
#define CURLINESIZE 230 //320   // Normally 220 addinf 100 temporarily till finalized

// static unsigned int currentLineMax = 0;    // String currentLine is used to collect / process WX input for each station loop.

// These arrays are used only in getMetars() Not global only defined outside function to limit memory fragmentation
static char currentAirport[5];   // KDTW KYIP ...
static char currentCondition[5]; // VFR LIFR ...
static char currentWind[4];      // current wind conditions expect 0 to 99 allow for three places + term JIC over 100 kts
static char currentGusts[4];     //                "                   "                   "
static char currentLine[CURLINESIZE]; // Buffer for collecting the data as received to progress for each station
static char currentWxstring[26]; // Wx notes TS -SN RA ...

// static byte ooMemCnt;

bool getMetars() {
//  Server.close();         // Stop connections
  total_C_StringLen = 0;
  boolean maxexcd = false;  // For debugging c-string size

  for (int l = 0; l < NUM_AIRPORTS; l++) {
    mtrsf[l].mtrlighting = false;
    mtrsf[l].rawText = offLine; //  for the .html panel display Default message if station does not report
  }

  lightningLeds.clear(); // clear out existing lightning LEDs since they're global
  fill_solid(leds, NUM_AIRPORTS, CRGB::Black); // Set everything to black just in case there is no report

  char c;
  byte wxcheck = DONE;                 // used for type of data being processed (to get rid of the nested if else statements)
  unsigned short int st;
  unsigned short int en;
  uint32_t t;
  uint32_t gtimeout;
  //std::vector<unsigned short int> led;  // Why? led is only used as a holder to pass a single integer
  int led = -1;                           // becomes the value for the string led currently being processed each loop
  //String currentAirport = "";   // char[5];
  //char* currentAirport = new char[6 * sizeof(char)];

  int cnt = 0;    // used as index for
  currentAirport[cnt] = '\0';
  currentCondition[cnt] = '\0';
  currentWind[cnt] = '\0';
  currentGusts[cnt] = '\0';
  currentWxstring[cnt] = '\0';
  int lncnt = 0;
  currentLine[lncnt] = '\0';
ooMemCnt = 0;   // counter for the number of stations getting the Out Of Memory message
  const char* rawText = &bigBlock[firstAvailable]; // Pointer for the next substring thats added to airport structure
  int counter = firstAvailable;       // start of wx raw text
  // AirportString =
  // {KBEH,KLWA,KAZO,KBTL,KRMY,KHAI,KIRS,KOEB,KJYM,KADG,KUSE,KTOL,KDUH,KTDZ,KPCW,KTTF,KARB,KYIP,KDTW,KONZ,KDET,KVLL,KMTC,KPHN,KD95,KPTK,KFNT,KRNP,KOZW,KTEW,KJXN,KFPK,
  //  KLAN,KY70,KGRR,KBIV,KMKG,KFFX,KRQB,KLDM,KMBL,KFKS,KCAD,KTVC,KACB,KCVX,KMGN,KPLN,KMCD,KSLH,KPZQ,KAPN,KOSC,KBAX,KCFS,KHYX,KMBS,KIKW,KAMN,KMOP,KY31,KHTL,KGOV,KGLR}
  Serial.print(F("GetMetars\t"));

#ifdef DEBUG
  showFree(true);
#endif

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  //Serial.println(F("\nStarting connection to server..."));
  client.setTimeout(8000);


  if (!client.connect(WXSERVER, 443)) {
    char str[81]; str[0] = '\0'; // For return of the SSLError results
#ifdef DEBUG
    showFree(true);
#endif
    Serial.print(F("Connection failed! : ")); Serial.println(client.getLastSSLError(str, 80)); Serial.println(str); //Serial.println(strlen(str));
    client.stop();
    return false;
  }

  t = millis(); // start time
  while (!client.connected()) {
    if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
      Serial.println(F("---Timeout---"));
      client.stop();
      return false;
    } else {
      Serial.print(F("."));
      delay(1000);
    }
  }

  // if you get a connection, report back via serial:
  FastLED.clear();    // Clears leds[] reset only once for all group(s) of downloads
  for (int wxLoopCnt = 0; wxLoopCnt < noOfAirportStrings; wxLoopCnt++) {
    Serial.print(F("Getting data ")); Serial.print(wxLoopCnt + 1); Serial.print(F(" Of ")); Serial.println(noOfAirportStrings);
    led = -1;                 // becomes the value for the led in the string thats currently being processed Also use -1 for to flag the first loop
    // Note: Setting led -1 forces clear on first loop

    rawText = &bigBlock[counter]; // Pointer for substring to add to airport structure

    Serial.println(F("Connected ..."));

    Serial.print(F("GET ")); Serial.print(BASE_URI); Serial.print(airportStrings[wxLoopCnt]); Serial.println(F(" HTTP/1.1")); Serial.print(F("Host: ")); Serial.println(WXSERVER);
    client.print(F("GET ")); client.print(BASE_URI); client.print(airportStrings[wxLoopCnt]); client.println(F(" HTTP/1.1")); client.print(F("Host: ")); client.println(WXSERVER);
    if (wxLoopCnt < (noOfAirportStrings - 1)) { // Request keep connection open
      Serial.println(F("Connection: Keep-Alive"));
      client.println(F("Connection: Keep-Alive"));
    } else {                               // Request close connection
      Serial.println(F("Connection: close"));
      client.println(F("Connection: close"));
    }
    Serial.println(); Serial.flush();
    client.println(); client.flush();

    gtimeout = millis() + (20 * 60 * 1000);
    while (!client.available() && gtimeout > millis()) {
      my_yield();
      //delay(10);
      ;;
    }

    //  FastLED.clear(); NOT HERE wipes out previous download



    /*
            Read from buffer Loop
    */
    //while (client.connected()) {
    short ic;		//	Not sure how Arduino handles chars do it the safe way
    while (client.available()) {
      if ((ic = client.read()) >= 0){
    	  c = (char) ic;
    	  if (isAscii(c)) {
          my_yield(); // Otherwise the WiFi stack can crash

          if ( lncnt - 2 < CURLINESIZE )  // Add to buffer else Out of buffer run out data until endl
            currentLine[lncnt++] = c;
          currentLine[lncnt] = '\0';
          if ( '\n' == c || '\r' == c ) {      // Ready for a new line
            /*
                                // Line example
                <METAR>
                  <raw_text>KJXN 222156Z 24007KT 10SM FEW029 28/21 A2993 RMK AO2 SLP129 T02780206 $</raw_text>
                  <station_id>KJXN</station_id>
                  <observation_time>2020-07-22T21:56:00Z</observation_time>
                  <latitude>42.27</latitude>
                  <longitude>-84.47</longitude>
                  <temp_c>27.8</temp_c>
                  <dewpoint_c>20.6</dewpoint_c>
                  <wind_dir_degrees>240</wind_dir_degrees>
                  <wind_speed_kt>7</wind_speed_kt>
                  <visibility_statute_mi>10.0</visibility_statute_mi>
                  <altim_in_hg>29.929134</altim_in_hg>
                  <sea_level_pressure_mb>1012.9</sea_level_pressure_mb>
                  <quality_control_flags>
                    <auto_station>TRUE</auto_station>
                    <maintenance_indicator_on>TRUE</maintenance_indicator_on>
                  </quality_control_flags>
                  <sky_condition sky_cover="FEW" cloud_base_ft_agl="2900" />
                  <flight_category>VFR</flight_category>
                  <metar_type>METAR</metar_type>
                  <elevation_m>305.0</elevation_m>
                </METAR>


            */

            lncnt = 0;
            currentLine[lncnt] = '\0';
          } // eoln
          switch (wxcheck) {
            case AIRPORT: {
                if ('<' != c) {
                  if (cnt < 5) currentAirport[cnt++] = c;
                  currentAirport[cnt] = '\0';
                } else {
                  if (maxexcd == true) {
                    maxexcd = false;
                    Serial.println(currentAirport);
                  }
                  wxcheck = DONE;
                  cnt = 0;
                  st = airportIndex[wxLoopCnt];
                  en = airportIndex[wxLoopCnt + 1];
                  // Serial.print(F("Searching from ")); Serial.print(st); Serial.print(F(" To ")); Serial.println(en);
                  do {
                    //if (strcmp_P(currentAirport.c_str(), airports[st]) == 0) {
                    if (strcmp_P(currentAirport, airports[st]) == 0) {
                      led = st;
                      st = en;    // FOUND IT! break
                    }
                    st++;         // increment even if found to force exit
                  } while (st <= en);
                }
                //            } else { // readingAirport
                break;
              }

            case CONDITION: {
                if ('<' != c) {
                  if (cnt < 5) currentCondition[cnt++] = c;
                  currentCondition[cnt] = '\0';
                } else {
                  cnt = 0;
                  wxcheck = DONE;
                }
                break;
              }

            case WIND: {
                if ('<' != c) {
                  if (cnt < 4) currentWind[cnt++] = c;
                  currentWind[cnt] = '\0';
                } else {
                  cnt = 0;
                  wxcheck = DONE;
                }
                break;
              }

            case GUST: {
                if ('<' != c) {
                  if (cnt < 4) currentGusts[cnt++] = c;
                  currentGusts[cnt] = '\0';
                } else {
                  cnt = 0;
                  wxcheck = DONE;
                }
                break;
              }

            case WX: {
                if ('<' != c) {
                  if (cnt < 25) currentWxstring[cnt++] = c;
                  currentWxstring[cnt] = '\0';
                } else {
                  cnt = 0;
                  wxcheck = DONE;
                }
                break;
              }

            case RAWTEXT: {
                if ('<' != c) {
                  if (counter < bigBlockSize) {
                    bigBlock[counter++] = c;
                  } else {
                    rawText = ooMem;
                  }
                } else {
                  wxcheck = DONE;
if(counter >= bigBlockSize) ooMemCnt++;
                  bigBlock[counter++] = '\0'; // terminate and ready for next airport
                  if (strlen(rawText) > rtmaxl) {
                    rtmaxl = strlen(rawText);
                    if (strlen(rawText) >= RTMAXLN) {
                      wxcheck = DONE;
                      Serial.print(F("\tRTMAXLN Exceded ")); Serial.print(strlen(rawText)); Serial.print(F(" ")); maxexcd = true;
                    }
                  }
                }
                break;
              }

            default: {
                if ('>' == c) {
                  if (cendsWith(currentLine, "<raw_text>")) { // start paying attention
                    if (led >= 0) { // Not the first station the previous station data should of been collected process the results before clearing
                      doColor(currentAirport, led, atoi(currentWind), atoi(currentGusts), currentCondition, currentWxstring, rawText); //, false);
                      led = -1;
                    }
                    wxcheck = RAWTEXT;
                    cnt = 0; // Reset everything with the change of WX station
                    currentAirport[cnt] = '\0';
                    currentCondition[cnt] = '\0';
                    currentWind[cnt] = '\0';
                    currentGusts[cnt] = '\0';
                    currentWxstring[cnt] = '\0';
                    rawText = &bigBlock[counter]; // pointer for the next block of Raw text to save
                  }  else {
                    if (cendsWith(currentLine, "<station_id>")) {
                      wxcheck = AIRPORT;
                    } else {
                      if (cendsWith(currentLine, "<wind_speed_kt>")) {
                        wxcheck = WIND;
                      } else {
                        if (cendsWith(currentLine, "<wind_gust_kt>")) {
                          wxcheck = GUST;
                        } else {
                          if (cendsWith(currentLine, "<flight_category>")) {
                            wxcheck = CONDITION;
                          } else {
                            if (cendsWith(currentLine, "<wx_string>")) {
                              wxcheck = WX;
                            }
                          }
                        }
                      }
                    }
                  } // else
                } // c == '>'
              } // default:
          } // case(wxcheck)

          t = millis(); // Reset timeout clock
        } else if ((millis() - t) >= (READ_TIMEOUT * 1000)) {   // Client.read
          Serial.println(F("---Timeout---"));
          fill_solid(leds, NUM_AIRPORTS, CRGB::Cyan); // indicate status with LEDs
          FastLED.show();
          client.stop();
          return false;
        }
    }	//client.read()
      int na = 500;     // I think I made the string search too fast loop finish b4 next char is received
      while (!client.available() && na > 0) { // Give some time for the next char to be received
        delay(10);
        my_yield();
        na--;
      }
    } // While connected

    // need to doColor this for the last airport in each group
    if ( led >= 0) {
      doColor(currentAirport, led, atoi(currentWind), atoi(currentGusts), currentCondition, currentWxstring, rawText); //, true);
    }

  } // loop loopWxGet
//  delay(100); my_yield(); delay(100); my_yield(); if (client.available()) Serial.println(F("--------------------------- More data Available ---------------------"));

  client.stop();
//  my_yield();
//  Server.begin();     // resume connections
  return true;
} // getMetars()


/*

                            loopLEDSectional

*/
//static  int retVal;
int loopLEDSectional() {
  retVal = 0;
  Serial.println(F("Getting METARs ..."));
  if (getMetars()) {
    Serial.println(F("Refreshing LEDs."));
    FastLED.show();
    if (lightningLeds.size() > 0) Serial.println(F("There is lightning"));    // Nice to see there is lightning in the serial monitor at the end of Station output
    cycleCount++;
    if(cycleErrCount > 0) cycleErrCount--;  // Give credit for good connections
  } else {
    retVal = loop_interval - RETRY_TIMEOUT;    // retVal will become loopcount Can't connect to server try again in 110 seconds (event happens when count = time)
    if (cycleErrCount > cycleCount) retVal = retVal - RETRY_TIMEOUT; // Longer delay for excessive errors
    cycleErrCount++; // count bad connections if too many force reboot
  }
  if (retVal < 0) retVal = 0; // JIC
  if(cycleErrCount > 15){
    Serial.println(F("Too many connection errors forcing reset"));
    ESP.reset();      // Not sure which one to use reports that .restart() has problems
    ESP.restart();
  }
  return retVal;      // new loop_time
} // loopLedSectional

#endif
