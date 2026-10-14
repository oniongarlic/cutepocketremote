# Remote control for Blackmagic cameras

CutePocketRemote is a desktop application (Windows, Linux, etc) for remote controlling
Blackmagic Design cameras that support Bluetooth, for example the Pocket Cinema Cameras.

License: GPLv3

## Known to work camera models

* Blackmagick Pocket Cinema Camera 6K G2

Other models are untested, but should work, feedback welcome.

## Currently implemented features

Most of the basic features available over the BT interface are implemented:

* Adjusting ISO, Shutter speed, Aperture. Set Auto aperture.
* Adjusting White Balance and Tint, with quick presets. Set Auto white balance.
* Recording video, Stop recording video
* Toggle to Play mode
* Capturing still images
* Time code display
* Focusing, slow, fast, auto, "Focus wheel". Trigger Auto focus.
* Supports selection from multiple cameras (currently only 1 camera at a time)
* View slate metadata

## Building

Requires Qt 6.6 or later, Windows or Linux.
Note: For BLE under Windows, you need to build with MSVC, mingw does not support bluetooth in Qt 6.

## Todo

* Editing of slate metadata
* Zoom support (Experimental, untested interface available)

## Bugs and known issues

* Pair the camera device with the OS Bluetooth interface first
* Windows does not disconnect connection properly, toggle Bluetooth on/off or disconnect from Bluetooth device list
* Under Windows the initial information about the camera settings is sometimes not received

