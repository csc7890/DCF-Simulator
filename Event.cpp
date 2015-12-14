#include "Event.h"

//////////////////////////////////   Event   //////////////////////////////////////////////////////////////////////////////////////////////////////

Event::Event()
{
	eventType = NULL;
	nodeID = NULL;
	eventTime = NULL;
	next = NULL;
}

//////////////////////////////////   EventManager   //////////////////////////////////////////////////////////////////////////////////////////////////////

EventManager::EventManager()
{
	event_head=NULL;
}

EventManager::~EventManager()
{
	Event* del_event = NULL;
	del_event = event_head;

	while(event_head != NULL)
	{
		event_head = event_head->next;
		delete del_event;
		del_event = event_head;
	}
}

//이벤트 매니저 구현부
void EventManager:: pushEvent(Event* newEvent)
{
	Event *pos, *prev;

	if(newEvent->eventTime > END_TIME){
		delete newEvent;
		return;
	}
	if(event_head == NULL) {
		event_head = newEvent;
		newEvent->next = NULL;
		return;
	}
	pos = event_head;
	while(pos != NULL){
		if(newEvent->eventTime < pos->eventTime)
			break;
		prev = pos;
		pos = pos->next;
	}
	if(pos == event_head){
		newEvent->next = event_head;
		event_head = newEvent;
	} else {
		newEvent->next = prev->next;
		prev->next = newEvent;
	}

	sortEvent();
};

void EventManager::sortEvent(void)
{
	Event *tmp_head = NULL;
	Event *temp, *temp1;
	Event *pos, *prev;

	while(event_head != NULL) {
		temp = new Event();
		temp->eventType = event_head->eventType;
		temp->eventTime = event_head->eventTime;
		temp->nodeID = event_head->nodeID;

		if(temp->eventTime>END_TIME)
		{
			temp1 = event_head;
			event_head = event_head->next;
			delete temp1;
			continue;
		}

		if(tmp_head == NULL){
			tmp_head = temp;
			temp->next = NULL;
		} 
		else {
			pos = tmp_head;
			while(pos != NULL){
				if(temp->eventTime <= pos->eventTime)// <을 <= 로 수정
				{
					if(temp->eventTime == pos->eventTime)//시간 같으면
					{
						if(temp->eventType <= pos->eventType)// evnet 타입 순서로 정렬
							break;
					}
					else//시간 같지 않고 작으면 break;
						break;
				}
				prev = pos;
				pos = pos->next;
			}
			if(pos == tmp_head){
				temp->next = tmp_head;
				tmp_head = temp;
			} else {
				temp->next = prev->next;
				prev->next = temp;
			}
		}

		temp1 = event_head;
		event_head = event_head->next;
		delete temp1;
	}

	event_head = tmp_head;
};

Event* EventManager:: getEvent(void)
{
	Event *temp;
	if(event_head == NULL){
		return NULL;
	}

	temp = event_head;
	event_head = event_head->next;

	return temp;
};

Event* EventManager::getEventHead()
{
	if(!existEvent()) {
		return NULL;
	}

	return event_head;
}

bool EventManager::existEvent()
{
	if(event_head == NULL) {
		return false;
	}

	return true;
}

Event* EventManager::searchEvent(int event_type)
{
	Event* current_event = event_head;

	while(current_event != NULL)
	{
		if(current_event->eventType == event_type)
			return current_event;

		current_event = current_event->next;
	}

	return NULL;
}

bool EventManager::isTxStart()
{
	Event* current_event = event_head;

	while(current_event->eventType != SIMUL_END)
	{
		if(current_event->eventType < SIMUL_END)
			return false;

		current_event = current_event->next;
	}

	return true;
}
