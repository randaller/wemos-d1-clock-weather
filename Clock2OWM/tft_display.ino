void tft_printDigits(int digits)
{
  if (digits < 10)
  {
    tft.print('0');
  }
  tft.print(digits);
}

void draw_triangle_up( int top_x, int top_y )
{
  for ( int x = top_x, y = top_y, len = 2; x >= top_x - 7; x--, y++, len += 2 )
  {
    tft.drawFastHLine( x, y, len, ILI9341_YELLOW );
  }
}

void draw_triangle_down( int top_x, int top_y )
{
  for ( int x = top_x, y = top_y, len = 2; x >= top_x - 7; x--, y--, len += 2 )
  {
    tft.drawFastHLine( x, y, len, ILI9341_CYAN );
  }
}

void shadow_text( int x, int y, String str )
{
  tft.setTextColor(BACK_COLOR);
  tft.setCursor(x - 3, y - 3);
  tft.print( str );

  tft.setTextColor(FORE_COLOR);
  tft.setCursor(x, y);
  tft.print( str );
}

void background_picture()
{
  if ( NIGHT_MODE )
  {
    tft.fillScreen(BACK_COLOR);
    return;
  }

  WiFiClient client;
  //client.setTimeout(10);

  if (!client.connect(flickr_host, http_port))
  {
    tft.fillScreen(BACK_COLOR);
    return;
  }

  client.print(String("GET ") + flickr_url + " HTTP/1.1\r\n" +
               "Host: " + flickr_host + "\r\n" +
               "User-Agent: Arduino_ESP8266_NTP_Clock\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("headers received");
      break;
    }
  }

  char useless_crlf = client.read();

  // read extra pixel to fix 1 pixel shift at left border
  char useless_r = client.read();
  char useless_g = client.read();
  char useless_b = client.read();

  const int IMG_BUF_SIZE = 640;     // (320 px * 2 lines)
  uint8_t bytes[IMG_BUF_SIZE * 2];
  uint16_t pixels[IMG_BUF_SIZE];

  // instead of main clearing screen (see also return above)
  tft.fillScreen(BACK_COLOR);

  tft.setAddrWindow(0, 0, 319, 239);

  int x = 0, y = 0;
  while ( client.available() )
  {
    /*
      // [3] bytes protocol
      //uint8_t buffer[3];
      //client.readBytes(buffer, 3);

      //uint16_t color = tft.color565( buffer[0], buffer[1], buffer[2] );
      // --- tft.drawPixel(x, y, color); ---
      //pixels[x] = color;

      // or [2] bytes protocol
      uint8_t buf[2];
      client.readBytes( buf, 2 );
      pixels[x] = ((uint16_t) buf[1] << 8) | buf[0];

      // increase pixel coordinates
      x++;
      if (320 == x) {
      tft.pushColors(pixels, 320);
      x = 0;
      y++;
      }

      // replaced with lines protocol below
    */

    // read whole line bytes
    client.readBytes( bytes, IMG_BUF_SIZE * 2 );
    for ( int i = 0; i < IMG_BUF_SIZE; i++ )
    {
      pixels[i] = ((uint16_t) bytes[i * 2 + 1] << 8) | bytes[i * 2];
    }
    tft.pushColors(pixels, IMG_BUF_SIZE);
    y += 2;

    // yield();

    if (240 == y) break;
  }
}

