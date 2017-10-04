 
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>  
#include <Wire.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

const int xPot = A1; // joystick x
const int yPot = A2; // joystick y

const int r_mot = 10;
const int r_d1 = 5; // Green
const int r_d2 = 4; // White/Green
const int r_enc1 = 2; // Brown
const int r_enc2; // White/Brown

const int l_mot = 9; 
const int l_d1 = 6; // Green
const int l_d2 = 8; // White/Green
const int l_enc1 = 3; // Brown
const int l_enc2; // White/Brown


double throttlex;
double throttley;
double max_volt = 0.8; // max output of 0.8*12V instead of supplied 12

bool turn = 0; // 1==right, 0==left
int direct; // 1-4 depending on direction
double yin; // Y input from joystick
double xin; // X input from joystick

int tmaxr=255; // Max speed multiplier for right motor
int tmaxl=255; // Max speed multiplier for left motor


unsigned int counterR=0; // Right wheel counter
unsigned int counterL=0; // Left wheel counter
double pastTime;
double currentTime;
double pastCounterR;
double currentCounterR;
double pastCounterL;
double currentCounterL;
double dT; // Change in time between calculations
double dL; // Change in speed of left motor
double dR; // Change in speed of right motor
double dS; // dL/dR


void setup(){
// ----- Initializing I/O pins ----- //
  pinMode(xPot,INPUT);
  pinMode(yPot,INPUT);
  pinMode(r_mot,OUTPUT);
  pinMode(r_d1,OUTPUT);
  pinMode(r_d2,OUTPUT);
  pinMode(l_mot,OUTPUT);
  pinMode(l_d1,OUTPUT);
  pinMode(l_d2,OUTPUT);

// ----- Setting digital pins 2 and 3 as interrupt pins ----- //
  attachInterrupt(digitalPinToInterrupt(l_enc1), countl, CHANGE);
  attachInterrupt(digitalPinToInterrupt(r_enc1), countr, CHANGE);
  
// ----- Serial Communications begin ----- //
  Serial.begin(9600);

// ----- I2C Communications begin ----- //
  Wire.begin();

// ----- LCD initialization(used for debugging as opposed to serial print) ----- //
  lcd.begin(20, 4);
}


void loop() {

direct = get_inputs(); // reads the joystick input
set_direction(direct); // sets forward/ reverse/ spin in place
set_left_speed(); // set left motor speed
set_right_speed(); //set right motor speed

pastTime=currentTime;
currentTime=millis();

pastCounterL=currentCounterL;
currentCounterL=counterL;

pastCounterR=currentCounterR;
currentCounterR=counterR;

dT=currentTime-pastTime;
dL=currentCounterL-pastCounterL;
dR=currentCounterR-pastCounterR;

dL=dL/dT;
dR=dR/dT;
dS=dL/dR;

lcd.setCursor(0,0);
lcd.print(tmaxr);
lcd.setCursor(0,1);
lcd.print(tmaxl);
lcd.setCursor(0,2);
lcd.print(throttlex);
lcd.setCursor(0,3);
lcd.print(dS);
int maxt = (255/max_volt);

  if (throttlex<0.001){
   lcd.setCursor(10,0);
lcd.print("fun");
    if (dS>1){
      
      if (tmaxl>200){
      tmaxl--;
       tmaxr=255;
       lcd.setCursor(10,1);
lcd.print("fun");
      }
      else if(tmaxl==200) {
        tmaxr ++;
        lcd.setCursor(10,3);
lcd.print("fun");
        }
    }
    else if (dS<1) {
      if (tmaxl<maxt){
      tmaxl++;

lcd.setCursor(10,2);
lcd.print("fun");
      }
      else{}
    }
    else{}


  }
//
//  if (throttley<0.0001){
//      tmaxl=255;
//      if (dS>1){
//      if (tmaxr>200){
//      tmaxr--;
//      }
//      else{}
//    }
//    else if (dS<1) {
//      if (tmaxr<maxt){
//      tmaxr++;
//      }
//      else{}
//    }
//      else{}
//  }
}

