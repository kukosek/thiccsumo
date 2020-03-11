#include "mbed.h"
#include <math.h> //floor()
#include "Moving.h"
#include "LineDetection.h"
#include "EnemyDetection.h"



DigitalOut myled(LED1);
Moving robotMove;
LineDetection line;
EnemyDetection enemyDetection;
enum states {
    STANDBY,
    INIT,
    AIMING,
    SCAN,
    LINE_ON_LINE;

    NONE
};
enum states interruptedNewState = NONE;

class LineFoundMoves {
    public:
        bool goingBackwardsSmooth;
        bool rotatingSmooth;
        LineFoundMoves();

        void startMoves(int8_t lineFoundPos);
        
        bool smoothUpdate();
        void stopMoves();
        
        
    private:
        Timeout actionTimeout;
        Timer smoothIncreaseTimer;

        bool moveState; //state variable used in smoothUpdate() 
        bool GOINGBACKWARDS; //like an enum
        bool ROTATING;  //like an enum

        void startGoingBackwards(int8_t lineFoundPos);

        double preciseSpeed;
        uint8_t lastAppliedSpeed;
        
        uint8_t savedLinePos;
        float backwardsLinePosProportionalK;
        float backwardsSpeedIncreasePerDeciSecond;
        uint8_t goingBackwardsIntendedSpeed;
        int8_t goingBackwardsIntendedDirection;
        float goingBackwardsSeconds;
        
        void startRotating();

        double preciseDirection;
        uint8_t lastAppliedDirection;

        double speedAtEndGoingBackwards;
        float rotatingDirectionIncreasePerDeciSecond;
        float rotatingSpeedIncreasePerDeciSecond;
        int8_t rotatingIntendedDirection;
        uint8_t rotatingIntendedSpeed;
        float rotatingSeconds;

        enum states stateAfterFinnish; 
        void finishMoves();
};

LineFoundMoves::LineFoundMoves(void){
    preciseSpeed=0.0;
    lastAppliedSpeed=0;
    GOINGBACKWARDS=true;
    ROTATING=false;
    moveState=GOINGBACKWARDS;

    //configuration of move style
    backwardsLinePosProportionalK=0.15;
    goingBackwardsSmooth=false;
    backwardsSpeedIncreasePerDeciSecond=5;
    goingBackwardsIntendedSpeed=20;
    goingBackwardsSeconds=1.0;
    
    rotatingSmooth=false;
    rotatingSpeedIncreasePerDeciSecond=10;
    rotatingIntendedSpeed=20;
    rotatingDirectionIncreasePerDeciSecond=10;
    rotatingIntendedDirection=100;
    rotatingSeconds=1.0;

    stateAfterFinnish = AIMING;
}

void LineFoundMoves::startMoves(int8_t lineFoundPos){
    startGoingBackwards(lineFoundPos);
}

bool LineFoundMoves::smoothUpdate() { //updates and returns true if there is nothing to update
    smoothIncreaseTimer.stop();
    if(moveState == GOINGBACKWARDS){ //returns true if the intended speed has been reached               
        double speedIncrease=rotatingSpeedIncreasePerDeciSecond*(smoothIncreaseTimer.read_us()*100000);
        uint8_t roundedNewSpeed=floor(preciseSpeed+speedIncrease);
        
        if (roundedNewSpeed<=goingBackwardsIntendedSpeed){
            preciseSpeed+=speedIncrease;
            
            if (lastAppliedSpeed!=roundedNewSpeed){ //check if this speed has been arleady applied to the motors
                robotMove.setMoveSpeed(roundedNewSpeed);
                lastAppliedSpeed=roundedNewSpeed;
            }
        }
    }else if(moveState == ROTATING){
        //Smooth speed update
        double speedIncrease= rotatingSpeedIncreasePerDeciSecond*(smoothIncreaseTimer.read_us()*100000);
        if (rotatingIntendedSpeed<speedAtEndGoingBackwards){
            speedIncrease=-speedIncrease;
        }
        uint8_t roundedNewSpeed=floor(preciseSpeed+speedIncrease);
        
        bool intendedSpeedReached;
        if ( //if we didn't reach the intended speed
        (rotatingIntendedSpeed > goingBackwardsIntendedSpeed and roundedNewSpeed<=goingBackwardsIntendedSpeed)
        or
        (rotatingIntendedSpeed < goingBackwardsIntendedSpeed and roundedNewSpeed>=goingBackwardsIntendedSpeed)){
            preciseSpeed+=speedIncrease;
            
            if (lastAppliedSpeed!=roundedNewSpeed){ //check if this speed has been already applied to the motors
                robotMove.setMoveSpeed(roundedNewSpeed);
                lastAppliedSpeed=roundedNewSpeed;
            }
            intendedSpeedReached = false;
        }else{
            intendedSpeedReached = true;
        }


        //Smooth direction update
        double directionIncrease= rotatingDirectionIncreasePerDeciSecond*(smoothIncreaseTimer.read_us()*100000);
        if (rotatingIntendedDirection < goingBackwardsIntendedDirection){
            directionIncrease=-directionIncrease;
        }
        uint8_t roundedNewDirection=floor(preciseDirection+directionIncrease);

        bool intendedDirectionReached;
        if ( // if we didn't reach the intended direction
        (rotatingIntendedDirection > goingBackwardsIntendedDirection and roundedNewDirection<=goingBackwardsIntendedDirection)
        or
        (rotatingIntendedDirection < goingBackwardsIntendedDirection and roundedNewDirection>=goingBackwardsIntendedDirection)){
            preciseDirection+=directionIncrease;
            
            if (lastAppliedDirection!= roundedNewDirection){ //check if this direction has been arleady applied to the motors
                robotMove.setMoveDirection(roundedNewDirection, true);
                lastAppliedDirection=roundedNewDirection;
            }
            intendedDirectionReached = false;
        }else{
            intendedDirectionReached = true;
        }

        if(intendedSpeedReached and intendedDirectionReached){
            return true;
        }
    }
    smoothIncreaseTimer.reset();
    smoothIncreaseTimer.start();
    return false;
}

