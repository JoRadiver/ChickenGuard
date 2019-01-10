//================================CODE SELECTORS=====================================//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
#define MOTOR_TYPE 1  //define 1 for stepper, any other integer for dc
#define PI_MODULE 1  //Define 1 to include
#define DISPLAY_MODULE 0  //Define 1 to include
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//================================CODE SELECTORS=====================================// 




//==============================TUNING VARIABLEN=====================================//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
#define LATITUDE 47.501                   //Position breitengrad
#define LONGITUDE 8.76                    //Position längengrad
#define GPS_INTERVALL 7200               //wie oft die Systemzeit Aktualisiert wird
#define STANDARD_INTERVALL 1800
#define DISPLAY_INTERVALL 30              //wie oft das display erneuert wird
#define PIREPORT_INTERVALL 1800          //wie oft ein report an den pi erstellt wird
#define PI_OVERRIDE_TIME 1800             //Zeit wie lange nach einem Pi befehl nicht kontrolliert wird
#define INTERRUPT_OVERRIDE_TIME 1800      //Zeit wie lange nach einem Interrupt befehl nicht kontrolliert wird
#define STEP_COOLDOWN_TIME 4          //Milisekunden bis ein weiterer Schritt gemacht werden darf.
#define STEPS_NEEDED 53379
const int timezzone = 0;                        //Zeitzone UTC+...
#define NACHT_VERSPAETUNG 0               //wie viele Minuten nach Sonnenuntergang soll geschlossen werden?
#define TAG_VERFRUEHUNG 0                //wie viele Minuten vor Sonnenaufgang soll geöffnet werden?
#define STARTZEIT 0, 0, 0, 1, 9, 1997
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//==============================TUNING VARIABLEN=====================================//


//Basic libaries
#include <TimeLib.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GPS.h>
#include <extendedoutputs.h>
#include <debugmanager.h>
#include <Daten.h>




//Pin Konfigurationen
#define INTERRUPT_PIN 2
#define OBEN_ENDSCHALTER 3
#define UNTEN_ENDSCHALTER 4
#define DIGITAL_I_3 5
#define DIGITAL_I_4 6
int GPS_RX = 7;
int GPS_TX = 8;
#define PWM_1 9
#define PWM_2 10
#define ANALOG_INPUT_1 A7
#define ANALOG_INPUT_2 A6

//Pin Konfiguration Extended output
#define LATCH_PIN 11
#define DATA_PIN 12
#define CLOCK_PIN 13


// Shift Register OUTPUTS
#define RELAIS_LICHT 1
#define RELAIS_HAAG 0
#define LED_1 5
#define LED_2 3

//debugger
DebugManager deb(&Serial);

//Hardwareobjekte
ExtendedOutput exout(11, 12, 13);

//Datenspeicher
Zeiten zeiten;
ExtendedZustand ist;
Zustand soll;


//GPS
#define GPSECHO false
SoftwareSerial gpsSerial(7, 8);  // GPS breakout/shield will use a
Adafruit_GPS gps(&gpsSerial);
boolean usingInterrupt = false;




//Picode
#if PI_MODULE == 1

  #include <PiManager.h>
  PiManager pi(&Serial, &zeiten, &ist, &soll, &deb);
  
#endif


//Display
#if DISPLAY_MODULE == 1

  #include <U8x8lib.h>
  #ifdef U8X8_HAVE_HW_SPI
  #include <SPI.h>
  #endif
  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

#endif

//Funktionsreferenzen (Deklaration in weiteren tabs)



//=======================================SETUP=======================================//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
void setup() {

  pinMode(INTERRUPT_PIN, INPUT);
  pinMode(OBEN_ENDSCHALTER, INPUT);
  pinMode(UNTEN_ENDSCHALTER, INPUT);
  pinMode(DIGITAL_I_3, INPUT);
  pinMode(DIGITAL_I_4, INPUT);
  pinMode(PWM_1, OUTPUT);
  pinMode(PWM_2, OUTPUT);
  pinMode(ANALOG_INPUT_1, INPUT);
  pinMode(ANALOG_INPUT_2, INPUT);
  
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);


  Serial.begin(9600);
  deb.activate();
  deb.dprintln(F("STUP"));
  delay(100);
  deb.dprintln(F("Arduino Booting Up"));
  delay(100);
  
  gps_setup();
  
  //gps_refresh();

  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt_handling, FALLING);

//Display
#if DISPLAY_MODULE == 1

  //Display
  u8x8.begin();
  u8x8.setPowerSave(0);
  
#endif

 #if MOTOR_TYPE == 1
 exout.current_step = 40000;

  while(digitalRead(OBEN_ENDSCHALTER)){
    exout.setStep(1);
    exout.push();
    delay(STEP_COOLDOWN_TIME);
    if (exout.current_step == 0){
      exout.current_step = 40000;
    }
  }
  for(int i = 4; i<8; i++){
    exout.digitalSet(i, 0);
  }
  exout.push();
  deb.dprintln("Adjustment done");
  exout.current_step = 0;
#else
  analogWrite(PWM_1, 40);
  analogWrite(PWM_2, 40);
#endif
  

  deb.dprintln(F("EOM"));
  deb.stop();
  setTime(0);
  zeiten.loop_zeit = now();
  zeiten.GPS_wecker = zeiten.loop_zeit;
  zeiten.Standard_wecker = zeiten.loop_zeit;
  zeiten.display_wecker = zeiten.loop_zeit;
  zeiten.PIreport_wecker = zeiten.loop_zeit;



  
}
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//=======================================SETUP=======================================//



//========================================LOOP=======================================//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
void loop() {
  //Timing
  zeiten.loop_zeit = now();

  //GPS
  if (zeiten.loop_zeit >= zeiten.GPS_wecker) {
    gpsRefresh();
  }
    
  //Calculate
  if (zeiten.loop_zeit >= zeiten.Standard_wecker) {
    Standard_Calculations();
  }
  
    
  
  //Picode
#if PI_MODULE == 1

  if (Serial.available() > 2) {
    deb.dprintln(F("Serial recognized"));
    PI_Calculations();    
  }

    
     
  
#endif

  //Interrupt
  if (zeiten.interrupt) {
    deb.dprintln("//==INTERRUPT==//");
    zeiten.interrupt = false;
    soll.toorstatus = !(soll.toorstatus % 2);
    soll.zaunstatus = 0;
    zeiten.GPS_wecker = zeiten.loop_zeit + INTERRUPT_OVERRIDE_TIME;
    zeiten.Standard_wecker = zeiten.loop_zeit + INTERRUPT_OVERRIDE_TIME;
  }
  //execute
  execute();

  //sensors
  sense();
  

  //Picode
#if PI_MODULE == 1

  if (zeiten.loop_zeit >= zeiten.PIreport_wecker) {
    pi.log();
    zeiten.PIreport_wecker = zeiten.loop_zeit + PIREPORT_INTERVALL;
  }
  
#endif

#if DISPLAY_MODULE == 1

  //Display
  if (zeiten.loop_zeit >= zeiten.display_wecker) {
    update_display();
    zeiten.display_wecker = zeiten.loop_zeit + DISPLAY_INTERVALL;
  }
  
#endif
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//========================================LOOP=======================================//
