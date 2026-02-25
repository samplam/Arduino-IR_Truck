#include <CytronEZMP3.h> // For the MP3 player.

#include <IRremote.h>
const uint8_t RECV_PIN = 2; // INPUT-IR receiver on pin 2.
IRrecv irrecv(RECV_PIN);
decode_results results;

#include "LedControl.h"
LedControl lc = LedControl(12, 11, 10, 1); // (PIN # for "DIN” on MAX7219, PIN # for “CLK” on MAX7219, PIN # for “CS” on MAX7219, qty of MAX7219 chips)

#include "Wire.h" // For I/O expansion board.


byte J[8] = {B01111110, B01111110, B00011000, B00011000, B11011000, B11011000, B11011000, B01110000};
byte O[8] = {B00111100, B01111110, B11000011, B11000011, B11000011, B11000011, B01111110, B00111100};
byte A[8] = {B00011000, B00111100, B01100110, B01100110, B01111110, B01111110, B11000011, B11000011};
byte N[8] = {B11000011, B11000011, B11100011, B11110011, B11011011, B11001111, B11000111, B11000011};
byte Heureux[8] = {B00111100, B01000010, B10100101, B10000001, B10100101, B10011001, B01000010, B00111100};
byte Neutre[8] = {B00111100, B01000010, B10100101, B10000001, B10111101, B10000001, B01000010, B00111100};
byte Triste[8] = {B00111100, B01000010, B10100101, B10000001, B10011001, B10100101, B01000010, B00111100};
const uint8_t PinAvMoteurGauche = 4; // OUTPUT-Forward direction left motor on pin 4.
const uint8_t PinArMoteurGauche = 5; // OUTPUT-Reverse direction left motor on pin 5.
const uint8_t PinPWMMoteurGauche = 6; // OUTPUT-PWM left motor on pin 6.
const uint8_t PinAvMoteurDroit = 7; // OUTPUT-Forward direction right motor on pin 7.
const uint8_t PinArMoteurDroit = 8; // OUTPUT-Reverse direction right motor on pin 8.
const uint8_t PinPWMMoteurDroit = 9; // OUTPUT-PWM right motor on pin 9.
uint8_t SortieAvMoteurGauche = 0; // Intermediate variable for forward direction output of the left motor.
uint8_t SortieArMoteurGauche = 0; // Intermediate variable for reverse direction output of the left motor.
uint8_t SortiePWMMoteurGauche = 0; // Intermediate variable for PWM output of the left motor.
uint8_t SortieAvMoteurDroit = 0; // Intermediate variable for forward direction output of the right motor.
uint8_t SortieArMoteurDroit = 0; // Intermediate variable for reverse direction output of the right motor.
uint8_t SortiePWMMoteurDroit = 0; // Intermediate variable for PWM output of the right motor.
uint8_t EntreeAvancer = 0; // Intermediate variable for the forward IR input.
uint8_t EntreeReculer = 0; // Intermediate variable for the reverse IR input.
uint8_t EntreeTournerG = 0; // Intermediate variable for the left turn IR input.
uint8_t EntreeTournerD = 0; // Intermediate variable for the right turn IR input.
uint8_t EntreeArreter = 0; // Intermediate variable for the stop IR input.
uint8_t EntreeJouer = 0; // Intermediate variable for the music IR input.
uint8_t Jouer = 1; // Intermediate variable to play the music.
uint8_t EntreeGirophares = 0; // Intermediate variable for the gyrophare IR input.
uint8_t Girophares = 0; // Intermediate variable for the gyrophare.
uint8_t TournerG = 0; // Variable to know if the truck is currently left turning.
uint8_t TournerD = 0; // Variable to know if the truck is currently right turning.
uint8_t LumieresGPIOA = 0b01111001; // Used to control the various lights of the truck. Put pins 1, 2 and 7 of GPIOA to ON. Inverted outputs because of sink mode.
uint8_t LumieresGPIOB = 0b11111110; // Used to control the various lights of the truck. Put pin 0 of GPIOB to ON. Inverted outputs because of sink mode.
uint8_t EnvoiLumieres1 = 0; // Used to control the lights.
uint8_t EnvoiLumieres2 = 0; // Used to control the lights.
uint16_t Chanson = 1; // Used to play music.
unsigned long currentMillis; // Used to accumulate milliseconds since the beginning of the program.
unsigned long ClignotantsMillis; // Used for the flashers.
const uint16_t ClignotantsDelay = 500; // Used for the flashers.

