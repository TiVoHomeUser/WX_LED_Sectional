#ifndef util_ino
#define util_ino 2

/*
 * 														Vic Utils
 *
 *                VicUtils © copyright 2020 Victor Wheeler myapps@vicw.net anyone is free to use just not restrict.
 *
 */

/*
 *
 *     			instead of using delay() use the timer for loop
 *
 *
 */
static const unsigned long secondIntervill = 1000;
// const unsigned long minuteIntervill = secondIntervill * 60;
// const unsigned long hourIntervill = minuteIntervill * 60;
// const unsigned long dayIntervill = hourIntervill * 24;

static unsigned long currentmills = secondIntervill;  	// force 1 second timer to do it on first loop
static unsigned long lastmills = 0;
static int m_overflow=0;								// Counter for the 49 day millis() overflow bug

boolean timeElapsed() {
  currentmills = millis();
  if(lastmills > currentmills){		// check for unsigned long overflow allows for more than 49 days up-time
	  m_overflow++;
	  lastmills = 0;				// start over loose any remainder from previous check
  }

  if ((currentmills - lastmills) >= secondIntervill) {
    lastmills = currentmills;
    return true;
  }
  return false;
}

// #define my_yield() {ESP.wdtFeed(); yield();}     // reported problems with yield() not always reseting the software watchdog timer
// my_yield():
//   ESP8266: defined in platform.h as {ESP.wdtFeed(); yield();}
//   ESP32:   defined in platform.h as {yield();}  (FreeRTOS handles the watchdog)
// The #define is already set by platform.h before this file is included.
#if !defined(my_yield)
 #define my_yield() {ESP.wdtFeed(); yield();}     // reported problems with yield() not always reseting the software watchdog timer
#endif

/*
 *
 *                  Convert signed short to Signed c-string
 *            minimum value of -32,768 and a maximum value of 32,767
 *
 */
static char buf[7];
char* b2Scs(signed short b){
  buf[0]='\0';
  itoa(b, buf,10);
  return buf;
}

/*
 *
 *                  Convert Unsigned short to Signed c-string
 *            minimum value of 0 and a maximum value of 65,535
 *
 */
char* b2UScs(unsigned short b){
  buf[0]='\0';
  itoa(b, buf,10);
  return buf;
}

/*
 *
 *                  Convert a byte to c-string (char[]) handy for strcat function
 *                  return:
 *                    null terminated c-string  "0" .. "255"
 *
 */
//static char buf[4]; reuse b2Scs buf
char* b2cs(byte b){
  buf[0]='\0';
  itoa(b, buf,10);
  return buf;
}

/*
 *
 *                  Format unsigned long to Gbytes, Kbytes, Mbytes, or bytes
 */
// TODO optimize to minimize ram memory usage sprintf ?
// Maybe formatted output not needed if consuming too much resorces 
 static char buffer[25];
char* formatBytes(unsigned long bytes) {
    buffer[0]='\0';
    const char *units[] = {"bytes", "Kbytes", "Mbytes", "Gbytes"};
    int i = 0;
    double dBytes = (double)bytes;

    // Determine the unit (K, M, G)
    while (dBytes >= 1024 && i < 3) {
        dBytes /= 1024;
        i++;
    }
    // Format with 2 decimal places and append unit
    snprintf(buffer, sizeof(buffer), "%.2f %s", dBytes, units[i]);
    return buffer;
}

/*
 *
 *                  Convert byte to 2 char Hex c-string without memory hungry printf
 *
 */
static char hexadecimalnum[3];
char* byteToHex(byte b){
   hexadecimalnum[0] = '0';
   hexadecimalnum[1] = '0';
   hexadecimalnum[2] = '\0';
   int quotient = b;
   int remainder = 0;
   int j=1;
    while (quotient != 0) {
        remainder = quotient % 16;
        if (remainder < 10){
            hexadecimalnum[j--] = (char) 48 + remainder; //48 + remainder; 48 = '0'
        } else {
            hexadecimalnum[j--] = (char) 55 + remainder; //55 + remainder;  65 = 'A'
        }
        quotient = quotient / 16;
    }
    return hexadecimalnum;
}

