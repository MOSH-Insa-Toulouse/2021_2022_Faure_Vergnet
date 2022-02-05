#include "rn2xx3.h"
#include <SoftwareSerial.h>
#include <string.h>
#include <MsTimer2.h> 
#include <avr/sleep.h>

#define LED_OFF HIGH
#define LED_ON  LOW

#define MESURE_PERIOD 30


/* DECLARATION DES PINs */
const int pinGasSensor = A0;


const int pinInterruptUp = 2;
const int pinInterruptDown = 3;

const int pinRedLED = 4;
const int pinBlueLED = 5;
const int pinGreenLED = 6;
const int pinYellowLED = 7;

const int pinBuzzer = 9;

const int pinRX = 10;
const int pinTX = 11;
const int pinRST = 12;
/* FIN DECLARATION DES PINs */

/* SELECTION DES MODULES UTILISES */
const bool useLoRa = true;
const bool useLEDs = true;
const bool useBuzzer = true;
const bool useInterrupts = true;
const bool usePeriodic = false;
const bool usePowerSaveMode = false;
/* FIN SELECTION DES MODULES UTILISES */

/* VARIABLES GLOBALE D'ETAT */
volatile bool gasDetected = false;
volatile bool endGasDetected = false;
volatile bool lecture = false;
/* FIN VARIABLES GLOBALE D'ETAT */

/* DECLARATION DES VARIABLES GLOBALES */
SoftwareSerial mySerial(pinRX, pinTX); // RX, TX
rn2xx3 myLora(mySerial);
/* FIN DECLARATION DES VARIABLES GLOBALES */




/******************************************/
/******************************************/
/************* DEBUT DU SETUP *************/
/******************************************/
/******************************************/

/* Fonction de setup du module LoRa */
void setupLoRa(boolean used) {
  if (used) {
    mySerial.begin(9600); //serial port to radio
    
    //reset rn2483
    pinMode(pinRST, OUTPUT);
    digitalWrite(pinRST, LOW);
    delay(500);
    digitalWrite(pinRST, HIGH);
    delay(100); //wait for the RN2xx3's startup message
    mySerial.flush();
  
    //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
    myLora.autobaud();
  
    //check communication with radio
    String hweui = myLora.hweui();
    while(hweui.length() != 16)
    {
      Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
      Serial.println(hweui);
      delay(10000);
      hweui = myLora.hweui();
    }
  
    //print out the HWEUI so that we can register it via ttnctl
    Serial.println("When using OTAA, register this DevEUI: ");
    Serial.println(myLora.hweui());
    Serial.println("RN2xx3 firmware version:");
    Serial.println(myLora.sysver());

      //configure your keys and join the network
    Serial.println("Trying to join TTN");
    bool join_result = false;
    
    const char *appEui = "0000000000000000";
    const char *appKey = "27AF4F436A03803BE0E96437B8AD270E";
  
    join_result = myLora.initOTAA(appEui, appKey);
  
    while(!join_result)
    {
      Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
      delay(60000); //delay a minute before retry
      join_result = myLora.init();
    }
    Serial.println("Successfully joined TTN");
  }
}

/* Fonction de setup des LEDs */
void setupLEDs(boolean used) {
  if (used) {
    pinMode(pinRedLED, OUTPUT);
    digitalWrite(pinRedLED, LED_OFF);
    pinMode(pinBlueLED, OUTPUT);
    digitalWrite(pinBlueLED, LED_OFF);
    pinMode(pinGreenLED, OUTPUT);
    digitalWrite(pinGreenLED, LED_OFF);
    pinMode(pinYellowLED, OUTPUT);
    digitalWrite(pinYellowLED, LED_OFF);
  }
}

/* Fonction de setup du buzzer */
void setupBuzzer(boolean used) {
  if (used) {
    pinMode(pinBuzzer, OUTPUT);
  }
}

/* Fonction de setup des interruptions */
void setupInterrupt(boolean used) {
  if (used) {
    attachInterrupt(digitalPinToInterrupt(pinInterruptUp), setAlertState, RISING); 
    attachInterrupt(digitalPinToInterrupt(pinInterruptDown), resetAlertState, FALLING); 
  }
}

/* Fonction de setup du Timer */
void setupPeriodic(boolean used) {
  if (used) {
    MsTimer2::set(MESURE_PERIOD*1000, setPeriodicState); 
    MsTimer2::start(); 
  }
}

/* Fonction de setup du power save mode */
void setupPowerSaveMode(boolean used) {
  if (used) {
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  }
}

/* Fonction d'initialisation globale */
void setup()
{
  Serial.begin(9600);
  
  setupLoRa(useLoRa);
  setupLEDs(useLEDs);
  setupBuzzer(useBuzzer);
  setupInterrupt(useInterrupts);
  setupPeriodic(usePeriodic);
  setupPowerSaveMode(usePowerSaveMode);

  
  if (useLEDs) {
    digitalWrite(pinYellowLED, LED_ON);
  }
}

/******************************************/
/******************************************/
/************** FIN DU SETUP **************/
/******************************************/
/******************************************/










/******************************************/
/******************************************/
/************* DEBUT HANDLER **************/
/******************************************/
/******************************************/

void setAlertState() {
   gasDetected = true;
   endGasDetected = false;
}

void resetAlertState() {
   gasDetected = false;
   endGasDetected = true;
}

