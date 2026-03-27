#ifndef PLATFORM_H
#define PLATFORM_H  20260325

/*
 * platform.h
 * Board-abstraction layer for WX_LED_Sectional.
 * Supports ESP8266 (original) and ESP32-C3 (new).
 *
 * Include this file FIRST in WX_LED_Sectional.ino, before all other headers.
 * Do NOT include ESP8266WiFi.h, WiFi.h, WebServer headers, or BearSSL
 * anywhere else — include only platform.h and let it pull in the right ones.
 */

// ─────────────────────────────────────────────────────────────────────────────
//      Core WiFi + SSL + WebServer + mDNS includes
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)

  #include <WiFi.h>
  #include <WiFiClientSecure.h>          // mbedTLS — built into ESP32 Arduino core
  #include <HTTPClient.h>
  #if HTML
    #include <WebServer.h>
    #include <ESPmDNS.h>
    inline void platform_MDNS_update() {;}    // Not needed for ESP32
  #endif
  #if AUTOCONNECT
    #include <WiFiManager.h>             // tzapu WiFiManager >= 2.0.17 required
  #endif

#elif defined(ESP8266)

  #if AUTOCONNECT
    #include <WiFiManager.h>
  #else
    #include <ESP8266WiFi.h>
  #endif
  #include <WiFiClientSecure.h>          // BearSSL
  #if HTML
    #include <ESP8266mDNS.h>
    #include <ESP8266WebServer.h>
    inline void platform_MDNS_update() { MDNS.update(); }    // Not needed for ESP32
#endif

#else
  #error "Unsupported platform: only ESP8266 and ESP32 are supported."
#endif


// ─────────────────────────────────────────────────────────────────────────────
//      WebServer type alias
//      Use WebServerClass everywhere instead of ESP8266WebServer / WebServer.
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
  #define WebServerClass WebServer
#else
  #define WebServerClass ESP8266WebServer
#endif


// ─────────────────────────────────────────────────────────────────────────────
//      SSL client type alias
//      On ESP8266: BearSSL::WiFiClientSecure
//      On ESP32:   WiFiClientSecure  (mbedTLS, no BearSSL namespace)
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
  typedef WiFiClientSecure WxSSLClient;
#else
  typedef BearSSL::WiFiClientSecure WxSSLClient;
#endif


// ─────────────────────────────────────────────────────────────────────────────
//      resetWxClient(client)
//      ESP8266: in-place destroy + reconstruct to flush BearSSL state/buffers.
//      ESP32:   stop() is sufficient; mbedTLS does not leak on reconnect.
// ─────────────────────────────────────────────────────────────────────────────
inline void resetWxClient(WxSSLClient& client) {
#if defined(ESP32)
  client.stop();
  client.setInsecure();                  // accept any cert — same policy as ESP8266 build
#else
  client.~WiFiClientSecure();            // explicit destructor — frees BearSSL buffers
  new (&client) BearSSL::WiFiClientSecure();
  client.setInsecure();
  client.setBufferSizes(512, 512);       // keep RAM footprint small on ESP8266
#endif
}


// ─────────────────────────────────────────────────────────────────────────────
//      RTC / soft-reboot persistence
//
//      ESP8266: ESP.rtcUserMemoryRead/Write via a packed 32-bit-aligned struct.
//
//      ESP32-C3: RTC_DATA_ATTR survives deep sleep but NOT a soft reset
//      (ESP.restart()).  We use Preferences (NVS flash) for soft-boot
//      persistence so the magic-number / light-offset / cause / totals all
//      survive exactly the same scenarios as on ESP8266.
// ─────────────────────────────────────────────────────────────────────────────

// Struct layout is identical on both platforms for easy comparison.
struct RtcData {
  uint16_t magic_number;
  int8_t   light_Offset;
  int8_t   cause;
  int16_t  totalCycleCount;
  int16_t  totalCycleErrCount;
} __attribute__((aligned(4)));

#if defined(ESP32)

  #include <Preferences.h>
  #define PREFS_NS  "wxled"              // NVS namespace
  #define PREFS_KEY "rtc"                // NVS key — whole struct stored as blob

  inline void platform_rtc_read(RtcData* dst) {
    Preferences prefs;
    prefs.begin(PREFS_NS, /*readOnly=*/true);
    prefs.getBytes(PREFS_KEY, dst, sizeof(RtcData));
    prefs.end();
  }

  inline void platform_rtc_write(const RtcData* src) {
    Preferences prefs;
    prefs.begin(PREFS_NS, /*readOnly=*/false);
    prefs.putBytes(PREFS_KEY, src, sizeof(RtcData));
    prefs.end();
  }

  typedef enum {      // Fake it until I find the esp32 equivalent if needed 
    WIFI_NONE_SLEEP,
    WIFI_LIGHT_SLEEP,
    WIFI_MODEM_SLEEP
  } WiFiSleepType_t;
  inline bool platform_WiFi_setSleepMode(uint8_t WIFI_NONE_SLEEP, int listenInterval=0) {  return true; }

