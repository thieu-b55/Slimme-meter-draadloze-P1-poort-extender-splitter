/*
* MIT License
*
* Copyright (c) 2025 thieu-b55
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


#include <Arduino.h>

HardwareSerial Serial3(PB11,PB10);      // RX, TX  

#define BLINKIE           PC13
#define CRC_OK            PA7
#define CRC_ERROR         PB1
#define OE                PB12

#define MAX_LENGTE_INT    1750

bool uitlezen_ok_bool = false;
bool einde_bool = false;
bool data_ok_bool = false;

char lees_char;
char char_0 = 0x00;
char char_1 = 0x00;
char char_2 = 0x00;
char char_3 = 0x00;
char char_4 = 0x00;
char char_5 = 0x00;
char char_6 = 0x00;
char buffer_char[2000];
char crc_char[4];

int rx_pos_int = 0;
int tx_pos_int = 0;

unsigned int crc_16_uint = 0;
unsigned int crc_uint = 0;

void setup() {
  delay(2000);
  pinMode(OE, INPUT_PULLDOWN);
  pinMode(BLINKIE, OUTPUT);
  pinMode(CRC_OK, OUTPUT);
  pinMode(CRC_ERROR, OUTPUT);
  digitalWrite(BLINKIE, false);
  digitalWrite(CRC_OK, false);
  digitalWrite(CRC_ERROR, false);
  Serial.begin(9600);
  Serial3.begin(115200);
}

void loop() {
  while((Serial.available()) && (uitlezen_ok_bool == false) && (rx_pos_int < MAX_LENGTE_INT)){
    lees_char = Serial.read();
    char_6 = char_5;
    char_5 = char_4;
    char_4 = char_3;   
    char_3 = char_2;
    char_2 = char_1;
    char_1 = char_0;
    char_0 = lees_char;
    buffer_char[rx_pos_int] = lees_char;
    if(einde_bool == false){
      crc_uint ^= (unsigned int)buffer_char[rx_pos_int];
      for(int x = 8; x > 0; x--){    
        if((crc_uint & 0x0001) != 0){      
          crc_uint >>= 1;                    
          crc_uint ^= 0xA001;
        }
        else{                           
          crc_uint >>= 1;  
        }      
      }
      if(lees_char == 0x21){
        einde_bool = true;
      }
    }
    rx_pos_int ++;
    if(char_6 == 0x21){
      digitalWrite(BLINKIE, (digitalRead(BLINKIE) ^ 1));
      uitlezen_ok_bool = true;
      einde_bool = false;
      crc_char[3] = char_2;
      crc_char[2] = char_3;
      crc_char[1] = char_4;
      crc_char[0] = char_5;
      crc_16_uint = (unsigned int)strtol(crc_char, NULL, 16);
      if(crc_uint == crc_16_uint){
        data_ok_bool = true;
        digitalWrite(CRC_OK, true);
        digitalWrite(CRC_ERROR, false);
      }
      else{
        data_ok_bool = false;
        digitalWrite(CRC_ERROR, true);
        digitalWrite(CRC_OK, false);
      }
      crc_uint = 0;
    }
  }
  if((uitlezen_ok_bool) && (data_ok_bool) && (digitalRead(OE))){
    tx_pos_int = 0;
    while(tx_pos_int < rx_pos_int){
      Serial3.write(buffer_char[tx_pos_int]);
      tx_pos_int ++;
    }
    uitlezen_ok_bool = false;
    data_ok_bool == false;
    rx_pos_int = 0;
  }
  if(((uitlezen_ok_bool) && (data_ok_bool == false)) || (rx_pos_int >= MAX_LENGTE_INT) ){
    crc_uint = 0;
    uitlezen_ok_bool = false;
    data_ok_bool == false;
    rx_pos_int = 0;
  }
  if((uitlezen_ok_bool) && (digitalRead(OE) == false)){
    crc_uint = 0;
    uitlezen_ok_bool = false;
    data_ok_bool == false;
    rx_pos_int = 0;
  }
}
