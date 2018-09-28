//============================================================================
//
//% Student Name 1: Hardeep Takhar
//% Student 1 #: 301270446
//% Student 1 hktakhar (email): stu1 (hktakhar@sfu.ca)
//
//% Student Name 2: Alfonso Diaz
//% Student 2 #: 301215253
//% Student 2 userid (email): stu2 (adiazalo@sfu.ca)
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: _everybody helped us/me with the assignment (list names or put 'None')__Yagnik,Vansh
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  ___________
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P2_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : ReceiverX.cpp
// Version     : September 3rd, 2017
// Description : Starting point for ENSC 351 Project Part 2
// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <string.h> // for memset()
#include <fcntl.h>
#include <stdint.h>
#include <iostream>
#include "myIO.h"
#include "ReceiverX.h"
#include "VNPE.h"

//using namespace std;

ReceiverX::
ReceiverX(int d, const char *fname, bool useCrc)
:PeerX(d, fname, useCrc), goodBlk(false), goodBlk1st(false), numLastGoodBlk(0)
{
	NCGbyte = useCrc ? 'C' : NAK; //condition ? result_if_true : result_if_false
}

void ReceiverX::receiveFile()
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	// should we test for error and set result to "OpenError" if necessary?
	transferringFileD = PE2(creat(fileName, mode), fileName);

	// ***** improve this member function *****

	// below is just an example template.  You can follow a
	// 	different structure if you want.

	// inform sender that the receiver is ready and that the
	//		sender can send the first block
	sendByte(NCGbyte);

	result = "after sendbyte";
		//Expecting multiple SOHs after sending 'C'
		//if sender does not support CRC we expect a single SOH

	while(PE_NOT(myRead(mediumD, rcvBlk, 1), 1), (rcvBlk[0] == SOH)){//I think this handles multiple SOHs, where is the "middle" NAK?
		//Code stopes at this point. Code below never gets executed.
			//myRead function does not work properly
		getRestBlk();
		sendByte(ACK);
		writeChunk();
//		sendByte(ACK); need another NAK here?
		numLastGoodBlk++;
	};


//	if (PE_NOT(myRead(mediumD, rcvBlk, 1), 1), rcvBlk[0] == EOT){   //Need to know how to read/write c
		sendByte(NAK); // NAK the first EOT
//	}
	PE_NOT(myRead(mediumD, rcvBlk, 1), 1);
//	if (PE_NOT(myRead(mediumD, rcvBlk, 1), 1), rcvBlk[0] == EOT){
		sendByte(ACK); // ACK the second EOT
//	}
	PE(close(transferringFileD));
	result = "Done"; // move this line above somewhere?


//
//	switch (state){
//		case '1':
//			if ((NCGbyte== 'C') && (Crcflg==true)){
//
//			}
//				break;
//
//		case'2':
//			if((NCGbyte=='C') && (Crcflg==false) ){
//				sendByte(NAK);
//				getRestBlk();
//				break;
//			}
//			break;
//
//		case '3':
//			if ((NCGbyte==NAK) && (Crcflg==true)){
//
//			}
//
//				break;
//		case '4':
//			if(NCGbyte==NAK && Crcflg==false){
//
//			}
//
//				break;
//	}
}
/* Only called after an SOH character has been received.
The function tries
to receive the remaining characters to form a complete
block.  The member variable goodBlk will be made false if
the received block formed has something
wrong with it, like the checksum being incorrect.  The member
variable goodBlk1st will be made true if this is the first
time that the block was received in "good" condition.
*/
void ReceiverX::getRestBlk()
{
	// ********* this function must be improved ***********
	PE_NOT(myReadcond(mediumD, &rcvBlk[1], REST_BLK_SZ_CRC, REST_BLK_SZ_CRC, 0, 0), REST_BLK_SZ_CRC);
	uint8_t verify_num;
	uint16_t crc;
	uint8_t SUM;
	char state = '1';

	switch (state){

			case '1':
				verify_num= rcvBlk[1]+rcvBlk[2];
					if (verify_num!=255){
						sendByte(NAK);
						goodBlk = false;
						//return;
					}else{
						state = '2';
					}
					break;

			case '2':

				if(numLastGoodBlk!=rcvBlk[1] || uint8_t(numLastGoodBlk+1)!=rcvBlk[1]){ //comparing current block number w/ previuos and expected block number
					can8(); //Sending 8 CAN bytes
					goodBlk = false;
					//return;
				}else{
					state = '3';
				}
				break;

			case '3':
				if (this->Crcflg){
					crc16ns(&crc,rcvBlk+3);
					if ((rcvBlk[132] == uint8_t(crc>>8)) && (rcvBlk[131] == uint8_t(crc))){
						goodBlk = true;
					//	return;
					}else{
						goodBlk = false;
						//return;
					}
				}
				state = '4';
				break;

			case '4':
				//adding all bytes in the recieved chunk
				SUM = 0;
				for (int i = 3 ; i < 131 ; i++){
					SUM = SUM + rcvBlk[i];
				}
				if(uint8_t(SUM) == rcvBlk[131]){
					goodBlk = true;
					state = 'F';
				}else{
					goodBlk = false;
				}
				break;

			case 'F': //Finished
				goodBlk1st = goodBlk = true;
				break;

	}


}

//Write chunk (data) in a received block to disk
void ReceiverX::writeChunk()
{
	PE_NOT(write(transferringFileD, &rcvBlk[DATA_POS], CHUNK_SZ), CHUNK_SZ);
}

//Send 8 CAN characters in a row to the XMODEM sender, to inform it of
//	the cancelling of a file transfer
void ReceiverX::can8()
{
	// no need to space CAN chars coming from receiver
	const int canLen=8; // move to defines.h
    char buffer[canLen];
    memset( buffer, CAN, canLen);
    PE_NOT(myWrite(mediumD, buffer, canLen), canLen);
}
