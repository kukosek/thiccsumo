#include "UltrasonicArray.h"
#include "mbed.h"
#include <vector>

#define TRIGGERHIGH_TIME_US 10
#define ULTRASONIC_TIMEOUT_US 38000
#define ULTRASONIC_TIMEOUT_TOLERANCE_US 1000
#define ULTRASONIC_WAIT_AFTER_ECHO_US 50000

//constructor without parameters. the modules are set up in setModules()
UltrasonicArray::UltrasonicArray(){
    //configuration
    firstModuleToPing=0;
    nextModuleStartToRight = false;
    speedOfSound = 0.0343; //speed of sound in cm/µs
    //runtime variables, do not edit
    attachedPingCb = false;
    scanning = false;
    repeatedlyScanning = false;
    singlePingActive = false;
}

//used to save the module's trigger and echo
struct UltrasonicArray::ultrasonicModule {
        DigitalOut* trigger;
        InterruptIn* echo;
};

/*struct Ultrasonic Array::ultrasonicPins {
        PinName triggerPin;
        PinName echoPin;
};*/

//define pins of your modules
void UltrasonicArray::setModules(uint8_t numberOfModules, PinName triggers[], PinName echoes[]){
    mNumberOfModules = numberOfModules;
    ultrasonicModules.clear();
    for(uint8_t i=0; i<mNumberOfModules; i++){
        struct ultrasonicModule moduleToPush;
        moduleToPush.trigger = new DigitalOut(triggers[i]);

        //checks if this pin was arleady used. if yes point to the InterruptIn object instead of creating a new InterruptIn
        bool pointToPreviousEcho = false;
        uint8_t pointIndex;
        for(uint8_t j=0; j<i; j++){
            if (echoes[j]==echoes[i]){
                pointToPreviousEcho = true;
                pointIndex = j;
                break;
            }
        }
        if (pointToPreviousEcho){
            moduleToPush.echo = ultrasonicModules[pointIndex].echo; 
        }else{
            moduleToPush.echo = new InterruptIn(echoes[i]);
        }
        
        ultrasonicModules.push_back(moduleToPush);
        ultrasonicModules[i].echo->rise(callback(this, &UltrasonicArray::echoRise));
        ultrasonicModules[i].echo->fall(callback(this, &UltrasonicArray::echoFall));
    }
}

/*private function gets called,
when the cycle of progressive measuring distance with each module finished*/
void UltrasonicArray::resetScanVars(){ 
    distFromStartModule = 0;
    nextModuleRight = nextModuleStartToRight;
    if (scanning and !repeatedlyScanning) {
        scanning = false;
    }
    currentScanDistances.clear();
    currentScanDistances.resize(mNumberOfModules);
}

//sets the num of ultrasonic module that pings first (variable firstModuleToPing)
void UltrasonicArray::setFirstModule(uint8_t moduleIndex){
    firstModuleToPing = moduleIndex;
}

/*private function - Returns the index of module it should ping next.
The sequence it returns depends on variables
mNumberOfModules, firstModuleToPing and nextModuleStartToRight.
example: mNumberOfModules = 9, firstModuleToPing = 4, nextModuleStartToRight = true
         4 5 3 6 2 7 1 8 0 | 4 5 3 6 2 7 1 8 0 | ...
example: mNumberOfModules = 9, firstModuleToPing = 4, nextModuleStartToRight = false
         4 3 5 2 6 1 7 0 9 | 4 3 5 2 6 1 7 0 9 | ...
*/
uint8_t UltrasonicArray::getNextModuleIndexToPing(){
    if (distFromStartModule == 0) {
        distFromStartModule++;
        return firstModuleToPing;
    }else{
        uint8_t moduleIndexToReturn;
        if(nextModuleRight == true){
            if (firstModuleToPing+distFromStartModule < mNumberOfModules){
               moduleIndexToReturn = firstModuleToPing+distFromStartModule;
            }else{
                if(firstModuleToPing-distFromStartModule >=0){
                    moduleIndexToReturn = firstModuleToPing-distFromStartModule;
                    nextModuleRight = !nextModuleRight;
                }else{
                    (*mCallbackFunc)(currentScanDistances);
                    resetScanVars();
                    return firstModuleToPing;
                }
            }
        }else{
            if (firstModuleToPing-distFromStartModule >= 0){
               moduleIndexToReturn = firstModuleToPing-distFromStartModule;
            }else{
                if(firstModuleToPing+distFromStartModule < mNumberOfModules){
                    moduleIndexToReturn = firstModuleToPing+distFromStartModule;
                    nextModuleRight = !nextModuleRight;
                }else{
                    (*mCallbackFunc)(currentScanDistances);
                    resetScanVars();
                    return firstModuleToPing;
                }
            }
        }
        if (nextModuleRight != nextModuleStartToRight){
            distFromStartModule++;
        }
        nextModuleRight = !nextModuleRight;
        return moduleIndexToReturn;
    }
}

