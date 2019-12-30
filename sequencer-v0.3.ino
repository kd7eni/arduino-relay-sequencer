/*
 * GHI Transmitter Power-up
 */

/*
 * Copyright (C) 2019, by Dale Edmons.  All rights reserved.
 *
 * License: GNU
 */

/* Testing */
#define TEST
//#undef TEST
#ifdef TEST
# define TESTDELAY 500
#endif	// Test

/* Negative or Postive Logic */
//#define NEG_LOGIC
#undef NEG_LOGIC

const char Version[] = "v0.3h\n";	// First debugged version: "v0.03f"

const long int lclBAUD = 9600;
const long int mSec  = 1; 		// Basic delay() unit of time
const long int Sec = 1000 * mSec;	// Derive seconds
const long int Min = 60 * Sec;		// derive minutes
const long int Hr  = 60 * Min;		// derive hours (causes compiler overflow)
#ifdef TEST
 const long int RelayDelay = (long int)(0.5 * Sec);	// 1/2 second during testing
 const long int SequencerDelay = 2 * Sec;	// 2 seconds during testin
#else
 const long int RelayDelay = 3 * Sec;	// Sequence Delays in milliseconds
 const long int SequencerDelay = 1 * Min;// Delay after Enable occurs
#endif	// Test

#ifdef NEG_LOGIC
 const bool SEQUENCER_ENABLE = LOW;	// Negative logic
 const bool SEQUENCER_DISABLE = HIGH;	// Negative logic
#else	// then use negative logic
 const bool SEQUENCER_ENABLE = HIGH;	// Positive logic
 const bool SEQUENCER_DISABLE = LOW;	// Positive logic
#endif

const int TRUE  = true;
const int FALSE = false;

/*
 * I/O
 */

/*
 * Series:  If more than four relays are required, then connect "SeriesChain" to
 *	"FeedbackIn" on the last unit.  For all other units, connect
 *	"SeriesChain" to the following unit and "FeebackIn" to the previous
 *	unit.  Thus, it will be known when all following units are connected.
 */
const int SER_out = 8;	// Output (Series units, connect to FB_in, pin 9, last or
			// ENA_in, pin 3, for non-last units)
const int FB_in =   9;	// Input  (feedback in)
const int FB_out = 10;	// Output (feedback out, connect to FB_in, pin 9)
const int ENA_out = 2;	// Output (Parallel units, connect to ENA_in, pin 3)
const int ENA_in =  3;	// Input  (sequencer enable)
const int Relay_1 = 7;	// Digital 7 out
const int Relay_2 = 6;	// Digital 6 out
const int Relay_3 = 5;	// Digital 5 out
const int Relay_4 = 4;	// Digital 4 out

// Prototypes
bool setRelays(long int);
bool clearRelays(long int);
void relay_test(void);
void relay_diag(char *msg);	// FIXME: need correct prototype, getting warnings
//void relay_diag(String *msg);

/* Run once on Startup */
void setup()
{
	Serial.begin(lclBAUD); // would certainly prefer faster speed.

	Serial.write("Arduino Booted.  Setting up Shield.\n");
	Serial.write(Version);
	// Initialize for Relay Shield.
	pinMode(ENA_in, INPUT);	// Primary Input
	pinMode(FB_in,  INPUT_PULLUP);	// 

	pinMode(SER_out, OUTPUT);	// Chained unit output
	pinMode(ENA_out, OUTPUT);	// Chained unit output
	pinMode(FB_out,  OUTPUT);	// Chained unit output
	pinMode(Relay_1, OUTPUT);	// Relay Shield
	pinMode(Relay_2, OUTPUT);	// 
	pinMode(Relay_3, OUTPUT);	// 
	pinMode(Relay_4, OUTPUT);	// 
	pinMode(LED_BUILTIN, OUTPUT);	// Visual Feedback

	// Ensure outputs are initially LOW
	digitalWrite(Relay_1, LOW);
	digitalWrite(Relay_2, LOW);
	digitalWrite(Relay_3, LOW);
	digitalWrite(Relay_4, LOW);

	digitalWrite(SER_out, LOW);
	digitalWrite(ENA_out, LOW);

	digitalWrite(LED_BUILTIN, LOW);

//	relay_test();	// Test only

	Serial.write("Setup complete.\n");
}

