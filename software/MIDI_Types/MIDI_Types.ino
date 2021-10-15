/**
 * MIDI Message Analysis
 * Lukas Severinghaus
 * September 27, 2021
 * 
 * Listens for MIDI messages and logs the types of messages received.
 * Use this to detect what messages the piano sends for different inputs.
 */

#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial7, MIDI);

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
}
byte received_types[50];
int current_index = 0;
void loop() {
  if(MIDI.read()){
    byte type = MIDI.getType();
    bool in_array = false;
    for(int i = 0; i < current_index; i++){
      if(received_types[i] == type){
        in_array = true;
      }
    }
    if(!in_array){
      received_types[current_index] = type;
      current_index += 1;
    }
  }

  if(millis() % 100 == 0){
    for(int i = 0; i < current_index; i++){
      Serial.print(int(received_types[i]));
      Serial.print(", ");
    }
    Serial.println();
  }
}
