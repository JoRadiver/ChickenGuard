#if DISPLAY_MODULE == 1

  void update_display() {
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(16, 0, "Sonnenaufgang in:");
    char buff[12];
    timespan_to_String(zeiten.Sonnenaufgang - zeiten.loop_zeit).toCharArray(buff, 12);
    u8x8.drawString(32, 12, buff);
    u8x8.drawString(16, 24, "Sonnenuntergang in:");
    timespan_to_String(zeiten.Sonnenuntergang - zeiten.loop_zeit).toCharArray(buff, 12);
    u8x8.drawString(32, 36, buff);
  }
  
#endif
