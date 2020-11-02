/*
 * peripheral usage:
 *
 * Timer3 : time base 2.5 ms, for OC1 & OC2
 *
 * Timer5 : Floor IR LED, 100 us
 *
 * OC1 & OC2 : motor PWMs
 *
 * SPI2 : Floor sensor
 *
 * Interrupt: Timer3, Timer5, SPI2
 *
 *
 * Plug wifi module "ESP-01s" onto the main board.
 * MAKE SURE BOTH POSITION AND ORIENTATION ARE CORRECT before power up.
 * Open MINICOM and type "AT+RST" followed by [Enter].
 * Some AT commands:
 * AT
 * AT+CWMODE=1
 * AT+CWLAP
 * AT+CWJAP="SSID","PASSWORD"   //AT+CWJAP="CF005","27666242"   //AT+CWJAP="CF005","31053106"
 * AT+CIFSR
 * AT+CIPSTART="UDP","0",0,3105,2
 */

#include <p32mx250f128d.h>
#include <math.h>
#include "Wheel.h"

namespace
{
			// preset speed 15
int floor4, flag;   //rc,lc sum of wheel count; rt,lt a monent of count
//unsigned int stp=0, rlen=0,temp=0;
static char string[] = "\n\r+FFFF  +FFFF  +FFFF  +FFFF  +FFFF  +FFFF \r";
					//	0  123456789012345678901234567890123456789012
char *strtest = "Pxxx1f92c61e62a300\n";
char *strtest2 = "Pxxx1595432162a300\n";
//static char buffer[64];

extern "C"	// handlers required by cdc stack
{

void SetPosition();

// Public functions for other modules


}


}

// anonymous namespace for privacy
static class RingBuffer {
public:
	RingBuffer(void) { read = write = 0; }
	char get(void) {
		char c = buffer[read];
		if (read == SIZE-1) read = 0;
		else read++;
		return c;
	}
	void save(char c) {
		buffer[write] = c;
		if (write == SIZE-1) write = 0;
		else write++;
	}
	int empty(void) { return read == write; }
private:
	enum {SIZE=128};		// an alternative to #define
	char buffer[SIZE];
	int read, write;
} tx;


static class ReadBuffer : public RingBuffer {
public:
	ReadBuffer(void) { free();}
	void save(char c) {
		RingBuffer::save(c);		// send to cdc (buffered) as usual
		if (!hold)					// strip UDP message into buffer
			switch (ipd) {
			case IPD:				// phase: read length
				if (c == ':') ipd = len ? -1 : 0;
				else {
					int i = c - '0';
					if (i > 9) ipd = 0;
					else len = len * 10 + i;
				}
				break;
			case -1:				// phase: save message
				buffer[index++] = c;
				if (!--len) {
					buffer[index] = 0;
					hold = true;	// message is ready for processing
				}
				break;
			default:				// phase: look for header "IPD,"
				ipd = ipd << 8 | c;
			}
	}

	//char* getString(void) { return hold ? buffer : 0; }
	bool ready(void){return hold;}
	char* getString(void) { return buffer; }
	void free(void) { ipd = len = index = 0; hold = false; }
private:
	enum {SIZE=100, IPD='I'<<24|'P'<<16|'D'<<8|','};
	int index, ipd, len;
	char buffer[SIZE];
	bool hold;
} rx;



