#ifndef util_ino
#define util_ino 1

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
  //   static int retval; now using global retVal
  retVal = 0;        // Accumulate single digits
  char rx_byte = 0;     // for input from serial monitor
  while (Serial.available() > 0) {    // is a character available?
    rx_byte = Serial.read();       // get a character
    // Sanity check
    if ((rx_byte >= '0') && (rx_byte <= '9')) {
      retVal = (retVal * 10);
      retVal = retVal + (int) (rx_byte - '0');
    }
    delay(10); // allow next byte to collect important will not collect all values without a delay
    my_yield();
  } // end: while (Serial.available() > 0)
  return retVal;
}

/*
 *
 *                  Convert signed short to Signed c-string
 *            minimum value of -32,768 and a maximum value of 32,767
 *
 */
static char buf[7];
char* b2Scs(signed short b){
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
  itoa(b, buf,10);
  return buf;
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
  if ( force || (ESP.getHeapFragmentation() >= 50) || (ESP.getFreeHeap() < 1000) ) {
    Serial.print(F("\n\tFree Heap = ")); Serial.print(ESP.getFreeHeap());
    Serial.print(F("\tHeapFragmentation = ")); Serial.print(ESP.getHeapFragmentation());
    Serial.print(F("\tMaxFreeBlockSize = ")); Serial.println(ESP.getMaxFreeBlockSize());
    // Serial.print("Free = "); Serial.println(ESP.freeMemory());
  }
}


/*
 *                                        Uptime()
 *                  Compute uptime return as a cstring days : hours : minutes : seconds
 *                  Note: Max Uptime counter is 49 days 17 hours 2 minutes and 47 Seconds
 *                  Update: 12/16 added 49 day overflow counter so should work for 999 * 49 days (Like that is going to happen)
 */
static unsigned long m_secs, m_mins;
static unsigned int m_hours, m_days;
static char m_uptimeCstr[] = "365:23:59:59"; // max value for millis() is 4,294,967,295 or 49.71 days
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
  strcat(m_uptimeCstr, b2cs(m_days));  strcat(m_uptimeCstr, ":");
  strcat(m_uptimeCstr, b2cs(m_hours)); strcat(m_uptimeCstr, ":");
  strcat(m_uptimeCstr, b2cs(m_mins));  strcat(m_uptimeCstr, ":");
  strcat(m_uptimeCstr, b2cs(m_secs));  //strcat(m_uptimeCstr, ":");
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

    const char lightingSymb[] PROGMEM = "&#9889;&#9889;";    // "ϟϟ" Lighting bolts
    //const static char lightingSymb[] PROGMEM = "&#9735;&#9735;&#9735;"; // "☇☇☇" NOAA weather symbol

    // goBack now used only once in testPage idLED and eprom red was used in Station page for out of memory message
    const char goBack[] PROGMEM = "<!DOCTYPE html> <script language=\"JavaScript\" type=\"text/javascript\"> setTimeout(\"window.history.go(-1)\",10); </script>";

    const char ooMem[] = "WX Out of Memory";   // This message is pointed to after the bigBlock array is full
    const char offLine[] = "Off-Line";         // Used if station does not return value

    unsigned int cycleCount = 0;     // for debug count the number of downloads since last reboot
    unsigned int cycleErrCount = 0;  // for debug count the number of failed downloads since last reboot

#endif
