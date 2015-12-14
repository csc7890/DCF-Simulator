#ifndef __NODE_H__
#define __NODE_H__

#include "common.h"
class EventManager;
class Event;

class Packet
{
public:
	// Transmission rate in Mbps
	int dataRate; //(전송률)
	int basicRate; //(기본 전송률)
	int pktType; //(RTS, CTS, ACK, Data)
	int soucAddress; //(출발 주소 (node_no))
	int destAddress1; //(목적 주소 (node_no))
	int destAddress2; //(최종 목적 주소 (node_no))
	//error(flag 에러 유무)

	int pktSize; //(패킷 사이즈)
	double duration; //(듀레이션 필드)	//	(PktSize / dataRate)

	Packet();
	void copyPacket(Packet* p);
};

class Node
{
public:

	//노드 기본 정보
	int node_no;
	int nx, ny; // (노드의 위치 좌표)
	

	//노드 이벤트 정보
	EventManager* eventManager;//각 노드마다 따로 있는 이벤트 매니저(각 노드의 이벤트 큐 관리) //EventQueue* eventQueue;


	//노드 송신 정보
	int tx_status ; //: NULL, RTS, CTS, DATA, ACK
	Packet trans_packet;// 마지막으로 송신한 패킷의 복사본


	SYSTEM_TIME bc; //(남은 back off 시간)

	int cw;//(현재 컨텐션 윈도우) __AC 추가 확장 가능성 코드 ->//int cw[AC_NUMBER]; 
	int retryCnt;//(재전송 시도횟수) __AC 추가 확장 가능성 코드 ->//int retryCnt[AC_NUMBER]; // STA Retry Count(SRC) 

	int successCount; //(성공 횟수)
	int failedCount;// (실패 횟수)
	/*
	__실패 횟수확장 가능성 변수
	int DATAFailedCnt;(Data 실패 횟수)	
	int RTSFailureCnt;(RTS 실패 횟수)
	int ACKFailureCnt;(ACK 실패 횟수)
	*/
	queue<Packet*> packet_Q;// __AC 추가 확장 가능성 코드 ->// queue<Packet*> packet_Q[AC_NUMBER]; 


	//노드 수신 정보
	Packet receive_packet; // 마지막으로 수신한 패킷의 복사본
	int rx_status ; //: NULL, RTS, CTS, DATA, ACK, COLLISION
	//RX_END 이벤트 발생 시 변경됨



	Node(){
		
	}
	~Node();
	Node(int ID, int nx, int ny);

	void inc_retryCnt() { retryCnt++; } //	__AC 추가 확장 가능성 코드 ->//void inc_retryCnt(int AC_No) { retryCnt[AC_No]++; }
	void destoryPacket();//	__AC 추가 확장 가능성 코드 ->//void destoryPacket(int AC_No);

	void inc_cw();//	__AC 추가 확장 가능성 코드 ->//void inc_cw(int AC_No);
	void insertPacket(Packet* packet);//	 __AC 추가 확장 가능성 코드 ->//void insertPacket(Packet* packet, int AC_No);
	Packet* getPacket();//	__AC 추가 확장 가능성 코드 ->//Packet* getPacket(int AC_No);

	Event* getEventHead();	// Event_Q의 Head를 반환하지만 큐에서 제외하지 않음
	Event* getEvent();		// Event_Q의 Head를 반환하며 큐에서 제외


	//추가된 함수
	bool createEvent(int event_type,SYSTEM_TIME system_Time, SYSTEM_TIME time);//Event 생성되면 1 아니면 0 리턴
	bool create_SIFS(SYSTEM_TIME time);
	bool create_DIFS(SYSTEM_TIME time);
	bool create_BACKOFF(SYSTEM_TIME time);
	bool create_TX_END(SYSTEM_TIME time);
	bool create_RX_END(SYSTEM_TIME system_Time, SYSTEM_TIME time);//RX_END 생성되면 1 아니면 0 리턴
	bool create_TIMER(SYSTEM_TIME time);
	bool create_NAV_END(SYSTEM_TIME time);
};

class NodeManager
{
private:
	vector <Node*> nodeList; //(노드 리스트)
	int node_no;

public :
	NodeManager()
	{
		node_no = 0;
	}
	~NodeManager();

	void insertNode();//노드 좌표 : 0~99 사이 값 랜덤
	void insertNode(int m);//노드 좌표 : 0~m 사이 값 랜덤
	void insertNode(int n, int m);//노드 좌표 : n~m 사이 값 랜덤 
	Node* getNode(int nodeID);
	int getNodeSequence(int nodeID);
	void deleteNode(int nodeID);


	double calDistance(int nodeID_1, int nodeID_2);
	bool isInBSS(int nodeID_1, int nodeID_2);

	vector<Node*>* allInBSS(int nodeID);
	vector<Node*>* allhiddenNode(int nodeID);
	vector<Node*>* getNodeList(){return &nodeList;}

	void existEmptyNode();

		//추가된 함수
	Event* getEvent();

};

#endif
