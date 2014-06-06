AqLeds
======

AqLeds configuration client, this client is designed to talk to
an aqleds device and setup all the parameters, timing, led profiles,
to make it work.

The application is designed to be portable (Linux, MacOSX and Windows),
built on Qt 4.7.4 and Qt Creator 2.4.1.

The application structure:

comm/aqleds.cpp     contains an API to talk to an aqleds device
comm/protocol.cpp   is the low level protocol that carries commands and 
                    responses from aqleds

comm/updater.cpp    is the firmware updating protocol

aqleds.cpp  	    is the UI controller
main.cpp 	    is the application startup
