# RF433-WiFi

## Overview (why?)
For a few years now I've been using this project with simple wireless wall outlets via MQTT to make them available to control in Home Assistant. These types of wireless remote outlets are really cheap when compared with other "smart" outlets (in 2017, I got a pack of 5 outlets for a little over $20.)

An ESP8266 Arduino device (I use a d1 mini) connects to an MQTT broker through WiFi. The Arduino accepts on/off messages from the broker. The messages are "translated" into coded signals that are sent to a connected 433MHz radio transmitter to turn the outlets on and off. Essentially, the Arduino plus RF transmitter pretends to be the remote for the outlets.

This project is still *very* rudimentary. For example, I've hard-coded the IP of the MQTT broker. Also, I reverse-engineered the codes that work with my particular 433MHz remote plugs. I'm putting this out here to see if anybody's interested in making this actually good. Like updating the ESP8266 web portal to do more than just the WiFi setup. At the very least, the MQTT setup could be on the web portal as well. It would also be nice to handle the remote code snooping through the portal. I have no idea if these remote signals are at all standardized. I snooped the signals from the remotes for my particular wireless outlets and hard-coded those; however, using this project shouldn't require having to modify the source code.

One important note: there is no way for the on/off state of the outlet(s) to be reported back to the Arduino. Home Assistant does an OK job of showing *some* state for the devices, but it can get out of sync (showing off state when on, or vice versa.) You can still turn on an outlet that's already on (or turn off one that's already off) so the sync is easy to fix. This is to say this is really only cosmetic in the Home Assitant interface when the outlets are turned on or off *not* through Home Assistant. Home Assistant keeps track of when *it* turns outlets on or off, but it can't know about other reasons ways the outlets might have been turned on or off.

## Building the software
The arduino project is a PlatformIO, VS Code project. After installing the PlatformIO plug-in in VS Code, you should be able to open the cloned repo folder in VS Code and build it. Then you can upload to the arduino. The PlatformIO INI has a few entries in it for different deployments, including over-the-air deployments (you need to start with USB, though.) Unfortunately, the OTA deployments have hard-coded IP's (hey, I said I wanted to MAKE this good, not that it IS good.) This is something that should be fixed (somehow.)

## Hardware
Here are the devices I'm using. I happened to buy these on Amazon, so linking to Amazon is the easiest way for me to show them here. These are not affliate links.

[Wireless Outlets](https://a.co/d/2EBIMxp) : These exact outlets are "currently unavailable." I got them for $21.48 in 2017. Similar outlets can be found on Amazon (and I'm sure elsewhere). Try searching Amazon for "wireless outlet".

[NodeMCU WeMos D1 Mini](https://a.co/d/5vs8sCT) : This is the ESP8266 module I use. It's nice and small.

[433 MHz Transmitter and Receiver](https://a.co/d/fXiLwbL) : The receiver is only connected and used when snooping the remote codes. I would like to update this so that the receiver is always connected and ready to figure out the signals from the remotes.

Diagrams for how to wire everything up are needed. For now (sorry), pin wiring needs to be inferred from the source code.

## 3D-Printed Case
I designed a 3D-printable case for this thing in Fusion 360. This was back when I was still learning about 3D-printing and Fusion. I've included the Fusion 360 file in this project. It's a binary blob (I don't know of a better way to put a Fusion file into a git repo). The case is two colors, printed as two separate objects that are then joined together. I definitely wouldn't make the case this way today, but I'm including it anyway.