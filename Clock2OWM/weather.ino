void get_weather()
{
  WiFiClientSecure client;
  //client.setTimeout(10);

  client.setInsecure();

  if (!client.connect(weather_host, https_port))
  {
    //Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + weather_url + " HTTP/1.1\r\n" +
               "Host: " + weather_host + "\r\n" +
               "User-Agent: Arduino_ESP8266_NTP_Clock\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("headers received");
      break;
    }
  }

  String line;
  while ( client.available() )
  {
    char c = client.read();
    line += c;
  }

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, line);

  // Test if parsing succeeds.
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.c_str());
    return;
  }

  // Get the root object in the document
  JsonObject root = doc.as<JsonObject>();

  double new_temperature = root["main"]["temp"];
  temperature_sign = ( new_temperature > temperature ) ? 1 : -1;
  temperature = new_temperature;

  double new_humidity = root["main"]["humidity"];
  humidity_sign = ( new_humidity > humidity ) ? 1 : -1;
  humidity = new_humidity;

  double new_pressure = root["main"]["pressure"];
  new_pressure = new_pressure * 100.0 / 133.3224;
  pressure_sign = ( new_pressure > pressure ) ? 1 : -1;
  pressure = new_pressure;

  //summary    = root["weather"][0]["main"];
  //precipType = root["weather"][0]["description"];

  feelsLike   = root["main"]["feels_like"];
  windSpeed   = root["wind"]["speed"];
  windBearing = root["wind"]["deg"];
  cloudCover  = root["clouds"]["all"];
  visibility  = root["visibility"];

  sunrise = root["sys"]["sunrise"];
  sunrise = sunrise + (time_t) root["timezone"];
  sunset  = root["sys"]["sunset"];
  sunset  = sunset + (time_t) root["timezone"];

  dewPoint = (temperature - (14.55 + 0.114 * temperature) * (1 - (0.01 * humidity)) -
              pow(((2.5 + 0.007 * temperature) * (1 - (0.01 * humidity))), 3) -
              (15.9 + 0.117 * temperature) * pow((1 - (0.01 * humidity)), 14));
}