#else

  #define RTC_OFFSET 32                  // word offset into ESP8266 RTC user memory
                                         // 32 is clear of WiFiManager (uses words 0-31)

  inline void platform_rtc_read(RtcData* dst) {
    ESP.rtcUserMemoryRead(RTC_OFFSET, (uint32_t*)dst, sizeof(RtcData));
  }

  inline void platform_rtc_write(const RtcData* src) {
    ESP.rtcUserMemoryWrite(RTC_OFFSET, (uint32_t*)src, sizeof(RtcData));
  }

  inline bool platform_WiFi_setSleepMode(WiFiSleepType_t WIFI_NONE_SLEEP, int listenInterval=0 ) {
    return WiFi.setSleepMode(WIFI_NONE_SLEEP);
  }
#endif  // RTC block


// ─────────────────────────────────────────────────────────────────────────────
//      Heap monitoring
//      ESP8266 exposes getHeapFragmentation() and getMaxFreeBlockSize().
//      ESP32 does not; we compute equivalents via heap_caps.
//
//      IMPORTANT — threshold rescaling for ESP32-C3:
//        The ESP32-C3 has ~300 KB heap vs ~40 KB on ESP8266.  The percentage-
//        based fragmentation guard in metars.h already works correctly on both
//        because it is proportional.  The absolute free-heap guard (< 1000
//        bytes in showFree) is very conservative on ESP32 and will essentially
//        never fire, which is fine.
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
  #include <esp_heap_caps.h>

  inline uint32_t platform_free_heap() {
    return (uint32_t)ESP.getFreeHeap();
  }
  inline uint32_t platform_max_free_block() {
    return (uint32_t)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  }
  inline uint8_t platform_heap_frag_pct() {
    uint32_t fr = platform_free_heap();
    uint32_t bl = platform_max_free_block();
    return (fr > 0) ? (uint8_t)(100u - (bl * 100u / fr)) : 100u;
  }
#else
  inline uint32_t platform_free_heap()      { return (uint32_t)ESP.getFreeHeap(); }
  inline uint32_t platform_max_free_block() { return (uint32_t)ESP.getMaxFreeBlockSize(); }
  inline uint8_t  platform_heap_frag_pct()  { return (uint8_t)ESP.getHeapFragmentation(); }
#endif


// ─────────────────────────────────────────────────────────────────────────────
//      Reboot helper — replaces my_reset() calls in utilities.h / setup.h
//      ESP8266: ESP.reset() = hard peripheral reset, ESP.restart() = SW reset.
//      ESP32:   only ESP.restart() exists.
// ─────────────────────────────────────────────────────────────────────────────
inline void platform_reset(bool hard = true) {
#if defined(ESP32)
  (void)hard;
  ESP.restart();
#else
  if (hard) ESP.reset(); else ESP.restart();
#endif
}


// ─────────────────────────────────────────────────────────────────────────────
//      yield / watchdog
//      ESP8266 needs explicit wdtFeed(); ESP32 FreeRTOS handles it.
//      my_yield() is originally defined in utilities.h for ESP8266.
//      We override it here for ESP32 before utilities.h is included.
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
  #define my_yield() { yield(); }
#endif
// ESP8266 keeps its existing definition from utilities.h:
//   #define my_yield() {ESP.wdtFeed(); yield();}


// ─────────────────────────────────────────────────────────────────────────────
//      PROGMEM / F() macro
//      On ESP32, PROGMEM is a no-op (unified address space) and F() is
//      already defined as a no-op in the ESP32 Arduino core.
//      All existing F() / PROGMEM / PSTR usage compiles cleanly on both — no
//      guards needed anywhere.
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
//      LED_BUILTIN note
//      ESP32-C3 SuperMini: the on-board WS2812 RGB LED shares GPIO8.
//      If your NeoPixel strip DATA line is also on GPIO8, either:
//        (a) change LED_DATA_PIN in user_settings.h to a free GPIO (3/4/5/6/7)
//        (b) define LED_BUILTIN to -1 and comment out setupBuiltInLED() calls.
//      No code change needed here — LED_BUILTIN is defined by the board variant.
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ESP32)
// #if LED_BUILTIN == 30
 #undef LED_BUILTIN
// #define LED_BUILTIN 8
#endif

#ifndef LED_BUILTIN
 #define LED_BUILTIN 8
#endif

#endif // PLATFORM_H
