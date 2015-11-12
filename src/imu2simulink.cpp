/* 
Author: James Bell
Email : jtb2013@gmail.com
Like, don't copy this I guess. Not because it's copyrighted, but because it's highly suspicious in terms of quality.

TODO:
Make imu port configurable from an external file
Make networking port configurable from an external file
*/

#include <xsens/xsportinfoarray.h>
#include <xsens/xsdatapacket.h>
#include <xsens/xssdidata.h>
#include <xsens/xstime.h>
#include <xsens/xstimestamp.h>
#include <xcommunication/legacydatapacket.h>
#include <xcommunication/int_xsdatapacket.h>
#include <xcommunication/enumerateusbdevices.h>

#include "deviceclass.h"
#include "readyThread.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>
#include <stdlib.h>
#include <conio.h>
#include <thread>
#include <unistd.h>

bool ready = false;
void readyCallback(bool r){
  ready=r;
}

int main(int argc, char** argv){ 
  DeviceClass device;
  std::ofstream logger;
  logger.open(std::string(std::getenv("HOME"))+std::string("/imu_data/imu_log.log"),std::ofstream::out);
  std::time_t nowt = std::time(0);
    std::tm* now = std::localtime(&nowt);
  logger << (now->tm_year+1900)<<'-'<<(now->tm_mon+1)<<'-'<<now->tm_mday<<' '<<now->tm_hour<<':'<<now->tm_min<<':'<<now->tm_sec<< std::endl;
  logger.close();
  logger.open(std::string(std::getenv("HOME"))+std::string("/imu_data/imu_log.log"),std::ofstream::out | std::ofstream::app);
  try{
  //Check command line options
  int c;
  char* cval = NULL;
  int portOut = 26015;      //First port to send data to. Rest of data is sent to subsequent ports.
  int portReady = 26100;    //Port to which program listens for ready command
  bool quat = false;        //When true, output delta-quaternion and delta-acceleration
  int rate = 100;           //IMU update rate
  bool waitReady = false;   //When false, will start sending data to port automatically
  bool logOutputs = false;  //When true, will log all data sent to ports to ~/imu_data/data_imu.txt
  while((c = getopt(argc,argv,"p:P:qr:wl"))!=-1)
    switch(c){
      case 'p'://Set portOut
        portOut = atoi(optarg);
        logger<<"Option p set to "<<optarg<<std::endl;
	break;
      case 'P'://Set portReady
        portReady = atoi(optarg);
	logger<<"Option P set to "<<optarg<<std::endl;
        break;
      case 'q'://Quaternion
        quat = true;
	logger<<"Option q set to true"<<std::endl;
        break;
      case 'r'://set IMU update rate
        rate = std::min(atoi(optarg),100);
	logger<<"Option r set to "<<optarg<<std::endl;
        break;
      case 'w'://Wait ready
        waitReady = true;
	logger<<"Option w set to true"<<std::endl;
        break;
      case 'l'://Logging
        logOutputs = true;
        logger<<"Option l set to true"<<std::endl;
        break;
    }
    std::string portName = "/dev/ttyUSB0";
    int baudRate = 115200;
    
    // Use the first detected device
    XsPortInfo mtPort(portName,XsBaud::numericToRate(baudRate));

    
    // Open the port with the detected device
    logger << "Opening port..." << std::endl;
    if (!device.openPort(mtPort)){
      logger << "Could not open port. Aborting."<<std::endl;
      logger.close();
      throw std::runtime_error("Could not open port. Aborting.");
    }

    // Put the device in configuration mode
    logger << "Putting device into configuration mode..." << std::endl;
    bool setConfigMode = false;
    for(int i=0;i<20 && !setConfigMode;i++){
      setConfigMode = device.gotoConfig();
    }
    if (!setConfigMode){ // Failed to put the device into configuration mode before configuring the device
      logger << "Could not put device into configuration mode. Aborting."<<std::endl;
      logger.close();
      throw std::runtime_error("Could not put device into configuration mode. Aborting.");
    }
    logger << "Put device into configuration mode. Requesting type"<<std::endl;
    // Request the device Id to check the device type
    mtPort.setDeviceId(device.getDeviceId());
    logger << "Checking credentials"<<std::endl; 
    // Check if we have an MTi / MTx / MTmk4 device
    if ( !mtPort.deviceId().isMtMk4()){
      logger << "" <<std::endl;
      logger.close();
      throw std::runtime_error("No MTi-100 device found. Aborting.");
    }
    logger << "Found a device with id: " << mtPort.deviceId().toString().toStdString() << " @ port: " << mtPort.portName().toStdString() << ", baudrate: " << mtPort.baudrate() << std::endl;

    try{
      // Print information about detected MTi / MTx / MTmk4 device
      logger << "Device: " << device.getProductCode().toStdString() << " opened." << std::endl;

      // Configure the device. Note the differences between MTix and MTmk4
      logger << "Configuring the device..." << std::endl;
      if (mtPort.deviceId().isMtMk4()){// || mtPort.deviceId().isFmt_X000())
	XsOutputConfiguration accConfig;
	XsOutputConfiguration quatConfig;
	if(!quat){
        	accConfig = XsOutputConfiguration(XDI_Acceleration, rate);
        	quatConfig = XsOutputConfiguration(XDI_RateOfTurn, rate);
        }else{
		accConfig = XsOutputConfiguration(XDI_DeltaV, rate);
        	quatConfig = XsOutputConfiguration(XDI_DeltaQ, rate);
	}
	XsOutputConfiguration sampleTimeConfig(XDI_SampleTimeFine, rate);
        XsOutputConfigurationArray configArray;
        configArray.push_back(accConfig);
        configArray.push_back(quatConfig);
        configArray.push_back(sampleTimeConfig);
        if (!device.setOutputConfiguration(configArray)){
          logger << "Could not configure MTmk4 device. Aborting." <<std::endl;
          logger.close();

          throw std::runtime_error("Could not configure MTmk4 device. Aborting.");
        }
      }
      else{
        logger << "Unknown device while configuring. Aborting." <<std::endl;
        logger.close();
        throw std::runtime_error("Unknown device while configuring. Aborting.");
      }

      // Put the device in measurement mode
      std::cout << "Putting device into measurement mode..." << std::endl;
      if (!device.gotoMeasurement()){
        logger << "Could not put device into measurement mode. Aborting." <<std::endl;
        logger.close();
        throw std::runtime_error("Could not put device into measurement mode. Aborting.");
      }

      //std::cout << std::string(79, '-') << std::endl;

      XsByteArray data;
      XsMessageArray msgs;
      std::ofstream out;
      if(logOutputs){
        out.open(std::string(std::getenv("HOME"))+std::string("/imu_data/data_imu.txt"),std::ofstream::out);
        if(quat){
          out << "Time,AccX,AccY,AccZ,QuatX,QuatY,QuatZ,QuatW"<<std::endl;
        }else{
          out << "Time,AccX,AccY,AccZ,GyroX,GyroY,GyroZ"<<std::endl;
        }
      }
      //Set up the udp sending addresses
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      struct sockaddr_in serv_addrs[8];
      for(int i = 0;i<8;i++){
        memset(&serv_addrs[i],'0',sizeof(serv_addrs[i]));
        serv_addrs[i].sin_family = AF_INET;
        serv_addrs[i].sin_port = htons(portOut+i);
        inet_pton(AF_INET, "127.0.0.1", &serv_addrs[i].sin_addr);
      }

      const int readyPort = portReady;
      std::thread checkReadyThread;
      checkReadyThread = std::thread(startReadyThread,readyPort,readyCallback);
      bool wasReady = false;
      XsTimeStamp tStart = XsTime::timeStampNow();
      //Start reading
      while (true){
	      XsTimeStamp ts = XsTime::timeStampNow();
        device.readDataToBuffer(data);
        device.processBufferedData(data, msgs);
        for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it){
          // Retrieve a packet
          XsDataPacket packet;
          if ((*it).getMessageId() == XMID_MtData2) {
            packet.setMessage((*it));
            packet.setDeviceId(mtPort.deviceId());
          }

          // Get the quaternion data
          int sampleTime = packet.sampleTimeFine();
          int dataSize = quat?8:7;
          double dArr[dataSize];
          if(quat){
            XsSdiData sdi = packet.sdiData();
            XsVector3 acc = sdi.velocityIncrement();
            XsQuaternion quat = sdi.orientationIncrement();
            dArr[0] = acc.at(0);
            dArr[1] = acc.at(1);
            dArr[2] = acc.at(2);
            dArr[3] = quat.x();
            dArr[4] = quat.y();
            dArr[5] = quat.z();
            dArr[6] = quat.w();
          }else{
            XsVector3 acc = packet.calibratedAcceleration();
            XsVector3 rotRate = packet.calibratedGyroscopeData();
            dArr[0] = acc.at(0);
            dArr[1] = acc.at(1);
            dArr[2] = acc.at(2);
            dArr[3] = rotRate.at(0);
            dArr[4] = rotRate.at(1);
            dArr[5] = rotRate.at(2);
          }
          
         
          uint8_t msg[60];
          int loc=0;
          if(ready || !waitReady){
            if(!wasReady && waitReady){
              wasReady = true;
              logger<<"Enabled"<<std::endl;
	      tStart = XsTime::timeStampNow();
            }
            memcpy(&msg[sizeof(int)*loc++],&sampleTime,sizeof(int));
            struct sockaddr_in tempAddr = serv_addrs[0];
            sendto(sock,msg,sizeof(int),0,(struct sockaddr *)&tempAddr,sizeof(tempAddr));
            if(logOutputs){
              XsTimeStamp tNow = XsTime::timeStampNow();
              out << sampleTime <<","<< (tNow.msTime()-tStart.msTime())<<",";
            }
            for(int i=1;i<dataSize;i++){
              tempAddr = serv_addrs[i];
              double d = dArr[i-1];
              uint8_t* msgD = reinterpret_cast<uint8_t*>(&d);
              sendto(sock, msgD, sizeof(double), 0,(struct sockaddr *)&tempAddr,sizeof(tempAddr));
              if(logOutputs)
                out<<d<<",";
            }
            if(logOutputs)
              out<<std::endl;
          }else if(wasReady && waitReady){
            wasReady = false;
            logger<<"Disabled"<<std::endl;
          }
        }
        msgs.clear();
	      XsTimeStamp ts2 = XsTime::timeStampNow();
        int delay = (int)(1000/rate);
        if(delay - (ts2.msTime() - ts.msTime()) > 0){
                      //logger<<"Sleeping for "<<(delay-(ts2.msTime()-ts.msTime()))<<std::endl;
		      XsTime::msleep(delay -(ts2.msTime() - ts.msTime()));
        }
      }
      out.close();
    }
    catch (std::runtime_error const & error){
      logger << error.what() <<std::endl;
      logger.close();
      std::cout << error.what() << std::endl;
    }
    catch (...){
      logger << "An unknown fatal error has occured. Aborting." <<std::endl;
      logger.close();
      std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
    }

    // Close port
    logger << "Closing port..." << std::endl;
    device.close();
  }
  catch (std::runtime_error const & error){
    logger << error.what() <<std::endl;
    logger.close();
    std::cout << error.what() << std::endl;
  }
  catch (...){
    logger << "An unknown fatal error has occured. Aborting." <<std::endl;
    logger.close();
    std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
  }

  logger << "Successful exit." << std::endl;
  logger.close();
  
  return 0;
}
