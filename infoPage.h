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

		Serial.print(F("LED_STR_DATA_PIN: ")); Serial.println(LED_STR_DATA_PIN);
		Serial.print(F("LED_BUILTIN: ")); Serial.println(LED_BUILTIN);
		Serial.print(F("Version: ")); Serial.println(VERSION);

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
												"<h1 align=\"center\"> <a href=\"/\">"));
	  										server.sendContent(hostname);
												server.sendContent(F("</a></h1>\n"
              	  	  	"<h3 align=\"center\"> Info </h3>"
	                      "<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">"));
    server.sendContent(F("<p>#WX Stations: "));

      server.sendContent(b2Scs(actualNumAirports));
      server.sendContent(F("<BR> MaxTxt Size = ")); server.sendContent(b2Scs(rtmaxl));
      server.sendContent("<BR>");
      server.sendContent(F(" Total = ")); server.sendContent(b2Scs(total_C_StringLen));
      server.sendContent("<BR>");
      server.sendContent(F(" OOM Count = ")); server.sendContent(b2Scs(ooMemCnt)); //String(currentLineMax).c_str());
      server.sendContent("<BR>");
      server.sendContent(F(" Connects "));

      server.sendContent(b2Scs(cycleCount));
      server.sendContent(F(" / ")); server.sendContent(b2Scs(cycleErrCount));

      server.sendContent(F(" Total "));
      server.sendContent(b2Scs(totalCycleCount));
      server.sendContent(F(" / ")); server.sendContent(b2Scs(totalCycleErrCount));
			server.sendContent(F("<BR>Last boot: "));
				switch(boot_reason){
					case b_Hard:
						server.sendContent(F("Hard Reset"));
						break;
					case	b_MemoryFrag:
						server.sendContent(F("Memory Frag"));
						break;
					case	b_init_MemFrag:
						server.sendContent(F("Boot Memory Frag"));
						break;
					case b_Connect:
						server.sendContent(F("Connection Errors"));
						break;
					case b_html:
						server.sendContent(F("Web Page "));
  				case	b_RebootTest:
						server.sendContent(F("Re-"));
					case	b_BootTest:
						server.sendContent(F("Boot Test"));
					break;
			}
			
			server.sendContent(F("<P>"));

#if defined(ESP8266)
  server.sendContent(F("Chip: ESP8266<BR>"));
#elif defined(ESP32)
  server.sendContent(F("Chip: ESP32<BR>"));
/* TODO
  server.sendContent("SDK Version: %s\n", ESP.getSdkVersion());
  server.sendContent("Partition Info:");
  const esp_partition_t* running = esp_ota_get_running_partition();
	server.sendContent("Running Partition Size: %u bytes\n", running->size);
*/

#endif
  server.sendContent(F("CPU Frequency: ")); server.sendContent(b2UScs(ESP.getCpuFreqMHz())); server.sendContent(F(" MHz<BR>"));
	server.sendContent(F("Flash Chip Real Size: ")); server.sendContent(formatBytes(ESP.getFlashChipSize()));	server.sendContent(F("<BR>"));

      server.sendContent(F(" Free = "));
			server.sendContent(formatBytes(platform_max_free_block()));
      server.sendContent(F("<BR>"));

      server.sendContent(F("\n\tFree Heap = "));
			server.sendContent(formatBytes(platform_free_heap()));
      server.sendContent(F("<BR>"));
      	
			server.sendContent(F("\tHeapFragmentation = ")); server.sendContent(b2cs(platform_heap_frag_pct())); //uint8_t
			server.sendContent(F("<BR>"));
			server.sendContent(F("\tLED String data pin = ")); server.sendContent(b2cs(LED_STR_DATA_PIN)); //uint8_t
			server.sendContent(F("\tBIL = ")); server.sendContent(b2cs(LED_BUILTIN)); //uint8_t
      
      server.sendContent(F("</h2>\n"
	                      "</body>"
	                      "</html>"));
	  server.client().stop();
}

#endif /* INFOPAGE_H_ */
