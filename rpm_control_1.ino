// AUTONOMOUS TOOLCART CAPSTONE PROJECT

// November 2017

// RPM Control System 

// Objective: 	Given a step rpm input, ensure motor output rpm 
// 							reaches specified value with minimum steady state 
//							error. 


#define BIT(a) (1<<(a))
const double max_volt = 0.5;


// Motor pins
int r_mot = 10;
int r_d1 = 8; 
int r_d2 = 6;
int l_mot = 9; 
int l_d1 = 4; 
int l_d2 = 5; 

// Encoder info
double ticks_rev_r = 235;
double ticks_rev_l = 235;
int l_enc = 2; // pin INT0
int r_enc = 3; // pin INT1

// Pot
int sensor = A0;
double sensor_value;

// CONTROL SYSTEM VARIABLES
// In present configuration, 16ms per compare match interrupt generated. 
// The frequency and period of one control loop iteration and is determined 
// both by clock prescalar and how many overflows are required to trigger cs_start. 

// Timing: 
volatile bool cs_start = false; // set to true by timer compare interrupt
volatile int waste = 0; // loops through until ready
int ms_per_cmp = 16; 

// number of compare matches before cs_start is triggered
int n_cmp = 4; // 
int cmp_count = 0;  

// Period of one control loop iteration = dt
double dt = ms_per_cmp*n_cmp*.001;


// Input, error and controller gains

double input_r_rpm;
double input_l_rpm;
int e_r,u_r,e_l,u_l; //error and control signals
double kp;// proportional term gain
double p_r,p_l;
double i_r = 0; // integral error term
double i_l = 0;
double ki; // integral term gain

// Feedback variables
double r_rpm[5]; // Smoothing function takes average of last 5 rpm readings. 
double l_rpm[5]; 
double r_sum = 0; //necessary for smoothing
double l_sum = 0;
double r_rpm_smoothed; // average of the last 5 recordings. 
double l_rpm_smoothed;
volatile int r_ticks = 0; //counts number of encoder ticks since last loop started.
volatile int l_ticks = 0;

//PC variables
const int n_vars = 7;
double send_info[n_vars];
int t1,t2;
double t = 0;


void setup() {
 
  config_pins();
  config_timer();
  config_interrupt();
  set_direction(0);

	for(int i = 0;i<5;i++){
    r_rpm[i] = 0;
  }
  Serial.begin(9600); 
  Serial.setTimeout(1);
  
  int go = ping_comp();
  
  while(!go){
    // if ping returns 0, there was a problem. flash the 13 LED. 
    digitalWrite(13,LOW);
    delay(100);
    digitalWrite(13,HIGH);
    delay(100);
  } 

 
  
  kp = 2;
  ki = 1.3;
  while(t<15){


    
    while(!cs_start){waste++;} // cs_start set to true in timer overflow fn
    
    						
  	int t1 = millis();
  	
  // 1. Receive command (input);`
  //  todo: add dan's code for joystick. 
  // 	todo: receive commands from computer. 
  // 	todo: incorporate kinematic equations (rpm_r and l based on R and Vlin)


  
    if(t<1){
      waste++;
    }else{
      read_inputs('p'); // sets input_r_rpm, input_l_rpm
    }
      
  // 2. READ SENSOR (OUTPUT)
    calc_rpm();
    
    
  //  3. COMPUTE ERROR SIGNAL (E = INPUT - OUTPUT)
  	e_r = (input_r_rpm - r_rpm_smoothed);
  	e_l = (input_l_rpm - l_rpm_smoothed);



  //  4. COMPUTE CONTROL SIGNAL (U)
  //  if error is negative, set the motor backwards.

    // Integral terms   
    i_r += ki*e_r*dt;
    i_l += ki*e_l*dt;

    // Proportional terms
    p_r = kp*e_r;
    p_l = kp*e_l;

    // Control signal calculation
    u_r = i_r + p_r;
    u_l = i_l + p_l;
    
    if(u_r < 0){
      set_r_mot(0);
      u_r = abs(u_r)*0.5;
    }else{
      set_r_mot(1);
    }
    
    if(u_l < 0){
      set_l_mot(0);
      u_l = abs(u_l)*0.5;
    }else{
      set_l_mot(1);
    }
  
    // Saturate Inputs 
  	if(u_r > 255) u_r = 255; 
    if(u_l > 255) u_l = 255; 
  
  
  // 5. Write actuators with control signal
    analogWrite(r_mot,u_r);
    analogWrite(l_mot,u_l);

  
    //6. Send to file
    send_info[0] = input_r_rpm;
    send_info[1] = r_rpm_smoothed;
    send_info[2] = e_r;
    send_info[3] = u_r;
    send_info[4] = t2;
    send_info[5] = t;
    send_info[6] = 0;
    send_to_pc(send_info,n_vars);
  
    
    //7. MISC  
    t2 = millis()-t1;
  	cs_start = false;
    t += dt; 
  }
  Serial.println('Q'); // tell PC the test is over
  analogWrite(r_mot,0);
  analogWrite(l_mot,0);
}

