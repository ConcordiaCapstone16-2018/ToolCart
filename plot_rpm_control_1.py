import matplotlib.pyplot as plt
import numpy 

# This program reads CSV data from "filename" and plots using matplotlib
# Choose which columns to plot at the bottom. Please indicate the 
# number of columns in the file by setting the n_cols variable. 


n_cols = 7
filename = "test_file.txt"
f = open(filename,'r')

# First determine the number of rows in the file
line_count = 0
for line in f:
	line_count += 1
f.close()
print("There are :", line_count, " data points in this file. \n")
data = numpy.zeros((line_count,n_cols))


# Populate array 'data' with data from file.  
f = open(filename,'r');

row_count = 0;

for line in f:
	
	row = line.rstrip().rsplit(',')
	for i in range(0,n_cols):
		data[row_count][i] = row[i]
	row_count += 1
	

# Unpack the data variables - Prepare for plotting. 	
input_r_rpm = data[:,0]
r_rpm_smoothed = data[:,1]
e_r = data[:,2]
u_r = data[:,3]
latency = data[:,4]
t = data[:,5]
nothin = data[:,6]

# Plot. 
plt.figure(1)
plt.xlabel("Time");
plt.ylabel("RPM");
plt.title("RPM response"); 
plt.plot(t,r_rpm_smoothed);
plt.plot(t,input_r_rpm);



plt.legend(["Response","Input"])

plt.figure(2)
plt.plot(t,e_r)
plt.plot(t,u_r)
plt.legend(["Error", "Control"])

plt.show()



