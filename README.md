android_input_bridge
====================

This repository contains tools to be able to control input devices remotely over the network or over an USB OTG port.

How to use it ?
====================

Build aibd for android and aib for linux, then:

On android side:
$ aibd 4242

On linux side:
$ adb forward tcp:4242 tcp:4242
$ aib 4242 /dev/input/by-id/<your_evdev_device>

Note: Actually only mouse is supported, support for keyboard is coming soon.
