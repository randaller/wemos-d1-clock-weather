#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341_fast.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#define TFT_CS   D10
#define TFT_DC   D9
#define TFT_RST  D8
#define TFT_MOSI D7
#define TFT_MISO D6
#define TFT_CLK  D5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

#define LED_PIN D2
#define LED_BRIGHTNESS 35
#define LED_BRIGHTNESS_DAY 230

#define BACK_COLOR ILI9341_BLACK
#define FORE_COLOR ILI9341_WHITE

#include "fonts/calibri20pt8b.h"
#include "fonts/calibri64pt8b.h"

#include <SSD1306Wire.h>
SSD1306Wire display(0x3C, D4, D3);

#include "fonts/image.h"               // https://www.online-utility.org/image/convert/to/XBM
#include "fonts/chewy_regular_36.h"    // http://oleddisplay.squix.ch/     // add unsigned to char in font file
#include "fonts/lato_light_6.h"
#include "fonts/dseg7_38.h"

boolean first_boot = true;
String  boot_time = "";

const char ssid[] = "WiFiSSID";           // your network SSID (name)
const char pass[] = "WiFIPassword";       // your network password

// Weather API
const int https_port = 443;
const char* weather_host = "api.openweathermap.org";
String weather_url = "/data/2.5/weather?units=metric&id=12345&appid=API_KEY";

// proxy to Flickr API
const int http_port = 80;
const char* flickr_host = "my.example.com";
String flickr_url = "/esp8266.php";

// NTP Servers
static const char ntpServerName[] = "0.fi.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";

const int timeZone = 3;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)

WiFiUDP Udp;
unsigned int localPort = 8888;

String months[13] = {"", "января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря"};
String days[8]    = {"", "Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота"};

time_t prevDisplay = 0; // when the digital clock was displayed

double temperature = 0.0;
double humidity = 0;
double pressure = 0;

String summary;
String precipType;
double feelsLike;
double dewPoint;
double windSpeed;
double windBearing;
double cloudCover;
double visibility;
time_t sunrise;
time_t sunset;

int temperature_sign = 0;
int humidity_sign = 0;
int pressure_sign = 0;

int wifi_channels_oled[15];
int wifi_channels_tft[15];

#define NIGHT_MODE ( hour() >= 0 ) && ( hour() <= 7 )

#define ENABLE_OTA_UPDATE true

void setup()
{
  // system_update_cpu_freq(160);

  // remove flickering https://stackoverflow.com/questions/42112357/how-to-implement-esp8266-5-khz-pwm
  // analogWriteRange(2);

  // remove flickering https://github.com/esp8266/Arduino/issues/836
  analogWriteFreq(32768);

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, LED_BRIGHTNESS);

  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(0, 26);
  tft.println("Init OLED");

  display.init();
  display.flipScreenVertically();
  display.setContrast( 7 );
  display.drawXbm(0, 0, a1550937659560_width, a1550937659560_height, a1550937659560_bits);
  display.display();

  Serial.begin(9600);
  delay(250);
  Serial.println("NTP Clock");
  Serial.print("Connecting to ");
  Serial.print(ssid);

  tft.print("Connecting");

  // WiFi.setAutoConnect(false);  // auto-connect at power up
  WiFi.setAutoReconnect(true);

  // way1 - not working
  // String stationname = "Wemos_NTP_Clock";
  // wifi_station_set_hostname((char *)stationname.c_str());

  // way2 - not working
  // WiFi.hostname("Wemos_NTP_clock");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    tft.print(".");
    Serial.print(".");
  }

  tft.println("");
  tft.println(WiFi.SSID());
  tft.print("IP: ");
  tft.println(WiFi.localIP());
  tft.println("Starting UDP");
  Udp.begin(localPort);
  tft.print("Local port ");
  tft.println(Udp.localPort());
  tft.println("Time sync");

  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  tft.println("Fetch weather");
  get_weather();

  if ( ENABLE_OTA_UPDATE )
  {
    tft.println("Begin OTA handler");
    // ArduinoOTA.setPassword("admin");
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)         Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR)     Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }

  tft.setTextSize(1);
  tft.fillScreen(ILI9341_BLACK);

  draw_fulltime();
  adjust_brightness();
}

void loop()
{
  if ( ENABLE_OTA_UPDATE )
  {
    ArduinoOTA.handle();
  }

  if (timeStatus() != timeNotSet)
  {
    // update the display only if time has changed
    if (now() != prevDisplay)
    {
      prevDisplay = now();

      refresh_display();
    }
  }
}
