#include "common.h"
#include "Simulator.h"
#include "Event.h"
#include "Node.h"

//////////////////////////////////   Simulator   //////////////////////////////////////////////////////////////////////////////////////////////////////
Simulator::Simulator(int node_No, ofstream* debugLog)
{
	this->node_no = node_No;
	this->debugLog = debugLog;
	this->end_Time = END_TIME;
	this->mode = MAC_MODE;
	this->packet_mode = POISSON;
	initialization();
}
Simulator::Simulator(int node_No, ofstream* debugLog, int end_Time)
{
	this->node_no = node_No;
	this->debugLog = debugLog;
	this->end_Time = end_Time;
	this->mode = MAC_MODE;
	this->packet_mode = POISSON;
	initialization();
}

Simulator::Simulator(int node_No, ofstream* debugLog, int end_Time , int mode)
{
	this->node_no = node_No;
	this->debugLog = debugLog;
	this->end_Time = end_Time;
	this->mode = mode;
	this->packet_mode = POISSON;
	initialization();
}
Simulator::Simulator(int node_No, ofstream* debugLog, int end_Time , int mode, int packet_mode)
{
	this->node_no = node_No;
	this->debugLog = debugLog;
	this->end_Time = end_Time;
	this->mode = mode;
	this->packet_mode=packet_mode;
	initialization();
}