/*
 *
 *          Optimized C string Replacement for the String endsWith() function
 *
 */
boolean cendsWith(const char* str1 , const char* str2){
  int len1=strlen(str1);
  int len2=strlen(str2);
  boolean retval = true;
  int i = 1;  // cString index starts at 0 cstring[len] is out of range
  if(len1  < len2){ // sanity check improves performance
    retval = false;
  } else {
    while(i <= len2 && retval == true){
        if(str1[len1-i] != str2[len2-i]) retval = false;
        i++;
    }
  }
  return retval;
}

/*

                                        Function for tracking memory usage
                                   force: false only display if low or fragmented memory
                                          true force display

*/
void showFree(boolean force) {
  uint8_t  frag  = platform_heap_frag_pct();
  uint32_t free  = platform_free_heap();
  uint32_t block = platform_max_free_block();
  if ( force || (frag >= 50) || (free < 1000) ) {
    Serial.print(F("\n\tFree Heap = "));       Serial.print(free);
    Serial.print(F("\tHeapFragmentation = ")); Serial.print(frag);
    Serial.print(F("\tMaxFreeBlockSize = "));  Serial.println(block);
  }
}


/*
 *                                        Uptime()
 *                  Compute uptime return as a cstring days : hours : minutes : seconds
 *                  Note: Max Uptime counter is 49 days 17 hours 2 minutes and 47 Seconds
 *                  Update: 12/16 added 49 day overflow counter so should work for 999 * 49 days (Like that is going to happen)
 */
#define INCLUDE_MIN_SEC false	// format return string true include minutes and seconds

static unsigned long m_secs, m_mins;
static unsigned int m_hours, m_days;

#if INCLUDE_MIN_SEC
	static char m_uptimeCstr[] = "Uptime (D:H:M:S) 365:23:59:59"; // max value for millis() is 4,294,967,295 or 49.71 days
#else
	static char m_uptimeCstr[] = "Uptime (D:H) 365:23"; // max value for millis() is 4,294,967,295 or 49.71 days
