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
	                      "<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">"));
	//  server.sendContent(Config.hostName.c_str());
	  server.sendContent(hostname);

	  server.sendContent(F("</h2>\n"
	                      "<h3 align=\"center\"> Info </h3>"
			  	  	  	  "This is where the info should be put"
	                      "</body>"
	                      "</html>"));
	  server.client().stop();
	  //my_Event = MY_TEST;
}

#endif /* INFOPAGE_H_ */
