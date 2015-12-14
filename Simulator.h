#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "common.h"

class EventManager;
class NodeManager;
class Event;
class Packet;
class Node;

class Simulator
{
private:
	NodeManager* nodeManager;

	ofstream* debugLog;
	
	SYSTEM_TIME system_Time;
	SYSTEM_TIME end_Time	;	//	(10 * 1000000)	 in us

	int mode; //1 : RTS/CTS 사용 mode			2 : RTS/CTS 사용하지 않는 mode
	int packet_mode;// packet type - CBR or POISSON or LOSTLESS_PACKET_TYPE

	long success;
	long failure;
	int node_no;

	unsigned long seed;

	//Transmission Time
	double Packet_SZ_us;
	double MAC_hdr_us;
	double PHY_hdr_us;
	double ACK_us;
	double RTS_us;
	double CTS_us;

public :
	Simulator(int node_No, ofstream* debugLog);
	Simulator(int node_No, ofstream* debugLog, int end_Time);
	Simulator(int node_No, ofstream* debugLog, int end_Time, int mode);
	Simulator(int node_No, ofstream* debugLog, int end_Time, int mode, int packet_mode);
	~Simulator();

	long getSuccessCount() { return success; }
	long getFailureCount() { return failure; }

	void initialization(void);
	void runSimulation(void);
	void generatePacket(int ID);
	void transmission_start(Node* tx_node, Packet* trans_packet);

	double random(void);
	SYSTEM_TIME interArrivalTime(double lamda);
	void printNodeInfo(ofstream& out);

	//추가된 함수
	void DIFS_action(Event* e);
	void SIFS_action(Event* e);
	void BACKOFF_action(Event* e);
	void TX_END_action(Event* e);
	void RX_END_action(Event* e);
	void TIMER_action(Event* e);
	void NAV_END_action(Event* e);
	void generateLostlessPacket(int ID);
	
	double calPacket_SZ_us(Node* nodeID_1, Node* nodeID_2);
	double calPacket_SZ_us();

};

#endif
