# CAPSTONE 2017 - AUTOMATED TOOLCART

#	1. Open a connection with the arduino. Check in the arduino IDE 
# 	 which port and baud rate are being used. 

#	2. Send character to Arduino indicating we're 'listening'

#	3. Wait for arduino to send an "ok" character - then we know to start saving the next thing 
#    it saves in to data. 


import serial
import time
import matplotlib.pyplot as plt
from drawnow import *
plt.ion;

# 1. Open connection


port = 'COM6';
rate = 9600;
ard = serial.Serial(port,rate,timeout=1)
filename = 'test_file.txt'
f = open(filename,'w');

if(ard.is_open == True):
	print("Connection established with Arduino over port: ", ard.name)
	
else:
	print("Connection not established. Restart.");
	
time.sleep(1)


# 2. Send "listening" byte 'B'. 
# Need to encode 'B' to its byte value to send. Then, 
# need to decode on the arduino side. 

ard.write('B'.encode('ascii'))

# 3. Wait for a reply from arduino. 

proceed = False

while(proceed == False):

	text_d = ard.read(1).decode('ascii');

	if(text_d == 'F'):
		print("Ok signal received from Arduino. Collecting data. ")
		proceed = True
	
	
	
	
# 4. Begin reading data

l_rpm_smoothed = []
l_rpm = []
r_rpm_smoothed = []
r_rpm = []
t_ard = []
voltage = []
latency = []




done = False; 
while(done == False):

	for i in range(1,8):
		t = str(ard.readline().decode('ascii').rstrip())
		
		if(t=='Q'):
			done = True;
			break
		
		if(i==1):
			l_rpm_smoothed.append(t);
			f.write(t)
			f.write(",")
			
		if(i==2):
			l_rpm.append(t)
			f.write(t)
			f.write(",")
		
		if(i==3): 
			r_rpm_smoothed.append(t)
			f.write(t)
			f.write(",")
		if(i==4): 
			r_rpm.append(t)
			f.write(t)
			f.write(",")
		if(i==5): 
			t_ard.append(t)
			f.write(t)
			f.write(",")
		if(i==6):
			voltage.append(t);
			f.write(t)
			f.write(",")
		if(i==7):
			latency.append(t);
			f.write(t);
			f.write("\n")
f.close() 


was = input("press key to finish")

