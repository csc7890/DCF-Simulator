#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <queue>

#include <stdio.h>

using namespace std;

typedef unsigned long SYSTEM_TIME;

//#define	END_TIME			(10 * 1000000)	// in us
#define	END_TIME			5000000
#define	MAX_SIM_NO			3
#define	MAX_TX_RX_NODE		100
#define	MAX_Q_SIZE			200
#define BSS_SCOPE			50

#define RX_NODE				0
#define RX_NODE_X_POINT		50
#define RX_NODE_Y_POINT		50

// traffic generation type
#define	CBR						1
#define	POISSON					2
#define LOSTLESS_PACKET_TYPE	3


// Traffic arrival rate
#define	ARRIVAL_CBR			100			// packet arrival rate in us
#define	ARRIVAL_POISSON		0.4		// packet arrival rate with poisson distribution (LAMDA)

/*
●RTS/CTS 사용 mode (RTS_CTS_USE_MODE :1)
1. RTS
   duration = RTS + CTS + DATA + ACK + SIFS*3
2. CTS
   duration = 수신한 packet의 duration - (RTS + SIFS)
3. DATA
   duration = 수신한 packet의 duration - (CTS + SIFS)
4. ACK   duration = 수신한 packet의 duration - (DATA + SIFS)

●RTS/CTS 사용하지 않는 mode (MAC_MODE:2)
1. DATA
   duration = DATA + SIFS + ACK
2. ACK
   duration = 수신한 packet의 duration - (DATA + SIFS)
   */

// Simulation
#define RTS_CTS_USE_MODE		1
#define DEFAULT_MODE			2
#define MAC_MODE				DEFAULT_MODE

#define DEBUG_PRINT				2 //Lv0 : 디버그 메세지 출력 안함	Lv1 : 디버그 메세지 부분 출력		Lv2 : 디버그 메세지 전체 출력

//Event
//추가된 이벤트 종류 : SIFS, DIFS, BACKOFF, TX_END, RX_END, TIMMER, NAV
#define	PACKET_GEN			2000
#define SIFS_EVENT			1001
#define DIFS_EVENT			1002
#define BACKOFF_EVENT		1003
#define TX_END_EVENT		1004
#define RX_END_EVENT		1005
#define TIMER_EVENT			1006
#define NAV_END_EVENT		907
#define	SIMUL_END			1008

// TX,RX status
//#define NULL_STATUS			20
#define RTS_TYPE			21
#define CTS_TYPE			22
#define DATA_TYPE			23
#define ACK_TYPE			24

// Channel status
#define	IDLE				1
#define	BUSY				2
#define	COLLISION			3

// Delay bound of multimedia traffic
#define	DelayBound			33000	// in usec


// Contention Window_Size
#define	CWMin				31
#define	CWMax				1023
#define	RetryLimit			7


// Data size in bytes
#define	Data				1000
#define	IP					0
#define	UDP					0
#define	RTP					0

// Transmission rate in Mbps
#define	DataRate			54	// for data packet
#define	BasicRate			6	// for control packet

// Packet size in bits
#define	Packet_SZ			(Data + IP + UDP + RTP) * 8
#define	MAC_hdr				(30 * 8)//240
#define	PHY_hdr				(17 * 8) //136
#define	RTS					(20 * 8 + PHY_hdr)//296
#define	CTS					(14 * 8 + PHY_hdr)//248
#define	ACK					(14 * 8 + PHY_hdr)//248

// in us
#define PropagationDelay        1
#define	SlotTime				9
#define	SIFS					16
#define	DIFS					(SIFS + 2 * SlotTime)//34
#define PIFS					(SlotTime + SlotTime)
#define EIFS					((ACK/BasicRate)+SIFS+DIFS)//54


/*
// Transmission rate in Mbps set(2)
#define	DataRate			11	// for data packet
#define	BasicRate			1	// for control packet

// Packet size in bits set(2)
#define	Packet_SZ			(Data + IP + UDP + RTP) * 8
#define	MAC_hdr				224
#define	PHY_hdr				192
#define	RTS					(20 * 8 + PHY_hdr)
#define	CTS					(14 * 8 + PHY_hdr)
#define	ACK					(14 * 8 + PHY_hdr)

// in us set(2)
#define PropagationDelay        1
#define	SlotTime				20
#define	SIFS					10
#define	DIFS					50
#define PIFS					(SlotTime + SlotTime)
#define EIFS					((ACK/BasicRate)+SIFS+DIFS)//54
*/


//AC
#define AC_NUMBER				1 //시뮬레이션에서 사용할 AC 갯수
#define AC_1					0
#define AC_2					1
#define AC_3					2
#define AC_4					3



#endif
