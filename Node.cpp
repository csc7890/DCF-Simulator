#include "Node.h"
#include "Event.h"
//////////////////////////////////   Packet	//////////////////////////////////////////////////////////////////////////////////////////////////////

Packet::Packet()
{
	dataRate = 0;
	basicRate = 0;
	pktType = 0;
	soucAddress = 0;
	destAddress1 = RX_NODE;
	destAddress2 = RX_NODE;
	pktSize = 0;
	duration = 0;
}

void Packet::copyPacket(Packet* p)
{
	dataRate = p->dataRate;
	basicRate = p->basicRate;
	pktType = p->pktType;
	soucAddress = p->soucAddress;
	destAddress1 = p->destAddress1;
	destAddress2 = p->destAddress2;
	pktSize = p->pktSize;
	duration = p->duration;
}

//////////////////////////////////   Node   //////////////////////////////////////////////////////////////////////////////////////////////////////
Node::Node(int ID, int nx, int ny)
{
	node_no=ID;
	this->nx=nx;
	this->ny=ny;

	this->eventManager = new EventManager();

	bc = 0;
	tx_status = NULL;
	rx_status = NULL;

	cw = CWMin;
	retryCnt = 0;

	successCount=0;
	failedCount=0;
}

Node::~Node()
{
	while(!packet_Q.empty())
	{
		delete packet_Q.front();
		packet_Q.pop();
	}

	while(eventManager->existEvent())
		delete eventManager->getEvent();

	delete eventManager;
}

void Node::inc_cw()
{
	cw = (cw << 1) + 1;
	
	if(cw > CWMax) {
		cw = CWMax;
	}
}

void Node::insertPacket(Packet* packet)
{
	if(packet_Q.size() >= MAX_Q_SIZE)
	{
		cout << node_no << "노드 packet Q 에서 overflow 발생" << endl;
		delete packet;

		return;
	}

	packet_Q.push(packet);
}

Packet* Node::getPacket()
{
	if(packet_Q.size() <= 0)
	{
		cout << node_no << "노드 packet_Q 가 비어있어 Pop 수행이 불가능합니다." << endl;
		exit(0);
	}

	Packet* packet = packet_Q.front();
	return packet;
}

void Node::destoryPacket()
{
	if(packet_Q.empty())
	{
		cout << node_no << "노드 packet_Q 가 비어있어 패킷삭제가 불가능합니다.";
		exit(0);
	}

	packet_Q.pop();
}

Event* Node::getEventHead()
{
	return eventManager->getEventHead();
}

Event* Node::getEvent()
{
	return eventManager->getEvent();
}

bool Node::createEvent(int event_type, SYSTEM_TIME system_Time, SYSTEM_TIME time)
{
	bool create_success=false;
	switch(event_type)
	{
	case SIFS_EVENT:
		create_success=create_SIFS(time);
		break;
		
	case DIFS_EVENT:
		create_success=create_DIFS(time);
		break;

	case BACKOFF_EVENT:
		create_success=create_BACKOFF(time);
		break;

	case TX_END_EVENT:
		create_success=create_TX_END(time);
		break;

	case RX_END_EVENT:
		create_success=create_RX_END(system_Time,time);
		break;

	case TIMER_EVENT:
		create_success=create_TIMER(time);
		break;

	case NAV_END_EVENT:
		create_success=create_NAV_END(time);
		break;

	}
	return create_success;
}
bool Node::create_SIFS(SYSTEM_TIME time)
{
	Event* e = new Event();
	e->eventType = SIFS_EVENT;
	//e->eventTime = time + SIFS;
	e->eventTime = time;
	e->nodeID = node_no;

	eventManager->pushEvent(e);
	eventManager->sortEvent();
	return true;
}

bool Node::create_DIFS(SYSTEM_TIME time)
{

	Event* difs_event = eventManager->searchEvent(DIFS_EVENT);

	if(difs_event != NULL)
	{
		// DIFS_EVENT 발생 시간 >= 기존 DIFS_EVENT 발생 시간 이면 DIFS_EVENT는 생성되지 않는다.
		if(difs_event->eventTime <= time) {
			return false;
		}
		else{
			difs_event->eventTime=time;//기존 DIFS대체 
			return true;
		}
	}


	Event* e = new Event();
	e->eventType = DIFS_EVENT;
	//e->eventTime = time + DIFS;
	e->eventTime = time;
	e->nodeID = node_no;

	eventManager->pushEvent(e);
	eventManager->sortEvent();
	return true;
}

