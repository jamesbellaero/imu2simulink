#include "readyThread.h"


void startReadyThread(const int &port, void (*callback)(bool)){
      //Set up the udp receiving socket
      struct sockaddr_in ready_addr;
      memset(&ready_addr,'0',sizeof(ready_addr));
      ready_addr.sin_family = AF_INET;
      ready_addr.sin_port = htons(port);
      ready_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      int sock = socket(AF_INET, SOCK_DGRAM, 0);

      fd_set set;
      FD_ZERO(&set);
      FD_SET(sock,&set);
      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      select(sock+1,&set,NULL,NULL,&timeout);
      bind(sock,(struct sockaddr*)&ready_addr,sizeof(ready_addr));
      //Initialize as not ready to send
      bool state = false;
      while(true){
        //For rate control
        XsTimeStamp ts = XsTime::timeStampNow();
        //Read a single byte
        uint8_t msg[1];
        int recvlen = recv(sock,msg,1,0);
        if(msg[0] == 1 && !state){//If simulink is running and we're not running
          callback(true);
          state=true;
        }else if(msg[0] == 0 && state){//If simulink has stopped and we are running
          callback(false);
          state=false;
        }
        //Read at rate of 200 Hz. Should be at least twice speed of filter
        XsTimeStamp ts2 = XsTime::timeStampNow();
        if(5 - (ts2.msTime() - ts.msTime()) > 0){
		      XsTime::msleep(5-(ts2.msTime() - ts.msTime()));
	      }
      }
}