void setup() {
  irrecv.enableIRIn();
  irrecv.blink13(false);
  lc.shutdown(0, false); // (# of MAX7219, put to "false" to work).
  lc.setIntensity(0, 8); // (# of MAX7219, luminosity).
  pinMode (PinAvMoteurGauche, OUTPUT); // Put pin 4 in output mode.
  pinMode (PinArMoteurGauche, OUTPUT); // Put pin 5 in output mode.
  pinMode (PinPWMMoteurGauche, OUTPUT); // Put pin 6 in output mode.
  pinMode (PinAvMoteurDroit, OUTPUT); // Put pin 7 in output mode.
  pinMode (PinArMoteurDroit, OUTPUT); // Put pin 8 in output mode.
  pinMode (PinPWMMoteurDroit, OUTPUT); // Put pin 9 in output mode.
  Wire.begin(); // For I/O expansion board.
  Wire.beginTransmission(0x20); // I/O expansion board address.
  Wire.write(0x00); // Register IODIRA.
  Wire.write(0x00); // Put every A ports as outputs.
  Wire.write(0x12); // Select GPIOA pins.
  Wire.write(LumieresGPIOA);
  Wire.endTransmission();
  Wire.beginTransmission(0x20); // I/O expansion board address.
  Wire.write(0x01); // Register IODIRB.
  Wire.write(0x00); // Put every B ports as outputs.
  Wire.write(0x13); // Select GPIOB pins.
  Wire.write(LumieresGPIOB);
  Wire.endTransmission();
}

