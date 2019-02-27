//Referenzen
void calc_door();
void motor_step(bool dir);
void dc_start(bool dir);
void dc_stop();
void toggle_light();
/*
   TORSTATUS:
   Geschlossen 0
   Geöffnet    1
   Schliessend 2
   Öffnend     3
*/

//Deklarationen
//POST:: Führt alles aus was nicht dem soll zustand entspricht.
void execute() {
  //Tür
  sense();
  calc_door();
  tor_bewegen();
  //Licht
  if (soll.lichtstatus != ist.lichtstatus) {
    exout.digitalSet(RELAIS_LICHT, soll.lichtstatus);
    ist.lichtstatus = soll.lichtstatus;
  }

  //Zaun
  exout.digitalSet(RELAIS_HAAG, soll.zaunstatus);
  exout.push();
}


#if MOTOR_TYPE == 1
#else

//DC VERSION

//DC Fehler erkennen
void detect_error() {

}


void calc_door() {
  if (switch_debounce < millis()) {
    switch_debounce = millis() + 10;
  } else {
    return;
  }
  if ( !ist.oberer_endschalter && !ist.unterer_endschalter) {
    floating_state = true;
  }else if(ist.oberer_endschalter && ist.unterer_endschalter){
    fehlerbehebung(0);
  }
  if (floating_state){
	  if (ist.oberer_endschalter || (zeiten.loop_zeit > zeiten.Toor_stop_wecker && ist.toorstatus == opening) )  {
		if (ist.toorstatus == open || ist.toorstatus == opening) {
		  floating_state = false;
		  ist.toorstatus = open;
		  deb.dprintln(" arrived_top");
		  exout.digitalSet(LED_1, HIGH);
		  exout.digitalSet(LED_2, LOW);
		} else {
		  deb.dprintln(" rencountered_top");
		  fehlerbehebung(1);
		}
	  } else if (ist.unterer_endschalter || (zeiten.loop_zeit > zeiten.Toor_stop_wecker && ist.toorstatus == closing) ) {
		if (ist.toorstatus == closed || ist.toorstatus == closing) {
		  floating_state = false;
		  ist.toorstatus = closed;
		  deb.dprintln(" arrived bot");
		  exout.digitalSet(LED_1, LOW);
		  exout.digitalSet(LED_2, HIGH);
		  if(zeiten.Toor_stop_wecker < zeiten.loop_zeit){
			  Serial.println("Ptstop");
		  }
		} else {
		  fehlerbehebung(2);
		}
	  }
  }
  
  /*
    if (ist.oberer_endschalter) {
    if (ist.toorstatus == 3) {
      deb.dprintln("T_offen");
    }
    ist.toorstatus = 1;
    exout.digitalSet(LED_1, HIGH);
    exout.digitalSet(LED_2, LOW);
    } else if (ist.unterer_endschalter) {
    if (ist.toorstatus == 2) {
      deb.dprintln("T_zu");
    }
    ist.toorstatus = 0;
    exout.digitalSet(LED_1, LOW);
    exout.digitalSet(LED_2, HIGH);
    }*/
  exout.push();
}


// DC TOR FUNCTION
void tor_bewegen() {
  detect_error();
  if (ist.toorstatus != soll.toorstatus && ist.toorstatus < 2) { // Darf nur richtung wechseln wenn komplett.
    if (soll.toorstatus == open) {
      zeiten.Toor_stop_wecker = zeiten.loop_zeit + DOOR_TIME_NEEDED*2; //Takes 1.5 times as long to open than to close! Doesen't matter how long it pulls to far so overestimating is good.
      deb.dprintln(" sup");
	  deb.dprint(" st:");
	  deb.dprintln(zeiten.Toor_stop_wecker-zeiten.loop_zeit);
	  ist.toorstatus = opening; 
      dc_start(1);
    }
    else {
      //DC has not yet startet
      zeiten.Toor_stop_wecker = zeiten.loop_zeit + DOOR_TIME_NEEDED;
      ist.toorstatus = closing; 
      dc_start(0);
	  deb.dprintln(" sdown");
	  deb.dprint(" st:");
	  deb.dprintln(zeiten.Toor_stop_wecker-zeiten.loop_zeit);
    }
  }
  else if (ist.toorstatus == soll.toorstatus) {
    dc_stop();
  }
}

void dc_start(bool dir) {
  exout.digitalSet(6, !dir);
  exout.digitalSet(7, dir);
  exout.push();
  analogWrite(PWM_1, 40);
  analogWrite(PWM_2, 40);
}

void dc_stop() {
  exout.digitalSet(6, LOW);
  exout.digitalSet(7, LOW);
  exout.push();
  analogWrite(PWM_1, 0);
  analogWrite(PWM_2, 0);
}

