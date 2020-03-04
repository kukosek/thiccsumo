#ifndef UltrasonicArray_H
#define UltrasonicArray_H

#include "mbed.h"
#include <vector>

class UltrasonicArray {
public:
    UltrasonicArray();
    struct ultrasonicPins;
    void setModules(uint8_t numberOfModules, PinName triggers[], PinName echoes[]);
    void startMeasuring(void (*callbackFunc)(vector<float>));
    void singleScan(void (*callbackFunc)(vector<float>));
    void singlePing(uint8_t moduleIndex, void (*callback)(float));
    void attachPingCb(void (*callbackFunc)(float));
    void dettachPingCb();
    void nextScan();
    void stopMeasuring();
    uint8_t mNumberOfModules;
    void setFirstModule(uint8_t moduleIndex);
    bool nextModuleStartToRight;
    float speedOfSound;
private:
    struct ultrasonicModule;
    vector<ultrasonicModule> ultrasonicModules;
    void resetScanVars();
    void nextPing();
    void (*mCallbackFunc)(vector<float>);
    bool attachedPingCb;
    void (*mPingCbFunc)(float);
    bool singlePingActive;
    void (*mSinglePingCbFunc)(float);
    uint8_t firstModuleToPing;


    uint8_t getNextModuleIndexToPing();
    //getNextModuleIndexToPing runtime vars
    uint8_t distFromStartModule;
    bool nextModuleRight;


    Timeout triggerTiming;
    uint8_t moduleToPing;
    bool scanning; //sets to true at startMeasuring() and singleScan()
    bool repeatedlyScanning; //sets to true when calling startMeasuring()
    void startTrigger();
    void triggerLow();

    Timer echoTimer;
    void echoRise();
    vector<float> currentScanDistances;
    Timeout waitAfterEchoFall;
    void echoFall();
};
#endif