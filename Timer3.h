
#ifndef TIMER3_H_
#define TIMER3_H_

namespace Timer3
{

void init(void);			// initialize USB
void print(char *data);		// print to USB if ready
void wait(int t);			// wait t * period of timer 3
							// wait forever when t = 0
							// wait() can only be called at main()
bool LED(int);				// 0-off, 1=on, otherwise toggle
void setTimer(int);
bool timeout(void);			// need to be called from time-consuming while loops.

}

#endif /* TIMER3_H_ */


/*  EXAMPLE
#include "Timer3.h"

int main(void) {
	Timer3::init();
	Timer3::wait(12000);				// 12000 x 2.5ms = 30 sec
	Timer3::print((char*)"Hello!\r\n");
	Timer3::setTimer(10000);
	while (...) {
		if (Timer3::timeout()) break;
	}
	Timer3::wait(0);					// wait forever
}
 */
