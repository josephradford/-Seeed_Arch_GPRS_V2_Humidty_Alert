This project is written for the Arch GPRS V2.

It aims to provide a convenient method of alerting a user of humidity. However, the grove DHT22
sensor could easily be replaced with another Grove sensor with small changes to the code.

Directories:
 * DHT 
	- for accessing the grove
 * DS_1337 
	- interface to the real time clock (which is a separate chip, i.e. not part of the micro)
 * mbed 
	- all mbed libraries
 * SDFileSystem 
	- for accessing the SD card. You need to be careful to import the right one (link to my blog)
 * USBDevice 
	- interface to serial comms over USB (talk to it from a PC)
 * Handlers 
	- this is where almost all of my code lives

Handlers
The handlers have the same structure (although they do not inherit from a common base class, but they should). They have a run function, which is a state machine called from the main while loop in main.cpp. This will run continuous routines such as polling and checking if a request has been raised.

A request might be raised through a common request interface that passes an enum. However this might have to be more specific. Either way, the request is then handled in the state machine.

GroveDht22
 * Takes a measurement from the DHT22 hardware at a set interval
 * On success, sends to measurement handler

SdHandler
 * Initialises and polls the SD card, checks for errors, etc
 * Receives requests for writing a system message to the log, or writing a measurement to CSV

UsbComms
 * Checks to see if there is a connection to a PC
 * Receives requests to send messages to PC
 * Diverts incoming messages from PC to appropriate handlers

MeasurementHandler
 * GroveDht22 sends a measurement to this and it decides what to do with it
 * Stores values for schedules, thresholds, last measurements
 * Decides if a new measurement should be sent over SMS, SD
 * Receives a request for last measurement, state, etc, from either UsbComms or SmsHandler

GprsHandler (WIP)
 * Checks to see if there are any incoming messages, directs them appropriately
 * Gets requests from other handlers to send an SMS
 
 
 
 
To do:
	* Complete basic GprsHandler
    * Set a threshold for humidity (hardcoded to start with) and send messages at this threshold
    * Avoid sending more than x messages per y time (so don't get bombarded with messages)
    * Send messages to a list of numbers, not just one
    * Send messages on a schedule, e.g. send current humidity and temp once a day, so user knows it's OK
    * Make the above three points configurable over USB (threshold, time off, list of numbers, schedule)
    * Respond to certain messages, over SMS, to configure and make status known.
    * Make a base class for handlers to inherit from, so that interface stays consistent
