// Author: Maxwell Young
// Date: Feb 1, 2016

// This is the packet class for the C++ version of PA2 for CSE 4153/6153 
// Students are to use this packet class WITHOUT any modifications for PA2
// Warning: This class comes with no guarantees and is certainly breakable. Use at your own risk.

#include "packet.h"
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdio.h>

using namespace std;  // using standard namespace

// constructor for packet class
// d points to memory already allocated prior to this call OR ELSE should be set to NULL
packet::packet(int t, int s, int l, char * d){
    type = t;
    seqnum = s;
    length = l;
    data = d;
}

// returns the type of packet
int packet::getType(){
	return type;
}

// returns the sequence number
int packet::getSeqNum(){
	return seqnum;   
}

// returns the length
int packet::getLength(){
	return length;   
}

// returns pointer to data
char * packet::getData(){
	return data;   
}

// print function for testing purposes
void packet::printContents(){
	cout << "type: " << type << "  seqnum: " << seqnum << " length: " << length << endl;
	if(data != NULL)
		cout << "data: " << data << endl << endl;
	else
		cout << "data: null" << endl << endl; 
}

// This function serializes the data such that type, seqnum, length, and data values are placed 
// in a char array, spacket, and separated by a single space; that is, spacket contains the serialized data
void packet::serialize(char * spacket){
	sprintf (spacket, "%d %d %d %s", type, seqnum, length, data);	
}

// This function deserializes a char array, spacket, which is the result of a call to serialize (above)
// Warning: Will fail horribly if spacket does not have the correct format as provided by serialize()
// Warning: If length value is non-zero, then the data array must be instantiated or this will fault
void packet::deserialize(char * spacket){
	char * itr;
	itr = strtok(spacket," ");
	char * null_end;

		
	this->type = strtol(itr, &null_end, 10);
	
	itr = strtok(NULL, " ");
	this->seqnum = strtol (itr, &null_end, 10);
	
	itr = strtok(NULL, " ");
	this->length = strtol (itr, &null_end, 10);
	
	if(this->length == 0){
		data = NULL;
	}
	else{
		itr = strtok(NULL, "");
		for(int i=0; i < this->length; i++){ // copy data into char array
			this->data[i] = itr[i];
		}
	}
} // updated 2018