
#include "Timer3.h"
#include "Wheel.h"

void (*loop)(int) = [](int t){
	static bool state;
	if (t & 0xff) state = true;
	else {
		if (state) Timer3::LED(2);
		state = false;
	}
};
extern "C" {
void SetPosition();
bool checkFlag();
bool checkHead();
int check_RobotHead();
}

int main(void) {

	Timer3::init();
	Wheel::setSpeed(5,0);
	while (1) {
		Timer3::wait(100);
		SetPosition();
		//check the position and call the function to move
		if (!checkHead()){
			Wheel::setMode(Wheel::FORWARD);
			while (Wheel::getCount() < 200) Timer3::timeout();
			Wheel::setMode(Wheel::STOP);
			SetPosition();
			int intturnL = check_RobotHead()*3;
			if (intturnL < 0 ){
				intturnL -= intturnL;
				Wheel::setMode(Wheel::ROTATE_RIGHT);
			}else
				Wheel::setMode(Wheel::ROTATE_LEFT);

			while (Wheel::getCount() < intturnL) Timer3::timeout();
			Wheel::setMode(Wheel::STOP);
		}
		
	}
}