bool Node::create_BACKOFF(SYSTEM_TIME time)
{
	Event* e = new Event();
	/*
	if(bc <= 0)
	{
		bc = rand() % cw + 1;
	}
	*/
	e->eventType = BACKOFF_EVENT;
	//e->eventTime = time + (bc * SlotTime);
	e->eventTime = time;
	e->nodeID = node_no;

	eventManager->pushEvent(e);
	eventManager->sortEvent();

	return true;
}

bool Node::create_TX_END(SYSTEM_TIME time)
{
	Event* e = new Event();
	e->eventType = TX_END_EVENT;
	e->eventTime = time;
	e->nodeID = node_no;
	
	eventManager->pushEvent(e);
	eventManager->sortEvent();
	return true;
}

bool Node::create_RX_END(SYSTEM_TIME system_Time, SYSTEM_TIME time)
{
	
	Event* nav_event = eventManager->searchEvent(NAV_END_EVENT);
	Event* e = NULL;

	
	if(nav_event != NULL)
	{
		// NAV_END의 발생시간이 RX_END의 발생시간보다 작거나 같다면 NAV_END 이벤트 삭제
		if(nav_event->eventTime <= time) {
					//NAV 이벤트 삭제
			nav_event->eventTime=END_TIME+END_TIME;
		}
	}
	

	// 수신 패킷의 목적지가 내것 이라면 Timer 이벤트 삭제 
	// 수신 패킷의 타입 상관없이 목적지 내것이면 타이머 이벤트 삭제

	if(rx_status == NULL)
	{ // rx_status가 NULL이라면 수신 packet의 packet_type로 변경하고 RX_END 이벤트 생성
		// BACKOFF 중이었다면 BC값 저장하고 BACKOFF 이벤트 제거
		Event* backoff_event = eventManager->searchEvent(BACKOFF_EVENT);
		if(backoff_event != NULL)
		{
			if(backoff_event->eventTime == system_Time)
			{//벡오프 같이 끝났으므로 수신 무시
				return false;
			}
			else
			{
			this->bc = (backoff_event->eventTime - system_Time) / SlotTime;
			//백오프 삭제
			backoff_event->eventTime=END_TIME+END_TIME;
			}
		}

		Event* difs_event = eventManager->searchEvent(DIFS_EVENT);
		if(difs_event != NULL)
		{
			// DIFS 중 수신이 시작되면 DIFS 제거
				difs_event->eventTime=END_TIME+END_TIME;	
		}

		rx_status = receive_packet.pktType;

		e = new Event();
		e->eventType = RX_END_EVENT;
		e->eventTime = time;
		e->nodeID = node_no;

		eventManager->pushEvent(e);

	}
	else
	{ // rx_status가 NULL이 아니라면 COLLISION으로 설정
		rx_status = COLLISION; 

		// RX_END 이벤트가 존재하기때문에 시간비교 후 더 긴 시간으로 변경(충돌)
		e = eventManager->searchEvent(RX_END_EVENT);

		// 이벤트 시간비교 후 설정
		if(e != NULL)
		{
			if(e->eventTime < time) {
				e->eventTime = time;
			}
		}
		
		Event* timer = eventManager->searchEvent(TIMER_EVENT);
		if(timer != NULL)
		{
			if(timer->eventTime < time) {
				timer->eventTime = time;//물리적인 관점에서는 한 신호이므로 타이머 시간을 뒤에 것으로 늘린다(실제 동작에서는 타이머가 없기 때문에)
			}
		}

	}

	eventManager->sortEvent();
	return true;
}

bool Node::create_TIMER(SYSTEM_TIME time)
{

	Event* e = new Event();
	e->eventType = TIMER_EVENT;
	e->eventTime = time ;//time1 + SIFS + RTS | CTS | ACK

	e->nodeID = node_no;

	eventManager->pushEvent(e);
	eventManager->sortEvent();
	return true;
}

