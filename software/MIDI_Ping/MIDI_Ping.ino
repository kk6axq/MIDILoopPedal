/**
 * MIDI Ping
 * Lukas Severinghaus
 * September 9, 2021
 * 
 * Listens for note on and note off events on the MIDI input and sends back a randomly offset note of the same velocity and channel.
 * Useful for testing bidirectional MIDI communication.
 * Based on sample code from: https://www.pjrc.com/teensy/td_libs_MIDI.html
 */
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  Serial.println("MIDI Ping");
}

void loop() {
  if(MIDI.read()){
    byte type = MIDI.getType(); //Get the type of the message
    if(type == midi::NoteOn || type == midi::NoteOff){
      int note = MIDI.getData1();
      int velocity = MIDI.getData2();
      int channel = MIDI.getChannel();
      Serial.println(String("Got note event: channel: ") + channel + ", note: " + note + ", velocity: " + velocity);
      int offset = random(10);
      if(type == midi::NoteOn){
        MIDI.sendNoteOn(note+offset, velocity, channel);
      }else{
        MIDI.sendNoteOff(note+offset, velocity, channel);
      }
    }
  }
}