void set_direction(int dir){
 
  if(dir==0){                 //Forward
    
    digitalWrite(r_d1,LOW); //Right motor forward
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,HIGH); //Left motor forward
    digitalWrite(l_d2,LOW);
  }
  else if(dir==1){          // turn right on the spot
    
    digitalWrite(r_d1,LOW); //Right motor forward
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,LOW); //Left motor backward
    digitalWrite(l_d2,HIGH);
  }
   else if(dir==2){         //turn left on the spot
    
    digitalWrite(r_d1,HIGH); //Right motor backward
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,HIGH); //Left motor forward
    digitalWrite(l_d2,LOW);
   }
   else if(dir==3){          //Reverse
    
    digitalWrite(r_d1,HIGH); //Right motor backward
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,LOW);  //Left motor backward
    digitalWrite(l_d2,HIGH);
   }
  
}  



int get_inputs(){
      
    int dir;
    
 // ----- Read Joystick ----- //
    
    yin = analogRead(yPot);
    xin = analogRead(xPot);
 
 // ----- Map Joystick input from 0 -> 1024 to -512 -> 512 ----- //
    
    yin = map(yin, 1019, 7, -512, 512);
    xin = map(xin, 7, 1018,-512, 512);
    
 // ----- Determine Forwards or Reverse and turning direction ----- //
    
    if (yin > 0) dir = 0;
    else if (yin < 0) dir = 3;
    
    if ( xin < 0) turn=0; //Left turn
    if ( xin >0) turn=1; //right turn
    
 // ----- Divide x and y to achieve a max value of 255 ----- //
    
    xin = xin/2;
    yin = yin/2;
    
    
 // ----- modifying the throttle curve to allow greater sensitivity towards the center of the joystick ----- //
    
      throttlex = abs((xin * xin * xin )/ (65025.0 * 255.0*1.05));
      throttley = abs((yin * yin * yin )/ (65025.0 * 255.0*1.05));
    
    
 // ----- If no throttle is given the cart will spin in place ----- //
    
      if (throttley <0.0001 && throttlex !=0){
        if (turn==0) dir=2;
        else if (turn==1) dir=1;
      }
      
return dir; 
}


// ----- Setting the speed of the Right motor ----- //

void set_right_speed(){
  
  double rspeed;
      
      if (direct == 1 || direct == 2) rspeed = (tmaxr*throttlex*max_volt);
      else if (turn ==0) rspeed = throttley * tmaxr * max_volt;
      else if (turn ==1){
        if (throttlex<0.9) rspeed = ((throttley * tmaxr * max_volt)-(throttley * tmaxr * max_volt*0.75*throttlex));
        if (throttlex >=0.9) rspeed = ((throttley * tmaxr * max_volt)-(throttley * tmaxr * max_volt*throttlex));
      }
      
  if (rspeed<0) rspeed=0;
  if (rspeed > 254) rspeed=254;
  
  analogWrite(r_mot,rspeed);
}


// ----- Setting the speed of the Left motor ----- //

void set_left_speed(){

  double lspeed;
  
      if (direct == 1 || direct == 2) lspeed = (tmaxl*throttlex *max_volt);
      else if (turn == 1) lspeed = throttley * tmaxl * max_volt;
      else if (turn ==0){
        if (throttlex<0.9) lspeed = ((throttley * tmaxl * max_volt)-(throttley * tmaxl * max_volt* 0.75*throttlex));
        if (throttlex>=0.9) lspeed = ((throttley * tmaxl * max_volt)-(throttley * tmaxl * max_volt*throttlex));
      }
  
  if (lspeed < 0) lspeed=0;
  if (lspeed > 254) lspeed=254;
  
  analogWrite(l_mot,lspeed);
}

// ----- Left Encoder Counter ----- //

void countl() {
  counterL++;
}

// ----- Right Encoder Counter ----- //

void countr() {
  counterR++;
}


