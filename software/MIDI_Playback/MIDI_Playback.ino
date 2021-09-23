/**
 * MIDI Playback
 * Lukas Severinghaus
 * September 9, 2021
 * 
 * Listens for MIDI events then plays them back after 1000 are recorded or the instrument has been quiet for 1 second.
 */
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

typedef struct{
  int time;
  int note;
  int velocity;
  int channel;
} MIDI_Event;

MIDI_Event events[1001];
int current_index = 0;
long start_time = 0;
bool reading = true;
void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  Serial.println("MIDI Playback");
}
bool start_reading = true;
long last_touch_time = 0;
void loop() {
  if(MIDI.read() && reading){
    byte type = MIDI.getType(); //Get the type of the message
    if(type == midi::NoteOn || type == midi::NoteOff){
      start_reading = false;
      last_touch_time = millis();
      int note = MIDI.getData1();
      int velocity = MIDI.getData2();
      int channel = MIDI.getChannel();
      Serial.println(String("Got note event: channel: ") + channel + ", note: " + note + ", velocity: " + velocity);
      MIDI_Event new_event;

      if(current_index == 0){
        start_time = millis();
        new_event.time = 0;
      }else{
        new_event.time = millis() - start_time;
      }
      
      new_event.note = note;
      new_event.velocity = velocity;
      new_event.channel = channel;

      
      events[current_index] = new_event;

      
      current_index +=1;
      
      Serial.println(String("Now at index ") + current_index);
    }
  }
  if((millis() - last_touch_time > 1000 || current_index == 1000) && start_reading == false){
    Serial.println("Playing back");
    long playback_start_time = millis();
    long last_note_time = 0;
    for(int i = 0; i < current_index; i++){
      if(i != 0){
      last_note_time = events[i].time - events[i-1].time;
      }
      delay(last_note_time);
      MIDI.sendNoteOn(events[i].note, events[i].velocity, events[i].channel);
    }
    current_index = 0;
    start_reading = true;
    Serial.println("Ready to record again");
  }
}