Simulator::~Simulator()
{
	delete nodeManager;
//	delete eventManager;
}
void Simulator::initialization(void)
{
	system_Time = 0;
	success = 0;
	failure = 0;

	// Transmission time in us
	Packet_SZ_us = (double)Packet_SZ / (double)DataRate;
	MAC_hdr_us = (double)MAC_hdr / (double)DataRate;
	PHY_hdr_us = (double)PHY_hdr / (double)BasicRate;
	ACK_us = (double)ACK / (double)BasicRate;
	RTS_us = (double)RTS / (double)BasicRate;
	CTS_us = (double)CTS / (double)BasicRate;

	// Transmission Time
//	TX_Time_coll = (int)(MAC_hdr_us + PHY_hdr_us + Packet_SZ_us + SIFS + ACK_us);
//	TX_Time_succ = (int)(MAC_hdr_us + PHY_hdr_us + Packet_SZ_us + SIFS + ACK_us + 2 * PropagationDelay);


	nodeManager = new NodeManager();

	srand((unsigned)time(NULL));
	seed = rand()/0x7fff;

	Event *temp;


	// TX,RX Node 추가
	for(int i = 0; i < node_no; i++){
		//nodeManager->insertNode(35,65); //히든 노드 없는 상황
		nodeManager->insertNode(99); //히든 노드 많은 상황
	}

	// Packet Generation Event: First Event Time & Packet Type
	for(int i = 0; i < node_no; i++){
		vector <Node*> * nodeList =nodeManager->getNodeList();

		temp = new Event();
		temp->eventType = SIMUL_END;
		temp->eventTime = end_Time;
		nodeList->at(i)->eventManager->pushEvent(temp);//수정 필요(RX_NODE가 0번 노드 고정인 사항)
		if(i != RX_NODE){//AP가 아닐 때 패킷 생성

			if(packet_mode != LOSTLESS_PACKET_TYPE)
			{
				temp = new Event();
				temp->eventType = PACKET_GEN;
				temp->eventTime = system_Time + rand();	// first time of packet generation event
				temp->nodeID = i;

				nodeList->at(i)->eventManager->pushEvent(temp);
			}
			else
			{
				int node_ID=nodeList->at(i)->node_no;
				generateLostlessPacket(node_ID);
			}
		}
	}
};
void Simulator::generateLostlessPacket(int ID)
{
	Node* act_node =nodeManager->getNode(ID);

	// Generate New Packet
	if(act_node->packet_Q.size() >= MAX_Q_SIZE) {
		//큐가 꽉찼다면 패킷 생성 안한다.
	} 
	else 
	{
		Packet* packet = new Packet();
		packet->soucAddress = ID;
		packet->destAddress1 = RX_NODE; //수정 필요 (RX_NODE가 0번 노드 고정인 사항)
		packet->destAddress2 = RX_NODE;	//수정 필요 (RX_NODE가 0번 노드 고정인 사항)
		packet->dataRate = nodeManager->calDistance(RX_NODE, ID); //수정 필요(RX_NODE가 0번 노드 고정인 사항)
		packet->duration = 0;

		if(mode == RTS_CTS_USE_MODE)
		{ // rts_cts_use_mode의 경우 RTS를 보냄
			packet->dataRate = BasicRate;
			packet->pktSize = RTS;
			//packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + (Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS*3;
			packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + calPacket_SZ_us() + (ACK / BasicRate) + SIFS*3;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
		}
		else
		{ // default_mode의 경우 DATA를 보냄
			packet->dataRate = DataRate;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
			packet->pktSize = Packet_SZ;
			//packet->duration = (Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS;
			packet->duration = calPacket_SZ_us() + (ACK / BasicRate) + SIFS;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
		}

		act_node->insertPacket(packet);
	}

	//패킷을 만들고 송신을 시작
	if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL){
		if(act_node->eventManager->isTxStart())
		{
			if(DEBUG_PRINT > 1)
				(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::generateLostlessPacket]패킷을 만들고 송신을 시작 (DIFS 생성)"<<endl;
			SYSTEM_TIME difs_occur_time = system_Time + DIFS;
			act_node->createEvent(DIFS_EVENT,system_Time, difs_occur_time);
		}
	}
}
void Simulator::generatePacket(int ID)
{
	// Find Slot Boundary
	// Generate Packet Transmission Event
	// Generate Next Packet Event
	Node* act_node =nodeManager->getNode(ID);
	Event *temp;

	// Generate Next Packet Event
	temp = new Event();
	temp->eventType = PACKET_GEN;
	if(packet_mode == CBR) temp->eventTime = system_Time + ARRIVAL_CBR;
	if(packet_mode == POISSON) temp->eventTime = system_Time + interArrivalTime(ARRIVAL_POISSON);

	temp->nodeID = ID;
	act_node->eventManager->pushEvent(temp);

	// Generate New Packet
	if(act_node->packet_Q.size() >= MAX_Q_SIZE) {
		//큐가 꽉찼다면 패킷 생성 안한다.

	} 
	else 
	{
		Packet* packet = new Packet();
		packet->soucAddress = ID;
		packet->destAddress1 = RX_NODE; //수정 필요 (RX_NODE가 0번 노드 고정인 사항)
		packet->destAddress2 = RX_NODE;	//수정 필요 (RX_NODE가 0번 노드 고정인 사항)
		packet->dataRate = nodeManager->calDistance(RX_NODE, ID); //수정 필요 (RX_NODE가 0번 노드 고정인 사항)
		packet->duration = 0;

		if(mode == RTS_CTS_USE_MODE)
		{ // rts_cts_use_mode의 경우 RTS를 보냄
			packet->dataRate = BasicRate;
			packet->pktSize = RTS;
			//packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + (Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS*3;
			packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + calPacket_SZ_us() + (ACK / BasicRate) + SIFS*3;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
		}
		else
		{ // default_mode의 경우 DATA를 보냄
			packet->dataRate = DataRate;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
			packet->pktSize = Packet_SZ;
			//packet->duration = (Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS;
			packet->duration = calPacket_SZ_us() + (ACK / BasicRate) + SIFS;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
		}

		act_node->insertPacket(packet);
	}

	//패킷을 만들고 송신을 시작
	if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL){
		if(act_node->eventManager->isTxStart())
		{
			if(DEBUG_PRINT > 1)
				(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::generatePacket]패킷을 만들고 송신을 시작 (DIFS 생성)"<<endl;
			SYSTEM_TIME difs_occur_time = system_Time + DIFS;
			act_node->createEvent(DIFS_EVENT, system_Time, difs_occur_time);
		}
	}
}

