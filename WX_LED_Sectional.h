#ifndef WX_Led_Sectional_ino
#define WX_Led_Sectional_ino "Jan 2, 2021"

#define WX_DEBUG false            // Extra output to Serial.print port
#define DEBUG    false            // Debug for memory tracing

#if AUTOCONNECT
#include <WiFiManager.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>


// #define WXSERVER "www.aviationweather.gov"
// #define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="

/* NEW Oct 17 2023 change in aviaionweather web pages*/
#define WXSERVER "aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="

#define my_yield() {ESP.wdtFeed(); yield();}     // reported problems with yield() not always reseting the software watchdog timer

int retVal;		// To save memory used everywhere

int actualNumAirports  = NUM_AIRPORTS;  // Total without NULL locations
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
} mtrs;
mtrs mtrsf[NUM_AIRPORTS];

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define RETRY_TIMEOUT 2 * 60 //110 // Seconds before attempting connection again on failure

std::vector<unsigned short int> lightningLeds;
static byte ooMemCnt;


#endif
