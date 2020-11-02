

#include <p32mx250f128d.h>


extern "C" { // function provided by cdc stack
void PrintPosition();
void USBDeviceTasks(void);
void CDCDeviceTasks(void);
void USBDeviceInit(void);
bool USBCDCTxREADY(void);
void putsUSBUSART(char *data);	// including trailing zero
void __attribute__((weak)) CDCRxChars(char *c, int length) {}		// you got a message from cdc
void __attribute__((weak)) CDCTxReadyNotify(void) {}				// you may send message to cdc
} //extern "C"

void __attribute__((weak)) (*loop)(int);

namespace
{

volatile int count;		// keep changing by t3ISR
int timer;

void USBTasks(void) {
	USBDeviceTasks();
	U1OTGIR = 0xFF;
	IFS1bits.USBIF = 0;
	CDCDeviceTasks();
	if (loop) (*loop)(count);
}

} //anonymous

namespace Timer3
{

void init(void) { USBDeviceInit(); }

void print(char *data) { if (USBCDCTxREADY()) putsUSBUSART(data); }

// wait() can only be called inside main()
void wait(int t) { // t = 0 will wait forever
	if (!t) while (1) USBTasks();
	t += count;
	while (t != count) USBTasks();
}

void setTimer(int t) { timer = count + t; }

bool timeout(void) { USBTasks(); return timer == count; }

bool LED(int i) {
	switch (i) {
	case 0: return (LATACLR = 1);
	case 1: return (LATASET = 1);
	default: return (LATAINV = 1);
	}
}

} //Timer3

namespace Wheel {
void service(void);
void setRCount(int Rcount);
void setLCount(int Lcount);

}


class Counter {
public:
	Counter(void) { counts = last = 0; }
	void sample(short i) {
		short d = i - last;
		counts <<= 4;			// shift out old data
		counts |= d;			// record new data
		last = i;
	}
	int getCount(void) {			// sum up data in "counts"
		int sum = 0, temp = counts;
		for (int i = 0; i < 8; i++) {
			sum += temp & 15;
			temp >>= 4;
		}
		return sum;
	}
private:
	short last;
	int counts;		// holds 8 pcs of 4-bit samples
} left, right;





extern "C"
void __attribute__ ((interrupt)) t3ISR(void) {
	count++;
	Wheel::service();
	static int intRobCount;

	if (++intRobCount == 200){PrintPosition();intRobCount = 0;}
	right.sample(TMR2);
	left.sample(TMR4);
	int lc = left.getCount();
	int rc = right.getCount();
	Wheel::setLCount(lc);
	Wheel::setRCount(rc);

	LATCSET = 1<<9;				// turn on LED
	TMR5 = 0;					// reset delay timer
	T5CONbits.ON = 1;
	IEC0bits.T5IE = 1;
	IFS0bits.T3IF = 0;
}


