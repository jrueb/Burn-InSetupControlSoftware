# CMS Burn-In Setup Control Software

Instructions:

```
Ubuntu 18.04: sudo apt-get install qt5-qmake qt5-default qtcreator liblxi-dev konsole

qmake
make
./burnin
```

When using serial communication the Keithley needs to be set to a baud
rate of 19200, while the Julabo chiller needs to be set to 9600 baud.
Both need their terminator to be set to line feed.


Note: Needs the KDE program konsole in order to run DAQ commands
