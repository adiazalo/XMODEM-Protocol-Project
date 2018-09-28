/* Wrapper functions for ENSC-351, Simon Fraser University, By
 *  - Craig Scratchley
 * 
 * These functions may be re-implemented later in the course.
 */

#include <unistd.h>			// for read/write/close
#include <fcntl.h>			// for open/creat
#include <sys/socket.h> 		// for socketpair
#include "SocketReadcond.h"
#include <pthread.h>

int mySocketpair( int domain, int type, int protocol, int des[2] )
{
	int returnVal = socketpair(domain, type, protocol, des);
	return returnVal;
}

int myOpen(const char *pathname, int flags, mode_t mode)
{
	return open(pathname, flags, mode);
}

int myCreat(const char *pathname, mode_t mode)
{
	return creat(pathname, mode);
}

//ssize_t myRead( int fildes, void* buf, size_t nbyte ) //will comment
//{
//	return read(fildes, buf, nbyte );
//}

ssize_t myWrite( int fildes, const void* buf, size_t nbyte )
{
	return write(fildes, buf, nbyte );
}

int myClose( int fd )
{
	return close(fd);
}

int myTcdrain(int des) //used in SenderX in sendLastByte
{

//	How does the XMODEM sender know that all previously written characters have actually been transmitted
//	to the modem and that now is the time for it to dump glitch characters? Well, in the code, we are calling
//	myTcdrain() for it to know.



	return 1;
}

int myReadcond(int des, void * buf, int n, int min, int time, int timeout)
{
	return wcsReadcond(des, buf, n, min, time, timeout );
}

ssize_t myRead( int des, void* buf, size_t nbyte )
{
	// maybe deal with descriptors for files
	// myRead (for our socketpairs) reads a minimum of 1 byte
	return myReadcond(des, buf, nbyte, 1, 0, 0);
}

