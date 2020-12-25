#include "serial.h"
#include "tdgnss.h"

#define BAUDRATE B9600
int com_fd;

//serial port comm func ========================================
//set Vtime : the min overtime
//set Vmin  : the min character to read
//
// ======================================================
int OpenCom(char *com, unsigned char Vtime, unsigned char Vmin)
{
	struct termios newtio;

	com_fd = open( com, O_RDWR);

	if (-1 == com_fd)
	{
		D("Error : OpenCom\n");
		return FALSE;
	}

	memset(&newtio, 0, sizeof(struct termios));

	newtio.c_cflag |= (CLOCAL | CREAD);
	newtio.c_cflag |= BAUDRATE;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;
	newtio.c_cflag &= ~CRTSCTS;

	newtio.c_lflag = 0;

	//newtio.c_oflag=0;
	newtio.c_oflag &= ~OPOST;

	newtio.c_cc[VMIN] = Vmin;
	newtio.c_cc[VTIME] = Vtime;

	//newtio.c_iflag&=~(IXON|IXOFF|IXANY);
	newtio.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	cfsetispeed(&newtio, BAUDRATE);
	cfsetospeed(&newtio, BAUDRATE);

	tcsetattr(com_fd, TCSANOW, &newtio);
	return TRUE;
}

int CloseCom(void)
{
	if(-1 == com_fd)
		return FALSE;
	
	close(com_fd);
	com_fd = -1;
	return TRUE;
}
void BaudRateChange(int baud)
{
	if(-1 == com_fd)
		return;
	
	struct termios tio; 
	tcgetattr(com_fd, &tio);

	cfsetispeed(&tio, baud);
	cfsetospeed(&tio, baud);

	tcsetattr(com_fd, TCSANOW, &tio);

}
void DiscardBuffer(void)
{
	if(-1 == com_fd)
		return;

	tcflush(com_fd,TCIOFLUSH);
}

extern pthread_mutex_t UartWriteMutex; 

int TxData(unsigned char *buffer, int length)
{
	int tx_len = 0;
	if(-1 == com_fd)
		return FALSE;

	do
	{
		pthread_mutex_lock(&UartWriteMutex);
		tx_len = write( com_fd, buffer, length );
		pthread_mutex_unlock(&UartWriteMutex);
//		D("tx_len %d errno %d EINTR %d, EAGAIN %d b[2] %d b[3] %d.\n",tx_len, errno, EINTR, EAGAIN, buffer[2], buffer[3]);
	}
	while (tx_len < 0 && (errno == EINTR || errno == EAGAIN));

	return tx_len;
	
}

int RxData(unsigned char *buffer, int length)
{
	if(-1 == com_fd)
		return FALSE;

	return read(com_fd, buffer, length);
}

void Write55(void)
{
	byte buf[2] = {0x55, 0x55};

	if(com_fd != -1)
	{
		write(com_fd, buf, 2);
	}
}

