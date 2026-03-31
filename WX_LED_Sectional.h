#ifndef WX_Led_Sectional_ino
#define WX_Led_Sectional_ino "March 21, 2026"

#define WX_DEBUG false		      // Extra station output to Serial.print port

/*
 * WiFi / SSL / WebServer / mDNS includes have been moved to platform.h.
 * platform.h is included first in WX_LED_Sectional.ino and provides:
 *   - The right WiFi + WebServer + mDNS headers for ESP8266 or ESP32
 *   - WxSSLClient  typedef  (BearSSL::WiFiClientSecure | WiFiClientSecure)
 *   - WebServerClass define (ESP8266WebServer         | WebServer)
 *   - resetWxClient(client), platform_rtc_read/write(), platform_reset()
 *   - platform_free_heap(), platform_heap_frag_pct(), platform_max_free_block()
 *
 * Do NOT add board-specific WiFi/SSL/WebServer includes here.
 */

#define WXSERVER "aviationweather.gov"
#define   BASE_URI "/api/data/dataserver?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="

int retVal;		// To save memory used everywhere

int actualNumAirports  = NUM_OF_LEDS;  // Total without NULL locations
int airportStringsSize = 0;             // total number of char's needed
int noOfAirportStrings = 1;             // if dividing up for download how many substrings needed / used
char** airportStrings;                  // pointers to substrings
int* airportIndex;                      // Last location in airports[] for each airport substring limit station's

static unsigned int rtmaxl = 0;              // Study to find raw text max length
static unsigned int total_C_StringLen = 0;   // Study to find total length of raw text strings
static const unsigned int RTMAXLN = 200;      // max size for Raw Text message
  // leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
  // AIM defines 1 LIFR, 2 IFR, 3 MVFR, 4 VFR also can add WIND CIG FG Example "IFR FG" "LIFR CIG" "VFR WIND"
#define UNKWN 0
#define LIFR 1
#define IFR 2
#define MVFR 3
#define VFR 4
#define NOTUSED 99

typedef struct mtrs {
  byte mtrstat;         // 1 LIFR, 2 IFR, 3 MVFR, 4 VFR, 0 unknown, 99 NULL  leds[] color Green = VFR, Blue = MVFR, Magenta = LIFR, Red = IFR, Yellow = VFR+Wind, Black = unknown
  byte mtrspeed;        // wind in knots
  byte mtrgusts;        // Used for html button display quicker to check by led number instead of comparing an entire vector entry's for every led
  boolean mtrlighting;  // Used for html button display quicker to check by led number instead of comparing an entire vector entry's for every led
  const char* rawText;
} __attribute__((aligned(4))) mtrs;
mtrs mtrsf[NUM_OF_LEDS];

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define RETRY_TIMEOUT 2 * 60 //110 // Seconds before attempting connection again on failure

#define MAX_LIGHTNING 20
static unsigned short int lightningLeds[MAX_LIGHTNING];
static byte lightningLedsCount = 0;

static byte ooMemCnt;


#endif