void Simulator::runSimulation(void)
{
	int loop = 1;
	Event *temp;

	while(loop){
		temp = nodeManager->getEvent();
		system_Time = temp->eventTime;

		if(DEBUG_PRINT > 0)
		(*debugLog)<<"nodeID:"<<temp->nodeID<<" eventType:"<<temp->eventType<<" system_Time"<<system_Time<<endl;

		switch(temp->eventType){
		case PACKET_GEN :
			generatePacket(temp->nodeID);
			break;

		case SIFS_EVENT :
			SIFS_action(temp);
			break;

		case DIFS_EVENT :
			DIFS_action(temp);
			break;

		case BACKOFF_EVENT :
			BACKOFF_action(temp);
			break;

		case TX_END_EVENT :
			TX_END_action(temp);
			break;

		case RX_END_EVENT :
			RX_END_action(temp);
			break;

		case TIMER_EVENT :
			TIMER_action(temp);
			break;

		case NAV_END_EVENT :
			NAV_END_action(temp);
			break;

		case SIMUL_END :
			
			loop = 0;
			break;
		}

		delete temp;
	}
}

void Simulator::SIFS_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);

	if(act_node->tx_status != NULL || act_node->rx_status != NULL )
	{
		// tx_status : NULL이 아니라면 에러(발생할 수 없음)
		// rx_status : NULL이 아니라면 에러(발생할 수 없음)
		cout << "tx_status : " << act_node->tx_status << endl;
		cout << "rx_status : " << act_node->rx_status << endl;
		cout << "SIFS_action status error." << endl;
		exit(0);
	}

	Packet* reply_packet = new Packet();
	reply_packet->soucAddress = e->nodeID;
	reply_packet->destAddress1 = reply_packet->destAddress2 = act_node->receive_packet.soucAddress;

	switch(act_node->receive_packet.pktType)
	{
	case RTS_TYPE:
		act_node->tx_status = CTS_TYPE;
		reply_packet->dataRate = BasicRate;
		reply_packet->pktSize = CTS;
		reply_packet->duration = act_node->receive_packet.duration - (RTS/act_node->receive_packet.dataRate + SIFS);
		break;

	case CTS_TYPE:
		act_node->tx_status = DATA_TYPE;
		reply_packet->dataRate = DataRate; // 수정 필요 거리에 따른 DataRate로 변경
		reply_packet->pktSize = Packet_SZ;
		reply_packet->duration = act_node->receive_packet.duration - (CTS/act_node->receive_packet.dataRate + SIFS);
		break;

	case DATA_TYPE:
		act_node->tx_status = ACK_TYPE;
		reply_packet->dataRate = BasicRate;
		reply_packet->pktSize = ACK;
		//reply_packet->duration = act_node->receive_packet.duration - (Packet_SZ/act_node->receive_packet.dataRate + SIFS);
		reply_packet->duration = act_node->receive_packet.duration - (calPacket_SZ_us() + SIFS); //(Packet_SZ/act_node->receive_packet.dataRate + SIFS); 수정 필요 (거리에 따른 dataRate시 영향)
		break;

	default:
		cout << "Node No : " << e->nodeID << " / act_node->receive_packet.pktType : " << act_node->receive_packet.pktType << " / 발생할 수 없는 SIFS_action 이벤트 입니다." << endl;
		exit(0);
		break;
	}

	reply_packet->pktType = act_node->tx_status;

	if(DEBUG_PRINT > 0)
	{
		(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::SIFS_action]  "<<reply_packet->soucAddress<<"-->>"<<reply_packet->destAddress1<<"  pktType("<<reply_packet->pktType<<") 전송 시작"<<endl;
		if(DEBUG_PRINT > 1)
		{
			(*debugLog)<<"히든 리스트:";
			vector<Node*>* hiddenNodeList = new vector<Node*>();
			hiddenNodeList=nodeManager->allhiddenNode(reply_packet->soucAddress);
			for(int i=0; i < hiddenNodeList->size(); i++)
			{
				(*debugLog)<<hiddenNodeList->at(i)->node_no<<" ";
			}
			(*debugLog)<<endl;
		}	
	}

	//전송 시작
	transmission_start(act_node,reply_packet);

	delete reply_packet;
}

