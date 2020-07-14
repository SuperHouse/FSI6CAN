#!/bin/sh
/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avrdude \
		-C/Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf \
		-p m32m1 -P usb -c usbasp \
		-e
#say done
