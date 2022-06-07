#ifndef LEDTest_ino
#define LEDTest_ino Jan 1, 2021

/*
 *                                          testPage()
 *                                  Cycle ALL LEDS for visual check accessed through network
 *                                  I.E. http://wx_sectional.local/test or by IP address if mDNS is not available
 *
 */
void testPage(void){
  Serial.print(F("Test Page     \tClient IP = "));
  Serial.print(server.client().remoteIP().toString());
  Serial.print(F("\tWiFi SSID: ")); Serial.println(WiFi.SSID());

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send_P( 200, "text/html", htmlHeadStr);


  /*                            *****  Allready Pre loaded in htmlStr *****
   *  "<!DOCTYPE html> <html>\n"
   *  "<head>\n"
   *  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
   *  "<title>WX_Sectional</title>\n"
   *  "<script type=\"text/javascript\">setTimeout(\"location.reload()\",120000);</script>\n";
   */
  server.sendContent(F("<script type=\"text/javascript\"> setTimeout(\"location.replace(location.origin)\", 60000); </script>\n"  // 1000 = 1 Sec test takes about 20 seconds
                      "</head>\n"
                      "<body>"
                      "<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">"));
//  server.sendContent(Config.hostName.c_str());
  server.sendContent(hostname);

  server.sendContent(F("</h2>\n"
                      "<h3 align=\"center\"> LED Test in progress... Please Wait 60 Seconds</h3>"
                      "</body>"
                      "</html>"));
  server.client().stop();
  my_Event = MY_TEST;
} // testPage()

/*
 *
 *                                                                  ID LED
 *                                          Called by WEB with wxid argument for the led to ID (flash)
 *
 */
void iDLED(void){
//if(server.arg("wxid").toInt() == NULL) Serial.println("NULL");
//TODO Check for valid integer invalid chars return int 0
  if(server.hasArg("wxid")){
	unsigned short airportnumber = server.arg("wxid").toInt();
	if(airportnumber < NUM_AIRPORTS) {
    Serial.print(F("Hello iDLED called LED "));
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send_P( 200, "text/html", htmlHeadStr);
    server.sendContent(F("<script type=\"text/javascript\"> setTimeout(\"window.history.go(-1)\", 25000); </script>\n"   // 1000 = 1 Sec test takes about 9 seconds
                       "</head>\n"
                        "<body>"
                        "<h2 align=\"center\" style=\"color:black;margin:20px;\">"));
    server.sendContent(hostname);
    server.sendContent(F("</h2>\n"
    					"<h3 align=\"center\" style=\"color:"));
							server.sendContent(CRGBtoHex(leds[airportnumber]));
    server.sendContent(F(";margin:5px;\">\n"
    					"<br><br>"));
    server.sendContent(mtrsf[airportnumber].rawText);	// Display raw data because tooltip sometimes doesn't work or is hard to use
    server.sendContent(F("<br><br></h3>\n"
    					"<h4 align=\"center\" style=\"color:lightgray;margin:10px;\">LED ID: "));
    server.sendContent_P(airports[airportnumber]);
    server.sendContent(F(" in progress... Please Wait</h4>"
                        "</body>"
                        "</html>"));
    server.client().stop();
    Serial.println(server.arg("wxid"));
    idLED(server.arg("wxid").toInt());
  } else {
    Serial.println(F("N/A"));
    handleNotFound(); // FWTGBITN
  } // valid argument check
 } // has argument
}

/*
 *                      Force a reboot by setting high error count
 *                      http://wx_sectional.local/reboot
 */
void rebootPage(void){
Serial.println("Reboot page");
  loop_time = loop_interval - 15;    // Force reboot in 15 seconds
  cycleErrCount = 99;
  server.send(200, F("text/html"), goBack);    // goBack string stored in PROGMEM declared in WX_Sectional.h A it is used in 2 functions iDLED and Slider_page
  server.client().stop();
}

#endif
