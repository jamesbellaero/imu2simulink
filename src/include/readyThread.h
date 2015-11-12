#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <thread>
#include <string.h>
#include <xsens/xstime.h>
#include <xsens/xstimestamp.h>
#include <cstdint>

#ifndef READY_THREAD_H_
#define READY_THREAD_H_

void startReadyThread(const int &port, void (*callback)(bool));

#endif

