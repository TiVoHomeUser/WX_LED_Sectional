#ifndef infoPage_ino
#define infoPage_ino "Jan 3, 2021"
void espInfo(){

// ESP.restart() restarts the CPU.
Serial.println();
Serial.print(F("RestartReason\t\t"));
Serial.println(ESP.getResetReason());       // returns a String containing the last reset reason in human readable format.

Serial.print(F("Free Heap size\t\t"));
Serial.println(ESP.getFreeHeap());          // returns the free heap size.

Serial.print(F("Heap Fragmentation\t"));
Serial.println(ESP.getHeapFragmentation()); // returns the fragmentation metric (0% is clean, more than ~50% is not harmless)

Serial.print(F("Max FreeBlockSize\t"));
Serial.println(ESP.getMaxFreeBlockSize());  // returns the largest contiguous free RAM block in the heap, useful for checking heap fragmentation.
											// NOTE: Maximum ``malloc()``able block will be smaller due to memory manager overheads.

Serial.print(F("Chip ID\t\t\t"));
Serial.println(ESP.getChipId());            // returns the ESP8266 chip ID as a 32-bit integer.

Serial.print(F("Core Version\t\t"));
Serial.println(ESP.getCoreVersion());       // returns a String containing the core version.

Serial.print(F("Sdk Version\t\t"));
Serial.println(ESP.getSdkVersion());        // returns the SDK version as a char.

Serial.print(F("CPU FreqMHz\t\t"));
Serial.println(ESP.getCpuFreqMHz());        // returns the CPU frequency in MHz as an unsigned 8-bit integer.

Serial.print(F("SketchSize\t\t"));
Serial.println(ESP.getSketchSize());         // returns the size of the current sketch as an unsigned 32-bit integer.

Serial.print(F("FreeSketchSpace\t\t"));
Serial.println(ESP.getFreeSketchSpace());   // returns the free sketch space as an unsigned 32-bit integer.

Serial.print(F("SketchMD5\t\t"));
Serial.println(ESP.getSketchMD5());         // returns a lower case String containing the MD5 of the current sketch.

Serial.print(F("FlashChipId\t\t"));
Serial.println(ESP.getFlashChipId());        // returns the flash chip ID as a 32-bit integer.

Serial.print(F("FlashChipSize\t\t"));
Serial.println(ESP.getFlashChipSize());     // returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).

Serial.print(F("FlashChipRealSize\t"));
Serial.println(ESP.getFlashChipRealSize()); // returns the real chip size, in bytes, based on the flash chip ID.

Serial.print(F("FlashChipSpeed\t\t"));
Serial.println(ESP.getFlashChipSpeed());    // returns the flash chip frequency, in Hz.

//ESP.getCycleCount() returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.

//ESP.random() should be used to generate true random numbers on the ESP. Returns an unsigned 32-bit integer with the random number. An alternate version is also available that fills an array of arbitrary length. Note that it seems as though the WiFi needs to be enabled to generate entropy for the random numbers, otherwise pseudo-random numbers are used.

Serial.print(F("FlashCRC\t\t"));
Serial.println(ESP.checkFlashCRC());   // calculates the CRC of the program memory (not including any filesystems) and compares it to the one embedded in the image. If this call returns false then the flash has been corrupted. At that point, you may want to consider trying to send a MQTT message, to start a re-download of the application, blink a LED in an SOS pattern, etc. However, since the flash is known corrupted at this point there is no guarantee the app will be able to perform any of these operations, so in safety critical deployments an immediate shutdown to a fail-safe mode may be indicated.

Serial.print(F("getVcc\t\t\t"));
Serial.println(ESP.getVcc());          // may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. Add the following line to the top of your sketch to use getVcc:

}
void infoPage(){
	espInfo();
	server.send(200, "text/html", goBack);
}
#endif
