/*
IEEE 802.11 DCF Simulator
작성자 : 최승찬(Choi Seung Chan : csc7890@naver.com)
지도교수 : 김선명(Kim Sun Myeng : sunmyeng@kumoh.ac.kr)

This is an DCF Simulator developed at Kumoh National Institute of Technology,
Korea. It has been developed mainly for use in the DCF Simulation.
The code is released under the GNU General Public License (GPL). See
the GPL document for more information.

This release is based on IEEE 802.11 DCF. There are no guarantees 
that it implements all features correctly, although this is the goal.

The code has been successfully tested in a no hidden environment
using up to 40 nodes without problems.If you happen to experience 
less successful operation of this implementation, 
please contact the author(s) and describe your problems.

Requirements
============
* Window OS 

*/
#include "Simulator.h"

void printResult(int node_no, ofstream& out, long total_success, double total_failure);

int main(void)
{
	ofstream out;
	out.open("result.txt", ios::out);
	ofstream debugLog;
	
	if(DEBUG_PRINT > 0)
	{
	//파일 삭제
	debugLog.open("debugLog.txt", ios::out | ios::trunc);
	debugLog.close();

	//내용 추가
	debugLog.open("debugLog.txt", ios::out | ios::app);
	}
	
	int node_no;
	long total_success = 0;
	long total_failure = 0;

	Simulator* simulator;

	for(int i=10; i<=40; i+=2)
	{
		node_no = i;

		cout << endl << "NODE NO: " << node_no;
		if(DEBUG_PRINT > 0)
			debugLog << endl << endl << endl <<"****************************[SIMULATION START "<<i<<"]****************************"<< endl << "NODE NO: " << node_no<<endl;
		total_success = 0;
		total_failure = 0;

		for(int j=1; j<=MAX_SIM_NO; j++){
			cout << "*";
			simulator = new Simulator(i,&debugLog,END_TIME,RTS_CTS_USE_MODE,LOSTLESS_PACKET_TYPE);
			//Simulator(int node_No, int end_Time, int mode, int packet_mode);

			/*
			// mode
			#define RTS_CTS_USE_MODE		1
			#define DEFAULT_MODE			2
			#define MAC_MODE				DEFAULT_MODE

			// packet_mode
			#define	CBR						1
			#define	POISSON					2
			#define LOSTLESS_PACKET_TYPE	3
			*/
			
			simulator->runSimulation();

			total_success = total_success + simulator->getSuccessCount();
			total_failure = total_failure + simulator->getFailureCount();
			cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"<<endl;
			simulator->printNodeInfo(out);
			cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"<<endl;
			delete simulator;
		}

		printResult(node_no, out, total_success, total_failure);
		
	}

	debugLog.close();
	out.close();

	return 0;
}

void printResult(int node_no, ofstream& out, long total_success, double total_failure)
{
	double succ_per_sec, avg_success, avg_failure;

	avg_success = (double)total_success / (double)MAX_SIM_NO;
	avg_failure = (double)total_failure / (double)MAX_SIM_NO;

	succ_per_sec = (double)avg_success / ((double)END_TIME/(double)1000000);

	out << "Node:	" << node_no
		<< "	Throughput:	" << (double)(succ_per_sec*(Packet_SZ))/(double)(DataRate*1000000)
		<< "	Coll:	" << (float)avg_failure/(avg_success+avg_failure) << endl;
}