/*private function - Sets trigger pin to high
of module with index of variable moduleToPing,
sets an timeout to set it back to low*/
void UltrasonicArray::startTrigger() {
    ultrasonicModules[moduleToPing].trigger->write(1);
    triggerTiming.attach_us(callback(this, &UltrasonicArray::triggerLow), TRIGGERHIGH_TIME_US);
}

//private function - sets trigger pin to low of module with index of variable moduleToPing. used in function startTrigger()
void UltrasonicArray::triggerLow(){
    ultrasonicModules[moduleToPing].trigger->write(0);
}

//private function attached to every interruptin's rise
void UltrasonicArray::echoRise() {
    if (scanning){
        echoTimer.start();
    }
}
//private function attached to every interruptin's fall
void UltrasonicArray::echoFall() {
    if(scanning){
        echoTimer.stop();

        csdSize = currentScanDistances.size();

        int time = echoTimer.read_us();
        echoTimer.reset();  
        if(time >= ULTRASONIC_TIMEOUT_US-ULTRASONIC_TIMEOUT_TOLERANCE_US or
        time <= ULTRASONIC_TIMEOUT_US+ULTRASONIC_TIMEOUT_TOLERANCE_US) {
            float dist = (time/2) * speedOfSound;
            if (singlePingActive){
                singlePingActive = false;
                (*mSinglePingCbFunc)(dist);
            }else{
                currentScanDistances[moduleToPing] = dist;
            }
            if (attachedPingCb){
                (*mPingCbFunc)(dist);
            }
        }

        
        //Wait and then trigger next ping. This is beacause the sound may still be echoing.
        waitAfterEchoFall.attach_us(callback(this, &UltrasonicArray::nextPing), ULTRASONIC_WAIT_AFTER_ECHO_US);
    }

}

//private function - ping with the next ultrasonic module
void UltrasonicArray::nextPing() {
    moduleToPing = getNextModuleIndexToPing();
    startTrigger();
}

//starts repeated scans. the callback gets called when a scan finishes (all sensors measured distances)
void UltrasonicArray::startMeasuring(void (*callbackFunc)(vector<float>)) {
    mCallbackFunc = callbackFunc;

    scanning = true;
    repeatedlyScanning = true;
    resetScanVars();
    nextPing();
}

//stops repeated scans
void UltrasonicArray::stopMeasuring() {
    scanning = false;
    repeatedlyScanning = false;
}

//scan that doesn't repeat (callback will be called only once)
void UltrasonicArray::singleScan(void (*callbackFunc)(vector<float>)) {
    if (!repeatedlyScanning and !singlePingActive) {
        mCallbackFunc = callbackFunc;

        scanning = true;
        repeatedlyScanning = false;
        resetScanVars();
        nextPing();
    }
}

//Ping only with a specified module
void UltrasonicArray::singlePing(uint8_t moduleIndex, void (*callbackFunc)(float)) {
    if (!scanning and !repeatedlyScanning){
        mSinglePingCbFunc = callbackFunc;
        singlePingActive = true;
        moduleToPing = moduleIndex;
        startTrigger();
    }
}

//attach a function that will get called after every ping with each module
void UltrasonicArray::attachPingCb(void (*callbackFunc)(float)) {
    attachedPingCb = true;
    mPingCbFunc = callbackFunc;
}

//stop calling the attached ping callback function
void UltrasonicArray::dettachPingCb() {
    attachedPingCb = false;
}

//Skips pinging with remaining modules in current scan and starts a new scan
void UltrasonicArray::nextScan() {
    if (scanning and repeatedlyScanning) {
        (*mCallbackFunc)(currentScanDistances);
        resetScanVars();
    }
}