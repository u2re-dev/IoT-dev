// Put your own I2C address here
#define I2C_ADDR 0x32

//
#include <Wire.h>
#include <Keypad.h>

const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {0b0000+1, 0b0001+1, 0b0010+1, 0b0011+1},
  {0b0100+1, 0b0101+1, 0b0110+1, 0b0111+1},
  {0b1000+1, 0b1001+1, 0b1010+1, 0b1011+1},
  {0b1100+1, 0b1101+1, 0b1110+1, 0b1111+1}
};
byte rowPins[ROWS] = { 9, 8, 7, 6 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 5, 4, 3, 2 }; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



//
void I2C_TxHandler(void)
{
  Wire.write(uint8_t(keypad.getKey()));
}

//
void setup(){
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Wire.begin(I2C_ADDR);
  Wire.onRequest(I2C_TxHandler);
}

//
void loop(){

}