void loop() {

  /* LG remote codes :

    Button Power ON =
    Button Eject =
    Button 1 = B4B4DC23
    Button 2 = B4B43CC3
    Button 3 = B4B4BC43
    Button 4 = B4B47C83
    Button 5 =
    Button 6 =
    Button 7 = B4B4827D
    Button 8 = B4B442BD
    Button 9 = B4B4C23D
    Button Erase =
    Button 0 = B4B422DD
    Button Repeate = 4AB0F7B6
    Button Previous =
    Button Reverse =
    Button Forward =
    Button Next =
    Button Stop =
    Button Play = B4B48C73
    Button Pause =
    Button Home =
    Button ID music =
    Button Info =
    Button Up arrow = B4B4E21D
    Button Left arrow = B4B49A65
    Button Enter = B4B41AE5
    Button Right arrow = B4B45AA5
    Button Down arrow = B4B412ED
    Button Return =
    Button Title =
    Button Menu =
    Button Red =
    Button Green =
    Button Yellow =
    Button Blue =
    Button Marker =
    Button Search =
    Button Zoom =
    Button TV ON =
    Button Volume + =
    Button Channel + =
    Button Input =
    Button Volume - =
    Button Channel - =

  */

  // Stores the milliseconds since the program started.
  currentMillis = millis();

  if (irrecv.decode(&results)) { // IR input processing.

    // Forward.
    if (results.value == 0xB4B4E21D) {
      EntreeAvancer = 1;
    }

    // Backward.
    if (results.value == 0xB4B412ED) {
      EntreeReculer = 1;
    }

    // Turn left.
    if (results.value == 0xB4B49A65) {
      EntreeTournerG = 1;
    }

    // Turn right.
    if (results.value == 0xB4B45AA5) {
      EntreeTournerD = 1;
    }

    // Stop.
    if (results.value == 0xB4B41AE5) {
      EntreeArreter = 1;
    }

    // Gyrophares.
    if (results.value == 0xB4B422DD) {
      EntreeGirophares = 1;
    }

    // Displays "J".
    if (results.value == 0xB4B4DC23) {
      lc.setRow(0, 0, J[0]);
      lc.setRow(0, 1, J[1]);
      lc.setRow(0, 2, J[2]);
      lc.setRow(0, 3, J[3]);
      lc.setRow(0, 4, J[4]);
      lc.setRow(0, 5, J[5]);
      lc.setRow(0, 6, J[6]);
      lc.setRow(0, 7, J[7]);
    }

    // Displays "O".
    if (results.value == 0xB4B43CC3) {
      lc.setRow(0, 0, O[0]);
      lc.setRow(0, 1, O[1]);
      lc.setRow(0, 2, O[2]);
      lc.setRow(0, 3, O[3]);
      lc.setRow(0, 4, O[4]);
      lc.setRow(0, 5, O[5]);
      lc.setRow(0, 6, O[6]);
      lc.setRow(0, 7, O[7]);
    }

    // Displays "A".
    if (results.value == 0xB4B4BC43) {
      lc.setRow(0, 0, A[0]);
      lc.setRow(0, 1, A[1]);
      lc.setRow(0, 2, A[2]);
      lc.setRow(0, 3, A[3]);
      lc.setRow(0, 4, A[4]);
      lc.setRow(0, 5, A[5]);
      lc.setRow(0, 6, A[6]);
      lc.setRow(0, 7, A[7]);
    }

    // Displays "N".
    if (results.value == 0xB4B47C83) {
      lc.setRow(0, 0, N[0]);
      lc.setRow(0, 1, N[1]);
      lc.setRow(0, 2, N[2]);
      lc.setRow(0, 3, N[3]);
      lc.setRow(0, 4, N[4]);
      lc.setRow(0, 5, N[5]);
      lc.setRow(0, 6, N[6]);
      lc.setRow(0, 7, N[7]);
    }

    // Displays "Happy".
    if (results.value == 0xB4B4827D) {
      lc.setRow(0, 0, Heureux[0]);
      lc.setRow(0, 1, Heureux[1]);
      lc.setRow(0, 2, Heureux[2]);
      lc.setRow(0, 3, Heureux[3]);
      lc.setRow(0, 4, Heureux[4]);
      lc.setRow(0, 5, Heureux[5]);
      lc.setRow(0, 6, Heureux[6]);
      lc.setRow(0, 7, Heureux[7]);
    }

    // Displays "Neutral".
    if (results.value == 0xB4B442BD) {
      lc.setRow(0, 0, Neutre[0]);
      lc.setRow(0, 1, Neutre[1]);
      lc.setRow(0, 2, Neutre[2]);
      lc.setRow(0, 3, Neutre[3]);
      lc.setRow(0, 4, Neutre[4]);
      lc.setRow(0, 5, Neutre[5]);
      lc.setRow(0, 6, Neutre[6]);
      lc.setRow(0, 7, Neutre[7]);
    }

    // Displays "Sad".
    if (results.value == 0xB4B4C23D) {
      lc.setRow(0, 0, Triste[0]);
      lc.setRow(0, 1, Triste[1]);
      lc.setRow(0, 2, Triste[2]);
      lc.setRow(0, 3, Triste[3]);
      lc.setRow(0, 4, Triste[4]);
      lc.setRow(0, 5, Triste[5]);
      lc.setRow(0, 6, Triste[6]);
      lc.setRow(0, 7, Triste[7]);
    }

    // Play music.
    if (results.value == 0xB4B48C73) {
      EntreeJouer = 1;
    }

    irrecv.resume();
  }

  // Commands management.
  if (EntreeAvancer == 1) {
    TournerG = 0;
    TournerD = 0;
    if (SortieArMoteurGauche == 1 && SortieArMoteurDroit == 1) {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 0;
      SortiePWMMoteurDroit = 0;
      EntreeAvancer = 0;
    }
    else {
      SortieAvMoteurGauche = 1;
      SortieAvMoteurDroit = 1;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 255;
      SortiePWMMoteurDroit = 255;
      EntreeAvancer = 0;
    }
  }

  if (EntreeReculer == 1) {
    TournerG = 0;
    TournerD = 0;
    if (SortieAvMoteurGauche == 1 && SortieAvMoteurDroit == 1) {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 0;
      SortiePWMMoteurDroit = 0;
      EntreeReculer = 0;
    }
    else {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 1;
      SortieArMoteurDroit = 1;
      SortiePWMMoteurGauche = 255;
      SortiePWMMoteurDroit = 255;
      EntreeReculer = 0;
    }
  }

  if (EntreeTournerG == 1) {
    TournerG = 1;
    TournerD = 0;
    if ((SortieAvMoteurGauche == 1 && SortieAvMoteurDroit == 1) || (SortieArMoteurGauche == 1 && SortieArMoteurDroit == 1)) {
      SortiePWMMoteurGauche = 100;
      SortiePWMMoteurDroit = 255;
      EntreeTournerG = 0;
    }
    else if (SortieAvMoteurGauche == 1 && SortieArMoteurDroit == 1) {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 0;
      SortiePWMMoteurDroit = 0;
      EntreeTournerG = 0;
    }
    else {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 1;
      SortieArMoteurGauche = 1;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 100;
      SortiePWMMoteurDroit = 100;
      EntreeTournerG = 0;
    }
  }

  if (EntreeTournerD == 1) {
    TournerG = 0;
    TournerD = 1;
    if ((SortieAvMoteurGauche == 1 && SortieAvMoteurDroit == 1) || (SortieArMoteurGauche == 1 && SortieArMoteurDroit == 1)) {
      SortiePWMMoteurGauche = 255;
      SortiePWMMoteurDroit = 100;
      EntreeTournerD = 0;
    }
    else if (SortieArMoteurGauche == 1 && SortieAvMoteurDroit == 1) {
      SortieAvMoteurGauche = 0;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 0;
      SortiePWMMoteurGauche = 0;
      SortiePWMMoteurDroit = 0;
      EntreeTournerD = 0;
    }
    else {
      SortieAvMoteurGauche = 1;
      SortieAvMoteurDroit = 0;
      SortieArMoteurGauche = 0;
      SortieArMoteurDroit = 1;
      SortiePWMMoteurGauche = 100;
      SortiePWMMoteurDroit = 100;
      EntreeTournerD = 0;
    }
  }

  if (EntreeArreter == 1) {
    TournerG = 0;
    TournerD = 0;
    SortieAvMoteurGauche = 0;
    SortieAvMoteurDroit = 0;
    SortieArMoteurGauche = 0;
    SortieArMoteurDroit = 0;
    SortiePWMMoteurGauche = 0;
    SortiePWMMoteurDroit = 0;
    EntreeArreter = 0;
  }

  if (EntreeGirophares == 1) {
    Girophares = Girophares + 1;
    EntreeGirophares = 0;
    if (Girophares >= 2) {
      Girophares = 0;
    }
  }

  // Lights management.
  if ((currentMillis - ClignotantsMillis <= ClignotantsDelay) && (EnvoiLumieres1 == 0)) {
    if (SortieAvMoteurGauche == 1 || SortieArMoteurGauche == 1 || SortieAvMoteurDroit == 1 || SortieArMoteurDroit == 1) {
      bitWrite(LumieresGPIOA, 7, 1);
      bitWrite(LumieresGPIOB, 0, 1);
    }
    else {
      bitWrite(LumieresGPIOA, 7, 0);
      bitWrite(LumieresGPIOB, 0, 0);
    }
    if (TournerG == 1) {
      bitWrite(LumieresGPIOA, 0, 0);
      bitWrite(LumieresGPIOA, 6, 0);
    }
    else {
      bitWrite(LumieresGPIOA, 0, 1);
      bitWrite(LumieresGPIOA, 6, 1);
    }
    if (TournerD == 1) {
      bitWrite(LumieresGPIOA, 3, 0);
      bitWrite(LumieresGPIOB, 1, 0);
    }
    else {
      bitWrite(LumieresGPIOA, 3, 1);
      bitWrite(LumieresGPIOB, 1, 1);
    }
    if (Girophares == 1) {
      bitWrite(LumieresGPIOA, 4, 0);
      bitWrite(LumieresGPIOA, 5, 1);
    }
    else {
      bitWrite(LumieresGPIOA, 4, 1);
      bitWrite(LumieresGPIOA, 5, 1);
    }
    Wire.beginTransmission(0x20); // Expansion chip address.
    Wire.write(0x12); // Select the GPIOA pins.
    Wire.write(LumieresGPIOA);
    Wire.endTransmission();
    Wire.beginTransmission(0x20); // Expansion chip address.
    Wire.write(0x13); // Select the GPIOB pins.
    Wire.write(LumieresGPIOB);
    Wire.endTransmission();
    EnvoiLumieres1 = 1;
  }
  if (((currentMillis - ClignotantsMillis > ClignotantsDelay) && (currentMillis - ClignotantsMillis <= (2 * ClignotantsDelay))) && EnvoiLumieres2 == 0) {
    if (SortieAvMoteurGauche == 1 || SortieArMoteurGauche == 1 || SortieAvMoteurDroit == 1 || SortieArMoteurDroit == 1) {
      bitWrite(LumieresGPIOA, 7, 1);
      bitWrite(LumieresGPIOB, 0, 1);
    }
    else {
      bitWrite(LumieresGPIOA, 7, 0);
      bitWrite(LumieresGPIOB, 0, 0);
    }
    if (TournerG == 1) {
      bitWrite(LumieresGPIOA, 0, 1);
      bitWrite(LumieresGPIOA, 6, 1);
    }
    if (TournerD == 1) {
      bitWrite(LumieresGPIOA, 3, 1);
      bitWrite(LumieresGPIOB, 1, 1);
    }
    if (Girophares == 1) {
      bitWrite(LumieresGPIOA, 4, 1);
      bitWrite(LumieresGPIOA, 5, 0);
    }
    else {
      bitWrite(LumieresGPIOA, 4, 1);
      bitWrite(LumieresGPIOA, 5, 1);
    }
    Wire.beginTransmission(0x20); // Expansion chip address.
    Wire.write(0x12); // Select the GPIOA pins.
    Wire.write(LumieresGPIOA);
    Wire.endTransmission();
    Wire.beginTransmission(0x20); // Expansion chip address.
    Wire.write(0x13); // Select the GPIOB pins.
    Wire.write(LumieresGPIOB);
    Wire.endTransmission();
    EnvoiLumieres2 = 1;
  }
  if (currentMillis - ClignotantsMillis >= (2 * ClignotantsDelay)) {
    ClignotantsMillis = currentMillis;
    EnvoiLumieres1 = 0;
    EnvoiLumieres2 = 0;
  }

  // Music playing management.
  if (EntreeJouer == 1) {
    Jouer = Jouer + 1;
    EntreeJouer = 0;
  }
  if (Jouer == 1) {

    Jouer = Jouer + 1;
  }
  if (Jouer == 3) {

    Jouer = 0;
  }

  digitalWrite (PinAvMoteurGauche, SortieAvMoteurGauche);
  digitalWrite (PinArMoteurGauche, SortieArMoteurGauche);
  analogWrite (PinPWMMoteurGauche, SortiePWMMoteurGauche);
  digitalWrite (PinAvMoteurDroit, SortieAvMoteurDroit);
  digitalWrite (PinArMoteurDroit, SortieArMoteurDroit);
  analogWrite (PinPWMMoteurDroit, SortiePWMMoteurDroit);
}