bool Node::create_NAV_END(SYSTEM_TIME time)
{
	Event* current_NAV = eventManager->searchEvent(NAV_END_EVENT);
	
	// NAV 중이라면 생성하려는 NAV_END와 진행중인 NAV_END 이벤트의 발생시간 비교후 긴 것으로 대체
	if(current_NAV != NULL)
	{
		if(current_NAV->eventTime < time) {
			current_NAV->eventTime = time;
		}

		eventManager->sortEvent();
		return true;
	}

	// NAV중이지 않다면 NAV 이벤트 생성
	Event* e = new Event();
	e->eventType = NAV_END_EVENT;
	e->eventTime = time;
	e->nodeID = node_no;

	eventManager->pushEvent(e);
	eventManager->sortEvent();
	return true;
}


//////////////////////////////////   NodeManager   //////////////////////////////////////////////////////////////////////////////////////////////////////

NodeManager::~NodeManager()
{
	for(int i=0; i < nodeList.size(); i++)
	{
		delete nodeList[i];
	}

	//nodeList.clear();
}

//노드 매니저 구현부
void NodeManager::insertNode()
{
	int nx=0;
	int ny=0;

	if(node_no == RX_NODE) {
		nx = 50;
		ny = 50;
	}
	else{
		do{
			nx = rand() % 100;// x좌표 0~99 랜덤 값
			ny = rand() % 100;// y좌표 0~99 랜덤 값
		}while(sqrt(pow(nx - RX_NODE_X_POINT, 2) + pow(ny - RX_NODE_Y_POINT, 2)) > BSS_SCOPE);
		// AP의 좌표 (50,50), BSS범위 50 이를 벗어나는 단말기의 좌표를 재설정
	}

	Node* newNode= new Node(node_no,nx,ny);

	nodeList.push_back(newNode);
	node_no++;//노드 카운트 증가
};

void NodeManager::insertNode(int m)
{
	int nx=0;
	int ny=0;

	if(node_no == RX_NODE) {
		nx = 50;
		ny = 50;
	}
	else{
		do{
			nx = rand() % (m+1);// x좌표 0~m 랜덤 값
			ny = rand() % (m+1);// y좌표 0~m 랜덤 값
		}while(sqrt(pow(nx - RX_NODE_X_POINT, 2) + pow(ny - RX_NODE_Y_POINT, 2)) > BSS_SCOPE);
		// AP의 좌표 (50,50), BSS범위 50 이를 벗어나는 단말기의 좌표를 재설정
	}

	Node* newNode= new Node(node_no,nx,ny);

	nodeList.push_back(newNode);
	node_no++;//노드 카운트 증가
};

void NodeManager::insertNode(int n,int m)
{
	int nx=0;
	int ny=0;

	if(node_no == RX_NODE) {
		nx = 50;
		ny = 50;
	}
	else{
		do{
			nx = rand() % (m-n+1) + n;// x좌표 0~m 랜덤 값
			ny = rand() % (m-n+1) + n;// y좌표 0~m 랜덤 값
		}while(sqrt(pow(nx - RX_NODE_X_POINT, 2) + pow(ny - RX_NODE_Y_POINT, 2)) > BSS_SCOPE);
		// AP의 좌표 (50,50), BSS범위 50 이를 벗어나는 단말기의 좌표를 재설정
	}

	Node* newNode= new Node(node_no,nx,ny);

	nodeList.push_back(newNode);
	node_no++;//노드 카운트 증가
};

Node* NodeManager::getNode(int nodeID)
{
	for(int i=0; i < nodeList.size(); i++)
	{
		if(nodeList.at(i)->node_no == nodeID) {
			return nodeList.at(i);
		}
	}

	return NULL;
};

int NodeManager::getNodeSequence(int nodeID)
{

	for(int i=0; i < nodeList.size(); i++)
	{
		if(nodeList.at(i)->node_no == nodeID)
			return i;
	}

	cout << "노드가 없습니다"<<endl;
	return NULL;
};

void NodeManager:: deleteNode(int node_no)
{
	Node* delNode=getNode(node_no);
	nodeList.erase(nodeList.begin()+getNodeSequence(node_no));

	delete delNode;
	cout <<"node[" <<node_no <<"] 삭제 완료"<<endl ;
};

