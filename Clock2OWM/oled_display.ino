String oled_digits(int digits)
{
  if (digits < 10)
  {
    return String("0") + String(digits);
  }

  return String(digits);
}

void draw_bearing(int degree)
{
  display.drawCircle( 116, 40, 11 );
  display.drawLine( 116, 40, 116 + sin(radians(degree)) * 10, 40 + cos(radians(degree)) * -10 );
  display.drawLine( 116, 40, 116 + sin(radians(degree + 25)) * 3, 40 + cos(radians(degree + 25)) * -3 );
  display.drawLine( 116, 40, 116 + sin(radians(degree - 25)) * 3, 40 + cos(radians(degree - 25)) * -3 );
}

void draw_strength(float current, float min, float max, int x = 1)
{
  int mapped = (int) map ( current, min, max, 16, 0 );

  if ( mapped < 16 )
  {
    for (int y = 0; y < 64; y += 4)
    {
      display.setPixel( x, y );
    }
  }

  for (int y = 0 + (4 * mapped); y < 64; y += 4)
  {
    display.fillRect( x, y, 4, 2 );
  }
}

void draw_night()
{
  display.clear();

  for (int x = 1; x <= second() * 2.13; x += 4)
  {
    display.fillRect( x, 4, 3, 3 );
  }

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(DSEG7_38);

  String str;

  // hours
  if ( hour() <= 9 )
  {
    str = "0";
  }
  else
  {
    str = "";
  }
  str += String( hour() );
  display.drawString(32, 16, str);

  // delimiter blink
  if ( second() % 2 == 0 )
  {
    str = ":";
    display.drawString(64, 16, str);
  }

  // minutes
  if ( minute() <= 9 )
  {
    str = "0";
  }
  else
  {
    str = "";
  }
  str += String( minute() );
  display.drawString(96, 16, str);

  display.display();

  if ( second() == 59 )
  {
    display.setColor(INVERSE);
    for (int x = 1; x <= second() * 2.13; x += 4)
    {
      display.fillRect( x, 4, 3, 3 );
      delay(20);
      display.display();
    }
    display.setColor(WHITE);
  }
}

void draw_1306()
{
  // OLED burnout test
  //display.clear();
  //display.fillRect(0,0,128,64);
  //display.display();
  //return;

  if ( NIGHT_MODE )
  {
    display.clear();
    display.display();
    return;

    // draw_night();
    // return;
  }

  int cloudz = 0;

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);

  switch ( second() )
  {
    case 0:
      display.drawString(64, 0, F("Temperature"));
      display.setFont(Chewy_Regular_36);
      if ( temperature > 0.0 )
      {
        display.drawString(64, 16, "+" + String(temperature));
      }
      else
      {
        display.drawString(64, 16, String(temperature));
      }
      draw_strength( abs(temperature), 0, 30 );
      break;

    case 5:
      display.drawString(64, 0, F("Feels like"));
      display.setFont(Chewy_Regular_36);
      if ( feelsLike > 0.0 )
      {
        display.drawString(64, 16, "+" + String(feelsLike));
      }
      else
      {
        display.drawString(64, 16, String(feelsLike));
      }
      draw_strength( abs(feelsLike), 0, 30 );
      break;

    case 10:
      display.drawString(64, 0, F("Pressure"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String(pressure));
      draw_strength( pressure, 730, 780 );
      break;

    case 15:
      display.drawString(64, 0, F("Humidity"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String( (int) humidity ));
      draw_strength( humidity, 0, 100 );
      break;

    case 20:
      display.drawString(64, 0, F("Precipitation"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String("n/a"));
      //draw_strength( precipIntensity * 100, 0, 200 );
      break;

    case 25:
      display.drawString(64, 0, F("Dew point"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String(dewPoint));
      draw_strength( abs(dewPoint), 0, 30 );
      break;

    case 30:
      display.drawString(64, 0, F("Wind speed"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String(windSpeed));
      draw_strength( windSpeed, 0, 20 );
      break;

    case 35:
      display.drawString(64, 0, F("Wind bearing"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String( (int) windBearing ));
      draw_bearing( windBearing );
      break;

    case 40:
      display.drawString(64, 0, F("Clouds"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, String( (int) cloudCover ));
      draw_strength( cloudCover, 0, 100 );
      break;

    case 45:
      display.drawString(64, 0, F("Sunrise"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, oled_digits( (int) hour(sunrise) ) + ":" + oled_digits( (int) minute(sunrise) ) );
      break;

    case 50:
      display.drawString(64, 0, F("Sunset"));
      display.setFont(Chewy_Regular_36);
      display.drawString(64, 16, oled_digits( (int) hour(sunset) ) + ":" + oled_digits( (int) minute(sunset) ) );
      break;

    case 55:
      display.drawString(64, 0, "RSSI: " + String( WiFi.RSSI() ) );
      draw_strength( WiFi.RSSI(), -90, -30, 124 );

      // draw wifi from array
      for (int j = 1; j < 15; j++ )
      {
        int height = wifi_channels_oled[j];
        display.fillRect( j * 8, 64 - height, 7, height );
      }

      display.setFont(Lato_Light_6);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setColor(INVERSE);
      for (int chan = 1; chan < 15; chan++)
      {
        display.drawString( (chan * 8) + 1, 56, String( chan ) );
      }
      display.setColor(WHITE);
      break;

    default:
      break;
  }
  display.display();
}
