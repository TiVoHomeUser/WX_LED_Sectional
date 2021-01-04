#ifndef stationPage_ino
#define stationPage_ino "Jan 2, 2021"

/*
 *
 *        Converts the airport conditions stored as a integer to the airport condition String
 *        and for a bonus appends the wind conditions to the string
 *
 *
 */
static char metr_Retval[16];
char* metr_cond(unsigned short int airportnumber){
  metr_Retval[0] = '\0';
  // add metr condition
  //  mtrstat 1 VFR, 2 MVFR, 3 LIFR, 4 IFR 0 unknown 99 NULL maybe use leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
  switch(mtrsf[airportnumber].mtrstat){
    case 0:
      strcat( metr_Retval, "Not Reporting"); //"Unknown";
      return metr_Retval;
      break;
    case 1:
      strcat( metr_Retval, "VFR");
      break;
    case 2:
      strcat( metr_Retval, "MVFR");
      break;
    case 3:
      strcat( metr_Retval, "LIFR");
      break;
    case 4:
      strcat( metr_Retval, "IFR");
      break;
    default:
      strcat( metr_Retval, "????");
  }
  strcat( metr_Retval, " ");
  strcat( metr_Retval, b2cs(mtrsf[airportnumber].mtrspeed));                // mtrspeed wind in knots
  if( mtrsf[airportnumber].mtrgusts > mtrsf[airportnumber].mtrspeed) {      // Display gust speed in button even if it is less then WIND_THRESHOLD
    strcat( metr_Retval, " G");
    strcat( metr_Retval, b2cs(mtrsf[airportnumber].mtrgusts));
  }
  strcat( metr_Retval, " kts");

  return metr_Retval;
} //metrcnd(int)

/*
 * 
 *                                              Airport button page 
 *                              Button press identifies airport by flashing it's LED 
 *                              
 *
 *                                lEDButton()
 *
 *
 */
static const unsigned int lB_RetvalMax = (342 + RTMAXLN); // 342 + Raw Text maxlength
static char lB_Retval[lB_RetvalMax]; //[320]; //[275];    // note lighting "ϟϟ " adds 30 bytes "☇☇☇ " adds 44 bytes also allow for stations# > 99
char* lEDButton(byte airportnumber){ //unsigned short int airportnumber){
  lB_Retval[0] = '\0';
  strcat(lB_Retval, "<td><a style=\"color:");
  strcat(lB_Retval, CRGBtoHex(leds[airportnumber])); //.c_str());
  if( mtrsf[airportnumber].mtrgusts > WIND_THRESHOLD || mtrsf[airportnumber].mtrspeed > WIND_THRESHOLD) {   // Yellow text is hard to read so shadow all around the chars
    strcat(lB_Retval, ";text-shadow: -1px -1px #000000, 1px -1px #000000, -1px 1px #000000, 1px 1px #000000"); //; \">";                                                 // +84 chars
  }
  strcat(lB_Retval, "; \">");
    if(mtrsf[airportnumber].mtrlighting) strcat_P(lB_Retval, lightingSymb); // "&#9889;&#9889; "); // "ϟϟ ");                                                                                // +15 chars
    strcat( lB_Retval, "<div class=\"tooltip\">");
    strcat_P( lB_Retval, airports[airportnumber]); //.c_str());
    if(mtrsf[airportnumber].mtrlighting) strcat_P( lB_Retval,  lightingSymb ); //" &#9889;&#9889;"); //" ϟϟ" &#9889; High Voltage (looks light a lighting bolt)                             // +15 chars
    strcat( lB_Retval, "<span class=\"tooltiptext\">");
    strncat( lB_Retval, mtrsf[airportnumber].rawText, RTMAXLN);
    strcat( lB_Retval, "</span></div>");
    strcat( lB_Retval, " <button ");
    strcat( lB_Retval, " type=\"submit\" name='wxid' value='");
    strcat( lB_Retval, b2cs(airportnumber));
    strcat( lB_Retval, "' formmethod=\"get\">");   //?fname=value1&name2=value2 > \n";
    // add metr condition
    //  mtrstat 1 VFR, 2 MVFR, 3 LIFR, 4 IFR 0 unknown 99 NULL maybe use leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
    strcat( lB_Retval, metr_cond(airportnumber) );
    strcat( lB_Retval, " ");
    strcat( lB_Retval, "</button>");
    strcat( lB_Retval, "</a></td>\n");

    return lB_Retval;
}   // lEDButton(airportnumber)


