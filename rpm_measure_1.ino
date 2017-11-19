// AUTONOMOUS TOOLCART CAPSTONE PROJECT

// October 2017

// Objective:
//							Accurately measure the angular velocity
//							of the two motors. Measure error between
//							calculated rpm and rpm measured with 
//							laser tachometer. 
//
// 							Approach : 
//							Count the number of ticks within a certain 
//							period of time dt. 



#define BIT(a) (1<<(a))
const double max_volt = 0.5;


// Motor pins
const int r_mot = 10;
const int r_d1 = 8; 
const int r_d2 = 6;
const int l_mot = 9; 
const int l_d1 = 4; 
const int l_d2 = 5;
 
// Encoder info
const double ticks_rev_r = 235; // number of encoder ticks per revolution
const double ticks_rev_l = 235;
const int l_enc = 2; // pin INT0
const int r_enc = 3; // pin INT1

// Timing variables: 
volatile bool cs_start = false; //triggers loop to begin. Set to true by timer interrupt
volatile int waste = 0; 
int ms_per_cmp = 16; // Calculated from timer. 
int n_cmp = 6; // Number of interrupts generated before cs_start is set to true. 
double dt = ms_per_cmp*n_cmp*.001; // dt = period of loop
int cmp_count = 0; // number of interrupps 

double t = 0; // time at the beginning of the test

// Input variables:
int PWM; // Input to the motors
double v_lin; // Target linear velocity 
double ratio; // ratio between two motors velocities


// RPM variables 
double r_rpm[5]; // array used to calculate average of last 5 readings
double l_rpm[5]; 
double r_sum = 0; // to calc average.
double r_rpm_smoothed; // the average velocity
double l_sum = 0; 
double l_rpm_smoothed; 
volatile int r_ticks = 0; // counts the number of encoder ticks since the loop started (since cs_start was reset). 
volatile int l_ticks = 0;

// PC variables
const int n_vars = 7; // # of variables to send to PC. 
double send_info[n_vars]; // Output buffer
 
// MISC variables
double battery_v = 9;
double voltage; // voltage fed to the motor - depends on PWM.
int sensor = A0;
double sensor_value;

void config_pins();
void config_interrupt();
void config_timer();
void set_direction(int);
void read_inputs(int);
void read_inputs(double&, double&);
void read_inputs(char);
void set_motors(int, int);
void calc_rpm();
int ping_comp();
void send_to_pc(double[], int);

int t1,t2;



void setup() {
  
	config_pins();
	config_timer();
	config_interrupt();
  set_direction(0);
  Serial.begin(9600);
	Serial.setTimeout(1);

	// Establish connection with computer. 
	// ping_comp returns 1 when succesful. 
	int go = ping_comp();
	
	while(!go){
		// if ping returns 0, there was a problem. flash the 13 LED. 
		digitalWrite(13,LOW);
		delay(100);
		digitalWrite(13,HIGH);
		delay(100);
	} 
	
	// Required for the smoothing function 
  for(int i = 0;i<5;i++){
    r_rpm[i] = 0;
		l_rpm[i] = 0;
  }



	
	while(t<10){
	
		while(!cs_start){waste++;} 
    t1 = micros();
		// 1. Read inputs. 
		// read_inputs('p') - set PWM with potentiometer.
    // read_inputs('s') - send PWM with serial monitor
    // read_inputs(ratio,v_lin) - not working yet.
    // read_inputs(124) - set PWM to 124
		read_inputs('p');
		
		
		// 2. Write Motors
		// set_motors( left pwm, right pwm).
		// Currently sets both motors to the left speed. 
		set_motors(PWM,PWM);
		
		
		// 3. Calculate RPM
		// x_rpm_smoothed contains the 'filtered' velocity
		// x_rpm[4] contains the latest reading. 
		calc_rpm();
		
		
		// 4. Print
		send_info[0] = l_rpm_smoothed;
		send_info[1] = l_rpm[4];
		send_info[2] = r_rpm_smoothed;
		send_info[3] = r_rpm[4];
		send_info[4] = t;
    send_info[5] = voltage;
    send_info[6] = t2;
    
    // todo: add this
    //send_info[6] = time_taken_to_iterate;
		send_to_pc(send_info,n_vars);


		// 5. Reset 
		cs_start = false; 
		t += dt;
	  t2 = micros() - t1;
	}
	
	Serial.print('Q'); // PC side needs this
	set_motors(0,0);
	
}



void loop(){} 



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
	
	


void send_to_pc(double send_me[], int n){
	
	for(int i = 0;i<n;i++){
		Serial.println(send_me[i]);
	}
}
	
	

void read_inputs(double &ratio, double &v_lin){
	
	 // 1. Take inputs from serial monitor - ratio & v_lin 
  
	if(Serial.available()>0){
		PWM =  Serial.parseInt();
	}
	
	//Serial.print("overloaded with 2 arguments.\n");
}



void read_inputs(int desired_pwm){
	
	//Serial.print("overloaded with 1 argument.\n");
	PWM = desired_pwm;
}


void read_inputs(char f){
	
	// 1. Take inputs from serial monitor. If the input is out 
	//    of bounds then set it = 0;
  //Serial.print("overloaded with no arguments.\n");
	
	if(f=='s'){  
  	if(Serial.available()>0){
  		PWM =  Serial.parseInt();
  		if ((PWM < 0) | (PWM > 255)) PWM = 0;
  	}
	}
  if(f=='p'){
    sensor_value = analogRead(sensor);
    PWM = sensor_value * 255.0 / 1024.0;
    if(PWM < 0) PWM = 0;
    if(PWM > 255) PWM = 255;
  }
}


void set_motors(int l_pwm, int r_pwm){
	
	PWM = l_pwm;
	if(PWM < 0) PWM = 0;
  if(PWM >255) PWM = 255;  

  voltage = (PWM/255.0)*max_volt*battery_v;

  analogWrite(r_mot,PWM*max_volt);
  analogWrite(l_mot,PWM*max_volt);
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





void set_direction(int dir){
    
  if(dir==0){                 //go straight
    digitalWrite(r_d1,LOW);
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,HIGH);
    digitalWrite(l_d2,LOW);
  }
  else if(dir==1){            // turn right on the spot
    digitalWrite(r_d1,LOW);
    digitalWrite(r_d2,HIGH);
    digitalWrite(l_d1,LOW);
    digitalWrite(l_d2,HIGH);
  }
   else if(dir==2){
    digitalWrite(r_d1,HIGH); //turn left on the spot
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,HIGH);
    digitalWrite(l_d2,LOW);
   }
   else if(dir==3){
    digitalWrite(r_d1,HIGH); // go backwards
    digitalWrite(r_d2,LOW);
    digitalWrite(l_d1,LOW);
    digitalWrite(l_d2,HIGH);
   }
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