void Simulator::DIFS_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);

	// tx_status가 NULL이 아니라면 발생할 수 없음
	if(act_node->tx_status != NULL)
	{
		cout << "Node No : " << e->nodeID << " / tx_status : " << act_node->tx_status << " / 발생할 수 없는 DIFS_action 이벤트 입니다." << endl;
		exit(0);
	}

	// rx_status가 NULL, ACK_TYPE이 아니라면 발생할 수 없음
	if(act_node->rx_status != NULL && act_node->rx_status != ACK_TYPE)
	{
		cout << "Node No : " << e->nodeID << " / rx_status : " << act_node->rx_status << " / 발생할 수 없는 DIFS_action 이벤트 입니다." << endl;
		exit(0);
	}

	// 패킷 큐가 비어있지 않다면 BACKOFF 이벤트 생성
	if(act_node->packet_Q.size() > 0)
	{
		SYSTEM_TIME backoff_occur_time = 0;	// BACKOFF 이벤트 발생 시간을 담아두는 변수

		// bc 값이 0이 아니라면 system_time + bc * slot_time 시간으로 BACKOFF 이벤트 시간 설정
		if(act_node->bc != 0)
		{
			backoff_occur_time = system_Time + (act_node->bc * SlotTime);
		}
		else
		{// bc 값이 0이라면 system_time + 랜덤값(0~CW) * slot_time 시간으로 이벤트 시간 설정 
			SYSTEM_TIME bc=(rand() % (act_node->cw + 1));
			backoff_occur_time = system_Time + (bc) * SlotTime;
		}

		act_node->createEvent(BACKOFF_EVENT, system_Time, backoff_occur_time);
	}
}

void Simulator::BACKOFF_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);

	// tx_status가 NULL이 아니라면 발생할 수 없음
	if(act_node->tx_status != NULL)
	{
		cout <<"eTime:"<<e->eventTime<<"success:"<<this->success<<"failure:"<<this->failure<<"  ";
		cout << "Node No : " << e->nodeID << " / tx_status : " << act_node->tx_status << " / 발생할 수 없는 BACKOFF_action 이벤트 입니다." << endl;
		exit(0);
	}

	// rx_status가 NULL이 아니라면 발생할 수 없음
	if(act_node->rx_status != NULL)
	{
		cout <<"eTime:"<<e->eventTime<<"success:"<<this->success<<"failure:"<<this->failure<<"  ";
		cout << "Node No : " << e->nodeID << " / rx_status : " << act_node->rx_status << " / 발생할 수 없는 BACKOFF_action 이벤트 입니다." << endl;
		exit(0);
	}

	act_node->bc=0;//BACKOFF 끝났으므로 bc값 0으로 설정

	// mode에 따라 RTS를 보낼지 DATA를 보낼지 결정
	if(mode == RTS_CTS_USE_MODE) {
		act_node->tx_status = RTS_TYPE;
	}
	else{
		act_node->tx_status = DATA_TYPE;
	}

	// 보낼 packet 생성
	Packet* target_packet = act_node->packet_Q.front();

	Packet* trans_packet = new Packet();
	trans_packet->soucAddress = e->nodeID;
	trans_packet->destAddress1 = trans_packet->destAddress2 = target_packet->destAddress1;
	trans_packet->pktType = act_node->tx_status;
	
	if(DEBUG_PRINT > 0)
	{
		(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::BACKOFF_action]  "<<trans_packet->soucAddress<<"-->>"<<trans_packet->destAddress1<<"  pktType("<<trans_packet->pktType<<") 전송 시작"<<endl;
		if(DEBUG_PRINT > 1)
		{
			(*debugLog)<<"히든 리스트:";
			vector<Node*>* hiddenNodeList = new vector<Node*>();
			hiddenNodeList=nodeManager->allhiddenNode(trans_packet->soucAddress);
			for(int i=0; i < hiddenNodeList->size(); i++)
			{
				(*debugLog)<<hiddenNodeList->at(i)->node_no<<" ";
			}
			(*debugLog)<<endl;
		}
	}
 
	if(mode == RTS_CTS_USE_MODE)
	{ // rts_cts_use_mode의 경우 RTS를 보냄
		trans_packet->dataRate = BasicRate;
		trans_packet->pktSize = RTS;
		//trans_packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + (Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS*3;
		trans_packet->duration = (RTS / BasicRate) + (CTS / BasicRate) + calPacket_SZ_us() + (ACK / BasicRate) + SIFS*3;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
	}
	else
	{ // default_mode의 경우 DATA를 보냄
		trans_packet->dataRate = DataRate;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
		trans_packet->pktSize = Packet_SZ;
		//trans_packet->duration =(Packet_SZ / DataRate) + (ACK / BasicRate) + SIFS;
		trans_packet->duration = calPacket_SZ_us() + (ACK / BasicRate) + SIFS;	// 수정 필요 거리에 따른 DataRate으로 변경해야함
	}

	//전송 시작
	transmission_start(act_node,trans_packet);

	delete trans_packet;
}

