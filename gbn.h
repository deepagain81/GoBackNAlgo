// This is a GBN sliding window implementation (header)
// Author: Deepak Chapagain
// Date: 10/23/2018

#ifndef GBN_H
#define GBN_H

class gbn{
private:
	char* windows[7];	//window of size 7
	int send_base;	    //current seq
	int last_acked_seq;	//last acked seq received from server
	int active_win; 	// no of active windows

	int last_ack_sent;	// last seqnum of whose ack is sent
	int expected_seqnum;	// next seqnum expected
	int ack_send_base;

public:
	// functions
	gbn(int sb, int las,int aw, int lacks,int es, char*pl, int asb);

	bool win_available(int first, int last);	// checks if window space is available
	void append_win(int send_base, char* payload);	// adds payload to window
	void remove_win(int send_base);	//removes payload from window
	void resend_win();	//resend all window
	bool is_empty();
	char* get_window(int i);
	// getters!
	int get_last_acked_seq();
	int get_send_base();
	int get_last_ack_sent();
	int get_active_win();
	int get_expected_seqnum();
	int get_ack_send_base();


};//end gbn class

#endif

