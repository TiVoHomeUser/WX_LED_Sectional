#ifndef metars_ino
#define metars_ino "Feb 21, 2026"
// 2021/09/14 Fix for station's Button can display last metar conditions after a station goes off-line  V.W>
// 2026/02/21 static client hopfully prevent memory fragmentation

/* ============================================================================
 *  Reusable SSL client (prevents memory fragmentation)
 * ============================================================================
 */
static BearSSL::WiFiClientSecure* wxClient = nullptr;

// In metars.h — replace initWxClient() with this:
void resetWxClient() {
  if (wxClient != nullptr) {
    wxClient->stop();
    delete wxClient;
    wxClient = nullptr;
  }
  wxClient = new BearSSL::WiFiClientSecure();
  wxClient->setInsecure();
  wxClient->setTimeout(8000);
}

void initWxClient() {
  if (wxClient == nullptr) {
    wxClient = new BearSSL::WiFiClientSecure();
    wxClient->setInsecure();
    wxClient->setTimeout(8000);
  }
}

  void doColor(char* identifier, unsigned short int led, int wind, int gusts, char* condition, char* wxstring, const char* rawText){
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
    if(lightningLedsCount < MAX_LIGHTNING) lightningLeds[lightningLedsCount++] = led;
    mtrsf[led].mtrlighting = true;         // For Button to display lighting
  }

  // leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
  // 1 LIFR, 2 IFR, 3 MVFR, 4 VFR also can add WIND CIG FG Example "IFR FG" "LIFR CIG" "VFR WIND" *******

  if (strcmp(condition, "LIFR") == 0) { // || identifier == "LIFR") {
    leds[led] = CRGB::Magenta;
    mtrsf[led].mtrstat = LIFR;
  }
  else if (strcmp(condition, "IFR") == 0) {
    leds[led] = CRGB::Red;
    mtrsf[led].mtrstat = IFR;
  }
  else if (strcmp(condition, "MVFR") == 0) {
    leds[led] = CRGB::Blue;
    mtrsf[led].mtrstat = MVFR;
  }
  else if (strcmp(condition, "VFR") == 0) {
    mtrsf[led].mtrstat = VFR;
    if ((wind > WIND_THRESHOLD || gusts > WIND_THRESHOLD) && DO_WINDS) {
      leds[led] = CRGB::Yellow;
    } else {
      leds[led] = CRGB::Green;
    }
  } else {
    leds[led] = CRGB::Black;
    mtrsf[led].mtrstat = UNKWN;   // 99 for airports defined as NULL 0 will still display for un-reporting stations
  }
  mtrsf[led].mtrspeed = wind;
  mtrsf[led].mtrgusts = gusts;
  mtrsf[led].rawText = rawText;
  total_C_StringLen += strlen(rawText);
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
#define CURLINESIZE 320 //256 // In good weather 217 bytes max terminating line for each "/>" was 1024 with cr/lf // Working line buffer size I avoid using Strings due to memory overhead
// Starting with CURLINESIZE = 1000 without reseting the line pointer at the start of each station each increases the overflow by
//  first test		Second test
// 1 = 233			233
// 2 = 920	+687	1038	+805
// 3 = 1591	+671	1709	+761
// 4 = 2336	+745	2513	+804
// 5 = 3079	+743	3194	+681
// 6 = 3998	+901	3998	+804

// These arrays are used only in getMetars() Not global only defined outside function to limit memory fragmentation
static char currentAirport[5];   // KDTW KYIP ...
static char currentCondition[5]; // VFR LIFR ...
static char currentWind[4];      // current wind conditions expect 0 to 99 allow for three places + term JIC over 100 kts
static char currentGusts[4];     //                "                   "                   "
static char currentLine[CURLINESIZE]; // Working buffer used for collecting the data as received to progress for each station
static char currentWxstring[26]; // Wx notes TS -SN RA ...

