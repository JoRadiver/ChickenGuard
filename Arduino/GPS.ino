/*void gpsRefresh2(){
  gps.wakeup();
  gps.satellites = 0;
  Serial.println("looking for Satellites");
  while(gps.satellites==0){
    if (gps.newNMEAreceived()) {
      gps.parse(gps.lastNMEA());
    }
  }
  gps.standby();
  gpsClockDisplay();                             //Debug code
  setTime(gps.hour,gps.minute,gps.seconds,gps.day,gps.month,gps.year);
}*/





//POST:: Falls satelliten gefunden werden wird die Systemzeit aktualisiert
//Bei grossen Abweichungen wird die Zeit für die Neu berechnen funtion auf 60 sekunden gestellt
void gpsRefresh() {
#if NO_GPS_HARDWARE == 1
  //In no hardware mode don't refresh gps.
  return;
#endif
  zeiten.loop_zeit = now();
  gps.wakeup();
  gps.satellites = 0;
  deb.dprintln("looking for Satellites");
  while (gps.satellites == 0) {
    if (gps.newNMEAreceived()) {
      gps.parse(gps.lastNMEA());
    }
    if (zeiten.loop_zeit + 360 < now()) break;

  }
  gps.standby();
  if (gps.satellites > 0) {
    deb.dprintln("found Satellites");
    gpsClockDisplay();
    long differenz = now();
    setTime(gps.hour, gps.minute, gps.seconds, gps.day, gps.month, 1997);
    differenz = now() - differenz;
    zeiten.adjustTimes(differenz);
	Serial.print('D');
	Serial.print(differenz);
	Serial.println(';');
    //Bei Groben abweichungen von der Systemzeit neu berechnen
    if (differenz > 3|| differenz < -2) {
      
      zeiten.loop_zeit = now();
      zeiten.Standard_wecker = zeiten.loop_zeit;  // Sehr bald soll die öffnungszeit überprüft werden!
    }    
    zeiten.GPS_wecker = zeiten.loop_zeit + GPS_INTERVALL;
  }
  else {
    zeiten.GPS_wecker = zeiten.loop_zeit + 900;
    deb.dprintln(F("No Satellites found in 60s"));
  }
}



//POST:: gibt wahr zurück wenn a und b fast gleich sind
bool fast_gleich(unsigned int a, unsigned int b) {
  if (( b >= a - 1 ) && ( b <= a + 2)) {
    return true;
  }
  return false;
}


//POST:: Initialisiert die Kommunikation mit dem GPS
void gps_setup() {
#if NO_GPS_HARDWARE == 1
  return;
#endif
  deb.dprintln(F("GPS setup "));
  gps.begin(9600);                                    // Configure GPS to onlu output minimum data (location, time, fix).
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);      // Use a 1 hz, once a second, update rate.
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);          // Enable the interrupt to parse GPS data.
  //enableGPSInterrupt();
  //deb.dprintln(F("GPS begin "));
  useInterrupt(true);
  //deb.dprintln(F("e "));
}


//Strange Code
void useInterrupt(boolean v) {
  //deb.dprintln(F("interrupting"));
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  usingInterrupt = true;
}

SIGNAL(TIMER0_COMPA_vect) {
  char c = gps.read();
  // if you want to debug, this is a good time to do it!
  /*if (GPSECHO)
    if (c) UDR0 = c;  */
  // writing direct to UDR0 is much much faster than Serial.print
  // but only one character can be written at a time.
}