#endif

 char* uptime(){
  // moved to timeElapsed()  if(m_lastvalue > millis()) m_overflow++;
  m_secs=millis() /1000;                    // Convert to seconds
  m_secs = m_secs + (4294967 * m_overflow); // add 49 days 17 hours 2 minutes and 47 seconds for each overflow
                                            // 47 seconds =         47 seconds
                                            // 2  minutes =        120 seconds
                                            // 17 hour    =      61200 seconds
                                            // 49 days    =    4233600 seconds
                                            // 49:17:2:47 =    4294967 seconds
                                            // ulong max  = 4294967295 good for 999 49 day overflow counts

  m_mins =  m_secs / 60; m_hours = m_mins / 60; m_days = m_hours/24;
  m_secs -= m_mins * 60; m_mins -= m_hours * 60; m_hours -= m_days*24;
  m_uptimeCstr[0] = '\0';

#if INCLUDE_MIN_SEC
//  static char m_uptimeCstr[31] = "\0ptime (D:H:M:S) 365:23:59:59"; // max value for millis() is 4,294,967,295 or 49.71 days
  strcat(m_uptimeCstr, "Uptime (D:H:M:S) ");
#else
//  static char m_uptimeCstr[21] = "\0Uptime (D:H) 365:23"; // max value for millis() is 4,294,967,295 or 49.71 days
  strcat(m_uptimeCstr, "Uptime (D:H) ");
#endif

  strcat(m_uptimeCstr, b2cs(m_days));  strcat(m_uptimeCstr, ":");

#if INCLUDE_MIN_SEC //modified to remove minutes ad seconds.
  strcat(m_uptimeCstr, b2cs(m_hours)); strcat(m_uptimeCstr, ":");
  strcat(m_uptimeCstr, b2cs(m_mins));  strcat(m_uptimeCstr, ":");
  strcat(m_uptimeCstr, b2cs(m_secs));  //strcat(m_uptimeCstr, ":");
#else
  strcat(m_uptimeCstr, b2cs(m_hours));
#endif
  return m_uptimeCstr;
 }

  // Cycle built in LED to show we are alive
  void toggleBuiltInLED(){
    static boolean on_Board_LED_state = false;        // Toggle built in LED each loop
      if (on_Board_LED_state) {             // Use built in LED to show program is running
        digitalWrite(LED_BUILTIN, HIGH);    // Turn the LED off
        on_Board_LED_state = false;
      } else {
        digitalWrite(LED_BUILTIN, LOW);     // Turn the LED on
        on_Board_LED_state = true;
      }
  }

  /*
   * my_reset() — wraps platform_reset() from platform.h.
   *   tt=true  -> hard reset (ESP8266: ESP.reset(), ESP32: ESP.restart())
   *   tt=false -> soft reset (ESP8266: ESP.restart(), ESP32: ESP.restart())
   */
  void my_reset(bool tt = true){
    delay(200);
    if(tt == true) Serial.println(F("reset")); else Serial.println(F("restart"));
    Serial.flush();
    my_yield();
    platform_reset(tt);   // platform.h handles the right call per board
  }

  /*
    *
    *     Header for root, stations and test pages
    *
    */
    const char htmlHeadStr[] PROGMEM =
    "<!DOCTYPE html> <html>\n\
    <head>\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=1\" charset=\"ASCII\">\n\
    <title>WX_Sectional</title>\n";

    const char htmlReloadStr[] PROGMEM =
    "<script type=\"text/javascript\">setTimeout(\"location.reload()\",300000);</script>\n";


    const char erralrt[] PROGMEM = "\t\t\t\t******* ERROR ERROR ERROR *******";    // used 4 times in stationPage

    // TODO use less ram? const char br_tag[] PROGMEM = "<BR>";

    const char lightingSymb[] PROGMEM = "&#9889;&#9889;";    // "ϟϟ" Lighting bolts
    //const static char lightingSymb[] PROGMEM = "&#9735;&#9735;&#9735;"; // "☇☇☇" NOAA weather symbol

    // goBack now used only once in testPage idLED and eprom red was used in Station page for out of memory message
    const char goBack[] PROGMEM = "<!DOCTYPE html> <script language=\"JavaScript\" type=\"text/javascript\"> setTimeout(\"window.history.go(-1)\",10); </script>";

    const char ooMem[] PROGMEM = "WX Out of Memory";  // This message is pointed to after the bigBlock array is full
    const char offLine[] = "Off-Line";          // Used if station does not return value
                                                // NOTE: PROGMEM cause a trap when copied with stringcat() (use stringcatP())

    unsigned int cycleCount = 0;     // for debug count the number of downloads since last reboot
    unsigned int totalCycleCount = 0; // Total downloads after last boot
    uint16_t cycleErrCount = 0;  // for debug count the number of failed downloads since last reboot
    uint16_t totalCycleErrCount = 0;   // Total errors after last boot
/*
 *    Reboot check — soft boot magic number, light offset, boot cause, totals.
 *
 *    set_softboot(true,  &lightOffset, &cause) — save state before reboot
 *    set_softboot(false, &lightOffset, &cause) — read state after reboot
 *      returns true  = soft boot (magic number matched)
 *      returns false = hard/power-on boot
 *
 *    Uses platform_rtc_read() / platform_rtc_write() from platform.h:
 *      ESP8266 -> ESP.rtcUserMemoryRead/Write
 *      ESP32   -> Preferences (NVS), survives soft reset
 */

 //TODO global lightOffser should not be here should be in LEDString need to access in loop()
 static int8_t lightOffset = LIGHT_OFFSET; // get from user_settings NOTE: Moved out of LEDString.h for now
