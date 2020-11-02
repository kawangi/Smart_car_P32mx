

#include "Wheel.h"
#include <p32mx250f128d.h>

#define PERIOD 45000

extern "C"	// handlers required by cdc stack
{
class PID {
public:
	PID(int p, int in, int id, int d) {
			P = p;
			In = in; Id = id;	// I break "I" into two parts to avoid decimal
			D = d;
			acc = last = 0;
	}
	void reset(void) {acc = last = 0;}
	int control(int x, int target) {
			int error = x - target;
			int diff = x - last;
			last = x;
			acc += error;
			if (acc > 2000000000) acc = 2000000000;
			if (acc < -2000000000) acc = -2000000000;
			return - P * error - In * (acc >> Id) - D * diff;
	}
private:
	int P, In, Id, D, acc, last;
}pid_speed(1500,0,0,0), pid_track(1500,800,0,1500);

unsigned short bound(int control) {
	if (control < 0) return 0;
	return control > PERIOD ? PERIOD : control;
}
}

namespace Wheel {
int rt,lt,Wheel_Count, Lsp,Rsp;

void service(void){

}
//set and clr bits to control the moving
void Move_Left(void){
		TRISCSET = 0b100;
		TRISCCLR = 0b010;
		OC1RS = 15000;
		OC2RS = 15000;
}

void Move_Right(void){
		TRISCSET = 0b010;
		TRISCCLR = 0b100;
		OC1RS = 15000;
		OC2RS = 15000;
}

void Move_Front(int Rsp,int Lsp){
		TRISCSET = 0b110;
		OC1RS = Rsp;
		OC2RS = Lsp;
}

void Move_Back(int Rsp,int Lsp){
		TRISCCLR = 0b110;
		OC1RS = Rsp;
		OC2RS = Lsp;
}

void Move_Stop(void){
		OC1RS = 0;
		OC2RS = 0;
}

void setMode(int action){
	Wheel_Count= 0;
	switch (action){

	case STOP:
		Move_Stop();
		break;

	case FORWARD:
		Move_Front(Rsp,Lsp);
		break;

	case BACKWARD:
		Move_Back(Rsp,Lsp);
		break;

	case ROTATE_LEFT:
		Move_Left();
		break;

	case ROTATE_RIGHT:
		Move_Right();
		break;
	}

}

void setSpeed(int Sp,int track){

	int control_speed = pid_speed.control(getRightCount() + getLeftCount(), Sp);
	int control_track = pid_track.control(getRightCount() - getLeftCount(), track);
	Rsp = bound(control_speed - control_track);
	Lsp = bound(control_speed + control_track);

}


void setRCount(int Rcount){rt = Rcount; Wheel_Count+=Rcount;}
void setLCount(int Lcount){lt = Lcount; Wheel_Count+=Lcount;}


unsigned int getCount(){return Wheel_Count / 2 ;}
unsigned int getRightCount(){return rt;}
unsigned int getLeftCount(){return lt;}

unsigned int getRSp(){return Rsp;};
unsigned int getLSp(){return Lsp;};

} /* namespace Wheel */