void fehlerbehebung(int type) {
  dc_stop();
  bool led_state = false;
  unsigned long blink_time = millis();
  exout.digitalSet(LED_1, led_state);
  exout.digitalSet(LED_2, led_state);
  exout.push();
  switch (type) {
    case 1: //Beim herunterlassen überdreht und oberen endschalter wieder betätigt.
      //Schritt eins
      {
        unsigned long tt_stop = now();
        dc_start(1);
        while (now() < (tt_stop + 1) && ! digitalRead(OBEN_ENDSCHALTER));
        if (!digitalRead(OBEN_ENDSCHALTER)) {
          //nach 1 sekunde oberer endschalter nicht gelöst-> abbruch, blinken aktivieren
          dc_stop();
          break;
        } else {
          tt_stop = now() + DOOR_TIME_NEEDED * 3;
          while (digitalRead(OBEN_ENDSCHALTER)) {
            //weiterfahren bis oberer endschalter wieder gedrückt wird
            if (millis() > blink_time) {
              exout.digitalSet(LED_1, !led_state);
              exout.digitalSet(LED_2, !led_state);
              led_state = !led_state;
              blink_time = millis() + 500;
              exout.push();
            }
            //oder die zeit abgelaufen ist.
            if (tt_stop < now()) {
              dc_stop();
              break;
            }
          }
          if (tt_stop > now()) {
            deb.dprintln("Error Probably Ressolved");
            floating_state = false;
            ist.toorstatus = open;
            dc_stop();
            return;
          } else {
            dc_stop();
          }
        }
      }
      break;
    case 2:
      //beim heraufziehen unteren endschalter wieder aktiviert:
      {
        unsigned long tt_stop = now() + 4;
        dc_start(1);
        while (tt_stop > now()) {
          if (digitalRead(UNTEN_ENDSCHALTER)) {
            deb.dprintln("Error Probably Resolved");
            ist.toorstatus = closed;
            dc_stop();
            floating_state = false;
            return;
          }
        }
      }
      break;
    default: break;
  }
  dc_stop();
  deb.dprintln("Error not resolved");
  //stay blinking as long as user does not take over.
  while (!manual_control());
  floating_state = false;
  deb.dprintln("Preseted");
}

//returns true if user has ordered exit, zero if timeout.
bool manual_control() {
  unsigned long timeout = now() + 900;
  Serial.println("Hm_init;");
  dc_stop();
  unsigned long blink_time = millis();
  bool led_state = false;
  delay(2000);
  bool end_manual = false;
  bool remote_access = false;
  while (!(!digitalRead(INTERRUPT_PIN) && !digitalRead(DIGITAL_I_3)) && !end_manual){
    if (millis() > blink_time) {
      exout.digitalSet(LED_1, !led_state);
      exout.digitalSet(LED_2, !led_state);
      led_state = !led_state;
      blink_time = millis() + 500;
      exout.push();
    }
    if (!digitalRead(INTERRUPT_PIN)) {  //Upper key pressed
      dc_start(1);
	  remote_access = false;
    } else if (!digitalRead(DIGITAL_I_3)) {  //Lower key pressed
      dc_start(0);
	  remote_access = false;
    } else {  //stop if no key is pressed, but only if running is not a command from the pi
		if (!remote_access){
			dc_stop();
		}
    }
    if (timeout < now()) {
      Serial.println("Pt_end;");
	  deb.stop();
      return false;
    }
	if (Serial.available()>3){
		if (Serial.peek() == 's'){
			Serial.read();
			if (Serial.read() != '2'){
				Serial.println("FJ ist dran. Warte.;");
			}else{
				switch(Serial.read()){
					case '1':
						Serial.print('P');
						Serial.print('O');
						Serial.print(!digitalRead(INTERRUPT_PIN));
						Serial.print('U');
						Serial.print(!digitalRead(DIGITAL_I_3));
						Serial.println(';');
						break;
					case '2':
						dc_start(1); 
						Serial.println("P up;");
						remote_access = true;
						break;
					case '3':
						dc_start(0); 
						Serial.println("P down;");
						remote_access = true;
						break;
					case '4':
						Serial.println("P stop;");
						remote_access = false;
						dc_stop(); 
						break;
					case '5':
						timeout += 3600; break;
					case '6':
						end_manual = true; break;
					default:
						break;
				}
			}
		}
		else{
			while (Serial.available()){
				if (Serial.peek() == 's'){
					break;
				}else{
					Serial.read();
				}
			}
				
		}
		
	}
  }
  dc_stop();
  delay(2000);
  Serial.print("Pm_end_manual");
  if(!digitalRead(OBEN_ENDSCHALTER)){
	  ist.toorstatus = open;
  } else if(!digitalRead(UNTEN_ENDSCHALTER)){
	  ist.toorstatus = closed;
  } else{
	  ist.toorstatus = soll.toorstatus;
  }
  Serial.print(soll.toorstatus);
  Serial.print(ist.toorstatus);
  Serial.println(';');
  zeiten.Toor_stop_wecker = now();
  return true;
}

#endif
