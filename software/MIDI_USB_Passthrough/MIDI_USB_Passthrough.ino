/**
 * MIDI USB Passthrough
 * Lukas Severinghaus
 * October 15 2021
 * 
 * Forwards MIDI messages from device to computer and computer to device.
 */
#define TYPE_NOTE_ON 0x90
#define TYPE_NOTE_OFF 0x80
#define TYPE_CONTROL_MODE 0xB0 
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial7, MIDI);

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
}
void loop() {
  readMIDIHardware();

  readUSBMIDI();
}

/**
 * Checks for incoming MIDI messages from the hardware interface, only allows processing note on, note off, and control mode messages.
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
    }
  }
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