/*
 *                              Station Page (int)
 *                              call with number of to display in the table of stations
 */
void stationPage(int columns){
//   if(wxbusy) return;
  Serial.print(F("Hello from Stations ")); 
 
  Serial.print(columns); Serial.print(F(" x ")); Serial.print( (int) ceil(actualNumAirports / columns));
  Serial.print(F("\tClient IP = ")); Serial.println(server.client().remoteIP().toString());
  Serial.print(F("\tActual Num Airports = ")); Serial.print(actualNumAirports);
  
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send_P( 200, "text/html", htmlHeadStr);
  server.sendContent_P(htmlReloadStr);
  /*                            *****  Allready Pre loaded in htmlStr *****
   *  "<!DOCTYPE html> <html>\n"
   *  "<head>\n"
   *  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
   *  "<title>WX_Sectional</title>\n"
   *  "<script type=\"text/javascript\">setTimeout(\"location.reload()\",120000);</script>\n";
   */
  server.sendContent(F("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
                     "table.center { margin-left:auto; margin-right:auto; }"
                     "table, th, td { border: 1px solid black; }\n"
                     "button {display: block;width: 180px;background-color: #34495e;border: none;color: white;padding: 13px 30px;text-decoration:"
                     "none;font-size: 15px; margin: 0px auto 25px;cursor: pointer;border-radius: 4px;}\n"
                     ".tooltip {\n position: relative;\n display: inline-block;\n border-bottom: 1px\n dotted black;\n }\n"
                     ".tooltip .tooltiptext {\n visibility: hidden;\n width: 180px;\n background-color: black;\n color: #fff;\n text-align: center;\n border-radius: 6px;\n padding: 5px 0;\n position: absolute;\n z-index: 1;\n bottom: 150%;\n left: 25%;\n margin-left: -60px;\n }\n"
                     ".tooltip .tooltiptext::after {\n content: \"\";\n position: absolute;\n top: 100%;\n left: 50%;\n margin-left: -5px;\n border-width: 5px;\n border-style: solid;\n border-color: black transparent transparent transparent;\n }\n"
                     ".tooltip:hover .tooltiptext {\n visibility: visible;\n }\n"
                     "</style>\n"
                     "</head>\n"
                     "<body>"
                     "<h1><a href=\"/\">ESP8266 WX_Sectional LED</a></h1>\n"
                     "<h3>"));
  if(lightningLeds.size() > 0) server.sendContent_P(lightingSymb);   // Display Lighting symbols in page header
  server.sendContent(F(" Stations "));
  if(lightningLeds.size() > 0) server.sendContent_P(lightingSymb);   // Display Lighting symbols in page header
  server.sendContent(F( "</h3>\n"
                        "<form action=\"/wxID\">\n"
                        "<table class=\"center\" border=\"3\">\n"
                        "\n<tr>"));
  int row = 1; // int maxrow = 3; // iphone 3 macbook 8;
  for(int i=0; i < NUM_AIRPORTS; i++){
    if( mtrsf[i].mtrstat < 99 ){    // 99 for NULL location skip it
      server.sendContent(lEDButton(i));
      row++;
      if(row > columns){
        row = 1;
        server.sendContent(F("</tr>\n<tr>"));
      }
    } // else { Serial.print("Null station found "); Serial.println(i); }
  };
  server.sendContent(F("</tr>\n"
                       "</table>"
                       "</form>\n"
                       "</body>\n"
                       "</html>\n"));
  server.client().stop();

#ifdef DEBUG
  showFree(true);
#endif

  return; 
}   // stationPage()


/*
 *   TODO create a jump page that determins screen size the call with option that cover width
 *
 */

void stationPage0(){
 stationPage((int) 999);
 loop_time = loop_interval - 15;
}
void stationPage8x8() {  stationPage((int) 8); }   // For a Desktop large screen device
void stationPage4x16(){  stationPage((int) 4); }   // For a Mobile device horz
void stationPage2x32(){  stationPage((int) 2); }   // For a Mobile device vert




#endif