void loop(){}



void send_to_pc(double send_me[], int n){
  
  for(int i = 0;i<n;i++){
    Serial.println(send_me[i]);
  }
}




ISR(INT0_vect){
  l_ticks++;
}




ISR(INT1_vect){
  r_ticks++;
}




ISR(TIMER2_COMPA_vect){
	cmp_count++;
	if(cmp_count==n_cmp){
		cs_start = true;
		cmp_count = 0;
	}
}






void set_r_mot(int dir){
  
  if(dir==1){
    
    // go forward
    digitalWrite(r_d1,LOW);
    digitalWrite(r_d2,HIGH);
    
  }else{
    
    // go backward
    digitalWrite(r_d1,HIGH);
    digitalWrite(r_d2,LOW);
    
  }
}






void set_l_mot(int dir){

  if(dir==1){
    digitalWrite(l_d1,HIGH);
    digitalWrite(l_d2,LOW);
  }else{
    digitalWrite(l_d1,LOW);
    digitalWrite(l_d2,HIGH);
  }
  
}





void config_pins(){
  
  pinMode(r_mot,OUTPUT);
  pinMode(r_d1,OUTPUT);
  pinMode(r_d2,OUTPUT);
  pinMode(r_enc,INPUT_PULLUP);   
  pinMode(l_mot,OUTPUT);
  pinMode(l_d1,OUTPUT);
  pinMode(l_d2,OUTPUT);
  pinMode(l_enc,INPUT_PULLUP);
  pinMode(sensor,INPUT_PULLUP);
}






void config_timer(){
  
  // SET TIMER REGISTERS 
  // With prescalar at 1024 and output compare interrupt at 250 ticks, 
  // compare interrupt vector every 16ms.  
  TCCR2A = 0;
  TCCR2B = 0;  
  TCCR2B = BIT(CS22) | BIT(CS21) | BIT(CS20); 
  TIMSK2 = BIT(OCIE2A); 
  OCR2A = 250; 
}








void config_interrupt(){
  
  // Set external interrupt registers
  // enable interrupts on puns 2 and 3
  
  EIMSK = 0;
  EIMSK = BIT(INT0) | BIT(INT1); 
  EICRA = BIT(ISC11) | BIT(ISC10) | BIT(ISC01)| BIT(ISC00); 
}




void read_inputs(char f){
  
  // 1. Take inputs from serial monitor. If the input is out 
  //    of bounds then set it = 0;
  //Serial.print("overloaded with no arguments.\n");
  int PWM;
  
  if(f=='s'){  
    if(Serial.available()>0){
      PWM =  Serial.parseInt();
      if ((PWM < 0) | (PWM > 500)) PWM = 0;
    }
  }
  if(f=='p'){
    sensor_value = analogRead(sensor);
    PWM = sensor_value * 400.0 / 1024.0;
  }

  input_r_rpm = PWM;
  input_l_rpm = PWM;
  
}







void set_direction(int dir){

  // direction is what direction you want, d1 and d2 
  
  if(dir==0){                 //go straight
    digitalWrite(r_d1,LOW);
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,HIGH);
    digitalWrite(l_d2,LOW);
  }
  else if(dir==1){            // turn right 
    digitalWrite(r_d1,LOW);
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,LOW);
    digitalWrite(l_d2,HIGH);
  }
   else if(dir==2){
    digitalWrite(r_d1,HIGH); //turn left 
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,HIGH);
    digitalWrite(l_d2,LOW);
   }
   else if(dir==3){
    digitalWrite(r_d1,HIGH);
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,LOW);
    digitalWrite(l_d2,HIGH);
   }
}



void calc_rpm(){
  
  for(int i = 0 ; i < 4 ; i++){
    r_rpm[i] = r_rpm[i+1];
    l_rpm[i] = l_rpm[i+1];
  }
  
  r_rpm[4] = 60*(r_ticks/ticks_rev_r) / dt;
  l_rpm[4] = 60*(l_ticks/ticks_rev_r) / dt;
 
  for(int i = 0;i<5;i++){
    r_sum += r_rpm[i];
    l_sum += l_rpm[i];
  }

  r_rpm_smoothed = r_sum/5;
  l_rpm_smoothed = l_sum/5;
  r_sum = 0;
  l_sum = 0;

  r_ticks = 0; 
  l_ticks = 0;
}


int ping_comp(){
  
  while(!Serial.available()){waste++;}
  
  char ch = (char)Serial.read();
  
  if(ch == 'B'){
    digitalWrite(13,HIGH);
    Serial.print('F');
    return 1;
  }
  else return 0;
}