#define MAGIC_REBOOT_FLAG 0xB003		// max 16 bits
//#define RTC_OFFSET 32               	// Starting location to save data
// RTC_OFFSET is defined in platform.h for ESP8266 only.
    									// Could be 0 there is no conflict with WiFiManger
 enum boot_Cause{
  b_Hard,
  b_MemoryFrag,
  b_init_MemFrag,
  b_Connect,
  b_html,
  b_BootTest,
  b_RebootTest
};
static int8_t boot_reason = b_Hard;

// rtcData struct is defined in platform.h (shared layout, platform-specific storage)
static RtcData rtcData;
boolean set_softboot(boolean setSoftBoot, int8_t *lightOffset , int8_t *cause){
  Serial.print(F("Size rtc = ")); Serial.println(sizeof(rtcData));
	 if(setSoftBoot){
		 rtcData.magic_number      = MAGIC_REBOOT_FLAG;
		 rtcData.light_Offset      = *lightOffset;
     rtcData.cause              = *cause;
     rtcData.totalCycleCount    = totalCycleCount;
     rtcData.totalCycleErrCount = totalCycleErrCount;
		 platform_rtc_write(&rtcData);   // platform.h: RTC mem (ESP8266) or NVS (ESP32)
		 return true;
	 } else {
		  platform_rtc_read(&rtcData);   // platform.h: RTC mem (ESP8266) or NVS (ESP32)
		  if (rtcData.magic_number == MAGIC_REBOOT_FLAG) {
		    *lightOffset              = rtcData.light_Offset;
        *cause                    = rtcData.cause;
        totalCycleCount           = rtcData.totalCycleCount;
        totalCycleErrCount        = rtcData.totalCycleErrCount;
        rtcData.magic_number = 0;
        rtcData.cause        = 0;
		    platform_rtc_write(&rtcData);
		    return true;
		  } else {
		    return false;
		  }
	 }
 }

/*
 *                     getCommand()
 *          gets numeric input from the serial console
 *          returns
 *            integer value of the char or chars received
 *            -1 no numeric values received
 *          ignores non numeric chars
 *
 *          Changed to use Serialevent much cleaner way to get input
 */
int getCommand() {
  // Returns: the LED number to identify (>= 0) or -1 if no valid digit received.
  // Returning -1 for non-digit input (noise, CR, LF, framing errors) prevents
  // idLED(0) from firing on every spurious UART byte, which caused visible
  // color changes every few seconds in steady state.
  retVal = -1;       // -1 = no valid numeric input received yet
  boolean gotDigit = false;
  char rx_byte = 0;
  while (Serial.available() > 0) {
    rx_byte = Serial.read();
      switch(rx_byte){
        case  'B':
        case  'b':
            boot_reason = b_BootTest;
              set_softboot(true, &lightOffset, &boot_reason);
              my_reset(true);
        break;
        case  'R':
        case  'r':
          boot_reason = b_RebootTest;
          set_softboot(true, &lightOffset, &boot_reason);
          my_reset(false);
        break;
        case  'T':
        case  't':
          my_Event = MY_TEST;
        break;
        default:
          if ((rx_byte >= '0') && (rx_byte <= '9')) {
            if (!gotDigit) { retVal = 0; gotDigit = true; }  // first digit: clear the -1
            retVal = (retVal * 10) + (int)(rx_byte - '0');
          } else {
            // Non-digit, non-command byte (CR, LF, noise) — consume and ignore.
            delay(10);
            my_yield();
          }
        break;
      }
  }
  if (gotDigit) { Serial.print(F("GetCommand RetVal = ")); Serial.println(retVal); }
  return retVal;
}

#endif
