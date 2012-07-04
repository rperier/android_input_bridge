android_input_bridge
====================

This repository contains tools to be able to control input devices remotely over the network or over an USB OTG port.

How to use it ?
====================

Build these tools using the Android build system, then :

On android side:
$ aibd 4242

On linux side:

1)° Over the network
$ aib <device_ip> 4242 /dev/input/by-id/<your_evdev_device>

2)° Over USB OTG port
$ adb forward tcp:4242 tcp:4242
$ aib 127.0.0.1 4242 /dev/input/by-id/<your_evdev_device>

Note: Actually only mouse is supported, support for other evdev compatible devices are coming soon.

