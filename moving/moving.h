#ifndef moving_H
#define moving_H
 
#include "mbed.h"

//this is class where i define the controls of robot's moves
class moving { 
public:
    moving();
    void setMoveDirection(int8_t direction, bool backwards); //direction: -128 is rotating on z axis to left, 0 go straight, 127 rotating to the right
    void setMoveSpeed(uint8_t speed); //speed is percents ( 0 - 100 )
    void enableMotors();
    void disableMotors();
private:
    //EDIT THESE variables for initMotorIO, setMotorIO
    PwmOut* left1;
    PwmOut* left2;
    PwmOut* right1;
    PwmOut* right2;
    
    
    //local class variables where the entered parameters are saved
    float mSpeed;
    int8_t mDirection;
    bool mBackwards;
    
    void initMotorIO(); //called in the constructor
    void setMotorIO(); //this function gets called when you set move speed or direction
};

#endif