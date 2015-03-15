// +----------------------------------------------------------------------------------+
// |    Magla Autonomous Vehicle v3 - ©2015, Antonis Maglaras (maglaras@gmail.com)    |
// +----------------------------------------------------------------------------------+
#include <NewPing.h>
#include <Servo.h>
// ------------------------------------------------------------------------------------
#define TRIGGER_PIN    4        // Ultrasonic Sensor Trigger Pin
#define ECHO_PIN       7        // Ultrasonic Sensor Echo Pin
#define MAX_DISTANCE  250       // Ultrasonic Sensor Maximum Distance (up to 400-500cm)
// ------------------------------------------------------------------------------------
#define HeadServoPin 2          // Head Servo Pin
#define TailServoPin A0         // Tail Servo Pin
// ------------------------------------------------------------------------------------
// motor A connected between A01 and A02
// motor B connected between B01 and B02
#define STBY 10                 // Standby
// Motor A
#define PWMA 6                  // Speed control 
#define AIN1 9                  // Direction
#define AIN2 8                  // Direction
// Motor B
#define PWMB 5                  // Speed control
#define BIN1 11                 // Direction
#define BIN2 12                 // Direction
// ------------------------------------------------------------------------------------
#define LED1Pin 13              // Pin for LED #1 (Get Distance)
#define LED2Pin 3               // Pin for LED #2 (Running)
#define LEFTPin A3              // Pin for LEFT LED
#define RIGHTPin A2             // Pin for RIGHT LED
#define BUTTON1 A4              // Input Button
volatile int Speed;             // Speed Variable

Servo Head;                     // Head (Ultrasonic) Servo
Servo Tail;                     // Tail Servo
NewPing Sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

#define MAXSPEED    150         // Maximum Speed
#define MINSPEED     75         // Minimum Speed
#define MINDISTANCE  50         // Minimum Distance (in cm)
#define HEADCENTER   93         // Head Servo Center
#define TAILCENTER   90         // Tail Servo Center
#define TAILLEFT      0         // EPA - Tail Left
#define TAILRIGHT   173         // EPA - Tail Right



// ------------ SETUP ---------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(LED1Pin, OUTPUT);
  pinMode(LED2Pin, OUTPUT);
  pinMode(LEFTPin, OUTPUT);
  pinMode(RIGHTPin, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  Head.attach(HeadServoPin);
  Head.write(HEADCENTER);
  Tail.attach(TailServoPin);
  Tail.write(TAILCENTER);
  Speed = MINSPEED;
  while (digitalRead(BUTTON1)==HIGH)
  { 
    Led1(HIGH);
    Led2(0);
    digitalWrite(LEFTPin, HIGH);
    digitalWrite(RIGHTPin, LOW);
    delay(100);
    Led1(LOW);
    Led2(255);
    digitalWrite(LEFTPin, LOW);
    digitalWrite(RIGHTPin, HIGH);
    delay(100);
  }
  Led1(LOW);
  Led2(0);
  digitalWrite(LEFTPin, LOW);
  digitalWrite(RIGHTPin, LOW);
}



// ------------ LOOP ----------------------------------------------------------------------
void loop()
{     
  int Distance = GetDistance();
  if (Distance>MINDISTANCE)
  {
    if (Distance==999)
      Speed=MAXSPEED;
    if ((Distance>=150) && (Distance<999))
      Speed=Speed+10;
    if ((Distance>=70) && (Distance<150))
      Speed=Speed-10;
    if ((Distance>=MINDISTANCE) && (Distance<70))
      Speed=MINSPEED;
    if (Speed>MAXSPEED)
      Speed=MAXSPEED;
    if (Speed<MINSPEED)
      Speed=MINSPEED;
    int x=0;
    if (Distance==999)
      x=255;
    else
      x=Distance;
    Led2(x-MINDISTANCE);
    Serial.print("Running - Center Distance: ");
    Serial.print(Distance);
    Serial.print(" - Speed: ");
    Serial.println(Speed);
    Go(Speed);
  }
  else
  {
    Led2(255);
    Serial.print("Stop! - ");
    Speed=0;
    Stop();
    MoveHead(0);
    delay(300);
    byte Right=GetDistance();
    Serial.print("Take a look - Right Distance: ");
    Serial.print(Right);
    delay(300);
    MoveHead(180);
    delay(350);
    byte Left=GetDistance();
    Serial.print(" - Left Distance: ");
    Serial.print(Left);   
    MoveHead(HEADCENTER);
    delay(300);
    byte Center=GetDistance();
    Serial.print(" - Center Distance: ");
    Serial.println(Center);   
    if (Center>MINDISTANCE)
      return;
    if ((Center<MINDISTANCE) && (Left<MINDISTANCE) && (Right<MINDISTANCE)) 
    {
      Serial.println("Turning 180 degreess - Less than 50cm");
      Turn180();
      return;
    }
    if ((Left>Right) && (Center<MINDISTANCE))
    {
      Serial.println("Turning Left");
      TurnLeft();
      return;
    }
    if ((Right>Left) && (Center<MINDISTANCE))
    {
      Serial.println("Turning Right");
      TurnRight();
      return;
    }
    if ((Left>Right) && (Left>Center))
    {
      Serial.println("Turning Left");
      TurnLeft();
      return;
    }
    if ((Right>Left) && (Right>Center))
    {
      Serial.println("Turning Right");
      TurnRight();
      return;
    }
    // Are we stuck? Backwards till more space...
    Backwards();
  }
}