void Simulator::TX_END_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);


	// tx_status가 NULL이라면 발생할 수 없는 이벤트
	if(act_node->tx_status == NULL)
	{
		cout << "Node No : " << e->nodeID << " / tx_status : " << act_node->tx_status << " / 발생할 수 없는 TX_END_action 이벤트 입니다." << endl;
		exit(0);
	}

	// tx_status != ACK or NULL 이라면 Timer이벤트 생성
	if(act_node->tx_status != NULL && act_node->tx_status != ACK_TYPE) {
		SYSTEM_TIME timer_occur_time = system_Time;

		// 수신 패킷에 따라 TIMER 발생시간 설정
		switch(act_node->tx_status)
		{

		case RTS_TYPE:
			timer_occur_time = system_Time + SIFS + (CTS / BasicRate);//CTS 기다려야 함..
			break;
		case CTS_TYPE:
			//timer_occur_time = system_Time + SIFS + (Packet_SZ / DataRate);
			timer_occur_time = system_Time + SIFS + calPacket_SZ_us();//DATA 기다려야 함..
			break;
		case DATA_TYPE:
			timer_occur_time = system_Time + SIFS + (ACK / BasicRate);//ACK 기다려야 함..	// DataRate 거리에 따른 값으로 수정 필요
			break;
		}

		act_node->createEvent(TIMER_EVENT, system_Time, timer_occur_time);
	}

	act_node->tx_status = NULL;

	if(act_node->tx_status == ACK_TYPE)
	{
		// packet_Q가 비어있지 않다면 DIFS 생성
		if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL)
		{	
			if(DEBUG_PRINT > 1)
			(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::TX_END_action] ACK_TYPE보낸 후 packet_Q가 비어있지 않다면 DIFS 생성"<<endl;

			SYSTEM_TIME difs_occur_time = system_Time + DIFS;
			act_node->createEvent(DIFS_EVENT, system_Time, difs_occur_time);
		}
	}

}

