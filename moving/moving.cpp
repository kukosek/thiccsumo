#include "moving.h"
#include "mbed.h"
void moving::enableMotors(){
    //some IO to enable your motor drivers or something
}
void moving::disableMotors(){
    //some IO to disable your motor drivers or something
    //i will just do call the init function to stop motors
    moving::initMotorIO();
}

moving::moving(){
    mSpeed=50.0; //this is the default speed (50%)
    mDirection=0;
    mBackwards=False;
    moving::initMotorIO();
    moving::enableMotors(); //enable motors after class initialization
}

void moving::initMotorIO(){
    left1 = new PwmOut(PA_11);
    left2 = new PwmOut(PA_8);
    right1 = new PwmOut(PA_3);
    right2 = new PwmOut(PA_1);
    
    left1->period_ms(2);
    left2->period_ms(2);
    left1->write(0.0f);
    left2->write(0.0f);
    
    right1->period_ms(2);
    right2->period_ms(2);
    right1->write(0.0f);
    right2->write(0.0f);
}

void moving::setMotorIO(){
    if (mBackwards){
        left2->write(0.0f);
        right1->write(0.0f);
        
        left1->write(mSpeed/100);
        
        right2->write(mSpeed/100);
    }else{
        left1->write(0.0f);
        right2->write(0.0f);
        
        left2->write(mSpeed/100);
        
        right1->write(mSpeed/100);
        
    }
}

void moving::setMoveSpeed(uint8_t speed) { //speed is percentual (0-100)
    mSpeed=speed;
    moving::setMotorIO();
}

/*direction, for example: -100 is rotating on z axis to left (both motors are rotating to opossite direction),
-50 only right motor rotates
0 go straight,
+50 only 
100 rotating to the right*/
void moving::setMoveDirection(int8_t direction, bool backwards) { 
    mDirection=direction;
    mBackwards=backwards; 
    moving::setMotorIO();   
}
