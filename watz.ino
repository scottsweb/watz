#include "PowerShield/PowerShield.h"

// control when particle connects to cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// enable retianed memory
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

// pins
int intled = D7;
int sensor = WKP;
int keepAwake = D0;

// general vars
char json[64];
char ip[24];

// timings
#define sleepTime 150 // 2.5 minutes before waking - during low power usage we force a check
#define updatePeriod 600 // update every 10 minutes

// function for interrupt
void pulse(void); 

// retained pulse vars
retained bool firstRun = true; // is this the first run?
retained float pulseCount = 0; // counts power pulses on sensor
retained unsigned long meterKW = 0; // counts towards a kwh 
retained int lastUpdate = 0; // set last update time to ensure a sync at start
retained int expectedWake = 0; // expected time to wake

// initiate power monitor
PowerShield batteryMonitor;

void setup() {
    
    // turn off core LED
    RGB.control(true);
    RGB.color(0, 0, 0);

    // set sensor as input
    pinMode(sensor, INPUT);
    
    // attach interrupt to sensor pin, when RISING
    attachInterrupt(sensor, pulse, RISING);
    
    // calculate if woken by time or wkp pin (this is wkp pin)
    if ((expectedWake - Time.now()) > 2) {

        // increment pulse count
        pulseCount += 1;
        
        // increment the kw total
        meterKW += 1;
    }
}

void loop() {
    
    if (firstRun) {
        
        // connect
        Particle.connect();

         // wait for wifi to connect (10 second timeout)
        if (waitFor(Particle.connected, 10000)) {
            
            // notify connection / boot
            RGB.color(0, 255, 0);

            // sync time with cloud on first run
            Particle.syncTime();
            
            // allow 5 seconds for sync of time
            delay(5000);
            
            // set a last update time to now
            lastUpdate = Time.now();
            
            // lights off
            RGB.color(0, 0, 0);
        }
        
        // run no more
        firstRun = false;
    }
    
    // publish once every 15 minutes(ish)
    if (Time.now() >= (lastUpdate + updatePeriod)) {
        publish();
    }
    
    // if pulse total equals 1000 then publish a meter count
    if (meterKW >= 1000) {
        publishTotal();
    }
    
    // deep sleep - wake up every 2.5 mins to keep data consistent if low power usage (no interrupts)
    // add a reed or hardward switch here for keeping awake - make flashing easier. Use D0
    // && (digitalRead(keepAwake) == HIGH)
    expectedWake = (Time.now() + sleepTime);
    System.sleep(SLEEP_MODE_DEEP, sleepTime);
}

void pulse() {
    
    // count pulses while booted
    // increment pulse count
    pulseCount += 1;
        
    // increment the kw total
    meterKW += 1;
}

void publish() {

    // connect
    Particle.connect();

    // wait for wifi to connect (10 second timeout)
    if (waitFor(Particle.connected, 10000)) {
        
        // starts the I2C bus
        batteryMonitor.begin(); 
        
        // sets up the fuel gauge
        batteryMonitor.quickStart();

        // sync time
        Particle.syncTime();
    
        // slow down or publish event fails
        delay(2000);
        
        // notify send
        // RGB.color(0, 0, 255);

        // prepare data
        // source: http://www.reuk.co.uk/Flashing-LED-on-Electricity-Meter.htm
        // 3600 seconds per hour
        float avgInterval = (Time.now() - lastUpdate) / pulseCount;  // average time between pulses since last update (seconds)
        float kw = 3600 / (avgInterval * 1000); // meter is 1000 imp/kWh
        
        // create json string
        sprintf(json,"{\"kw\": %.3f, \"count\": %.0f}", kw, pulseCount);

        // publish event to spark cloud
        bool success;
        success = Particle.publish("watz", json, 60, PRIVATE);
            
        // we need to check the data has arrived safely
        if (success) {
                
            // reset pulse count
            pulseCount = 0;
            
            // set last update time
            lastUpdate = Time.now();
        }
        
        // publish power details
        float cellVoltage = batteryMonitor.getVCell();
        float stateOfCharge = batteryMonitor.getSoC();
        sprintf(json,"{\"v\": %.2f, \"charge\": %.2f}", cellVoltage, stateOfCharge);
        Particle.publish("watzbatt", json, 60, PRIVATE);

        // slow down or publish event fails
        delay(2000);
    }
}

void publishTotal() {
    
    // connect
    Particle.connect();

    // wait for wifi to connect (10 second timeout)
    if (waitFor(Particle.connected, 10000)) {
        
        // sync time
        Particle.syncTime();
        
        // slow down or publish event fails
        delay(2000);

        // publish event to spark cloud
        bool success;
        success = Particle.publish("watzup", "{\"meter\": 1}", 60, PRIVATE);
            
        // we need to check the data has arrived safely
        if (success) {
    
            // reset total pulse count
            meterKW = 0;
        }

        // slow down or publish event fails
        delay(2000);
    }
}