void Simulator::RX_END_action(Event* e)
{

	Node* act_node = nodeManager->getNode(e->nodeID);

	// NAV 이벤트 존재 유무로 NAV 중인지 확인
	Event* nav_event = act_node->eventManager->searchEvent(NAV_END_EVENT);

	// rx_status가 COLLISION이라면 
	if(act_node->rx_status == COLLISION)
	{
		if(DEBUG_PRINT > 1)
		(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::RX_END_action] ================rx_status가 COLLISION================"<<endl;

		act_node->rx_status = NULL;
		//NAV 중
		if(nav_event !=NULL)
		{
			if(nav_event->eventTime+DIFS <= system_Time+EIFS)
			{
				if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL)
				{
					SYSTEM_TIME eifs_occur_time = system_Time + EIFS;//datarate에 따라서 수정 필요
					act_node->createEvent(DIFS_EVENT, system_Time, eifs_occur_time);
					if(DEBUG_PRINT > 1)
						(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::RX_END_action] 충돌시 NAV중인 경우 NAV_END+DIFS시간 보다 길때 EIFS 이벤트 생성"<<endl;
				}
			}
			return ;
		}
		else
		{ //NAV중 아님
			// 수신한 패킷의 타입이 NULL이 아니면 EIFS생성
			if(act_node->receive_packet.pktType != NULL )
			{
				if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL)
				{
					SYSTEM_TIME eifs_occur_time = system_Time + EIFS;//datarate에 따라서 수정 필요
					act_node->createEvent(DIFS_EVENT, system_Time, eifs_occur_time);
					if(DEBUG_PRINT > 1)
						(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::RX_END_action] 충돌시 NAV중 아니면 packet_Q가 비어있지 않을 때 EIFS 이벤트 생성"<<endl;
				}
				return ;
			}
		}
	}
	else
	{ // rx_status !=  COLLISION  충돌 아닐 때

		Event* timer = act_node->eventManager->searchEvent(TIMER_EVENT);
		// TIMER 이벤트 존재 유무로 TIMER 중인지 확인
		if(timer != NULL && act_node->receive_packet.destAddress1 == act_node->node_no)
		{
			if(mode == RTS_CTS_USE_MODE)
			{
				if(act_node->receive_packet.pktType==CTS_TYPE || act_node->receive_packet.pktType==DATA_TYPE || act_node->receive_packet.pktType==ACK_TYPE)
				{
					//타이머 삭제
					timer->eventTime=END_TIME+END_TIME;
				}
			}
			else
			{
				if(act_node->receive_packet.pktType==ACK_TYPE)
					timer->eventTime=END_TIME+END_TIME;

			}
			act_node->eventManager->sortEvent();
		}

		SYSTEM_TIME nav_occur_time = system_Time;
		//SYSTEM_TIME tx_end_time = system_Time + (trans_packet->pktSize / trans_packet->dataRate);	// TX_END 시간 = system_time + packet 전송 소요시간
		//packet->duration = (Data / DataRate) + (ACK / BasicRate) + SIFS;	

		// 수신 패킷에 따라 NAV 발생시간 설정 및 ACK를 수신했다면 NAV_END 삭제하지 않고 그대로 놔둠
		switch(act_node->receive_packet.pktType)
		{
		case RTS_TYPE:
			nav_occur_time = system_Time + act_node->receive_packet.duration - (RTS / BasicRate);//(RTS / BasicRate); (act_node->receive_packet.pktSize / act_node->receive_packet.dataRate)
			break;
		case CTS_TYPE:
			nav_occur_time = system_Time + act_node->receive_packet.duration - (CTS / BasicRate);// (CTS / BasicRate); (act_node->receive_packet.pktSize / act_node->receive_packet.dataRate)
			break;
		case DATA_TYPE:
			//nav_occur_time = system_Time + act_node->receive_packet.duration - (Packet_SZ / DataRate);
			nav_occur_time = system_Time + act_node->receive_packet.duration - calPacket_SZ_us();// calPacket_SZ_us();	// DataRate 거리에 따른 값으로 수정 필요 (act_node->receive_packet.pktSize / act_node->receive_packet.dataRate)
			break;
		case ACK_TYPE:
			nav_occur_time = system_Time + act_node->receive_packet.duration - (ACK / BasicRate);// (ACK / BasicRate); (act_node->receive_packet.pktSize / act_node->receive_packet.dataRate)
			break;
		}

		// NAV 유무에 따른 동작
		if(nav_event != NULL)
		{// NAV 중
			if(act_node->receive_packet.destAddress1 != act_node->node_no && nav_event->eventTime < nav_occur_time)
			{ // 수신 패킷의 목적지가 내 것이 아니고, 현재 NAV_END < 생성될 NAV_END 시발생 시간이라면 생성될 NAV로 시간 재설정
				nav_event->eventTime = nav_occur_time;
				act_node->eventManager->sortEvent();
			}
			if(act_node->receive_packet.destAddress1 == act_node->node_no)
			{// 수신한 패킷의 목적지가 내것일 때
						act_node->rx_status = NULL; 
						return;
			}
		}
		else
		{// NAV 아님
			if(act_node->receive_packet.destAddress1 == act_node->node_no)
			{// 수신한 패킷의 목적지가 내것일 때
				// 수신 패킷의 타입이 ACK가 아니라면 SIFS 생성 및 rx_status 설정
				if(act_node->receive_packet.pktType != ACK_TYPE) {
				
					SYSTEM_TIME sifs_occur_time = system_Time + SIFS;
					act_node->createEvent(SIFS_EVENT, system_Time, sifs_occur_time);
				}
				else
				{ 
					//전송 성공 처리
					if(DEBUG_PRINT > 0)
					{
					(*debugLog)<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl
						<<"+++++++++++++"<<" [success :"<<this->success+1<<"]"<<"+++++++++"<<endl
						<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
					}
					success++;
					act_node->cw = CWMin;
					act_node->retryCnt = 0;
					act_node->successCount++;	// Number of success transmissions
					if(packet_mode != LOSTLESS_PACKET_TYPE)
						act_node->destoryPacket();
					act_node->rx_status = NULL;

					// 수신 패킷의 타입이 ACK라면 packet_Q가 비어있지 않을 때 DIFS 이벤트 생성 및 rx_status NULL로 설정
					if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL) {
						if(DEBUG_PRINT > 1)
						(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::RX_END_action] 수신 패킷의 타입이 ACK라면 packet_Q가 비어있지 않을 때 DIFS 이벤트 생성"<<endl;
						SYSTEM_TIME difs_occur_time = system_Time + DIFS;
						act_node->createEvent(DIFS_EVENT, system_Time, difs_occur_time);

					}


				}
		
			}
			else
			{// 수신한 패킷의 목적지가 내것이 아닐 때
				// 수신한 패킷의 타입이 NULL이 아니라면 NAV_END 이벤트 생성
				if(act_node->receive_packet.pktType != NULL) {
					act_node->createEvent(NAV_END_EVENT, system_Time, nav_occur_time);
					
				}
			}
		}
	}

	act_node->rx_status = NULL;

}

