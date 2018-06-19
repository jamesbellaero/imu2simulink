/* 
Author: James Bell
Email : jtb2013@gmail.com
Like, don't copy this I guess. Not because it's copyrighted, but because it's highly suspicious.

TODO:
Make imu port configurable from an external file
Make networking port configurable from an external file
*/

#include <xsens/xsportinfoarray.h>
#include <xsens/xsdatapacket.h>
#include <xsens/xssdidata.h>
#include <xsens/xstime.h>
#include <xcommunication/legacydatapacket.h>
#include <xcommunication/int_xsdatapacket.h>
#include <xcommunication/enumerateusbdevices.h>

#include "deviceclass.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <conio.h>

int main(int argc, char* argv[])
{
	DeviceClass device;
  std::ofstream logger;
  logger.open(std::string(std::getenv("HOME"))+std::string("/imu_data/imu_log.log"),std::ofstream::out | std::ofstream::app);
	try
	{
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
		if (!device.gotoConfig()) // Put the device into configuration mode before configuring the device
		{
      logger << "Could not put device into configuration mode. Aborting."<<std::endl;
      logger.close();
			throw std::runtime_error("Could not put device into configuration mode. Aborting.");
		}

		// Request the device Id to check the device type
		mtPort.setDeviceId(device.getDeviceId());

		// Check if we have an MTi / MTx / MTmk4 device
		if ( !mtPort.deviceId().isMtMk4())
		{
      logger << "" <<std::endl;
      logger.close();
			throw std::runtime_error("No MTi-100 device found. Aborting.");
		}
		logger << "Found a device with id: " << mtPort.deviceId().toString().toStdString() << " @ port: " << mtPort.portName().toStdString() << ", baudrate: " << mtPort.baudrate() << std::endl;

		try
		{
			// Print information about detected MTi / MTx / MTmk4 device
			logger << "Device: " << device.getProductCode().toStdString() << " opened." << std::endl;

			// Configure the device. Note the differences between MTix and MTmk4
			logger << "Configuring the device..." << std::endl;
			if (mtPort.deviceId().isMtMk4())// || mtPort.deviceId().isFmt_X000())
			{
				XsOutputConfiguration accConfig(XDI_DeltaV, 100);
				XsOutputConfiguration quatConfig(XDI_DeltaQ, 100);
				XsOutputConfiguration sampleTimeConfig(XDI_SampleTimeFine, 100);
				XsOutputConfigurationArray configArray;
				configArray.push_back(accConfig);
				configArray.push_back(quatConfig);
				configArray.push_back(sampleTimeConfig);
				if (!device.setOutputConfiguration(configArray))
				{
          logger << "Could not configure MTmk4 device. Aborting." <<std::endl;
          logger.close();

					throw std::runtime_error("Could not configure MTmk4 device. Aborting.");
				}
			}
			else
			{
        logger << "Unknown device while configuring. Aborting." <<std::endl;
        logger.close();
				throw std::runtime_error("Unknown device while configuring. Aborting.");
			}

			// Put the device in measurement mode
			std::cout << "Putting device into measurement mode..." << std::endl;
			if (!device.gotoMeasurement())
			{
        logger << "Could not put device into measurement mode. Aborting." <<std::endl;
        logger.close();
				throw std::runtime_error("Could not put device into measurement mode. Aborting.");
			}

			//std::cout << std::string(79, '-') << std::endl;

			XsByteArray data;
			XsMessageArray msgs;
			std::ofstream out;
  		out.open(std::string(std::getenv("HOME"))+std::string("/imu_data/data_imu.txt"),std::ofstream::out);
			out << "Time,AccX,AccY,AccZ,QuatW,QuatX,QuatY,QuatZ"<<std::endl;
			out.close();
      
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      struct sockaddr_in serv_addr;
      memset(&serv_addr,'0',sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(26004);
      inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

			while (!_kbhit())
			{
				device.readDataToBuffer(data);
				device.processBufferedData(data, msgs);
				for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it)
				{
					// Retrieve a packet
					XsDataPacket packet;
					if ((*it).getMessageId() == XMID_MtData2) {
						packet.setMessage((*it));
						packet.setDeviceId(mtPort.deviceId());
					}

					// Get the quaternion data
					XsSdiData sdi = packet.sdiData();
					XsQuaternion quat = sdi.orientationIncrement();
					XsVector3 acc = sdi.velocityIncrement();
					int sampleTime = packet.sampleTimeFine();
					double ax,ay,az,qw,qx,qy,qz;
					ax = acc.at(0);
					ay = acc.at(1);
					az = acc.at(2);
					qx = quat.x();
					qy = quat.y();
					qz = quat.z();
					qw = quat.w();
          char msg[60];
          int loc=0;
          memcpy(&msg[sizeof(int)*loc++],&sampleTime,sizeof(int));
          memcpy(&msg[sizeof(double)*loc++],&ax,sizeof(double));
	  			memcpy(&msg[sizeof(double)*loc++],&ay,sizeof(double));
	  			memcpy(&msg[sizeof(double)*loc++],&az,sizeof(double));
          memcpy(&msg[sizeof(double)*loc++],&qx,sizeof(double));
          memcpy(&msg[sizeof(double)*loc++],&qy,sizeof(double));
          memcpy(&msg[sizeof(double)*loc++],&qz,sizeof(double));
          memcpy(&msg[sizeof(double)*loc++],&qw,sizeof(double));
          sendto(sock, msg, sizeof(msg), 0,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
					out.open(std::string(std::getenv("HOME"))+std::string("/imu_data/data_imu.txt"),std::ofstream::out | std::ofstream::app);
      		out << sampleTime <<","<<ax<<","<<ay<<","<<az<<","<<qw<<","<<qx<<","<<qy<<","<<qz<<std::endl;
					out.close();
					// Convert packet to euler
					XsEuler euler = packet.orientationEuler();

				}
				msgs.clear();
				XsTime::msleep(0);
			}
		}
		catch (std::runtime_error const & error)
		{
      logger << error.what() <<std::endl;
      logger.close();
			std::cout << error.what() << std::endl;
		}
		catch (...)
		{
      logger << "An unknown fatal error has occured. Aborting." <<std::endl;
      logger.close();
			std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
		}

		// Close port
		logger << "Closing port..." << std::endl;
		device.close();
	}
	catch (std::runtime_error const & error)
	{
    logger << error.what() <<std::endl;
    logger.close();
		std::cout << error.what() << std::endl;
	}
	catch (...)
	{
    logger << "An unknown fatal error has occured. Aborting." <<std::endl;
    logger.close();
		std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
	}

	logger << "Successful exit." << std::endl;
  logger.close();
  
	return 0;
}