Event* NodeManager::getEvent()
{
	int select_node=0;
	SYSTEM_TIME most_rapid_time = END_TIME;
	Event* tmp;

	for(int i=0; i < node_no; i++)
	{
		tmp = nodeList[i]->getEventHead();
	
		if(tmp == NULL)
			continue;

		if(tmp->eventTime <= most_rapid_time)
		{
			if(tmp->eventTime == most_rapid_time)//시간 같으면 
			{
				Event* most_rapid_evnet=nodeList[select_node]->getEventHead();
				if(tmp->eventType <= most_rapid_evnet->eventType)//evnet 타입으로 순서로 선택
				{
					most_rapid_time = tmp->eventTime;
					select_node = i;
				}
			}
			else//시간 같지 않고 작으면 
			{
				most_rapid_time = tmp->eventTime;
				select_node = i;
			}
		}

	}

	return nodeList[select_node]->getEvent();
}

double NodeManager:: calDistance(int nodeID_1, int nodeID_2)
{
	double distance;
	Node* N1=getNode(nodeID_1);
	Node* N2=getNode(nodeID_2);
	double dx=(N2->nx) - (N1->nx);
	double dy=(N2->ny) - (N1->ny);
	//거리 구하는 공식  선분AB = √ ( x2  -  x1 )² + (  y2  -  y1  )²
	distance = sqrt(pow(dx, 2)+pow(dy, 2));   
	return distance;
};

bool NodeManager::isInBSS(int nodeID_1, int nodeID_2)
{
	double distance = calDistance(nodeID_1,nodeID_2);

	if(distance > BSS_SCOPE)
		return false;
	else
		return true;
};

vector<Node*>* NodeManager:: allInBSS(int nodeID)
{
	vector<Node*>* neighborNodeList = new vector<Node*>();

	for(int i=0; i < nodeList.size(); i++)
	{
		if(nodeList[i]->node_no != nodeID && isInBSS(nodeList[i]->node_no, nodeID))
		{
			neighborNodeList->push_back(nodeList[i]);
		}
	}

	return neighborNodeList;
};

vector<Node*>* NodeManager:: allhiddenNode(int nodeID)
{
	vector<Node*>* hiddenNodeList = new vector<Node*>();

	for(int i=0; i < nodeList.size(); i++)
	{
		if(nodeList[i]->node_no != nodeID && !isInBSS(nodeList[i]->node_no, nodeID))
		{
			hiddenNodeList->push_back(nodeList[i]);
		}
	}

	return hiddenNodeList;
};

void NodeManager::existEmptyNode()
{
	for(int i=0; i < nodeList.size(); i++)
	{
		if(nodeList.at(i)->packet_Q.size() <= 0)
		{
			cout << endl << i << "번 노드 packet_Q 비었음";
			exit(0);
		}
	}
}

/* __AC 추가 확장 가능성 코드
Node::Node(int ID,int channel, int nx, int ny)
{
	node_no=ID;
	this->channel=channel;
	this->nx=nx;
	this->ny=ny;

	nav_start=0;
	nav_end=0;
	back_off=0;

	for(int i=0; i <= AC_NUMBER; i++)
	{
		cw[i] = CWMin;
		retryCnt[i] = 0;
	}

	successCount=0;
	failedCount=0;
	delayBound=33000;
}

Node::~Node()
{
	for(int i=0; i < AC_NUMBER; i++)
	{
		while(!packet_Q[i].empty())
		{
			delete packet_Q[i].front();
			packet_Q[i].pop();
		}
	}
}

void Node::inc_cw(int AC_No)
{
	cw[AC_No] = (cw[AC_No] << 1) + 1;
	
	if(cw[AC_No] > CWMax) {
		cw[AC_No] = CWMax;
	}
}

void Node::insertPacket(Packet* packet, int AC_No)
{
	if(packet_Q[AC_No].size() >= MAX_Q_SIZE)
	{
		cout << node_no << "노드 AC" << AC_No << "packet Q 에서 overflow 발생" << endl;
		delete packet;

		return;
	}
	packet_Q[AC_No].push(packet);
}

Packet* Node::getPacket(int AC_No)
{
	if(packet_Q[AC_No].size() <= 0)
	{
		cout << node_no << "노드 AC_" << AC_No+1 << " packet_Q 가 비어있어 Pop 수행하지 못했습니다." << endl;
		exit(0);
	}

	Packet* packet = packet_Q[AC_No].front();
	return packet;
}

void Node::destoryPacket(int AC_No)
{
	if(packet_Q[AC_No].empty())
	{
		cout << node_no << "노드의 AC" << AC_No << "큐가 비어있어 패킷삭제가 불가능합니다.";
		exit(0);
	}

	packet_Q[AC_No].pop();
}
*/
