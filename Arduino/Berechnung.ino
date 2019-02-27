//--Standard--//
//Referenzen
bool fast_gleich(unsigned int a, unsigned int b);
void berechne_sonnenzeiten(unsigned long jetzt , unsigned long* zeiten);
byte berechne_tageszeit(unsigned long* sonnenzeiten);
void berechne_wecker();
//Implemetationen

//POST::Berechnet den nächsten Aufgang und Untergang und speichert sie in zeiten ab.
//Setzt den soll.toorstatus auf offen am Tag und auf geschlossen am Abend
void Standard_Calculations() {
  deb.dprintln(F("Berechne Neu"));
  //Berechne nächsten Sonnenaufgang und untergang
  unsigned long kurzspeicher[2]; //[aufgang, untergang]
  berechne_sonnenzeiten(zeiten.loop_zeit, kurzspeicher);
  zeiten.Tageszeit = berechne_tageszeit(kurzspeicher);
  switch (zeiten.Tageszeit) {
    case before_sunrise:
      zeiten.Sonnenaufgang = kurzspeicher[0];
      zeiten.Sonnenuntergang = kurzspeicher[1];
      break;
    case day:
      zeiten.Sonnenuntergang = kurzspeicher[1]; //Heutiger Abend
      berechne_sonnenzeiten(zeiten.loop_zeit + 86400, kurzspeicher);
      zeiten.Sonnenaufgang = kurzspeicher[0];  //Morgiger Morgen
      break;
    case after_sunset:
      berechne_sonnenzeiten(zeiten.loop_zeit + 86400, kurzspeicher);
      zeiten.Sonnenaufgang = kurzspeicher[0];
      zeiten.Sonnenuntergang = kurzspeicher[1];
      break;
    default:
      ist.fehler += 1;
  }
  if(zeiten.Tageszeit == day){
	soll.toorstatus = 1;
  }
  else{
	soll.toorstatus = 0;
  }
  berechne_wecker();
}


//POST::Stellt die zeit ein wann die standard_Calculations funktion das nächste mal aufgerufen wird.
void berechne_wecker() {
  switch (zeiten.Tageszeit) {
    case day:
      zeiten.Standard_wecker = zeiten.Sonnenuntergang + 30; //30 sekunden nach sonnenuntergang
      break;
    case before_sunrise:
    case after_sunset:
      zeiten.Standard_wecker = zeiten.Sonnenaufgang + 30; //30 sekunden nach sonnenaufgang
      break;
  }
}

//POST:: returns the current DayTime
DayTimes::DayTime berechne_tageszeit(unsigned long* sonnenzeiten) {
  unsigned long sonnenaufgang = sonnenzeiten[0];
  unsigned long sonnenuntergang = sonnenzeiten[1];
  if (zeiten.loop_zeit < sonnenaufgang)
    return DayTimes::before_sunrise;
  else if (zeiten.loop_zeit >= sonnenaufgang && zeiten.loop_zeit < sonnenuntergang)
    return DayTimes::day;
  else if (zeiten.loop_zeit >= sonnenuntergang)
    return DayTimes::after_sunset;
}

double grad(double rad) { //converts radians to degrees
  return ((rad * 360) / (2 * PI));
}

