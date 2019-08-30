// This is a Go-Back-N C++ implementation
// Author: Deepak Chapagain
// Date: 10/23/2018

#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include "gbn.h"

using namespace std;

//------------------------- Sliding window of length N-------------------

gbn::gbn(int sb, int las, int aw, int lackS, int es, char* pl, int asb)
{
	send_base = sb;
	last_acked_seq = las;
	active_win = aw;
	last_ack_sent = lackS;
	expected_seqnum = es;
	windows[0]= pl;
	ack_send_base = asb;

}// end gnb construct

// ------------Function to check empty window--------------
bool gbn::win_available(int first, int last)
{
	return (active_win<7);
}// end win_available

void gbn::append_win(int index, char*payload)
{
	windows[index] = payload;
	active_win = (active_win +1)%8;
	expected_seqnum = (index+1);
	expected_seqnum = (expected_seqnum%7);
	ack_send_base = index;

}// end append_win

bool gbn::is_empty()
{
	return (active_win==0);
}

void gbn::remove_win(int index)	//removes payload from window
{
	windows[index] = ((char*)0);
	send_base = (index+1)%7;
	if(active_win >0){
		active_win = (active_win -1)%7;
	}
	last_ack_sent = (index)%8;
}// end remove_win

void gbn::resend_win()
{
	cout<<"Resending...."<<endl;
	// may wanna return send_base and resend windows again
}//end resend_win

//--------------------------- Get all parameters-----------------
char* gbn::get_window(int i){
	char* payload;
	payload = (char*)malloc(64);
	memset(payload, 0, 64);
	if(windows[i] != (char*)0)
	{
		payload = windows[i] ;
		return payload;	
	}
	return (char*)0;

}

int gbn::get_last_acked_seq(){
	return last_acked_seq;
}

int gbn::get_send_base(){
	return send_base;
}

int gbn::get_last_ack_sent(){
	return last_ack_sent;
}

int gbn::get_active_win(){
	return active_win;
}

int gbn::get_expected_seqnum(){
	return expected_seqnum;
}

int gbn::get_ack_send_base(){
	return ack_send_base;
}