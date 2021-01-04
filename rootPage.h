#ifndef ROOTPAGE_INO
#define ROOTPAGE_INO 1


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
//  if(wxbusy) return;

  Serial.print(F("Hello from rootPage     \tClient IP = "));
  Serial.print(server.client().remoteIP().toString());
  Serial.print(F("\tWiFi SSID: ")); Serial.println(WiFi.SSID());
  Serial.print(F(" Light Sensor = ")); Serial.print(brightness);
  Serial.print(F(" rtmaxl = ")); Serial.print(rtmaxl);
  Serial.print(F("\t # WX stations = ")); Serial.println(actualNumAirports);

  // Serial.print(F("\tcurrentLineMax = ")); Serial.println(currentLineMax);

  Serial.print(F("\tUptime = ")); Serial.print(uptime());


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
  server.sendContent(hostname); server.sendContent(F(" on ")); server.sendContent(WiFi.SSID()); server.sendContent(F("</h1>"));

  server.sendContent("<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">\n"); server.sendContent(b2Scs(cycleCount)); //String(cycleCount).c_str());
  server.sendContent(" / "); server.sendContent(b2Scs(cycleErrCount)); //String(cycleErrCount).c_str());
  server.sendContent(" Light = "); server.sendContent(b2Scs(brightness)); //String(brightness).c_str());
  server.sendContent(" Adjust = "); server.sendContent(b2Scs(lightOffset)); //String(lightOffset).c_str());

  //Server.sendContent(F(" currentLineMax = ")); Server.sendContent(b2Scs(currentLineMax)); //String(currentLineMax).c_str());


  server.sendContent(F("</h2>\n<h6 align=\"center\" style=\"color.blue;margin:15px;\">"));
  server.sendContent(F( copyright)); server.sendContent(F(" ")); server.sendContent(F(compiledate));
  server.sendContent(F("</h6>\n"
                       "<h4 align=\"center\" style=\"color.blue;margin:15px;\">build ")); server.sendContent(F( compiledate));
  	  	  	  	  	   server.sendContent(F(" Uptime: ")); server.sendContent(uptime());
  	  	  	  	  	   server.sendContent(F(" Next update: ")); server.sendContent(b2Scs((loop_interval - loop_time) / 60));	// Minutes
  	  	  	  	  	   server.sendContent(F(":"));
  	  	  	  	  	   server.sendContent(b2Scs((loop_interval - loop_time) - (((loop_interval - loop_time) / 60) * 60)));		// Seconds
                       server.sendContent(F("</h4>\n<p><h4 align=\"center\"> # of WX Stations ")); server.sendContent(b2Scs(actualNumAirports));
                       server.sendContent(F(" MaxTxt Size = ")); server.sendContent(b2Scs(rtmaxl));
                       server.sendContent(F(" Total = ")); server.sendContent(b2Scs(total_C_StringLen));
                       server.sendContent(F(" OOM Count = ")); server.sendContent(b2Scs(ooMemCnt)); //String(currentLineMax).c_str());
                       server.sendContent("</h4><p>\n");
 server.sendContent(F( "<h5 align=\"center\"> "
                       "<a href=\"stationsL\">Stations: Desktop,</a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsW\"> Mobile Wide,</a></a>&nbsp;&nbsp;&nbsp;&nbsp;"
                       "<a href=\"stationsN\"> Mobile Narrow</a></h5>\n"
                       "<p></p><p style=\"padding-top:15px;text-align:center\">"
                       "<a href=\"test\"> Test</a></p>\n"));
  slider();
  server.sendContent(F("</body>\n"
                       "</html>\n"));
  server.client().stop();

#ifdef DEBUG
  showFree(true);
#endif

  return;
}

#endif
