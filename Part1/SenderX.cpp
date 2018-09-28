//============================================================================
//
//% Student Name 1: Hardeep Kaur Takhar
//% Student 1 #: 301270446
//% Student 1 hktakhar@sfu.ca
//
//% Student Name 2: Alfonso Diaz
//% Student 2 #: 301215253
//% Student adiazalo@sfu.ca
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: TA's
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  Given txt files used for reference
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P1_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : SenderX.cc
// Version     : September 3rd, 2017
// Description : Starting point for ENSC 351 Project
// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <iostream>
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <errno.h>
#include <fcntl.h>	// for O_RDWR
# include <math.h> 	//math
#include <valarray>     // std::valarray

#include "myIO.h"
#include "SenderX.h"

using namespace std;

SenderX::SenderX(const char *fname, int d)
:PeerX(d, fname), bytesRd(-1), blkNum(255)
{
}

//-----------------------------------------------------------------------------

/* tries to generate a block.  Updates the
variable bytesRd with the number of bytes that were read
from the input file in order to create the block. Sets
bytesRd to 0 and does not actually generate a block if the end
of the input file had been reached when the previously generated block was
prepared or if the input file is empty (i.e. has 0 length).
*/
//helper func checksum
//int checksum(chunk){
//	int index =0;
//	int ascii[128];
//	while(index < 128){
//		ascii[index]=int(chunk[index]);
//		index++;
//	}


//}

void SenderX::genBlk(blkT blkBuf)
{
	int blkNumComp = 255-blkNum; //complement of blkNum
	uint8_t checksum;

	if (-1 == (bytesRd = myRead(transferringFileD, &blkBuf[3], CHUNK_SZ ))){ //note: &blkBuf[3] since this is the starting position of the chunk
		ErrorPrinter("myRead(transferringFileD, &blkBuf[0], CHUNK_SZ )", __FILE__, __LINE__, errno);
		cout << "ERROR HERE" << endl;// for testing
	}
	uint8_t eotPointer = EOT; //EOT byte

	// checking value of blkNum
	while(blkNum >255){
		blkNum=blkNum-256;
	}
	blkBuf[0] = SOH;
	blkBuf[1]=blkNum;
	blkBuf[2]=blkNumComp;

//2 cases of Crcflg
	if(!Crcflg){

		int paddingPos;//Starting position for padding
		int EOTPos=132;
		unsigned char* buf;
		int chk_sum=0;//uint8_t chk_sum=0;

		//Padding
		paddingPos=bytesRd+3;
		if(bytesRd!=CHUNK_SZ){
			cout<<"paddingPos"<<paddingPos<<" bytesRd"<<bytesRd<<endl; //for testing
			while (paddingPos<131){
				blkBuf[paddingPos]= CTRL_Z; //Adding CTRL_Z as padding
				paddingPos++;
			};
		};
		//checksum
		for(int i=3;i<CHUNK_SZ+3;i++){
			chk_sum+=blkBuf[i];
		};
		//checking checksum
		while(chk_sum >255){
			chk_sum=chk_sum-256;
		}
		blkBuf[131] = uint8_t(chk_sum);
		//EOT
		if(bytesRd ==0){
			myWrite(mediumD, &eotPointer, 1);
			myWrite(mediumD, &eotPointer, 1);
		}
	}else{


		int paddingPosCRC;//Starting position for padding for CRC case
		unsigned char* bufCRC;
		//Padding
		paddingPosCRC=bytesRd+3;
		if(bytesRd!=CHUNK_SZ){
			while (paddingPosCRC<132){
				blkBuf[paddingPosCRC]= CTRL_Z; //Adding CTRL_Z as padding

				paddingPosCRC++;
			};
		};

		uint16_t crc_temp;

		crc16ns(&crc_temp,&blkBuf[3]); //note: &blkBuf[3] since this is the starting position of the chunk
		cout <<"crc_temp"<<crc_temp<<endl; //for testing

		//shifting
		//low 8 bytes
		blkBuf[132]=static_cast <uint8_t> (crc_temp);
		cout <<"low"<<blkBuf[131]<<endl; //for testing
		//high 8 bytes
		int high = crc_temp;
		high  >>=8;
		blkBuf[131] = static_cast <uint8_t> (high);// high should be sent first
		cout <<"high"<<blkBuf[132]<<endl; //for testing

		//EOT
		if(bytesRd ==0){
			myWrite(mediumD, &eotPointer, 1);
			myWrite(mediumD, &eotPointer, 1);
		}
	};
	};

void SenderX::sendFile()
{
	transferringFileD = myOpen(fileName, O_RDWR, 0);
	int num_of_bytes_written; // var that stores # of bytes written
	if(transferringFileD == -1) {
		cout /* cerr */ << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	}
	else {
		cout << "Sender will send " << fileName << endl;

		blkNum = 1; // but first block sent will be block #1, not #

		// do the protocol, and simulate a receiver that positively acknowledges every
		//	block that it receives.

		// assume 'C' or NAK received from receiver to enable sending with CRC or checksum, respectively
		//cout << "first block" << blkBuf<< endl; //blkBuf[]
		//int check_open; // temp var for opening file
		genBlk(blkBuf); // prepare 1st block
		//blkT chunk(128) = blkBuf;
		while (bytesRd)
		{
			//cout<< blkBuf << endl;
			blkNum ++; // 1st block about to be sent or previous block was ACK'd

			// ********* fill in some code here to send a block ***********
			if(Crcflg){
				myWrite(mediumD, blkBuf, BLK_SZ_CRC);

			}else{
				myWrite(mediumD, blkBuf, BLK_SZ);
			}

			//check for size of bytesRd
			genBlk(blkBuf); // prepare next block
			// assume sent block will be ACK'd

		};
		// finish up the protocol, assuming the receiver behaves normally
		// ********* fill in some code here ***********
		//genBlk(blkBuf);
		//(myClose(transferringFileD));
		if (-1 == myClose(transferringFileD))
			ErrorPrinter("myClose(transferringFileD)", __FILE__, __LINE__, errno);
		result = "Done";
	}
}