void draw_fulltime()
{
  //  if ( NIGHT_MODE )
  //  {
  //    for (int i = LED_BRIGHTNESS; i >= 0; i--)
  //    {
  //      analogWrite(LED_PIN, i);
  //      delay(1);
  //    }
  //  }

  // background_picture();

  tft.fillScreen(BACK_COLOR);

  // current time
  tft.setFont(&calibri64pt8b);

  // time shadow
  tft.setTextColor(BACK_COLOR);
  tft.setCursor(-2, 159);
  tft.print(" ");
  tft_printDigits(hour());
  tft.print(":");
  tft_printDigits(minute());

  tft.setTextColor(FORE_COLOR);
  tft.setCursor(-5, 162);
  tft.print(" ");
  tft_printDigits(hour());
  tft.print(":");
  tft_printDigits(minute());

  // another texts with small font
  tft.setFont(&calibri20pt8b);

  String str;
  int len;

  // day of week
  str = utf8rus( days[ weekday() ] );
  len = str.length() * 16;
  shadow_text( 160 - len / 2, 40, str );

  // day and month and year
  str = utf8rus( String( day() ) + " " + months[ month() ] + " " + year() );
  len = str.length() * 18;
  shadow_text( 160 - len / 2, 72, str );

  // weather line
  int weather_y = 200;

  // temperature
  str = String( temperature, 1 ) + "*C";
  len = str.length() * 16;
  shadow_text( 160 - len / 2, weather_y, str );

  // pressure
  str = String( pressure, 0 );
  shadow_text( 10, weather_y, str );

  // humidity
  str = String( humidity, 0 );
  shadow_text( 256, weather_y, str );

  // temperature trend
  switch ( temperature_sign )
  {
    case 1:
      draw_triangle_up( 163, 205 );
      break;
    case -1:
      draw_triangle_down( 163, 212 );
      break;
    default:
      break;
  }

  // humidity trend
  switch ( humidity_sign )
  {
    case 1:
      draw_triangle_up( 280, 205 );
      break;
    case -1:
      draw_triangle_down( 280, 212 );
      break;
    default:
      break;
  }

  // pressure trend
  switch ( pressure_sign )
  {
    case 1:
      draw_triangle_up( 40, 205 );
      break;
    case -1:
      draw_triangle_down( 40, 212 );
      break;
    default:
      break;
  }

  // reset font to small
  tft.setFont();

  // save boot time
  if ( first_boot )
  {
    // boot_time = String( String(hour()) + ":" + String(minute()) );
    char boot_time_char[5];
    sprintf( boot_time_char, "%02d:%02d", hour(), minute() );
    boot_time = String( boot_time_char );
    first_boot = false;
  }

  //tft.fillRect( 285, 1, 35, 9, BACK_COLOR );
  tft.setCursor(285, 1);
  tft.print( boot_time );

  // print last reset reason
  tft.setCursor(1, 1);
  tft.println( ESP.getResetReason().c_str() );

  // print heap fragmentation
  tft.setCursor(180, 1);
  tft.print( ESP.getHeapFragmentation() );
  tft.print( "% " );
  tft.print( ESP.getFreeHeap() );
  tft.print( " bytes" );

  // draw wifi sign
  if ( WiFi.status() == 3 )
  {
    tft.fillRect( 148, 224, 32, 3, ILI9341_GREEN );
    tft.fillRect( 152, 228, 24, 3, ILI9341_GREEN );
    tft.fillRect( 156, 232, 16, 3, ILI9341_GREEN );
    tft.fillRect( 160, 236, 8,  3, ILI9341_GREEN );
  }
  else
  {
    tft.fillRect( 148, 224, 32, 3, ILI9341_MAROON );
    tft.fillRect( 152, 228, 24, 3, ILI9341_MAROON );
    tft.fillRect( 156, 232, 16, 3, ILI9341_MAROON );
    tft.fillRect( 160, 236, 8,  3, ILI9341_MAROON );
  }

  // draw weather
  tft.setCursor( 1, 222 );
  tft.print( "Wind: " );
  tft.print( windSpeed );
  tft.print( " m/s, " );
  tft.print( (int) windBearing );
  tft.print( " deg" );

  tft.setCursor( 1, 232 );
  tft.print( "CL: " );
  tft.print( (int) ( cloudCover ) );
  tft.print( "%, * ");
  tft_printDigits( (int) hour(sunrise) );
  tft.print( ":" );
  tft_printDigits( (int) minute(sunrise) );
  tft.print( " - ");
  tft_printDigits( (int) hour(sunset) );
  tft.print( ":" );
  tft_printDigits( (int) minute(sunset) );

  // rects around places
  tft.drawRoundRect( 1, 12, 319, 157, 5, ILI9341_WHITE );

  tft.drawRoundRect( 1, 171, 79, 45, 5, ILI9341_WHITE );
  tft.drawRoundRect( 82, 171, 162, 45, 5, ILI9341_WHITE );
  tft.drawRoundRect( 246, 171, 74, 45, 5, ILI9341_WHITE );

  // can be safely removed
  draw_wifi_from_array();

  //  if ( NIGHT_MODE )
  //  {
  //    for (int i = 0; i <= LED_BRIGHTNESS; i++)
  //    {
  //      analogWrite(LED_PIN, i);
  //      delay(1);
  //    }
  //  }
}

void draw_wifi_from_array()
{
  tft.fillRect(190, 216, 130, 34, BACK_COLOR);

  for (int j = 1; j < 15; j++ )
  {
    int height = wifi_channels_tft[j];
    if ( height > 0 )
    {
      // highlight current channel
      int bar_color = FORE_COLOR;
      if ( j == WiFi.channel() )
      {
        bar_color = ILI9341_GREEN;
      }

      tft.fillRect( 190 + (j * 8), 240 - height, 7, height, bar_color );
      // tft.drawRect( 190 + (j * 8), 240 - height, 7, height, bar_color );
    }

    // dots at top of channels
    // tft.fillRect( 190 + (j * 8) + 4, 216, 1, 1, FORE_COLOR );
  }
}

void draw_seconds_square()
{
  if ( second() % 2 == 0 )
  {
    tft.fillRect( 160, 105, 14, 14, ILI9341_WHITE);
    tft.fillRect( 160, 149, 14, 14, ILI9341_WHITE);
  }
  else
  {
    tft.fillRect( 160, 105, 14, 14, ILI9341_BLACK);
    tft.fillRect( 160, 149, 14, 14, ILI9341_BLACK);
  }

  // print seconds
  tft.fillRect( 160, 85, 14, 14, ILI9341_BLACK);
  tft.setCursor( 161, 85 );
  tft_printDigits( second() );
}

void adjust_brightness()
{
  if ( ( hour() >= 10 ) && ( hour() <= 23 ) )
  {
    analogWrite(LED_PIN, LED_BRIGHTNESS_DAY);
  }
  else
  {
    analogWrite(LED_PIN, LED_BRIGHTNESS);
  }
}
