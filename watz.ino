// This #include statement was automatically added by the Particle IDE.
#include "flashee-eeprom/flashee-eeprom.h"
using namespace Flashee;
FlashDevice* flash;

// pins
int intled = D7;
int sensor = D0;

// general vars
unsigned long lastSync = millis();
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
char json[64];
char ip[24];

// pulse vars
void pulseInt(void);		    // function for interrupt
float pulseCount = 0;  			// counts power pulses on sensor
unsigned long pulseTotal = 0;   // total power pulses since sketch started

// set to true to debug over serial
bool debug = false;

void setup() {

	// debug over serial
	if (debug) {
		Serial.begin(9600);
		while(!Serial.available()) {
			Serial.println("Press any key to start...");
			delay(2000);
		}
		Serial.println("Booting watz v1.0");

		// grab local IP
		IPAddress nim_ip = WiFi.localIP();
		sprintf(ip, "%d.%d.%d.%d", nim_ip[0], nim_ip[1], nim_ip[2], nim_ip[3]);

		// output IP over serial
		Serial.println(ip);
		Serial.println("--------------------");
	}

	// set led as output
	pinMode(intled, OUTPUT);

	// set sensor as input
	pinMode(sensor, INPUT);

	// turn off core LED
	RGB.control(true);
	RGB.color(0, 0, 0);

	// prepare the flash memory for writing
	flash = Devices::createWearLevelErase();

	// if debugging then set pulse total to zero
	if (debug) {
		flash->write((uint8_t) 0, 1);
	}

	// get pulse total from storage
	flash->read(pulseTotal, 1);

	// attach interrupt to LED pin, when RISING
	attachInterrupt(sensor, pulseInt, RISING);

	// get the correct time
	Particle.syncTime();

	// give time for time sync
	// allow time to flash the core after reset
	delay(15000);

	// turn off wifi
	WiFi.off();
}

void loop() {

	// publish once every 15 minutes
	if (Time.minute() == 00 || Time.minute() == 15 || Time.minute() == 30 || Time.minute() == 45) {
		if (Time.second() == 0) {
			publish();
		}
	}

	// if pulse total equals 1000 then publish a meter count
	if (pulseTotal >= 1000) {
		publishTotal();
	}
}

void pulseInt() {

	// visually indicate the blink has been detected on internal LED D7
	digitalWrite(intled, HIGH);
	delay(100);
	digitalWrite(intled, LOW);

	// increment pulse count
	pulseCount++;

	// increment the pulse total
	pulseTotal++;

	// write the total to storage in case of power problem
	flash->write((uint8_t) pulseTotal, 1);

	// output pulse info
	if (debug) {
		Serial.print("Pulse Count: ");
		Serial.println(pulseCount);
		Serial.print("Pulse Total: ");
		Serial.println(pulseTotal);
	}
}

void publish() {

	// bail if no pulses are recorded
	if (pulseCount == 0) {
		return;
	}

	// show feedback via serial
	if (debug) {
		Serial.println("Publishing...");
	}

	// turn on the wifi and connect
	WiFi.on();
	Particle.connect();

	// wait for wifi to connect (10 second timeout)
	if (waitFor(Particle.connected, 10000)) {

		// slow down or publish event fails
		delay(1000);

		// prepare data
		// source: http://www.reuk.co.uk/Flashing-LED-on-Electricity-Meter.htm
		// 3600 seconds per hour
		float avgInterval = 900 / pulseCount;  // average time between pulses in 15 minute (900 second) interval
		float kw = 3600 / (avgInterval * 1000); // meter is 1000 imp/kWh

		// create json string
		sprintf(json,"{\"kw\": %.3f, \"count\": %.0f}", kw, pulseCount);

		// output data to serial
		if (debug) {
			Serial.println(json);
		}

		// publish event to spark cloud
		bool success;
		success = Particle.publish("watz", json, 60, PRIVATE);

		// we need to check the data has arrived safely
		if (success) {

			// show green led
			// RGB.color(0, 255, 0);
			// delay(100);
			// RGB.color(0, 0, 0);

			// show feedback via serial
			if (debug) {
				Serial.println("Data sent via publish event (watz)");
			}

			// reset pulse count
			pulseCount = 0;

		} else {

			// show red led
			RGB.color(255, 0, 0);
			delay(100);
			RGB.color(0, 0, 0);

			// show error via serial
			if (debug) {
				Serial.println("Error: publish event (watz) failed to send");
			}
		}

		// sync to time server once per day
		if (millis() - lastSync > ONE_DAY_MILLIS) {
			Particle.syncTime();
			lastSync = millis();
		}

		// slow down or publish event fails
		delay(1000);
	}

	// turn off the wifi to save power, leave it on if we are at 1000 total flashes
	if ( pulseTotal <= 999 ) {
		WiFi.off();
	}
}

void publishTotal() {

	// show feedback via serial
	if (debug) {
		Serial.println("Publishing Total...");
	}

	// turn on the wifi
	WiFi.on();
	Particle.connect();

	// wait for wifi to connect (10 second timeout)
	if (waitFor(Particle.connected, 10000)) {

		// slow down or publish event fails
		delay(1000);

		// publish event to spark cloud
		bool success;
		success = Particle.publish("watzup", "{\"meter\": 1}", 60, PRIVATE);

		// we need to check the data has arrived safely
		if (success) {

			// show green led
			// RGB.color(0, 255, 0);
			// delay(100);
			// RGB.color(0, 0, 0);

			// show feedback via serial
			if (debug) {
				Serial.println("Data sent via publish event (watzup)");
			}

			// reset total pulse count
			pulseTotal = 0;
			flash->write((uint8_t) pulseTotal, 1);

		} else {

			// show red led
			RGB.color(255, 0, 0);
			delay(100);
			RGB.color(0, 0, 0);

			// show error via serial
			if (debug) {
				Serial.println("Error: publish event (watzup) failed to send");
			}
		}

		// slow down or publish event fails
		delay(1000);
	}

	// turn off the wifi to save power
	WiFi.off();
}