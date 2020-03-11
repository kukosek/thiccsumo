#include "Moving.h"
#include "mbed.h"

#define minRotatingSpeed 15
void Moving::enableMotors(){
    //some IO to enable your motor drivers or something
}
void Moving::disableMotors(){
    //some IO to disable your motor drivers or something
    //i will just do call the init function to stop motors
    Moving::initMotorIO();
}

Moving::Moving(){
    mSpeed=50.0; //this is the default speed (50%)
    mDirection=0;
    mBackwards=false;
    Moving::initMotorIO();
    Moving::enableMotors(); //enable motors after class initialization
}

void Moving::initMotorIO(){
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

void Moving::setMotorIO(){
    if (mBackwards){
        if (mDirection==0){ //going STRAIGHT BACKWARDS
            left1->write(0.0f);
            right2->write(0.0f);
            left2->write(mSpeed/100);
            right1->write(mSpeed/100);
            
            
        }else if(mDirection>0){ //Turning RIGHT BACKWARDS
            if (mDirection>50){
                left1->write(0.0f);
                right1->write(0.0f);
                
                left2->write(mSpeed/100);
                float pulsewidth= (mDirection*(mSpeed/100))/100;
                if (pulsewidth>=0){
                    right2->write(pulsewidth);
                }else{
                    right2->write(0.0f);
                }
            }else{
                left2->write(0.0f);
                right1->write(0.0f);
                
                right2->write(mSpeed/100);
                
                float pulsewidth=(mSpeed-(mDirection*(mSpeed/50)))/100;
                if (pulsewidth>=0){
                    left1->write(pulsewidth);
                }else{
                    left1->write(0.0f);
                }
            }
            
        
        }else if(mDirection<0){ //turning LEFT BACKWARDS
            
            
            if (mDirection<-50){
                left1->write(0.0f);
                right1->write(0.0f);
                
                right2->write(mSpeed/100);
                float pulsewidth= (abs(mDirection)*(mSpeed/100))/100;
                if (pulsewidth>=0){
                    left2->write(pulsewidth);
                }else{
                    left2->write(0.0f);
                }
            }else{
                left2->write(0.0f);
                right1->write(0.0f);
                
                right2->write(mSpeed/100);
                
                float pulsewidth=(mSpeed-(abs(mDirection)*(mSpeed/50)))/100;
                if (pulsewidth>=0){
                    left1->write(pulsewidth);
                }else{
                    left1->write(0.0f);
                }
            }
        }
        
        
        
        
    }else{ //GOING FORWARD     
        if (mDirection==0){ //going STRAIGHT
            left1->write(0.0f);
            right2->write(0.0f);
            left2->write(mSpeed/100);
            right1->write(mSpeed/100);
            
            
        }else if(mDirection>0){ //Turning RIGHT
            if (mDirection>50){
                left1->write(0.0f);
                right1->write(0.0f);
                left2->write(mSpeed/100);
                float pulsewidth= (mDirection*(mSpeed/100))/100;
                if (pulsewidth>=0){
                    right2->write(pulsewidth);
                }else{
                    right2->write(0.0f);
                }
            }else{
                left1->write(0.0f);
                right2->write(0.0f);
                left2->write(mSpeed/100);
                
                float pulsewidth=(mSpeed-(mDirection*(mSpeed/50)))/100;
                if (pulsewidth>=0){
                    right1->write(pulsewidth);
                }else{
                    right1->write(0.0f);
                }
            }
            
        
        }else if(mDirection<0){ //turning LEFT
            left2->write(mSpeed/100);
            right1->write(mSpeed/100);
            
            if (mDirection<-50){
                left2->write(0.0f);
                right2->write(0.0f);
                
                right1->write(mSpeed/100);
                float pulsewidth= (abs(mDirection)*(mSpeed/100))/100; //100*0.2
                if (pulsewidth>=0){
                    left1->write(pulsewidth);
                }else{
                    left1->write(0.0f);
                }
            }else{
                left1->write(0.0f);
                right2->write(0.0f);
                  
                right1->write(mSpeed/100);
                
                float pulsewidth=(mSpeed-(abs(mDirection)*(mSpeed/50)))/100; //TODO itw bad
                if (pulsewidth>=0){
                    left2->write(pulsewidth);
                }else{
                    left2->write(0.0f);
                }
            }
        }
    }
}

void Moving::setMoveSpeed(uint8_t speed) { //speed is percentual (0-100)
    if (speed>0){
        mSpeed= (speed - 0) * (100 - minRotatingSpeed) / (100 - 0) + minRotatingSpeed;
    }else{
        mSpeed = 0;
    }
    Moving::setMotorIO();
}

/*direction, for example: -100 is rotating on z axis to left (both motors are rotating to opossite direction),
-50 only right motor rotates
0 go straight,
+50 only 
100 rotating to the right*/
void Moving::setMoveDirection(int8_t direction, bool backwards) { 
    mDirection=direction;
    mBackwards=backwards; 
    Moving::setMotorIO();   
}