//POST:: Berechnet aus einer UNIX Time stamp den Sonnenaufgang und den Sonnenuntergang am jetztigen Tag
void berechne_sonnenzeiten(unsigned long jetzt, unsigned long *zeiten) { //berechnet die dämmerungen des Tages ausgehend von einer zeit in UNIX, and der angegebenen Adresse und eine Später.
  //datumsumrechnungen
  unsigned long today_unix = jetzt - ((long)hour(jetzt) * 3600) - ((long)minute(jetzt) * 60) - (long)second(jetzt); // heutiger Tag in UNIX
  double microsoft_date = ((float)today_unix / 86400.0) + 25569.0; //Heutiger Tag in Microsoft Time
  double time_past_local_midnight = 0.50; //in microsoft time format, here irrelevant
  double julian_day = microsoft_date + 2415018.5 + time_past_local_midnight - timezzone / 24.0; //achtung: Timezone wird verwendet in time.h, deshalb hier timezzone
  double julian_century = (julian_day - 2451545.0) / 36525.0; //korrekter output sehr genau
  //datumsumrechnungen
  //berechnung der Dämmerungszeiten
  double mean_obliq_elliptic = 23 + (26 + ((21.448 - julian_century * (46.815 + julian_century * (0.00059 - julian_century * 0.001813)))) / 60) / 60; //korrigert, korrekter output, sehr genau
  double eccent = 0.016708634 - julian_century * (0.000042037 + 0.0000001267 * julian_century); //korrekter output, sehr genau
  double oblic_corr = mean_obliq_elliptic + 0.00256 * cos(radians(125.04 - 1934.136 * julian_century)); //korrekter output, sehr genau, sollte obliq_corr heissen!
  double geom_mean_anom_sun = 357.52911 + julian_century * (35999.05029 - 0.0001537 * julian_century); //korrekter output, sehr genau
  double geom_mean_long_sun = 360 + fmod((280.46646 + julian_century * (36000.76983 + julian_century * 0.00003032)), 360.000); // output ok, leicht ungenau nach 6 stellen:hier ist etwas falsch... komische REST funktion in excel... jetzt wenigstens approximiert
  double sun_eq_of_ctr = sin(radians(geom_mean_anom_sun)) * (1.914602 - julian_century * (0.004817 + 0.000014 * julian_century)) + sin(radians(2 * geom_mean_anom_sun)) * (0.019993 - 0.000101 * julian_century) + sin(radians(3 * geom_mean_anom_sun)) * 0.000289; //korrekt, korrigiert 5 stellen genau
  double sun_true_long = geom_mean_long_sun + sun_eq_of_ctr; //korrekt, 5 stellen genau
  double sun_app_long = sun_true_long - 0.00569 - 0.00478 * sin(radians(125.04 - 1934.136 * julian_century)); //korrekt, 5 stellen genau
  double var_y = tan(radians(oblic_corr / 2)) * tan(radians(oblic_corr / 2)); //korrekt sehr genau
  double sun_declination = grad(asin(sin(radians(oblic_corr)) * sin(radians(sun_app_long)))); //korrekt, 5 stellen genau.
  double eq_of_time = 4 * grad(var_y * sin(2 * radians(geom_mean_long_sun)) - 2 * eccent * sin(radians(geom_mean_anom_sun)) + 4 * eccent * var_y * sin(radians(geom_mean_anom_sun)) * cos(2 * radians(geom_mean_long_sun)) - 0.5 * var_y * var_y * sin(4 * radians(geom_mean_long_sun)) - 1.25 * eccent * eccent * sin(2 * radians(geom_mean_anom_sun))); //korrigiert, 4 stellen genau. in mminutes
  double ha_sunrise = grad(acos(cos(radians(90.833)) / (cos(radians(LATITUDE)) * cos(radians(sun_declination))) - tan(radians(LATITUDE)) * tan(radians(sun_declination)))); //korrekt, ungefähr 5 stellen
  double solar_noon = (720 - 4 * LONGITUDE - eq_of_time + timezzone * 60) / 1440;
  double sunrise_microsoft_time = (solar_noon * 1440 - ha_sunrise * 4) / 1440; //days since 1.1.1900
  double sunset_microsoft_time = (solar_noon * 1440 + ha_sunrise * 4) / 1440;
  double sunrise_linux_time = sunrise_microsoft_time * 86400;           //seconds since 1.1.1970
  double sunset_linux_time = sunset_microsoft_time * 86400;             //seconds since 1.1.1970

  *zeiten = sunrise_linux_time + today_unix - TAG_VERFRUEHUNG * 60;
  *(zeiten + 1) = sunset_linux_time + today_unix + NACHT_VERSPAETUNG * 60;
}
//--Standard--//


#if PI_MODULE == 1

  //Picode
  void PI_Calculations() {
#if MOTOR_TYPE == 1
	pi.handleInput();
#else
    switch(pi.handleInput()){
		case -1:
			manual_control();
		default:
			return;
	}
#endif
  }
#endif
