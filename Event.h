#ifndef __EVENT_H__
#define __EVENT_H__

#include "common.h"

class Event
{
public:
	int eventType; //이벤트 종류 : SIFS, DIFS, BACKOFF, TX_END, RX_END, TIMMER, NAV
	SYSTEM_TIME eventTime; //(각 이벤트의 시간)
	int nodeID; //(노드 번호)
	//int traffic_type; //(트레픽 타입) // CBR or POISSON
	Event *next;

	Event();
};

//////////////////////////////////   EventManager   //////////////////////////////////////////////////////////////////////////////////////////////////////
class EventManager
{
private:
	Event *event_head;

public :
	EventManager();
	~EventManager();

	void pushEvent(Event* NewEvent);
	void sortEvent(void);

	Event* getEvent(void);//Head 이벤트 반환 후 헤드의 Next로 헤드 교체
	Event* getEventHead();//Head 이벤트 반환
	bool existEvent();
	Event* searchEvent(int event_type);
	bool isTxStart();
};



#endif
