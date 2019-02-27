//POST:: liest alle werte aus denn Sensoren ein
void sense() {
#if NO_IO_HARDWARE == 1
//THIS CODE IS ONLY FOR TESTING AND IF THERE IS NO EXTERNAL HARDWARE AVALIABLE!
  if (ist.toorstatus != soll.toorstatus){
    if(war_offen == 1){
      ist.oberer_endschalter = soll.toorstatus%2; 
      ist.unterer_endschalter = !soll.toorstatus%2;
      ist.toorstatus = soll.toorstatus;
      ist.temparatur = 5;
      war_offen = 0;
    }else{
      war_offen = 1;
      ist.oberer_endschalter = 0;
      ist.unterer_endschalter = 0;
      ist.temparatur = 6;
    }
  }
  return;
  //
#endif
  ist.oberer_endschalter = !digitalRead(OBEN_ENDSCHALTER);
  ist.unterer_endschalter = !digitalRead(UNTEN_ENDSCHALTER);
  ist.temparatur = analogRead(ANALOG_INPUT_1);
}


//wird bei einem Interrupt an pin 2 aufgerufen
void interrupt_handling(){
  zeiten.interrupt = true;
  UDR0 = 'T';
}







