//POST:: liest alle werte aus denn Sensoren ein
void sense() {
  ist.oberer_endschalter = !digitalRead(OBEN_ENDSCHALTER);
  ist.unterer_endschalter = !digitalRead(UNTEN_ENDSCHALTER);
  ist.temparatur = analogRead(ANALOG_INPUT_1);
}


//wird bei einem Interrupt an pin 2 aufgerufen
void interrupt_handling(){
  zeiten.interrupt = true;
  UDR0 = 'T';
}







