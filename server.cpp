// This is an implementation of GBN protocol 
// Name: Deepak Chapagain
// Server.cpp

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
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

//-------------------Function to write data to out.txt-------------
void write_to_out(char* data)
{
	char stringToWrite[30];
	strcpy(stringToWrite, data);
	FILE* file;
	file = fopen("output.txt","a+");
	fseek(file, 0, SEEK_END);
	fputs(stringToWrite, file);
	fclose(file);
}//end write_to_out

void write_to_log(int seqnum)
{
	string s = to_string(seqnum);
	s = "acknum: "+s+'\n';
	char const *pchar = s.c_str();  //use char const* as target type
	FILE* file;
	file = fopen("arrival.log","a+");
	fseek(file, 0, SEEK_END);
	fputs(pchar, file);
	fclose(file);
}//end write_to_out


//###########################-Main-################################
int main(int argc, char const *argv[])
{
	int servSocket = 0;
	remove("output.txt");	// Deleting old file
	remove("arrival.log");
	if ((servSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)	
	{
		perror("servSocket");
		return -1;
	}
//-------------------server socket to bind port---------------------
	struct sockaddr_in servAddr;
	memset((char*)&servAddr, 0, sizeof(servAddr));	
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(atoi(argv[2]));//listening port of server	
	servAddr.sin_addr.s_addr = htons(INADDR_ANY);

//--------------------bind-------------------------
	if ((bind(servSocket, (struct sockaddr*)&servAddr, sizeof(servAddr))) <0)
	{
		perror("bind");
		return -1;
	}

//-------------------------- declear receiving payload---------------------------
	char* payload;
	payload = (char*)malloc(64);
	memset(payload, 0, 64);	//clear out the memory

	char* data;
	data = (char*)malloc(32);
	memset(data, 0, 32);	//clear out the memory

//-------------------------- new socket to store client information
	struct sockaddr_in clntAddr;
	socklen_t clen = sizeof(clntAddr);
	int newSocket = (socket(AF_INET, SOCK_DGRAM, 0));

//------------------------ Receive Data--------------------------	
	int end_while1 = 0;
	gbn my_gbn(0,0,0,0,0,payload,0);		//GBN object
	packet servo_packet(0, 0, 30, data);
	bool new_payload = false;
	int end_transaction = 0;
	bool first_time = true;
	 while(1)
	 {
		if(servo_packet.getData() == (char*)0)
		{
			end_while1 = 1;	//Last payload, so time to end transaction
			goto skip_recv;
		}
		
		cout<<"recv..."<<endl;
		if((recvfrom(servSocket,payload, 37, 0, (struct sockaddr*)&servAddr, &clen))<0)
		{
			perror("Receive data");
			return -1;
		}
		skip_recv:
		servo_packet.deserialize(payload);	//Seperate type, seqnum, length, and data	
		servo_packet.printContents();
		
		if(servo_packet.getSeqNum() == my_gbn.get_expected_seqnum())
		{
			//put in window because it's new payload
			my_gbn.append_win(servo_packet.getSeqNum(), servo_packet.getData());	
			write_to_out(servo_packet.getData());	//commit write to file
			new_payload = true;
		}else{
			cout<<"Timer on..."<<endl;
			// signal(SIGALRM, &sigalrm_handler);
			// alarm(5);
			goto resend_packets;
		}

//---------------------- Send ack back-----------------------------------------------
		resend_packets:	
		int type = 0;
		if(servo_packet.getType()==3){
			type = 2;
		}
		packet ack_packet(type, my_gbn.get_ack_send_base(), 0, (char*)0);
		ack_packet.serialize(payload);
		cout<<"->Ack packet<-"<<endl;
		ack_packet.printContents();
//---------------------Log-------------------------------------
		write_to_log(ack_packet.getSeqNum());		

//------------------- Sending---------------------------------------------------
		//send ack to emulator and emulator will forward it to client
		servAddr.sin_port = htons(atoi(argv[3]));	//port of emulator
		sleep(2);
		first_time = false;
		if((sendto(servSocket,payload,42,0,(struct sockaddr*)&servAddr,clen ))<0)
		{
			perror("send ack");
			return -1;
		}

//----------------------Update window---------------------------
		if(new_payload){
			my_gbn.remove_win(my_gbn.get_ack_send_base());
		}
		cout<<"------------------------------------------"<<endl;

//----------------------Break loop------------------------------------------
		if(end_while1==1 || type==2){
			break;	// end of transmission
		}
		end_transaction +=1;

	}// end while

	close(newSocket);
	close(servSocket);
}//end main