// ------------ TURN 180 ------------------------------------------------------------------
void Turn180()
{
  digitalWrite(LEFTPin, HIGH);
  digitalWrite(RIGHTPin, HIGH);  
  Stop();
  MoveTail(TAILRIGHT);
  Move(1,200,1);
  Move(2,200,0);
  delay(400);
  Stop();
  MoveTail(TAILCENTER);
  delay(150);
  digitalWrite(LEFTPin, LOW);
  digitalWrite(RIGHTPin, LOW);  
}


// ------------ TURN LEFT -----------------------------------------------------------------
void TurnLeft()
{
  digitalWrite(LEFTPin, HIGH);  
  Stop();
  MoveTail(TAILRIGHT);
  Move(1, 200, 1);
  Move(2, 200, 0);
  delay(200);
  Stop();
  MoveTail(TAILCENTER);
  delay(150);
  digitalWrite(LEFTPin, LOW);
}



// ------------ TURN RIGHT ----------------------------------------------------------------
void TurnRight()
{
  digitalWrite(RIGHTPin, HIGH);
  Stop();
  MoveTail(TAILLEFT);
  Move(1, 200, 0);
  Move(2, 200, 1);
  delay(200);
  Stop();
  MoveTail(TAILCENTER);
  delay(150);
  digitalWrite(RIGHTPin, LOW);
}



// ------------ GO ------------------------------------------------------------------------
void Go(byte Speed)
{  
  Move(1,Speed,1);
  Move(2,Speed,1);
}



// ------------ GO BACKWARDS --------------------------------------------------------------
void Backwards()
{
  Move(1,MINSPEED,0);
  Move(2,MINSPEED,0);
  while (GetDistance()<MINDISTANCE)
  {   
    digitalWrite(LEFTPin,HIGH);
    digitalWrite(RIGHTPin,HIGH);
  }
  digitalWrite(LEFTPin,LOW);
  digitalWrite(RIGHTPin,LOW);
  Stop();
}



// ------------ CONTROL MOTORS ------------------------------------------------------------
void Move(int motor, int speed, int direction)
{ //motor: 0 for B 1 for A
  //speed: 0 is off, and 255 is full speed
  //direction: 0 clockwise, 1 counter-clockwise
  digitalWrite(STBY, HIGH); //disable standby
  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;
  if(direction == 1){
    inPin1 = HIGH;
    inPin2 = LOW;
  }
  if(motor == 1){
    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speed);
  }else{
    digitalWrite(BIN1, inPin1);
    digitalWrite(BIN2, inPin2);
    analogWrite(PWMB, speed);
  }
}



// ------------ STOP MOTORS ---------------------------------------------------------------
void Stop()
{
  // enable standby  
  digitalWrite(STBY, LOW); 
}



// ------------ GET DISTANCE --------------------------------------------------------------
int GetDistance()
{
  Led1(HIGH);
  delay(50);
  unsigned int uS = Sonar.ping();
  int apostash = (uS / US_ROUNDTRIP_CM);
  if (apostash == 0)
    apostash=999;
  Led1(LOW);
  return apostash;
}



// ------------ MOVE HEAD (ULTRASONIC) ----------------------------------------------------
void MoveHead(byte pos)
{
  Head.write(pos);
  delay(300);
}



// ------------ MOVE TAIL FOR TURNING -----------------------------------------------------
void MoveTail(byte pos)
{
  Tail.write(pos);
  delay(250);
}



// ------------ LED #1 --------------------------------------------------------------------
void Led1(byte Status)
{
  switch (Status)
  {
    case HIGH:
      digitalWrite(LED1Pin, HIGH);
      break;
    case LOW:
      digitalWrite(LED1Pin, LOW);
      break;
  }
}



// ------------ LED #2 --------------------------------------------------------------------
void Led2(byte light)
{
  analogWrite(LED2Pin, light);
}
