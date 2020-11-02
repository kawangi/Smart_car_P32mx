
#ifndef WHEEL_H_
#define WHEEL_H_

namespace Wheel {
enum {STOP,FORWARD,BACKWARD,ROTATE_LEFT,ROTATE_RIGHT };
void service(void);
void setMode(int action);
void setSpeed(int s,int track);
void setRCount(int Rcount);
void setLCount(int Lcount);

void Move_Left(void);
void Move_Right(void);
void Move_Front(int Rsp,int Lsp);
void Move_Back(int Rsp,int Lsp);
void Move_Stop(void);

unsigned int getCount();
unsigned int getRightCount();
unsigned int getLeftCount();
unsigned int getRSp();
unsigned int getLSp();

} /* namespace Wheel */

#endif /* WHEEL_H_ */
