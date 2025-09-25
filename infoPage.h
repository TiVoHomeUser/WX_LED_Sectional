/*
 * infoPage.h
 *
 *  Created on: Jun 26, 2024
 *      Author: imac
 */

#ifndef INFOPAGE_H_
#define INFOPAGE_H_

void infoPage(void){

	  Serial.print(F("Info Page     \tClient IP = "));
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
              	  	  	  "<h3 align=\"center\"> Info </h3>"
	                      "<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">"));
	//  server.sendContent(Config.hostName.c_str());
	  server.sendContent(hostname);
      server.sendContent(F("<p>#WX Stations "));

      server.sendContent(b2Scs(actualNumAirports));
      server.sendContent(F(" MaxTxt Size = ")); server.sendContent(b2Scs(rtmaxl));
      server.sendContent("\n");
      server.sendContent(F(" Total = ")); server.sendContent(b2Scs(total_C_StringLen));
      server.sendContent("<BR>");
      server.sendContent(F(" OOM Count = ")); server.sendContent(b2Scs(ooMemCnt)); //String(currentLineMax).c_str());
      server.sendContent("<BR>");
      server.sendContent(F(" Free = ")); server.sendContent(b2UScs(ESP.getMaxFreeBlockSize()));
      server.sendContent("<BR>");
      server.sendContent(F(" Connects "));
      server.sendContent(b2Scs(cycleCount)); //String(cycleCount).c_str());
      server.sendContent(F(" / ")); server.sendContent(b2Scs(cycleErrCount)); //String(cycleErrCount).c_str());
      server.sendContent("<BR>");
showFree(true);
      //if ( force || (ESP.getHeapFragmentation() >= 50) || (ESP.getFreeHeap() < 1000) ) {
        server.sendContent(F("\n\tFree Heap = ")); server.sendContent(b2UScs(ESP.getFreeHeap()));	//uint32_t
        server.sendContent("<BR>");
       server.sendContent(F("\tHeapFragmentation = ")); server.sendContent(b2cs(ESP.getHeapFragmentation())); //uint8_t
       server.sendContent("<BR>");
        server.sendContent(F("\tMaxFreeBlockSize = ")); server.sendContent(b2UScs(ESP.getMaxFreeBlockSize())); //uint16_t
        // Serial.print("Free = "); Serial.println(ESP.freeMemory());
      //}

      server.sendContent(F("</h2>\n"
//	                      "<h3 align=\"center\"> Info </h3>"
	                      "</body>"
	                      "</html>"));
	  server.client().stop();
	  //my_Event = MY_TEST;
}

#endif /* INFOPAGE_H_ */