/*
 * loop:	Relay Power Sequencer
 */
void loop()	// choose which loop
{
	//loop1();
	loop2();
}

void loop1()	// Single Unit only
{
	if (digitalRead(ENA_in) == HIGH) {
		setRelays(RelayDelay);
	} else if (digitalRead(ENA_in) == LOW) {
		clearRelays(RelayDelay);
	} else {
		Serial.write("loop: Error!\n");
	}
}

void loop2()	// Multi-Unit Sequencing
{
	bool a, b, A, B;
	a = digitalRead(ENA_in);	// ENA(ble)_in
	b = digitalRead(FB_in);		// F(eed)B(ack)_in

	A = (a == HIGH);
	B = (b == LOW);

	relay_diag("loop: top\n"); // give details of internal state on Serial Monitor

	if ( A == B ) {	//  0 0 || 1 1 = HOLD
		Serial.write("Relay Hold\n");
		relay_diag("loop: 0 0 / 1 1 HOLD\n");
	} else if ( !A && B ) {	// 0 1 = clear relays
		Serial.write("Clearing Relays\n");
		clearRelays(RelayDelay);
		relay_diag("loop: 0 1 Clear\n");
	} else if ( A && !B ) {	// 1 0 = set relays
		Serial.write("Setting Relays\n");
		setRelays(RelayDelay);
		relay_diag("loop: 1 0 Set\n");
	} else {
		Serial.write("loop: Processing Error!\n");
	}
}

bool setRelays(long int thisDelay)
{
	int i;

	// Yes, the shield is built backwards...
	for (i=7; i>=4; i--)  {	// Relay 1=7...Relay 4=4
		delay(thisDelay);
		digitalWrite(i, HIGH);
	}

	digitalWrite(SER_out, HIGH);	// NOTE: SER_out is directly wired to
		// pin 9, FB_in, on the last unit.
	// Wait for signal to propogate....
	while(digitalRead(FB_in) == LOW) ;	// NULL, do nothing

	digitalWrite(FB_out, HIGH);	// Always last in both cases
}

bool clearRelays(long int thisDelay)
{
	int i, j;

	// Reverse order from setRelay()...

	digitalWrite(SER_out, LOW);
	// Wait for signal to propogate....
	j=0;
	while(digitalRead(FB_in) == HIGH) ;	// NULL, do nothing

	for (i=4; i<=7; i++)  {	// Relay 1=7...Relay 4=4
		delay(thisDelay);
		digitalWrite(i, LOW);
	}

	digitalWrite(FB_out, LOW);	// Always last in both cases
}


/*
 * relay_test()	Power-up test to verify correct function of the relays and circuits.
 *	Use only during testing.
 */
void relay_test(void)
{
	/* FIXME: add pushbutton to sequence up/down manually. */

	Serial.write("relay_test: begin\n");

	Serial.write("relay_test: Setting relays 1-4\n");
	setRelays(TESTDELAY);
	Serial.write("relay_test: Clearing relays 4-1\n");
	clearRelays(TESTDELAY);
	Serial.write("relay_test: done\n");

// HALT	(uncomment to enable)
	Serial.write("relay_test: looping.  Reset to re-test.\n");
	while(1) ;
}

void relay_diag(char *msg)
{
	Serial.write(msg);
	if(digitalRead(ENA_in) == HIGH) {	// ENA_in
		Serial.write("ENA_in: HIGH\n");
	} else if(digitalRead(ENA_in) == LOW) {
		Serial.write("ENA_in: LOW\n");
	} else if(digitalRead(ENA_out) == HIGH) {// ENA_out
		Serial.write("ENA_out: HIGH\n");
	} else if(digitalRead(ENA_out) == LOW) {
		Serial.write("ENA_out: LOW\n");
	} else if(digitalRead(FB_in) == HIGH) {	// FB_in
		Serial.write("FB_in: HIGH\n");
	} else if(digitalRead(FB_in) == LOW) {
		Serial.write("FB_: LOW\n");
	} else if(digitalRead(FB_out) == HIGH) {// FB_out
		Serial.write("FB_out: HIGH\n");
	} else if(digitalRead(FB_out) == LOW) {
		Serial.write("FB_out: LOW\n");
	}
}
