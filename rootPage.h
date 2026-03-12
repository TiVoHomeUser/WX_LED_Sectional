#ifndef ROOTPAGE_INO
#define ROOTPAGE_INO 1

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
    server.sendContent(F("\" max=\"127\" min=\"-128\" >\n"
                         "<br>\n"
                         "<input type=\"submit\">\n"
                         "</form>\n"
                         "<output form=\"numform\" id=\"x\" name=\"x\" for=\"b\"></output><br>\n </h3>"));
}

/*
 *
 *                                                      Slider Page
 *                            Called by a HTML page get for program to progress the slider values
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
    // FastLED.setBrightness(svalue);   // scale  a 0-255 value for how much to scale all LEDs before writing them out
    lightOffset = svalue;   // Slide value -128 ... 127
  }
  //const static char goBack[] PROGMEM = "<!DOCTYPE html> <script language=\"JavaScript\" type=\"text/javascript\"> setTimeout(\"window.history.go(-1)\",10); </script>";
  server.send(200, "text/html", goBack);    // goBack string stored in PROGMEM declared in WX_Sectional.h A it is used in 2 functions iDLED and Slider_page
  server.client().stop();
}

/*
 *
 *
 *                                                Main page
 *                          I.E. http://wx_sectional.local/  or by IP address if mDNS is not available
 *                          Test button cycles the led through basic colors
 *                          Slider at bottom of page adjusts brightness of all LEDS
 *
 *
 */

void rootPage(){
  Serial.print(F("Hello from rootPage     \tClient IP = "));
  Serial.print(server.client().remoteIP().toString());
  Serial.print(F("\tWiFi SSID: ")); Serial.print(WiFi.SSID());
  Serial.print(F(" Light Sensor = ")); Serial.print(brightness);
  Serial.print(F(" rtmaxl = ")); Serial.print(rtmaxl);
  Serial.print(F("\t # WX stations = ")); Serial.println(actualNumAirports);
  Serial.print(F("\tUptime = ")); Serial.print(uptime());
  Serial.print(F(" C/E ")); Serial.print(cycleCount); Serial.print(F(" / ")); Serial.print(cycleErrCount);
  Serial.print(F(" Total ")); Serial.print(totalCycleCount); Serial.print(F(" / ")); Serial.println(totalCycleErrCount);

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send_P( 200, "text/html", htmlHeadStr);
  server.sendContent_P(htmlReloadStr);
  /*                            *****  Already Pre-loaded in htmlStr *****
   *  "<!DOCTYPE html> <html>\n"
   *  "<head>\n"
   *  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
   *  "<title>WX_Sectional</title>\n"
   *  "<script type=\"text/javascript\">setTimeout(\"location.reload()\",120000);</script>\n";
   */
  server.sendContent(F("</head><body><h1 align=\"center\" style=\"color:lighttgray;margin:20px;\">\n" )); //<h6 \"color: #DFF2FB;\">\n")); // h6 {color: #DFF2FB;}\n"

  if(lightningLedsCount > 0)
	  server.sendContent_P(lightingSymb);
  server.sendContent(hostname); /* server.sendContent(F(" on ")); server.sendContent(WiFi.SSID()); */
  if(lightningLedsCount > 0)
	  server.sendContent_P(lightingSymb);
  server.sendContent(F("</h1>"));

  server.sendContent(F("<h6 align=\"center\" style=\"color.blue;margin:15px;\">"));
  server.sendContent(F( copyright)); server.sendContent(F(" ")); server.sendContent(F(compiledate));
  server.sendContent(F("</h6>\n"
                       "<h4 align=\"center\" style=\"color.blue;margin:15px;\">")); //build ")); server.sendContent(F( compiledate));
  	  	  	  	  	  server.sendContent(uptime());

	  	   	   	   	   server.sendContent(F(" Next update: ")); server.sendContent(b2Scs((loop_interval - loop_time) / 60));	// Minutes
  	  	  	  	  	   server.sendContent(F(":"));
  	  	  	  	  	   server.sendContent(b2Scs((loop_interval - loop_time) - (((loop_interval - loop_time) / 60) * 60)));		// Seconds
                       if(cycleErrCount > 0){
                    	server.sendContent(F(" Conn/Err: "));
                        server.sendContent(b2Scs(cycleCount)); //String(cycleCount).c_str());
                        server.sendContent(F(" / ")); server.sendContent(b2Scs(cycleErrCount)); //String(cycleErrCount).c_str());
                       }
                       server.sendContent(F("</h4><p>\n"));
#if INFO_PAGE
 server.sendContent(F( "<h5 align=\"center\"> "
                       "<a href=\"stationsL\">Stations: Desktop,</a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsW\"> Mobile Wide,</a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsN\"> Mobile Narrow</a></h5>\n"
                       "<p></p><p style=\"padding-top:15px;text-align:center\">"
                       "<a href=\"test\"> Test</a>"
                       "<a href=\"info\">  &#x24D8;</a></p>\n"   // (i)
		 ));
#else
 server.sendContent(F( "<h5 align=\"center\"> "
                       "<a href=\"stationsL\">Stations: Desktop,</a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsW\"> Mobile Wide,</a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsN\"> Mobile Narrow</a></h5>\n"
                       "<p></p><p style=\"padding-top:15px;text-align:center\">"
                       "<a href=\"test\"> Test</a></p>\n"
		 ));
#endif
                       
#if USE_LIGHT_SENSOR
  server.sendContent(F("<h4 align=\"center\" style=\"color:lighttgray;margin:20px;\">\n"));
  server.sendContent(F(" Light = ")); server.sendContent(b2Scs(brightness)); //String(brightness).c_str());
  server.sendContent(F(" Adjust = ")); server.sendContent(b2Scs(lightOffset)); //String(lightOffset).c_str());
  server.sendContent(F("</h4>"));
#endif // USE_LIGHT_SENSOR
  slider();
  server.sendContent(F("</body>\n</html>\n"));
  server.client().stop();
  return;
}

#endif
