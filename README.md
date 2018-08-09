First install all the stuff for MTSDK from https://www.xsens.com/download/MT/4.8/MT_Software_Suite_Linux_4.8.tar.gz
Note this requires gcc/g++ 4.7 (not 4.6, not 4.8)
Also note that the lab has in depth documentation for setting up the BeagleBone itself.

Then compile with 'make'

run ./imu2simulink 


By default, opens /dev/ttyUSB0 at 115200 Hz and writes to 127.0.0.1:26016 through 127.0.0.1:26022
Writes an int to the first port, acceleration as doubles to the next 3 ports, and gyro as doubles to the last 3.

COMMAND LINE ARGUMENTS
-p <port number>: Specify the lowest port for imu data sending
-P <port number>: Specify the receive port for the enable signal
-w              : Wait until an enable signal is received to start sending data
-q              : Output delta-V and delta-Q rather than acc and gyro. Outputs to 4 ports rather than 3.
-l              : Logging to ~/imu_data/data_imu.txt
-r <rate>       : Operation rate (up to 100 Hz)

To write imu data with -w, you must send a '1' to 127.0.0.1:26100 (or whatever is specified by -P <###>)


***********************FOR CODE MAINTAINERS*************************
So, you want to make changes? There are a few things for you to learn first.
The only documentation worth anything for the MT SDK comes from their source code docs, found in /usr/local/xsens/doc
You can copy these from the BeagleBone to your computer with PuttySCP or from a Linux/Mac terminal with scp -r /usr/local/xsens/doc .

There are only 3 C++ files that have been edited/added in the imu2simulink code. These are imu2simulink.cpp, readyThread.cpp, and deviceclass.cpp
imu2simulink.cpp 
  This is the bread and butter of the program, if by bread and butter I mean that it is the meat and potatoes.
  It does the following tasks in order
    1. Open an error log and record the current date (although the date is rather pointless given that there isn't an ntp server)
    2. Parse command line arguments 
    3. Initialize the port for the IMU
    4. Open the IMU and put it into configuration mode
    5. Check if IMU is the right type. If so, set it to the correct rate and outputs
    6. Put IMU into measurement mode
    7. Create the sockets to send to simulink
    8. Start the readyThread that waits for a signal to enable or disable logging to the udp ports. 
    9. Main loop
      a. Take down the start time for the loop
      b. Read in data from the device
      c. Parse the data
      d. If enabled, send the data over the sockets and log it.
      e. Wait so that the rate is forcibly maintained.
    10. Close everything down in the event of a failure and log it.
  Note that the enable signal is important to ensure that only the most recent of data is read on Simulink's side.
  An improvement could be to remove the rate enforcement in the loop and instead loop on the data read until new data is received. This means that it could not be used to help run the robot loop, however.
deviceclass.cpp
  I don't know what most of the functions do in here, but I did change one. waitForMessage used to hang if the IMU wasn't initialized before the BeagleBone, so I added in a timeout. Now imu2simulink resends the configure command whenever it times out rather than hanging infinitely waiting on a response.

  In general this class is just used for convenience. Examples in the imu2simulink code should get you to where you need to be with it.
readyThread.cpp
  Used to monitor a port for a signal to enable or disable the IMU. Functions by calling a callback function whenever the signal switches from 1 to 0 or 0 to 1. Honestly, this was my first attempt at multithreading in C/C++ without MPI so don't hate.

