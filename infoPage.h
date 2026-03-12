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
	  server.sendContent(hostname);
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
  server.sendContent(F("CPU Frequency: ")); server.sendContent(b2UScs(ESP.getCpuFreqMHz())); server.sendContent(F(" MHz<BR>"));
	server.sendContent(F("Flash Chip Real Size: ")); server.sendContent(formatBytes(ESP.getFlashChipSize()));	server.sendContent(F("<BR>"));
#elif defined(ESP32)

  server.sendContent("Chip: ESP32");
/* TODO
  server.sendContent("CPU Frequency: %u MHz\n", getCpuFrequencyMhz());
  server.sendContent("Flash Chip Size: %u bytes\n", ESP.getFlashChipSize());
  server.sendContent("Free Heap: %u bytes\n", ESP.getFreeHeap());
  server.sendContent("SDK Version: %s\n", ESP.getSdkVersion());
  server.sendContent("Partition Info:");
  const esp_partition_t* running = esp_ota_get_running_partition();
	server.sendContent("Running Partition Size: %u bytes\n", running->size);
*/

#endif

      server.sendContent(F(" Free = ")); // server.sendContent(b2UScs(ESP.getMaxFreeBlockSize()));
			server.sendContent(formatBytes(ESP.getMaxFreeBlockSize()));
      server.sendContent(F("<BR>"));

      server.sendContent(F("\n\tFree Heap = "));// server.sendContent(b2UScs(ESP.getFreeHeap()));	//uint32_t
			server.sendContent(formatBytes(ESP.getFreeHeap()));
      server.sendContent(F("<BR>"));
      	
			server.sendContent(F("\tHeapFragmentation = ")); server.sendContent(b2cs(ESP.getHeapFragmentation())); //uint8_t
			server.sendContent(F("<BR>"));
      
      server.sendContent(F("</h2>\n"
  	  	  	  "<h1 align=\"center\"> <a href=\"/\">ESP8266 WX_Sectional LED</a>\n </h1>"

//	                      "<h3 align=\"center\"> Info </h3>"
	                      "</body>"
	                      "</html>"));
	  server.client().stop();
	  //my_Event = MY_TEST;
}

#endif /* INFOPAGE_H_ */
