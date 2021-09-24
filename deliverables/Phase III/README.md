# Phase III
Phase III transferred the project from a breadboard to proto board, enabling a more permanent solution. Aspects of this were completed in conjunction with Phase II, as the hardware design files for Phase II required the dimensions of the PCB from Phase III.

# Proto-board layout
The transfer from breadboard to proto-board went well. There was one minor issue with a pullup resistor that was in the wrong location (see dcd258a), but the rest of the assembly process went smoothly. Two PCBs had to be joined to create enough space for the parts, which is somewhat suboptimal, but so far it has held up well.

U2-Pin-Header was also added to the Bill of Materials as I didn't add the female pin headers that seated the microcontroller at an earlier stage.
**TODO: Add photo**

# Foot pedal
Pianists are already familiar with using foot pedals for controlling the sustain on a piano or keyboard, so I decided to use a footpedal as the control for the loop pedal. After some research, I found an open source design from Adafruit for a fully 3D printable foot pedal that interfaced with a common tactile switch. The design, available here: https://www.thingiverse.com/thing:2829107, prints in three pieces and can be easily glued or screwed together. I chose to screw the switch into the bracket and glue the bracket into the base.

# Bill of Materials
The Bill of Materials and schematic got an update as the foot pedal got added and some miscellaneous parts and connectors were included. The only item not listed on the BOM, as far as I know, is the wire that runs between the foot pedal and the base unit. I'm currently using salvaged wire that I got at no cost, so unless I move this to commercial production, I won't add a source for a 2 conductor cable.

# Assembly
The order of assembly is fairly straightforward:
* Assemble the PCB
* Glue the connector bracket to the connectors. For this I used hot glue for ease of removability if ever needed.
* Insert the captive nuts into the slots on the connector bracket and USB end cap.
* Fit and glue the window into the slot in the enclosure tube.
* Slide the PCB into the enclosure tube, with the connectors at the slightly closed end.
* Install the USB end cap to hold the PCB in place.
* Install the 2 M3x12 screws to hold the connector bracket and USB end cap in place.
**TODO: Add photo**