void LineFoundMoves::stopMoves() {
    actionTimeout.detach();
}

//private class funcs
void LineFoundMoves::startGoingBackwards(int8_t lineFoundPos){
    savedLinePos=lineFoundPos;
    goingBackwardsIntendedDirection=floor(-lineFoundPos*backwardsLinePosProportionalK);
    printf("%d\r\n",goingBackwardsIntendedDirection);
    robotMove.setMoveDirection(goingBackwardsIntendedDirection,true);
    if (goingBackwardsSmooth){
        robotMove.setMoveSpeed(0);
        smoothIncreaseTimer.start();
    }else{
        robotMove.setMoveSpeed(goingBackwardsIntendedSpeed);
    }
    actionTimeout.attach(callback(this, &LineFoundMoves::startRotating),goingBackwardsSeconds);
    
}

void LineFoundMoves::startRotating(){
    speedAtEndGoingBackwards=preciseSpeed;
    actionTimeout.attach(callback(this, &LineFoundMoves::finishMoves),rotatingSeconds);
    if (rotatingSmooth){
        smoothIncreaseTimer.start();
    }else{
        robotMove.setMoveDirection(rotatingIntendedDirection, false);
        robotMove.setMoveSpeed(rotatingIntendedSpeed);
    }
}

void LineFoundMoves::finishMoves(){
    actionTimeout.detach();
    interruptedNewState = stateAfterFinnish;
}

LineFoundMoves lineFoundMoves;

/*Replacing button - later attaching a function
that changes the state to STANDBY/INIT when
pressing a key on PC */
RawSerial pc(USBTX, USBRX);

enum states state;


/* state AIMING - PID runtime and config variables */
//PID config
#define AIMING_Kp 0.2
#define AIMING_Kd 0
#define AIMING_IntendedEnemyPos 0
#define AIMING_RotationDirection 100
//PID runtime vars
int8_t lastError = 0;

void processEnemyPos(bool enemyFound, int8_t enemyPosition) {
    switch (state) {
        case AIMING:{
            int8_t error = enemyPosition-AIMING_IntendedEnemyPos;
            if (enemyPosition>0) {
                robotMove.setMoveDirection(AIMING_RotationDirection, false);
            }else if(enemyPosition<0) {
                robotMove.setMoveDirection(-AIMING_RotationDirection, false);
            }
            uint8_t result = (abs(error)*AIMING_Kp) + abs(error-lastError)*AIMING_Kd; 
        
            printf("%d --> %d\r\n",enemyPosition, result);
            if (result<=100){
                robotMove.setMoveSpeed(result);
            }else if (result>100) {
                robotMove.setMoveSpeed(100);
            }
            lastError = error;
            break;
        }
    }
}

void setState(enum states stateToSet){
    switch(stateToSet) {
        case STANDBY:{
            robotMove.disableMotors();
            
            enemyDetection.stopDetecting();
            state=stateToSet;
            break;
        }
        case INIT: {
            line.setGroundColor();
            robotMove.enableMotors();
            enemyDetection.startDetecting(&processEnemyPos);
            state=stateToSet;
            break;
        }
        case LINE_AFTER: {
            state=stateToSet;
            lineFoundMoves.startMoves(line.getLinePos());
            break;
        }
        case SCAN: {
            robotMove.setMoveDirection(0,false);
            robotMove.setMoveSpeed(20);
            state=stateToSet;
            break;
        }
        default: {
            state=stateToSet;
        }
    }
}



void button(){
    pc.putc(pc.getc());
    if (state == STANDBY){ 
        interruptedNewState = INIT;
    }else{
        interruptedNewState = STANDBY;
    }
}

int main() {
    printf("Main start\r\n");

    pc.attach(&button);

    
    setState(INIT);
    while(1) {
        if (interruptedNewState != NONE) {
            setState(interruptedNewState);
            interruptedNewState = NONE;
        }

        if (state != STANDBY){
            if (line.isOnLine() and state != LINE_AFTER){
                setState(LINE_AFTER);
            }
            switch (state) {
                case INIT: {
                    setState(AIMING);
                    break;
                }
                case LINE_AFTER:{
                    
                    break;
                }  
                case SCAN:{
                    break;
                }
                default: {
                    
                }
            }
        }
    }
}
