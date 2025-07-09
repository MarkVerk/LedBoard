#include <Arduino.h>

#define DIN1 2
#define CLK1 3
#define CS1 4

#define DIN2 5
#define CLK2 6
#define CS2 7


uint16_t buf[16] = {0};

int pos = 8;


uint32_t lastMove = 0;

int obstacle_stage = 0;
int obstacle_height = 0;

int since_obstacle = 0;

void upd() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CS1, 0);
    shiftOut(DIN1, CLK1, MSBFIRST, i + 1);
    shiftOut(DIN1, CLK1, MSBFIRST, (buf[7 - i] >> 8) & 0xff);
    shiftOut(DIN1, CLK1, MSBFIRST, i + 1);
    shiftOut(DIN1, CLK1, MSBFIRST, buf[7 - i] & 0xff);
    digitalWrite(CS1, 1);
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(CS2, 0);
    shiftOut(DIN2, CLK2, MSBFIRST, i + 1);
    shiftOut(DIN2, CLK2, MSBFIRST, (buf[15 - i] >> 8) & 0xff);
    shiftOut(DIN2, CLK2, MSBFIRST, i + 1);
    shiftOut(DIN2, CLK2, MSBFIRST, buf[15 - i] & 0xff);
    digitalWrite(CS2, 1);
  }
}


void game_over() {
  for (int i = 0; i < 16; i++) buf[i] = 0xffff;
  upd();
  pos = 8;
  obstacle_stage = 0;
  obstacle_height = 0;
  since_obstacle = 0;
  delay(2000);
  for (int i = 0; i < 16; i++) buf[i] = 0;
  upd();
}

void write_reg(uint8_t reg, uint8_t val) {
  digitalWrite(CS1, 0);
  shiftOut(DIN1, CLK1, MSBFIRST, reg);
  shiftOut(DIN1, CLK1, MSBFIRST, val);
  shiftOut(DIN1, CLK1, MSBFIRST, reg);
  shiftOut(DIN1, CLK1, MSBFIRST, val);
  digitalWrite(CS1, 1);
  digitalWrite(CS2, 0);
  shiftOut(DIN2, CLK2, MSBFIRST, reg);
  shiftOut(DIN2, CLK2, MSBFIRST, val);
  shiftOut(DIN2, CLK2, MSBFIRST, reg);
  shiftOut(DIN2, CLK2, MSBFIRST, val);
  digitalWrite(CS2, 1);
}

void setup() {
  pinMode(DIN1, OUTPUT);
  pinMode(CLK1, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(DIN2, OUTPUT);
  pinMode(CLK2, OUTPUT);
  pinMode(CS2, OUTPUT);
  digitalWrite(CS1, 1);
  digitalWrite(CS2, 1);
  write_reg(0x09, 0); // Decode
  write_reg(0x0a, 5); // Brightness
  write_reg(0x0b, 7); // Scan Limit
  write_reg(0x0c, 1); // Power
  write_reg(0x0f, 0); // Display Test
  upd();
  randomSeed(analogRead(A0));
}



void loop() {
  if (buf[pos] & 0b10) {
    game_over();
  }
  for (int i = 0; i < 16; i++) {
    buf[pos] &= 0b1111111111111110;
  }
  if (millis() - lastMove >= 40) {
    lastMove = millis();
    for (int i = 0; i < 16; i++) {
      buf[i] >>= 1;
    }
    
    if (since_obstacle > 5 && obstacle_stage == 0) {
      obstacle_stage++;
      obstacle_height = random(0, 15);
    }

    since_obstacle++;

    if (obstacle_stage > 0) {
      for (int i = 0; i < obstacle_height - 2; i++) {
        buf[i] |= 1 << 15;
      }
      for (int i = obstacle_height + 1; i < 16; i++) {
        buf[i] |= 1 << 15;
      }
      obstacle_stage++;
      if (obstacle_stage > 3) {
        obstacle_stage = 0;
      }
      since_obstacle = 0;
    }
  }
  if (buf[pos] & 0b11111) {
    for (int i = 0; i < 16; i++) {
      if (!(buf[i] & 0b11111)) {
        pos = i;
        break;
      }
    }
  }
  buf[pos] |= 1;
  upd();
}