/* RC Car programm
   Arduino Nano, L298N (motor driver), servo motor 9g
   About programm:
   -
   -
   -
   ----------------------
   Autor - Mykhailo Bilyi
*/

#include <Servo.h> //add Servo library
#include <SoftwareSerial.h>
#include "Wire.h"
#include <MPU6050_light.h>

Servo myservo; //initialize Servo function
SoftwareSerial esp(2, 3); //Rx, Tx
MPU6050 mpu(Wire);

#define servo 4 //Servo motor pin
#define IN1 8 //Forward main motor bool listener pin
#define IN2 10 //Backward main motor bool listener pin
#define ENA 11 //PWM listener pin (if 0 > motor isn't work)
int go;
String sdata="";
int rotate;
bool manualControl = false;
bool myGyro = false;

int minSpeed = 50; //normal - 150
int maxSpeed = 180; // normal - 234
int superSpeed = 255;

float multiplier = maxSpeed - minSpeed;
float maxMultiplier = superSpeed - minSpeed;
float coefficient = multiplier / maxMultiplier;
float superMultiplier = multiplier / coefficient;

void setup() {
  Serial.begin(9600);
  esp.begin(9600);
  pinMode(servo, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  myservo.attach(servo); //initialize and attach servomotor
  if (myGyro) {
    Wire.begin(); //this start i2c interface
    mpu.begin(); //initialize and start MPU sensor library
    Serial.println(F("Calculating gyro offset, do not move MPU6050"));
    delay(1000);
    mpu.calcGyroOffsets();
    Serial.println("Done!\n");
    delay(3000);
  }

  Serial.println("Ready");
}

int motorMode(float power, bool side = 0, bool mode = 0 ) {
  //sides: 0 - forward (default), 1 - backward
  //modes: 0 - standart (default), 1 - fast
  float fmotorSpeed = 0;

  if (mode == false & power > 0 & power <= 100) { //if normal mode
    fmotorSpeed = power * multiplier * 0.01 + minSpeed;
  }
  else if (mode == true & power > 0 & power <= 100) { //if turbo mode
    fmotorSpeed = power * superMultiplier * 0.01 + minSpeed;
  }
  else { //if number is bigger - than stop all motors.
    fmotorSpeed = 0;
  }

  int motorSpeed = fmotorSpeed;

  int modeFactor = 1;
  if (power >= 0 & power <= 100) {
    if (side == false) {
      digitalWrite(IN1, HIGH); //forward motor 1
      digitalWrite(IN2, LOW);
      analogWrite(ENA, motorSpeed);
      Serial.println("MOTOR FORWARD. SPEED: ");
      Serial.println(motorSpeed);
    } else {
      digitalWrite(IN1, LOW); //backward motor 1
      digitalWrite(IN2, HIGH);
      analogWrite(ENA, motorSpeed);
      Serial.println("MOTOR BACKWARD. SPEED: ");
      Serial.println(motorSpeed);
    }
    /*
      switch (side){
      case false:
        digitalWrite(IN1, LOW); //forward motor 1
        digitalWrite(IN2, HIGH);
        analogWrite(ENA, motorSpeed);
        Serial.println("MOTOR FORWARD");
        Serial.println(motorSpeed);
        break;
      case true:
        digitalWrite(IN1, HIGH); //forward motor 1
        digitalWrite(IN2, LOW);
        analogWrite(ENA, motorSpeed);
        Serial.println("MOTOR BACKWARD");
        Serial.println(motorSpeed);
        break;
      }
    */
  } else {
    Serial.println("Error. Power value is too big. Try value from 0 to 100");
  }
  return 0;
}

int servoMode(int angle, int correct = 0) {
  // numbers between 0-180, 90-normal position
  if (angle >= 0 & angle <= 180) {
    myservo.write(angle);
    Serial.println("STEER ANGLE: ");
    Serial.println(angle);
  }
  return 0;
}

void loop() {
  //go = motorMode(speed, back/forward, normal/fast);
  //rotate = servoMode(angle);
  /*
    if(myGyro){
    mpu.update();
    Serial.print(F("ANGLE      X : "));Serial.print(mpu.getAngleX());
    Serial.print("\tY : ");Serial.print(mpu.getAngleY());
    Serial.print("\tZ : ");Serial.println(mpu.getAngleZ());
    Serial.println(F("=======================================================\n"));
    }
  */
  cmdInterrupt();
}

void cmdInterrupt(){
  byte ch;
  String valStr;
  int val;

  if(esp.available()){
    ch = esp.read();

    sdata += (char)ch;

    if (ch=='\r'){
      sdata.trim();

      switch(sdata.charAt(0)){
        case 'r':
          if (sdata.length()>1){
            valStr = sdata.substring(1);
            val = valStr.toInt();
            Serial.print("Rotate angle: ");
            Serial.println(val);
            rotate = servoMode(val);
          }
          break;
        case 'f':
          if (sdata.length()>1){
            valStr = sdata.substring(1);
            val = valStr.toInt();
            Serial.print("Speed value: ");
            Serial.println(val);
            go = motorMode(val, 0);
          }
          break;
        case 'b':
          if (sdata.length()>1){
            valStr = sdata.substring(1);
            val = valStr.toInt();
            Serial.print("Speed value: ");
            Serial.println(val);
            go = motorMode(val, 1);
          }
          break;
      }
      sdata = "";
    }
  }
}
