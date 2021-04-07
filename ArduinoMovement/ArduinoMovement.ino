#include <SoftwareSerial.h>
SoftwareSerial BT(0, 1); // Arduino RX, TX

char inChar;

void setup() {
  // put your setup code here, to run once:
  pinMode(7,OUTPUT);//IN2
  pinMode(8,OUTPUT);//IN3
  pinMode(9,OUTPUT);//ENA
  pinMode(11,OUTPUT);//ENB
  pinMode(12,OUTPUT);//IN4
  pinMode(4,OUTPUT);//IN1
  pinMode(2,OUTPUT);//LED
  digitalWrite(7,LOW);
  digitalWrite(8,LOW);
  digitalWrite(12,LOW);
  digitalWrite(4,LOW);
  digitalWrite(2,LOW);
  analogWrite(11,0);
  analogWrite(9,0);
  Serial.begin(9600);
  BT.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available()&& BT.available()){
    inChar=BT.read();
    doStuff();
  }
}

void doStuff(){
  if (inChar=='o'){
    digitalWrite(2,HIGH);
  }
  else if (inChar=='c'){
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    digitalWrite(12,LOW);
    digitalWrite(4,LOW);
    digitalWrite(2,LOW);
  }
  else if (inChar=='f'){
    analogWrite(11,160);
    analogWrite(9,160);
    digitalWrite(4,LOW);
    digitalWrite(7,HIGH);
    digitalWrite(8,HIGH);
    digitalWrite(12,LOW);
    Serial.println('f');
  }
  else if (inChar=='l'){
    analogWrite(11,0);
    analogWrite(9,100);
    digitalWrite(4,LOW);
    digitalWrite(7,HIGH);
    Serial.println('l');
  }
  else if (inChar=='r'){
    analogWrite(11,100);
    analogWrite(9,0);
    digitalWrite(8,HIGH);
    digitalWrite(12,LOW);
    Serial.println('r');
  }
  else if (inChar=='n'){
    digitalWrite(4,LOW);
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    digitalWrite(12,LOW);
    Serial.println('n');
  }
  else if (inChar=='R'){
    analogWrite(11,160);
    analogWrite(9,100);
    Serial.println('R');
  }
  else if (inChar=='L'){
    analogWrite(11,100);
    analogWrite(9,160);
    Serial.println('L');
  }
}