extern "C"	// handlers required by cdc stack
{

void putUSBUSART(char *data, unsigned char length);

void usshort(unsigned u, char *c) {		// unsigned short to ASCII string
	int i, t, r;
	for (i = 0; i < 4; i++) c[i] = '0';	// clear string
	c[i] = '0';							// default zero
	while (u) {							// u must be unsigned short
		t = u / 10;
		r = u - t * 10;
		u = t;
		c[i--] = '0' + r;
	}
}

//power function
long int mpow (int x,int n)
{
    int i;
    int number = 1;

    for (i = 0; i < n; ++i)
        number *= x;
    return(number);
}

//calculate the distance
short int Caldistance(int a,int b, int x, int y)
{
	short int d;
	d=sqrt(mpow((x-a),2)+ mpow((y-b),2));
	return (int)d;
}


float CalDegree(int Calx,int Caly){

	if (Calx == 0 && Caly == 0)
		return 0;
	else
		return 57*(float)(atan2(Calx, Caly));   // return the angle between Caly
												// 180/M_PI = 57.29
}



static class GetPosition {
public:
	GetPosition(void) {Reset(); }
	void save_Bx(short int x) {b_x = (short int)x;}
	void save_By(short int x) {b_y = (short int)x;}
	void save_Rx(short int x) {r_x = (short int)x;}
	void save_Ry(short int x) {r_y = (short int)x;}

	short int get_Rx(void) {return r_x;}
	short int get_Ry(void) {return r_y;}
	short int get_Bx(void) {return b_x;}
	short int get_By(void) {return b_y;}

	void save_Distance(void){ Dis = Caldistance(b_x, b_y, r_x, r_y);}
	void save_Degree(void){ Deg =(int)(CalDegree((r_x-b_x),(r_y-b_y)));}

	short int get_Dis(void) {return Dis;}
	short int get_Deg(void) {return Deg;}

	void SetRobotHead(bool RHead){ RobotHead = RHead ;}
	bool GetRobotHead(void){return RobotHead;}
	void Reset(void){b_x = 0,b_y = 0,r_x = 0,r_y = 0;Deg =Dis = 0;RobotHead = false;}
private:
	short int b_x,b_y,r_x,r_y;
	short int Deg,Dis;
	bool RobotHead ;
} MyRobot,Last_Robot;

//hex to dec
int hex_convert(char *getS){
	unsigned short int data = 0;
	unsigned char clr = 0b00001111;
	unsigned char chrdata[3];


	for(int i = 0;i < 3; i++){
		if ((getS[i] >> 4) == 0b11)
			chrdata[i] = getS[i];
		else
			chrdata[i] =(getS[i] + 9);
		chrdata[i] &= clr;
	}
	//data = chrdata[0] << 8 | (chrdata[1] << 4 | chrdata[2]);
	data = chrdata[0]*256 + chrdata[1]*16 + chrdata[2] * 1;
	return data;
}


//----------------------DEMO 5


int check_RobotHead(){
	short int drobot = CalDegree((MyRobot.get_Rx()-Last_Robot.get_Rx()),(MyRobot.get_Ry()-Last_Robot.get_Ry()));
	MyRobot.SetRobotHead(true);
	return drobot;
}

//send the position to computer via wifi
void PrintPosition(void){

    usshort(MyRobot.get_Bx(), &string[3]);
    usshort(MyRobot.get_By(), &string[10]);
    usshort(MyRobot.get_Rx(), &string[17]);
    usshort(MyRobot.get_Ry(), &string[24]);
    usshort(MyRobot.get_Dis(), &string[31]);

    short int x = check_RobotHead();
	if(x < 0 ){
		string[37] = '-';
		x = -(x);
	}
	usshort(x, &string[38]);

	//MyRobot.Reset();
    putUSBUSART(string, 43);

}

void SetPosition(void){
	char *str;
	//if(rx.ready())
		str = strtest;
		//str = rx.getString();
	//else
	//	return;
	if(!str){ return ;}
	if (str[0] == 'A'){return;}
	Last_Robot = MyRobot;
	MyRobot.save_Bx(hex_convert(&str[4]));
	MyRobot.save_By(hex_convert(&str[7]));
	MyRobot.save_Rx(hex_convert(&str[10]));
	MyRobot.save_Ry(hex_convert(&str[13]));
	MyRobot.save_Distance();
	MyRobot.save_Degree();

	if(!MyRobot.GetRobotHead())
		strtest = strtest2;
	rx.free();

}

void CDCRxChars(char *c, int length) {		// you got a message from cdc
	for (int i = 0; i < length; i++) {
		if (c[i] != '\n') {					// skip linefeed
			tx.save(c[i]);
			if (c[i] == '\r') tx.save('\n');// add linefeed after carriage return
		}
	}
	IEC1bits.U2TXIE = 1;
}

void CDCTxReadyNotify(void) {				// you may send message to cdc
	static char buffer[64];
	unsigned char len = 0;
	while (!rx.empty()) buffer[len++] = rx.get();
	if (len) putUSBUSART(buffer, len);
}

}//extern


int getFloorReading(void) { return floor4; }
bool checkFlag(void) { int f = flag; flag = 0; return f; }
unsigned getOcr1(void){return OC1RS;}
unsigned getOcr2(void){return OC2RS;}
extern "C"	// handlers required by cdc stack
{

bool checkHead(void){return MyRobot.GetRobotHead();}
}



extern "C"
{

void __attribute__ ((interrupt)) t5ISR(void) {
	SPI2CONbits.CKP = 1;		// load data
	LATCCLR = 1<<9;				// turn off LED
	IEC1bits.SPI2RXIE = 1;		// enable interrupt receive done
	SPI2BUF = 0;				// dummy write
	IEC0bits.T5IE = 0;			// disable delay timer
	T5CONbits.ON = 0;
	IFS0bits.T5IF = 0;
}

void __attribute__ ((interrupt)) spi2ISR(void) {
	SPI2CONbits.CKP = 0;
	floor4 = SPI2BUF;			// read
	IEC1bits.SPI2RXIE = 0;		// disable interrupt
	IFS1bits.SPI2RXIF = 0;
}

void __attribute__ ((interrupt)) u2ISR(void) {
	while (U2STAbits.URXDA) rx.save(U2RXREG);
	IFS1bits.U2RXIF = 0;
	if (IEC1bits.U2TXIE) {
		while (!U2STAbits.UTXBF) {
			if (tx.empty()) { IEC1bits.U2TXIE = 0; break; }
			else
				U2TXREG = tx.get();
		}
		IFS1bits.U2TXIF = 0;
	}
}

} // ISRs



