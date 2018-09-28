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

// Helpers: _everybody helped us/me with the assignment (list names or put 'None')__

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

// File Name   : myIO.cpp

// Version     : October, 2017

// Description : Wrapper functions for ENSC-351 -- Starting point for ENSC 351 Project Part 3

// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)

//============================================================================





#include <unistd.h>                        // for read/write/close

#include <fcntl.h>                        // for open/creat

#include <sys/socket.h>                 // for socketpair

#include "SocketReadcond.h"

#include <mutex>

#include <condition_variable>

#include <vector>

#include "AtomicCOUT.h"

#include <iostream>





using namespace std;

std::mutex mtxGlobal;

class Part3Class
{
        public:
                std::mutex mtxPublic;
                condition_variable cv;
                condition_variable cvr;
                int *Buffer;
                int desPair;
                int desSource;
                void busy(){
                	cout<<"BUSY"<<endl;
					std::unique_lock<std::mutex> lock(mtxPublic);
					(*Buffer)-= 1;
					cv.wait(lock,[this]{return (*Buffer)<=0;});
					(*Buffer) = 0;
                }

//                void myFunction(int des, int n, vector<Part3Class*> test){
//                	std::unique_lock<std::mutex> l_Read(mtxPublic);
//                	if (*Buffer < n) {
//                    	test[des]->cvr.wait(l_Read,[this]{
//                    			return (*Buffer)<=0;
//                    	});
//                	}
//                }
};
vector<Part3Class*> mutexVctr;

int mySocketpair( int domain, int type, int protocol, int des[2] )
{
        int sktNum = socketpair(domain, type, protocol, des);
        Part3Class * desPtr_0 = new Part3Class();
        Part3Class * desPtr_1 = new Part3Class();
        int max_size=max(des[0], des[1]);
        if ( max_size > int(mutexVctr.size())){ std::unique_lock<std::mutex> lockGlobal(mtxGlobal); mutexVctr.resize(max_size+1, nullptr);}

        mutexVctr[des[0]] = desPtr_0;
        mutexVctr[des[1]] = desPtr_1;

        int *alias = new int();
        *alias = 0;
        desPtr_0->Buffer = alias;
        desPtr_1->Buffer = alias;
        desPtr_0->desSource = des[0];
        desPtr_1->desSource = des[1];
        desPtr_0->desPair = des[1];
        desPtr_1->desPair = des[0];
        return sktNum;
}

int myOpen(const char *pathname, int flags, mode_t mode)
{
        return open(pathname, flags, mode);
}



int myCreat(const char *pathname, mode_t mode)
{
        return creat(pathname, mode);
}



int myReadcond(int des, void * buf, int n, int min, int time, int timeout)
{
		//lock
        std::unique_lock<std::mutex> l_Read(mutexVctr[mutexVctr[des]->desPair]->mtxPublic);
        int dataRead = wcsReadcond(des, buf, n, min, time, timeout );
        cout<<"dataRead "<<dataRead<<"  min: "<<min<<endl;

//     	if(min <= *(mutexVctr[des]->Buffer)){ // && mutexVctr[des]->desPair == -1){
			if(dataRead > 0){
				*(mutexVctr[des]->Buffer) -= dataRead;
			}
//      }

        if (*(mutexVctr[des]->Buffer) <= 0 || !*(mutexVctr[des]->Buffer)) {
                mutexVctr[mutexVctr[des]->desPair]->cv.notify_all(); // might be different condition variable
                mutexVctr[des]->cv.notify_all();
        }

        if (dataRead == n){
        	l_Read.unlock();
        }
        return dataRead;

}

ssize_t myRead( int fildes, void* buf, size_t nbyte )
{
        return ssize_t(myReadcond(fildes, buf, nbyte, 1, 0, 0));
}

ssize_t myWrite( int fildes, const void* buf, size_t nbyte )
{
        std::unique_lock<std::mutex> l_Write(mutexVctr[mutexVctr[fildes]->desPair]->mtxPublic);
        int dataWrite = write(fildes, buf, nbyte );

        if(dataWrite != -1){
        	cout<<"dataWrite is not -1"<<endl;
        	*(mutexVctr[fildes]->Buffer)+=dataWrite;
        	if(dataWrite >=nbyte){
        		mutexVctr[mutexVctr[fildes]->desPair]->cv.notify_one();

        	}
        }
//    if dataWrite !=-1
//        		buff+=dataWrite
//				dataWrite>=needed
//				read.cv_notify one
        return dataWrite;

}



int myClose( int fd )
{
//	l_Write.release();
//	CloseHandle(mtxPublic);
//		close cond lock and cond mutex
//		if pair is not equal to -1?
//				unique lock
//				defer lock
//				permission repair lock

        return close(fd); //return finish when closing
}

int myTcdrain(int des)
{
        mutexVctr[des]->busy();
        return 0;
}

