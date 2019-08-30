// This is an implementation of GBN protocol 
// Name: Deepak Chapagain
// client.cpp
#include <stdio.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>
#include <sstream>
#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include "packet.h"
#include "packet.cpp"
#include "gbn.cpp"
#include "gbn.h"
#include <signal.h>


using namespace std;
// sig_atomic_t sig = 0;

// struct timeval tv;
// time_t tv_sec = 2;
// suseconds_t tv_usec = 0;

// ---------------Read data from file----------------
char* get_data(const char*filename, int pos)
{
	FILE *file;
	char *databuf;
	databuf = (char*)malloc(32);
	memset(databuf, 0, 32);	//clear out the mem
	file = fopen("file.txt", "rb");
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);	// pointer position
	rewind(file);	//sets pointer in file to beginning
	fseek(file, pos, SEEK_SET);
	int result = fread(databuf,1,30,file);
	if(result==0)
	{
		return ((char*)0);
	}
	return databuf;
}// end read_file

//--------------------Payload to send-------------------------
char* payload_2send(int type, int seqnum, int size, char* data)
{
	char* payload;
	payload = (char*)malloc(64);
	memset(payload, 0, 64);
	packet mypacket(type, seqnum, size, data);
	mypacket.serialize(payload);
	return payload;
}// end payload_2send

void write_to_log(int seqnum, int isAck)
{
	string s = to_string(seqnum);
	s = "seqnum: "+s+'\n';
	char const *pchar = s.c_str();  //use char const* as target type
	const char* filename ="seqnum.log";
	if(isAck){
		filename = "ack.log";
	}

	FILE* file;
	file = fopen(filename,"a+");
	fseek(file, 0, SEEK_END);
	fputs(pchar, file);
	fclose(file);
}//end write_to_out


//#####################-Main-####################################
int main(int argc, char const *argv[])
{
	clock_t start;
	remove("seqnum.log");
	remove("ack.log");
	double duration = 2.0;
	int mySocket = 0;
	if ((mySocket = socket(AF_INET, SOCK_DGRAM,0)) < 0) 
	{
		perror("socket");
		return -1;
	}
	struct hostent *s;
	s = gethostbyname(argv[1]);
	struct sockaddr_in serverAddr;
	memset((char*)&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));	//port for emulator
	bcopy((char*)s->h_addr, (char*)&serverAddr.sin_addr.s_addr, s->h_length);
	socklen_t slen = sizeof(serverAddr);

//---------------------------- Read data from file ----------------------
	int send_base = 0;	// at the start
	int last_acked_seq = 0;	//last acked seq received from server
	int active_win = 0 ; 	// no of active windows
	int last_ack_sent = 0;	// last seqnum of whose ack is sent
	int expected_seqnum = 0;	// next seqnum expected
	
	char* data;
	data = (char*)malloc(32);
	memset(data, 0, 32);

	gbn mygbn(0,0,0,0,0,data,0);		//GBN object
	int position = 0;
	int type = 1;
	int index = 0;
	int end_while1 = 0;
	bool first_time = true;
	int port;
	int varSocket;
	int i;
	
	while(1)
	{
		make_windows_again:
//------------------------- Make Window--------------------------
	// make window here. That includes data, only, from 
	// file. Once it's time to send that data, make packet
	// and send it.

		while(index < 7)	// make 7 of them
		{
			data = get_data(argv[3], position);
			if(data == (char*)0){
				break;
			}//end if
			position += 30;
			mygbn.append_win(index, data);
			index = (index+1)%8;
		}// end while
		
//-------------------Serialize----------------------------------
		//resend_packets:

		char* payload;
		payload = (char*)malloc(64);
		memset(payload, 0, 64);

		

		if(!mygbn.is_empty())	// if not empty
		{
			for(i=0;i<(mygbn.get_active_win());i++)
				{
					resend_packets:
					// char* payload;
					// payload = (char*)malloc(64);
					// memset(payload, 0, 64);
					data = mygbn.get_window(i);	// get ith window
					payload = payload_2send(type, i, 30, data);	//serialized payload
					if(mygbn.get_window(i+1) == (char*)0 && !first_time){
						payload = payload_2send(3, i, 30, data);
					}
					// Send packets
					if((sendto(mySocket, payload, 42, 0, (struct sockaddr*)&serverAddr, slen))< 0)
					{
						perror("sendto");
						return -1;
					}
					cout<<"Sent data:"<<payload<<endl;
					write_to_log(i, 0);	//Write seqnum to seqnum.log
					index -= 1;
					if(data == (char*)0)
					{
						end_while1 = 1;
						break;
					}
				}// end for

		}else if(data==(char*)0){	// if empty
			end_while1 = 1;
		}else{
			goto make_windows_again;//because window is empty
		}// end if else
		cout<<"-------------------------------"<<endl;


// -----------------------------------Receive ack----------------------------
		//packet struct and receiving payload
		packet my_packet(1, 0, 0, (char*)0);
		char* ack_load;
		ack_load = (char*)malloc(64);
		memset(ack_load, 0, 64);

//------------------ Receive Ack -------------------------------

		struct sockaddr_in clntAddr;
		socklen_t clen = sizeof(clntAddr);
		int newSocket = (socket(AF_INET, SOCK_DGRAM, 0));
		clntAddr.sin_port = htons(atoi(argv[3]));	//listening port of client
		clntAddr.sin_addr.s_addr = htons(INADDR_ANY);
		int var = 0;
//--------------------Binding--------------------------------------
		if(first_time)
		{
			if ((bind(newSocket, (struct sockaddr*)&clntAddr, sizeof(clntAddr))) <0)
			{
				perror("bind");
				return -1;
			}
		}
		bool flag = false;
		while(!flag && (mygbn.get_active_win()>=0 ||mygbn.get_active_win()<7))
		{
			
			if(end_while1 == 1 ){
				break;
			}
//---------------------------------------------Signal interrupter--------------------
			if(!first_time){
				newSocket= varSocket;
				clntAddr.sin_port = (port);	//listening port of client
			}
			cout<<"Ack recv..."<<endl;
			if((recvfrom(newSocket,ack_load, 42, 0, (struct sockaddr*)&clntAddr, &clen))<0)
			{
				perror("Receive data");
				return -1;
			}
			port = ntohs(clntAddr.sin_port);
			varSocket = newSocket;
			my_packet.deserialize(ack_load);
			my_packet.printContents();
//----------------------Log-----------------------------------
			write_to_log(my_packet.getSeqNum(), 1);
//------------------------Update window-------------------------
			if(mygbn.get_send_base() == my_packet.getSeqNum())	//
			{
				mygbn.remove_win(my_packet.getSeqNum());	
				//cout<<"Ack got for seqnum: "<<my_packet.getSeqNum()<<endl;
				//cout<<"active_win: "<<mygbn.get_active_win()<<endl;
				if(mygbn.get_active_win()==0){
					flag = true;
				}
			}else{
				cout<<"Resend packet again!"<<endl;
				//mygbn.resend_win();	//resend
				cout<<"active_win: "<<mygbn.get_active_win()<<endl;
				first_time = false;
				i = mygbn.get_send_base();
				goto resend_packets;
			}

		}// end while loop
		first_time = false;

		if(end_while1 == 1 ){
			break;
		}
		//close(newSocket);
	}//end while1
	close(mySocket);
}// end main


//*************************** To do**************************************
			// test on pluto