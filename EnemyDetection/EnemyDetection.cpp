#include "EnemyDetection.h"
#include "mbed.h"
#include "UltrasonicArray.h"

#define NUMBER_OF_MODULES 3

#define CENTER_DIST_INDEX 1
#define RIGHT_DIST_DIR 40

#define DIST_GREATERTHAN_CM 0
#define DIST_LESSTHAN_CM 10

void (*EnemyDetection::mEnemyPosCallback)(bool, int8_t);

EnemyDetection::EnemyDetection(){
    ultraArray = new UltrasonicArray();
    
    PinName triggerPins[NUMBER_OF_MODULES] = {PB_1, PB_0, PA_4};
    PinName echoPins[NUMBER_OF_MODULES] = {PB_4, PA_12, PA_5};
    ultraArray->setModules(NUMBER_OF_MODULES,triggerPins, echoPins);
    ultraArray->setFirstModule(1);
}

void EnemyDetection::distCallback(vector<float> distances) {
    bool enemyDetected = false;
    float directionsSum = 0;
    uint8_t numberOfDirections = 0;
    for(uint8_t i=0; i<NUMBER_OF_MODULES; i++) { //for every distance
        
        if (distances[i] > DIST_GREATERTHAN_CM and distances[i] < DIST_LESSTHAN_CM){
            enemyDetected = true;
            numberOfDirections++;
            directionsSum += (i-CENTER_DIST_INDEX) / (NUMBER_OF_MODULES-1-CENTER_DIST_INDEX) * RIGHT_DIST_DIR;
        }
    }
    if (enemyDetected) {
        (*EnemyDetection::mEnemyPosCallback)(enemyDetected, int8_t(directionsSum/numberOfDirections)); //return the average direction
    }else{
        (*EnemyDetection::mEnemyPosCallback)(enemyDetected, 0);
    }
}

void EnemyDetection::startDetecting(void (*callbackFunc)(bool, int8_t)){
    ultraArray->startMeasuring(&EnemyDetection::distCallback);
    mEnemyPosCallback = callbackFunc;
}

void EnemyDetection::stopDetecting(){
    ultraArray->stopMeasuring();
}