void Simulator::TIMER_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);

	// tx_status가 NULL, ACK 일 때 에러
	if(act_node->tx_status == NULL && act_node->tx_status == ACK)
	{
		cout << "Node No : " << e->nodeID << " / tx_status : " << act_node->tx_status << " / 발생할 수 없는 TIMER_action 이벤트 입니다." << endl;
		exit(0);
	}

	// rx_status가 NULL이 아니라면 에러 아님!! 히든이면 가능
	if(act_node->rx_status != NULL)
	{
		//실패 처리는 하고 수신 중인 rx_end는 발생
	}
	else
	{
		// 패킷 큐가 비어있지 않고, rx_status가 NULL 이라면 DIFS이벤트 생성
		if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL)
		{
			if(DEBUG_PRINT > 1)
				(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::TIMER_action]  패킷 큐가 비어있지 않고, rx_status가 NULL 이라면 DIFS이벤트 생성"<<endl;
			SYSTEM_TIME difs_occur_time = system_Time + DIFS;
			act_node->createEvent(DIFS_EVENT, system_Time, difs_occur_time);
		}
	}

	//전송 실패 처리
	if(act_node->trans_packet.pktType != CTS_TYPE)
	{
		if(DEBUG_PRINT > 0)
		{
			(*debugLog)<<"------------------------------------------------------------"<<endl
				<<"------------"<<" [failure :"<<this->failure+1<<"]"<<"------------"<<endl
				<<"------------------------------------------------------------"<<endl;
		}
		failure++;
		act_node->failedCount++;	// Number of success transmissions

		act_node->inc_cw();		//cw값 2배로 증가
		act_node->inc_retryCnt();

		// 재전송 시도가 재전송 제한 횟수 이상일 경우 패킷삭제 및 cw값 초기화, 잔여 bc값 초기화
		if(act_node->retryCnt >= RetryLimit)
		{
			act_node->cw = CWMin;
			act_node->retryCnt = 0;
			act_node->bc = 0;
			if(packet_mode != LOSTLESS_PACKET_TYPE)
				act_node->destoryPacket();
		}
	}

}

void Simulator::NAV_END_action(Event* e)
{
	Node* act_node = nodeManager->getNode(e->nodeID);

	// tx_status가 NULL이 아니라면 에러
	if(act_node->tx_status != NULL)
	{
		cout << "Node No : " << e->nodeID << " / tx_status : " << act_node->tx_status << " / 발생할 수 없는 NAV_END_action 이벤트 입니다." << endl;
		exit(0);
	}

	// rx_status = NULL로 설정
	act_node->rx_status = NULL;

	// 패킷 큐가 비어있지 않을 때 DIFS 이벤트 생성
	if(act_node->packet_Q.size() > 0 && act_node->tx_status == NULL && act_node->rx_status == NULL) {
		if(DEBUG_PRINT > 1)
		(*debugLog)<<"[NODE:"<<act_node->node_no<<"  Simulator::NAV_END_action] NAV_END 후 packet_Q가 비어있지 않다면 DIFS 생성"<<endl;
		SYSTEM_TIME difs_occur_time = system_Time + DIFS;
		act_node->createEvent(DIFS_EVENT, system_Time, difs_occur_time);
	}
}

