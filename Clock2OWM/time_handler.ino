// This function being called once a second
void refresh_display()
{
  if ( ( minute() == 0 ) && ( second() == 0 ) )
  {
    adjust_brightness();
  }

  if ( ( second() % 5 == 0 ) || NIGHT_MODE )
  {
    draw_1306();
  }

  if ( second() == 0 )
  {
    draw_fulltime();
  }

  if ( ( second() % 10 == 0 ) || ( second() == 1 ) )
  {
    int wifi_nets = WiFi.scanComplete();
    if ( wifi_nets > 0 )
    {
      // cleanup data for screens
      for (int j = 1; j < 15; j++)
      {
        wifi_channels_oled[j] = 0;
        wifi_channels_tft[j] = 2;
      }

      // enumerate found wi-fi networks
      for ( int i = 0; i < wifi_nets; i++ )
      {
        int rssi = WiFi.RSSI(i);
        int chan = WiFi.channel(i);

        // save data for oled screen
        int height = map( rssi, -110, -10, 0, 48 );
        if ( height > wifi_channels_oled[chan] )
        {
          wifi_channels_oled[chan] = height;
        }

        // save data for tft screen
        height = map( rssi, -110, -10, 0, 35 );
        if ( height > wifi_channels_tft[chan] )
        {
          wifi_channels_tft[chan] = height;
        }
      }

      WiFi.scanDelete();
    }
    draw_wifi_from_array();
    WiFi.scanNetworks( true, true );
  }

  if ( ( minute() % 5 == 0 ) && ( second() == 15 ) )
  {
    get_weather();
  }

  draw_seconds_square();

  // draw_seconds_bar();
}
