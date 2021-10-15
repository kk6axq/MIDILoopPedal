/**
 * Loop Pedal V2
 * Lukas Severinghaus
 * October 15, 2021
 * 
 * Remake of loop pedal code.
 * 
 * Still TODO:
 * * Normalize function names either underscores or camelcase.
 */

#include <MIDI.h>
#include <Bounce.h>
// Configuration
MIDI_CREATE_INSTANCE(HardwareSerial, Serial7, MIDI);
#define RECORD_PIN 19
#define RESET_PIN 18
#define LED_PIN 23

// State machine states
#define STATE_RESET 0
#define STATE_WAITING 1
#define STATE_RECORD 2
#define STATE_PLAYBACK 3
#define STATE_DUBBING 4 //Simultaneous playback and recording

byte state_machine_state = STATE_WAITING;
bool state_transition = true;

bool reset_flag = false;
bool record_flag = false;

// MIDI packet types
#define TYPE_NOTE_ON 0x90
#define TYPE_NOTE_OFF 0x80
#define TYPE_CONTROL_MODE 0xB0 
#define TYPE_INVALID 0x00
// Button handlers
Bounce record_button = Bounce(RECORD_PIN, 20);
Bounce reset_button = Bounce(RESET_PIN, 20);

/**** MIDI Data ****/
typedef struct{
  int time;
  byte type;
  int d1; //Note
  int d2; //Velocity
  int channel;
} MIDI_Event;

MIDI_Event incoming_MIDI_event;
bool has_new_MIDI_event = false;

#include <FastLED.h>
CRGB led[1];

void setup() {
  initializeHardware();
}

void loop() {
  pollHardware(); // Check buttons and USB MIDI buffer

  
  
  switch(state_machine_state){
    case STATE_WAITING:
      state_waiting();
    break;

    case STATE_RECORD:
      state_record();
    break;

    case STATE_PLAYBACK:
      state_playback();
    break;

    case STATE_DUBBING:
      state_dubbing();
    break;
    
    case STATE_RESET:
      handleReset();
    break;
  }
}


/**** State Machine and Process Code ****/

void state_waiting(){
  if(state_transition){
    setLEDColor(250, 250, 250);
    Serial.println("Entering waiting state");
    state_transition = false;
  }
  if(record_flag){
    Serial.println("Record pressed, transitioning state");
    record_flag = false;
    incrementState();
  }
}

// Is this the first time recording?
bool record_first_record = true;
long record_start_time = 0;
long record_stop_time = 0;

long track_duration = 0;
void state_record(){
  if(state_transition){
    state_transition = false;
    setLEDColor(255, 0, 0);
    Serial.println("Start recording");
    if(record_first_record){
      record_start_time = millis();
    }
  }

  if(record_first_record && record_flag){
    record_first_record = false;
    record_flag = false;
    
    record_stop_time = millis();
    track_duration = record_stop_time - record_start_time;
    
    Serial.println(String("Done recording first time, track length is ") + track_duration + " ms");
    incrementState();
  }
}

long playback_start_time = 0;
void state_playback(){
  if(state_transition){
    state_transition = false;
    setLEDColor(0, 255, 0);
    playback_start_time = millis();
    Serial.println("Start playback");
  }
  
  play_event(millis() - playback_start_time);
  
  if(millis() > playback_start_time + track_duration){
    Serial.println("Done with playback, transitioning");
    incrementState();
  }
}

long dubbing_start_time = 0;
void state_dubbing(){
  if(state_transition){
    state_transition = false;
    setLEDColor(0, 0, 255);
    dubbing_start_time = millis();
    Serial.println("Start dubbing");
    incrementDubbingTrack();
  }

  if(has_new_MIDI_event){
    record_event(millis() - dubbing_start_time);
  }
  play_event(millis() - playback_start_time);
  
  if(millis() > dubbing_start_time + track_duration){
    Serial.println("Done with dubbing, transitioning");
    incrementState();
  }
}
/**
 * Called when the reset button is pressed, stops current playback and resets the system to defaults.
 */
void handleReset(){
  Serial.println("Reset triggered.");
  state_machine_state = STATE_WAITING;
  state_transition = true;
  //Set recording to record from new again
  record_first_record = true;
}

/**
 * Increments the state machine to the next state, depending on the current state.
 */