void Simulator:: transmission_start(Node* tx_node, Packet* trans_packet)
{

	tx_node->trans_packet.copyPacket(trans_packet);//마지막으로 보낸 패킷 기억
	// TX_END 이벤트 생성
	SYSTEM_TIME tx_end_time;
	if(trans_packet->pktType ==DATA_TYPE)
	{
		//tx_end_time = system_Time + (trans_packet->pktSize / trans_packet->dataRate);
		tx_end_time = system_Time + calPacket_SZ_us(); /// (trans_packet->pktSize / trans_packet->dataRate);	// TX_END 시간 = system_time + packet 전송 소요시간
	}
	else
	{
		tx_end_time = system_Time + (trans_packet->pktSize / trans_packet->dataRate); /// (trans_packet->pktSize / trans_packet->dataRate);	// TX_END 시간 = system_time + packet 전송 소요시간
	}
	SYSTEM_TIME rx_end_time =  tx_end_time;//RX_END시간 + 1us
	tx_node->createEvent(TX_END_EVENT, system_Time, tx_end_time);

	// 수신노드 RX_END 이벤트 생성
	vector<Node*>* neighborNodeList = nodeManager->allInBSS(tx_node->node_no);	// 인접 노드 리스트 획득
	for(int i=0; i < neighborNodeList->size(); i++)
	{
		Node* rx_node=neighborNodeList->at(i);

		// SIFS_EVENT가 Event_Q에 있다면(SIFS 중이라면) RX_END를 생성하지 않음  
		if(rx_node->eventManager->searchEvent(SIFS_EVENT) != NULL) {
			//송신 모드 이므로 수신 무시
			continue ;
		}

		Event* tx_end_event = rx_node->eventManager->searchEvent(TX_END_EVENT);
		if(tx_end_event != NULL)
		{
			//송신 모드 이므로 수신 무시 
			continue ;
		}

		/*
		Event* nav_event = rx_node->eventManager->searchEvent(NAV_END_EVENT);
		if(nav_event != NULL)
		{
		
			// RX_END 발생 시간이 < NAV 이면 RX_END는 생성되지 않는다.
			if(nav_event->eventTime > rx_end_time) {
				continue ;//NAV 중 일 때 수신 패킷 복사하면 안됬었는데.. 이유는 히든이 끼어들어서 엉뚱한 놈이 전송 성공 했었음.
			}
			
		}
		*/
			rx_node->receive_packet.copyPacket(trans_packet);	// 수신 패킷 복사
			rx_node->createEvent(RX_END_EVENT, system_Time, rx_end_time);//무조건 수신 패킷 복사 후 이벤트 생성해야함.
	}

	//neighborNodeList->clear();
	//delete neighborNodeList;

}
double Simulator::calPacket_SZ_us()
{
	/*
	 Packet_SZ_us = (double)Packet_SZ / (double)DataRate; 
	 MAC_hdr_us = (double)MAC_hdr / (double)DataRate;	
	 PHY_hdr_us = (double)PHY_hdr / (double)BasicRate;
	 */

	return Packet_SZ_us+MAC_hdr_us+PHY_hdr_us;
}

/*---------------------------------------------------------
Random Number and Variable Generation
-----------------------------------------------------------*/
double Simulator:: random(void) /* uniform distrubution from interval (0,1) */
{
	unsigned long m, r;
	m = 2147483647L; /* pow((double)2,(double)31)-1 */
	r = (4001 * seed + 97)%m;
	seed = r;
	return (double) r / m;
};

SYSTEM_TIME Simulator::interArrivalTime(double lamda)
{
	return (SYSTEM_TIME)(-1*log(random())/lamda);
}

void Simulator::printNodeInfo(ofstream& out)
{
	for(int i=0; i < node_no; i++) {
		out << i << "[" << nodeManager->getNode(i)->successCount << "]" << "\t";
	}
	out << "\t / Success Count : " << success <<endl;
	for(int i=0; i < node_no; i++) {
		out << i << "[" << nodeManager->getNode(i)->failedCount << "]" << "\t";
	}
	out << "\t / Failure Count : " << failure << endl;
}
