//Referenzen
void calc_door();
void motor_step(bool dir);
void dc_start(bool dir);
void dc_stop();
void toggle_light();
/*
 * TORSTATUS:
 * Geschlossen 0
 * Geöffnet    1
 * Schliessend 2
 * Öfnnend     3
 */

//Deklarationen
//POST:: Führt alles aus was nicht dem soll zustand entspricht.
void execute() {
  //Tür
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

void tor_bewegen() {
  if (ist.toorstatus != soll.toorstatus ) {
    if (zeiten.Step_cooldown <= millis()) {
      exout.setStep(soll.toorstatus % 2);
      zeiten.Step_cooldown = millis() + STEP_COOLDOWN_TIME;
    }
    else if (zeiten.Step_cooldown >= millis() + STEP_COOLDOWN_TIME*2)
    {
      //Overflow Protection
      zeiten.Step_cooldown = millis() + STEP_COOLDOWN_TIME;
    }
    ist.toorstatus = soll.toorstatus % 2 + 2;
  }
}


void calc_door() {
  if (ist.oberer_endschalter || exout.current_step == 0) {
    if (ist.toorstatus == 3) {
      deb.dprintln(F("Toor Offen"));
      for(int i = 4; i<8; i++){
        exout.digitalSet(i, 0);
      }
      exout.push();
    }
    ist.toorstatus = 1;

  }
  else if (ist.unterer_endschalter || exout.current_step >= exout.steps_needed) {
    if (ist.toorstatus == 2) {
      deb.dprint(exout.current_step);
      deb.dprint('|');
      deb.dprintln(exout.steps_needed);
      deb.dprintln(F(" Toor Geschlossen"));
      for(int i = 4; i<8; i++){
        exout.digitalSet(i, 0);
      }
      exout.push();
    }
    ist.toorstatus = 0;

  }
}

void set_zero(){
  
}

  #else

  //DC VERSION
  void calc_door() {
    if (ist.oberer_endschalter)
      ist.toorstatus = 1;
    else if (UNTEN_ENDSCHALTER) {
      ist.toorstatus = 0;
    }
  }


  // DC TOR FUNCTION
  void tor_bewegen() {
    if (ist.toorstatus % 2 != soll.toorstatus) {
      if (soll.toorstatus) {
        ist.toorstatus = 3; // motorstatus 3 (opening)
        dc_start(1);
      }
      else {
        ist.toorstatus = 2; //motorstatus 2 (closing)
        dc_start(0);
      }
    }
    else if (ist.toorstatus == soll.toorstatus) {
      dc_stop();
    }
  }

  void dc_start(bool dir){
    exout.digitalSet(0, HIGH);
    exout.digitalSet(1, dir);
    digitalWrite(PWM_1, HIGH);
  }

  void dc_stop(){
    exout.digitalSet(0, LOW);
    exout.digitalSet(0, LOW);
    digitalWrite(PWM_1, LOW);
  }

  #endif