void incrementState(){
  //Serial.print("Was in state: ");
  //Serial.print(state_machine_state);
  switch(state_machine_state){
    case STATE_WAITING:
      state_machine_state = STATE_RECORD;
      break;
    case STATE_RECORD:
      state_machine_state = STATE_PLAYBACK;
      break;
    case STATE_PLAYBACK:
      if(reset_flag){
        state_machine_state = STATE_RESET;
        reset_flag = false;
      }else if(record_flag){
        state_machine_state = STATE_DUBBING;
        record_flag = false;
      }
      break;
    case STATE_DUBBING:
      state_machine_state = STATE_PLAYBACK;
      break;
    default:
      state_machine_state = STATE_RESET;
      break;
  }
  state_transition = true;
  //Serial.print(" now in state: ");
  //Serial.println(state_machine_state);
}

/**** End State Machine and Process Code ****/

/**** Hardware Interface Code ****/

/**
 * Sets up hardware peripherals
 */
void initializeHardware(){
  pinMode(RECORD_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  usbMIDI.begin();
  FastLED.addLeds<1, WS2812B, LED_PIN, GRB>(led, 1);
  FastLED.setBrightness(40);
}

/**
 * Checks button states and sets the appropriate flag
 */
void pollHardware(){
  //Serial.println("Polling hardware");
  //Don't clear the flags, only set them, in case
  //the system can't get to them before the button changes again.
  if(record_button.update()){
    if(record_button.fallingEdge()) record_flag = true; //Serial.println("Record pressed");
  }
  if(reset_button.update()){
    if(reset_button.fallingEdge()) reset_flag = true; //Serial.println("Reset pressed");
  }

  //Check USB MIDI buffer and send any messages that have arrived.
  readUSBMIDI();
  readMIDIHardware();
}

/**
 * Checks for packet in the USB MIDI buffer, immediately sends it if one is present.
 */
void readUSBMIDI(){
  byte type, d1, d2, channel;
  if(usbMIDI.read()){
    type = usbMIDI.getType();
    d1 = usbMIDI.getData1();
    d2 = usbMIDI.getData2();
    channel = usbMIDI.getChannel();
    MIDI.send(type, d1, d2, channel);
    Serial.println(String("Got packet from USB: ") + type + ", " + d1 + ", " + d2 + ", " + channel);
  }
}

/**
 * Checks for incoming MIDI messages from the hardware interface, only allows processing note on, note off, and control mode messages.
 * Sends them over USB and saves the most recent message for processing.
 */
void readMIDIHardware(){
  byte type, d1, d2, channel;
  if(MIDI.read()){
    type = MIDI.getType();
    d1 = MIDI.getData1();
    d2 = MIDI.getData2();
    channel = MIDI.getChannel();
    if (type == TYPE_NOTE_ON || type == TYPE_NOTE_OFF || type == TYPE_CONTROL_MODE){
      Serial.println(String("Got packet from device: ") + type + ", " + d1 + ", " + d2 + ", " + channel);
      //This is a packet that needs to be processed.
      //Send it to the computer.
      sendToComputer(type, d1, d2, channel);

      //Save for processing and set the new message flag.
      incoming_MIDI_event.type = type;
      incoming_MIDI_event.d1 = d1;
      incoming_MIDI_event.d2 = d2;
      incoming_MIDI_event.channel = channel;

      has_new_MIDI_event = true;
    }
  }
}

/**
 * Sends a packet to the computer if it's one of the three valid types.
 */
void sendToComputer(byte type, byte d1, byte d2, byte channel){
  if(type == TYPE_NOTE_ON){
    usbMIDI.sendNoteOn(d1, d2, channel);
  }else if(type == TYPE_NOTE_OFF){
    usbMIDI.sendNoteOff(d1, d2, channel);
  }else if(type == TYPE_CONTROL_MODE){
    usbMIDI.sendControlChange(d1, d2, channel);
  }
}

void setLEDColor(byte r, byte g, byte b){
  led[0] = CRGB(r, g, b);
  FastLED.show();
}
/**** End Hardware Interface Code ****/

/**** Recording and Playback Code ****/

#define MAX_MIDI_TRACKS 10
#define MAX_MIDI_EVENTS 1500
int currentTrack = 0;
int currentTrackIndex = 0;
MIDI_Event recorded_events[MAX_MIDI_TRACKS][MAX_MIDI_EVENTS];
void incrementDubbingTrack(){
  currentTrackIndex = 0;
  if(currentTrack == MAX_MIDI_TRACKS-1){
    Serial.println("No more tracks to record on. Going back to playback");
    incrementState();
  }else{
    currentTrack += 1;
    Serial.println(String("Now recording on track ") + currentTrack); 
  }
}

void play_event(long relative_time){
  
}

void record_event(long relative_time){
  incoming_MIDI_event.time = relative_time;
  recorded_events[currentTrack][currentTrackIndex] = incoming_MIDI_event;
  currentTrackIndex += 1;
}
