/*
fht_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FHT_N 32 // set to 256 point fht

#include <FHT.h> // include the library

const byte pixel[3][4] = {
  {3, 2, 4, 0},
  {7, 5, 8, 1},
  {12, 13, 6, 2}
};

const int soundPin = A0;
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

void setup() {
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  randomSeed(0);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(6, OUTPUT);
}

int counter = 0;
const int s = 16;


//pixel1 band 10
float total1 = 0;
int shu1 = 120;
int shl1 = 80;
bool hold1 = false;
byte v1 = 255;

//pixel2 band 5
float total2 = 0;
int shu2 = 150;
int shl2 = 120;
bool hold2 = false;
byte v2 = 255;

//pixel3 band 3
float total3 = 0;
int shu3 = 120;
int shl3 = 80;
bool hold3 = false;
byte v3 = 255;

const bool logging = true;



void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    sei();

    
    total1 += fht_log_out[10];
    total2 += fht_log_out[5];
    total3 += fht_log_out[13];
    
    if(++counter == s){

      if(random(0, 200) < 2) toggleOther2Cols(2, 4);
      if(random(0, 200) < 2) toggleOther2Cols(7, 8);
      if(random(0, 200) < 2) toggleOther2Cols(13, 6);

      
      /**
       * Pixel 1
       */

      //average
      total1 /= s;
      Serial.print(total1 + 200);
      Serial.print(' ');
      
      if(total1 < shl1) hold1 = false;

      //decay light
      v1 *= 0.95;

      if(!hold1 && total1 > shu1){
        Serial.print(-10);
        Serial.print(' ');
        toggleOther2Cols(2, 4);
        v1 = 255;

        //activate hold1
        hold1 = true;
      }else{
        Serial.print(-30);
        Serial.print(' ');
      }

      
      //write pixel1
      analogWrite(3, pgm_read_byte(&gamma8[255 - v1]));

      //reset average
      total1 = 0;

      /**
       * Pixel 2
       */

      total2 /= s;
      Serial.print(total2 + 250);
      Serial.print(' ');

      if(total2 < shl2) hold2 = false;

      
      //decay light
      v2 *= 0.95;

      if(!hold2 && total2 > shu2){
        Serial.print(20);
        Serial.print(' ');
        toggleOther2Cols(7, 8);
        v2 = 255;

        //activate hold
        hold2 = true;
      }else{
        Serial.print(0);
        Serial.print(' ');
      }
      
      //write pixel
      analogWrite(5, pgm_read_byte(&gamma8[255 - v2]));
      
      //reset average
      total2 = 0;


      /**
       * Pixel 3
       */

      total3 /= s;
      Serial.print(total3 + 300);
      Serial.print(' ');

      if(total3 < shl3) hold3 = false;

      
      //decay light
      v3 *= 0.95;

      if(!hold3 && total3 > shu3){
        Serial.print(50);
        Serial.print(' ');
        toggleOther2Cols(12, 13);
        v3 = 255;

        //activate hold
        hold3 = true;
      }else{
        Serial.print(30);
        Serial.print(' ');
      }
      
      //write pixel
      analogWrite(6, pgm_read_byte(&gamma8[255 - v3]));
      
      //reset average
      total3 = 0;

      //reset counter
      counter = 0;
      Serial.println();
    }
  }
}

void toggleOther2Cols(byte pin1, byte pin2){
  switch(random(0, 3)){
    case 0:
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, LOW);
      break;
    case 1:
      digitalWrite(pin1, HIGH);
      digitalWrite(pin2, LOW);
      break;
    default:
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, HIGH);
  }
}