bool getMetars() {
  total_C_StringLen = 0;
  boolean maxexcd = false;  // For debugging c-string size

  for (int l = 0; l < NUM_AIRPORTS; l++) {
	  if(mtrsf[l].mtrstat != NOTUSED) {
		  mtrsf[l].mtrlighting = false;
		  mtrsf[l].rawText = offLine; //  for the .html panel display Default message if station does not report
		  mtrsf[l].mtrstat = UNKWN;	// Fix for station after it goes off-line
	  }
  }

  lightningLedsCount = 0; // clear out existing lightning LEDs since they're global

  fill_solid(leds, NUM_AIRPORTS, CRGB::Black); // Set everything to black just in case there is no report

  char c;
  byte wxcheck = DONE;                 // used for type of data being processed (to get rid of the nested if else statements)
  unsigned short int st;
  unsigned short int en;
  uint32_t t;
  uint32_t gtimeout;
  int led = -1;                           // becomes the value for the string led currently being processed each loop

  int cnt = 0;    // used as index for
  currentAirport[cnt] = '\0';
  currentCondition[cnt] = '\0';
  currentWind[cnt] = '\0';
  currentGusts[cnt] = '\0';
  currentWxstring[cnt] = '\0';
  int lncnt = 0;
  currentLine[lncnt] = '\0';

  const char* rawText = &bigBlock[firstAvailable]; // Pointer for the next substring thats added to airport structure
  int counter = firstAvailable;       // start of wx raw text
  // AirportString =
  // {KBEH,KLWA,KAZO,KBTL,KRMY,KHAI,KIRS,KOEB,KJYM,KADG,KUSE,KTOL,KDUH,KTDZ,KPCW,KTTF,KARB,KYIP,KDTW,KONZ,KDET,KVLL,KMTC,KPHN,KD95,KPTK,KFNT,KRNP,KOZW,KTEW,KJXN,KFPK,
  //  KLAN,KY70,KGRR,KBIV,KMKG,KFFX,KRQB,KLDM,KMBL,KFKS,KCAD,KTVC,KACB,KCVX,KMGN,KPLN,KMCD,KSLH,KPZQ,KAPN,KOSC,KBAX,KCFS,KHYX,KMBS,KIKW,KAMN,KMOP,KY31,KHTL,KGOV,KGLR}
  Serial.print(F("GetMetars\t"));
  initWxClient();  // Create client if not exists (first boot only)

  // Only connect if not already connected. Reusing the existing SSL session
  // avoids the BearSSL handshake heap churn that causes fragmentation.
  if (!wxClient->connected()) {
    Serial.print(F("(Re)connecting SSL... "));
      // Must reset the BearSSL context — reconnecting on a stale object
      // causes BR_ERR_BAD_STATE ("unknown error") after server-initiated close
    resetWxClient();
    if (!wxClient->connect(WXSERVER, 443)) {
      char str[81]; str[0] = '\0';
      Serial.print(F("Connection failed! : ")); Serial.println(wxClient->getLastSSLError(str, 80)); Serial.println(str);
      // wxClient->stop();
      // delay(200);
      // my_yield();
      return false;
    }
    Serial.println(F("connected."));
  } else {
    Serial.println(F("Reusing SSL session."));
  }
  // if you get a connection, report back via serial:
  FastLED.clear();    // Clears leds[] reset only once for all group(s) of downloads
  for (int wxLoopCnt = 0; wxLoopCnt < noOfAirportStrings; wxLoopCnt++) {
    Serial.print(F("Getting data ")); Serial.print(wxLoopCnt + 1); Serial.print(F(" Of ")); Serial.println(noOfAirportStrings);
    led = -1;                 // becomes the value for the led in the string thats currently being processed Also use -1 for to flag the first loop
    // Note: Setting led -1 forces clear on first loop

    rawText = &bigBlock[counter]; // Pointer for substring to add to airport structure

    // For batch 2+, server may have closed the Keep-Alive connection. Reconnect if needed.
    if (!wxClient->connected()) {
      Serial.print(F("Reconnecting for batch ")); Serial.println(wxLoopCnt + 1);
      if (!wxClient->connect(WXSERVER, 443)) {
        Serial.println(F("Reconnect failed"));
        resetWxClient();
        //wxClient->stop();
        //delay(200);
        //my_yield();
        return false;
      }
    }

    Serial.println(F("Connected ..."));

#if WX_DEBUG
    Serial.print(F("GET "));
    Serial.print(BASE_URI);
    Serial.println(airportStrings[wxLoopCnt]);
    Serial.println(F(" HTTP/1.1"));
    Serial.print(F("Host: "));
    Serial.println(WXSERVER);
    Serial.println(F("User-Agent: LED Sectional Client"));
    Serial.println();
    Serial.flush();
    my_yield();
#endif


    wxClient->print(F("GET "));
    wxClient->print(BASE_URI);
    wxClient->print(airportStrings[wxLoopCnt]);
    wxClient->println(F(" HTTP/1.1"));
    wxClient->print(F("Host: "));
    wxClient->println(WXSERVER);
    wxClient->println(F("User-Agent: LED Sectional Client"));
    if (wxLoopCnt < (noOfAirportStrings - 1)) { // Request keep connection open
      Serial.println(F("Connection: Keep-Alive"));
      wxClient->println(F("Connection: Keep-Alive"));
    } else {                               // Request close connection
      Serial.println(F("Connection: close"));
      wxClient->println(F("Connection: close"));
    }
    wxClient->println();		// seems to be important does not get data without this
    wxClient->flush();

    gtimeout = millis() + (20 * 60 * 1000);
    while (!wxClient->available() && gtimeout > millis()) {
      my_yield();
      ;;
    }
/*
 * 								New station data NOTE: data does not have any cr or lf's cr's and lf's added manually
 * <METAR>
 * 	<raw_text>METAR KBEH 201853Z AUTO 00000KT 10SM OVC090 23/20 A3007 RMK AO2 RAB06E24 SLP180 P0003 T02330200 $</raw_text>
 * 	<station_id>KBEH</station_id><observation_time>2025-09-20T18:53:00.000Z</observation_time>
 * 	<latitude>42.1290</latitude>
 * 	<longitude>-86.4152</longitude>
 * 	<temp_c>23.3</temp_c>
 * 	<dewpoint_c>20</dewpoint_c>
 * 	<wind_dir_degrees>0</wind_dir_degrees>
 * 	<wind_speed_kt>0</wind_speed_kt>
 * 	<visibility_statute_mi>10+</visibility_statute_mi>
 * 	<altim_in_hg>30.07</altim_in_hg>
 * 	<sea_level_pressure_mb>1018</sea_level_pressure_mb>
 * 	<quality_control_flags>
 * 		<auto>TRUE</auto>
 * 		<auto_station>TRUE</auto_station>
 * 		<maintenance_indicator_on>TRUE</maintenance_indicator_on>
 * 	</quality_control_flags>
 * 	<sky_condition sky_cover="OVC" cloud_base_ft_agl="9000"/>
 * 	<flight_category>VFR</flight_category>
 * 	<precip_in>0.03</precip_in>
 * 	<metar_type>METAR</metar_type>
 * 	<elevation_m>196</elevation_m>
 * </METAR>
 *
 * <METAR>
 * 	<raw_text>METAR KGLR 201853Z AUTO 09007G15KT 10SM CLR 21/13 A3020 RMK AO2 SLP227 T02060133</raw_text>
 * 	<station_id>KGLR</station_id>
 * 	<observation_time>2025-09-20T18:53:00.000Z</observation_time>
 * 	<latitude>45.0166</latitude>
 * 	<longitude>-84.6894</longitude>
 * 	<temp_c>20.6</temp_c>
 * 	<dewpoint_c>13.3</dewpoint_c>
 * 	<wind_dir_degrees>90</wind_dir_degrees>
 * 	<wind_speed_kt>7</wind_speed_kt>
 * 	<wind_gust_kt>15</wind_gust_kt>
 * 	<visibility_statute_mi>10+</visibility_statute_mi>
 * 	<altim_in_hg>30.20</altim_in_hg>
 * 	<sea_level_pressure_mb>1022.7</sea_level_pressure_mb>
 * 	<quality_control_flags>
 * 		<auto>TRUE</auto>
 * 		<auto_station>TRUE</auto_station>
 * 	</quality_control_flags>
 * 	<sky_condition sky_cover="CLR"/>
 * 	<flight_category>VFR</flight_category>
 * 	<metar_type>METAR</metar_type>
 * 	<elevation_m>403</elevation_m>
 * 	</METAR>
 *
 * 	</data></response>Refreshing LEDs.
 *
 */

    //  FastLED.clear(); NOT HERE wipes out previous download

    /*
            Read from buffer Loop
    */
    short ic;		//	Not sure how Arduino handles chars do it the safe way
    //int overflow = 0;
    // Manual find - avoids Stream::find() hidden heap allocation
    {
      const char target[] = "METAR";
      const byte tlen = 5;
      byte matched = 0;
      uint32_t findTimeout = millis() + (READ_TIMEOUT * 1000);
      while (millis() < findTimeout) {
        if (wxClient->available()) {
          char fc = (char)wxClient->read();
          if (fc == target[matched]) {
            if (++matched == tlen) break;
          } else {
            matched = (fc == target[0]) ? 1 : 0;
          }
        } else {
          my_yield();
        }
      }
    }

    while (wxClient->available()) {
      if ((ic = wxClient->read()) >= 0){
    	  c = (char) ic;
    	  if (isAscii(c)) {
          my_yield(); // Otherwise the WiFi stack can crash
          if ( (lncnt - 2) < CURLINESIZE ){  // Add to buffer else Out of buffer run out data until endl
            currentLine[lncnt++] = c;
          } else {
        	  //Serial.print(F("Exceeded line size by ")); Serial.println(overflow++);
        	  Serial.println(F("Line LBOF"));
            lncnt = 0;              // Reset buffer to get additional as buffer already processed
        	  currentLine[lncnt] = '\0';
          }
          currentLine[lncnt] = '\0';			// Terminate next char in line for string functions
        if( '\n' == c || '\r' == c || cendsWith(currentLine, "</")){	// Ready for a new line
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
                  do {
                    if (strcmp_P(currentAirport, airports[st]) == 0) {
                      led = st;
                      st = en;    // FOUND IT! break
                    }
                    st++;         // increment even if found to force exit
                  } while (st <= en);
                }
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
          //wxClient->stop();
          resetWxClient();
          delay(200);
          my_yield();
          return false;
        }
    }	//client.read()
      int na = 500;     // I think I made the string search too fast loop finish b4 next char is received
      while (!wxClient->available() && na > 0) { // Give some time for the next char to be received
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
  return true;
} // getMetars()


/*

                            loopLEDSectional

*/
int loopLEDSectional() {
  // Heap guard - updated for persistent-SSL memory model:
  // BearSSL buffers are now kept allocated across cycles so MaxFreeBlockSize
  // will legitimately read ~6KB and is no longer a useful trigger.
  // Only reboot on true fragmentation (heap swiss-cheesed beyond recovery)
  // or if free heap is so low a crash is imminent regardless.
  uint32_t freeHeap = ESP.getFreeHeap();
  uint8_t  frag     = ESP.getHeapFragmentation();
  Serial.print(F("Heap: free=")); Serial.print(freeHeap);
  Serial.print(F(" frag=")); Serial.print(frag);
  Serial.print(F(" maxBlock=")); Serial.println(ESP.getMaxFreeBlockSize());
  if (frag > 60 || freeHeap < 3000) {
    Serial.print(F("Heap degraded, rebooting. frag="));
    Serial.print(frag); Serial.print(F(" free=")); Serial.println(freeHeap);
    boot_reason = b_MemoryFrag;
    set_softboot(true, &lightOffset, &boot_reason);
    my_reset();
  }

  retVal = 0;
  Serial.println(F("Getting METARs ..."));
  if (getMetars()) {
    Serial.println(F("Refreshing LEDs."));
    FastLED.show();
    if (lightningLedsCount > 0) Serial.println(F("There is lightning"));    // Nice to see there is lightning in the serial monitor at the end of Station output
    cycleCount++; totalCycleCount++;
    if(cycleErrCount > 0) cycleErrCount--;  // Give credit for good connections
  } else {
    retVal = loop_interval - RETRY_TIMEOUT;    // retVal will become loopcount Can't connect to server try again in 110 seconds (event happens when count = time)
    if (cycleErrCount > cycleCount) retVal = retVal - RETRY_TIMEOUT; // Longer delay for excessive errors
    cycleErrCount++; totalCycleErrCount++;  
  }
  if (retVal < 0) retVal = 0; // JIC
  if(cycleErrCount > CONNECTION_ERR_RRBOOT){
	  Serial.println(F("Too many connection errors forcing reset"));
    boot_reason = b_Connect;
	  set_softboot(true, &lightOffset, &boot_reason);	// save soft reboot flag
	  my_reset();
  }
  return retVal;      // new loop_time
} // loopLedSectional

#endif
