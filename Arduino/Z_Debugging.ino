
//unsigned long time_debug;
void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(gps.hour);
  printDigits(gps.minute);
  printDigits(gps.seconds);
  Serial.print(" ");
  Serial.print(gps.day);
  Serial.print(" ");
  Serial.print(gps.month);
  Serial.print(" ");
  Serial.print(gps.year); 
  Serial.println(); 
}

//unsigned long time_debug;
void gpsClockDisplay() {
  // digital clock display of the time
  deb.dprint(gps.hour);
  printDigits(gps.minute);
  printDigits(gps.seconds);
  deb.dprint(F(" "));
  deb.dprint(gps.day);
  deb.dprint(F(" "));
  deb.dprint(gps.month);
  deb.dprint(F(" "));
  deb.dprint(gps.year);
  deb.dprintln();
}


void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  deb.dprint(':');
  if (digits < 10)
    deb.dprint('0');
  deb.dprint(digits);
}

String timespan_to_String(unsigned int tspan) {
  String s = (String)hour(tspan) + "h " + minute(tspan) + "m " + second(tspan) + "s";
  return s;
}
