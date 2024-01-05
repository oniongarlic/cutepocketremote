# Blackmagic design Bluetooth / BLE remote for desktops

CutePocketRemote is a desktop application (Windows, Linux, etc) for remote controlling 
Blackmagic Design cameras that support Bluetooth, for example the Pocket Cinema Cameras.

License: GPLv3

## Known to work camera models

* BlackMagick Design Pocket Cinema Camera 6k G2

## Features

Most of the basic features are implemented right now:

* Adjusting ISO, Shutter speed, Aperture. Auto aperture.
* Adjusting White Balance and Tint, with quick presets. Auto whitebalance.
* Recording, Stoping and Capturing still images
* Time code display
* Focusing, slow, fast, auto, "Focus wheel"
* Supports selection from multiple cameras (currently only 1 camera at a time)

## Building

Requires Qt 6.5 or later, Windows or Linux.
For working BLE under Windows, build with MSVC, mingw does not support bluetooth in Qt 6.

## Todo

* Display and editing of metadata
* Perhaps a nicer UI
* Zoom support

## Bugs

* Under Windows the initial information about the camera settings is sometimes not received
