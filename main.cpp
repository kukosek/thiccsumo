#include "mbed.h"
#include <math.h> //floor()
#include "Moving.h"
#include "LineDetection.h"
#include "EnemyDetection.h"
DigitalOut myled(LED1);
Moving robotMove;
LineDetection line;
EnemyDetection enemyDetection;
uint8_t STANDBY=0;
uint8_t INIT=1;
uint8_t SCAN=2;
uint8_t LINE=4;
int state = INIT;
void setState(uint8_t stateToSet){
    if (stateToSet==STANDBY){
        robotMove.disableMotors();
        state=stateToSet;
    }else if (stateToSet==INIT){
        robotMove.enableMotors();
        state=stateToSet;
    }else if (stateToSet==SCAN){
        robotMove.setMoveDirection(0,false);
        robotMove.setMoveSpeed(20);
        state=stateToSet;
    }
}

class LineFoundMoves {
    public:
        bool goingBackwardsSmooth;
        bool rotatingSmooth;
        LineFoundMoves(){
            preciseSpeed=0.0;
            lastAppliedSpeed=0;
            GOINGBACKWARDS=true;
            ROTATING=false;
            moveState=GOINGBACKWARDS;

            //configuration of move style
            backwardsLinePosProportionalK=0.4;
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
        }

        void startMoves(int8_t lineFoundPos){
            startGoingBackwards(lineFoundPos);
        }
        
        bool smoothUpdate(){ //updates and returns true if there is nothing to update
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
        void stopMoves(){
            actionTimeout.detach();
        }
        
        
    private:
        Timeout actionTimeout;
        Timer smoothIncreaseTimer;

        bool moveState; //state variable used in smoothUpdate() 
        bool GOINGBACKWARDS; //like an enum
        bool ROTATING;  //like an enum

        void startGoingBackwards(int8_t lineFoundPos){
            savedLinePos=lineFoundPos;
            goingBackwardsIntendedDirection=floor(-lineFoundPos*backwardsLinePosProportionalK);
            robotMove.setMoveDirection(goingBackwardsIntendedDirection,true);
            if (goingBackwardsSmooth){
                robotMove.setMoveSpeed(0);
                smoothIncreaseTimer.start();
            }else{
                robotMove.setMoveSpeed(goingBackwardsIntendedSpeed);
            }
            actionTimeout.attach(callback(this, &LineFoundMoves::startRotating),goingBackwardsSeconds);
            
        }

        double preciseSpeed;
        uint8_t lastAppliedSpeed;
        
        uint8_t savedLinePos;
        float backwardsLinePosProportionalK;
        float backwardsSpeedIncreasePerDeciSecond;
        uint8_t goingBackwardsIntendedSpeed;
        int8_t goingBackwardsIntendedDirection;
        float goingBackwardsSeconds;
        
        void startRotating(){
            speedAtEndGoingBackwards=preciseSpeed;
            actionTimeout.attach(callback(this, &LineFoundMoves::finishMoves),rotatingSeconds);
            if (rotatingSmooth){
                smoothIncreaseTimer.start();
            }else{
                robotMove.setMoveDirection(rotatingIntendedDirection, false);
                robotMove.setMoveSpeed(rotatingIntendedSpeed);
            }
        }

        double preciseDirection;
        uint8_t lastAppliedDirection;

        double speedAtEndGoingBackwards;
        float rotatingDirectionIncreasePerDeciSecond;
        float rotatingSpeedIncreasePerDeciSecond;
        int8_t rotatingIntendedDirection;
        uint8_t rotatingIntendedSpeed;
        float rotatingSeconds;

        void finishMoves(){
            actionTimeout.detach();
            setState(SCAN);
        }
};

int enemyPos;
bool enemFound = false;
void processEnemyPos(bool enemyFound, int8_t enemyPosition) {
    enemFound = enemyFound;
    if (enemyFound){
        enemyPos = enemyPosition;
    }
}

int main() {
    line.setGroundColor();
    enemyDetection.startDetecting(&processEnemyPos);
    while(1) {
        if (state!=STANDBY){
            if (line.isOnLine()){
                setState(LINE);
            }

            if (state==INIT){
                setState(SCAN);   
            }else if (state==SCAN){
                
            }
        }
    }
}