void setPeriodicState() {
   lecture = true;
}

/******************************************/
/******************************************/
/************** FIN HANDLER ***************/
/******************************************/
/******************************************/









/******************************************/
/******************************************/
/********** DEBUT FONCTIONS TEST **********/
/******************************************/
/******************************************/

void testGasSensor() {
  Serial.print("Valeur lue sur le capteur :  ");
  Serial.print(readValue());
  Serial.println(" V");
  delay(1000);
}

void testLoRa() {
  if (useLoRa) {
    static int i = 0;
    Serial.print("Envoi du TEST n°");
    Serial.println(i);
    String toSend = "TEST" + String(i);
    myLora.tx(toSend);
    Serial.println("Envoi terminé");
    i++;
  } else {
    Serial.println("Veuillez activer le module LoRa");
  }
  delay(20000);
}

void testLEDs() {
  int pause = 50;
  if (useLEDs) {
    digitalWrite(pinYellowLED, LED_OFF);            // RED + BLUE
    delay(pause);
    digitalWrite(pinBlueLED, LED_ON);
    delay(pause);
    digitalWrite(pinRedLED, LED_OFF);               // BLUE + GREEN
    delay(pause);
    digitalWrite(pinGreenLED, LED_ON);
    delay(pause);
    digitalWrite(pinBlueLED, LED_OFF);               // GREEN + YELLOW
    delay(pause);
    digitalWrite(pinYellowLED, LED_ON);
    delay(pause);
    digitalWrite(pinGreenLED, LED_OFF);               // YELLOW + RED
    delay(pause);
    digitalWrite(pinRedLED, LED_ON);
    delay(pause);
  } else {
    Serial.println("Veuillez activer les LEDs");
    delay(20000);
  }
}

void testBuzzer() {
  if (useBuzzer) {
    playAlertPompier();
  } else {
    Serial.println("Veuillez activer le Buzzer");
  }
  delay(10000); 
}

/******************************************/
/******************************************/
/*********** FIN FONCTIONS TEST ***********/
/******************************************/
/******************************************/











/******************************************/
/******************************************/
/************ DEBUT DU PROCESS ************/
/******************************************/
/******************************************/


/* Primitive connection TTN */
void connectTTN() {
  if (useLEDs) {
    digitalWrite(pinBlueLED, LED_ON);
  }
}

/* Primitive disconnection TTN */
void disconnectTTN() {
  if (useLEDs) {
    digitalWrite(pinBlueLED, LED_OFF);
  }
}

/* Primitive du buzzer */
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(pinBuzzer, HIGH);
    delayMicroseconds(tone);
    digitalWrite(pinBuzzer, LOW);
    delayMicroseconds(tone);
  }
}

/* Primitive du buzzer */
void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  
  // play the tone corresponding to the note name
  for (int i = 0; i < sizeof(names); i++) {
  //for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

/* Primitive du buzzer */
void playAlertPompier() {
  int length = 15; // the number of notes
  char notes[] = "bababababababa "; // a space represents a rest
  int beats[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  int tempo = 900;
  
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }
  }
}

/* Primitive de lecture de la valeur du capteur */
float readValue() {
  int value = 0;
  for (int i = 0; i<10; i++) {
    value += analogRead(pinGasSensor);
  }
  return ((float)value/10.0)*5.0/1024.0;
}

/* Gestion de l'alerte */
void gasDetectedHandler() {
  Serial.println("ALERTE présence de gaz !!!"); 
  if (useLEDs) {
    digitalWrite(pinRedLED, LED_ON);
  }
  if (useBuzzer) {
    playAlertPompier();
  }
  if (useLoRa) {
    connectTTN();
    String toSend = "ALERTE présence de gaz !!!";
    myLora.tx(toSend);
    disconnectTTN();
  }
}

/* Gestion de la fin de l'alerte */
void gasDetectionEndHandler() {
  Serial.println("Fin ALERTE présence de gaz !!!"); 
  if (useLoRa) {
    connectTTN();
    String toSend = "Fin ALERTE présence de gaz !!!";
    myLora.tx(toSend);
    disconnectTTN();
  }
  if (useLEDs) {
    digitalWrite(pinRedLED, LED_OFF);
  }
}

/* Fonction de detection du gaz */
void analyseGasAtmosphere() {

  if (lecture) {
    if (useLEDs) {
      digitalWrite(pinGreenLED, LED_ON);
    }
    float sensorValue = readValue();
    Serial.println(sensorValue);
    if (useLoRa) {
      connectTTN();
      String sensorValueStr = String(sensorValue, 1);
      myLora.tx(sensorValueStr);
      disconnectTTN();
    }
    lecture = false;
    if (useLEDs) {
      digitalWrite(pinGreenLED, LED_OFF);
    }
  }

  if (gasDetected) {
    gasDetectedHandler();
    while (!endGasDetected) {
      gasDetectedHandler();
    }
    gasDetectionEndHandler();
  }
  
  if (usePowerSaveMode) {
    sleep_mode();
  }
}

/* Processus */
void loop(){
  //testGasSensor();
  //testLoRa();
  //testLEDs();
  //testBuzzer();
  analyseGasAtmosphere();
}

/******************************************/
/******************************************/
/************* FIN DU PROCESS *************/
/******************************************/
/******************************************/
