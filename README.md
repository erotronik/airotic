# Airotic

This is our design for a LED bluetooth bubbler bottle.  

## Features

* Borosilicate glass bottle, easy to obtain, clean, and hard to break
* Colour LEDs light up the bottle (with or without liquid)
* Breath sensor detects each breath and changes the LEDs to suit
* Bluetooth server allows bottles to be customised and have colours changed
* Bluetooth proximity sensor allows bottles to change colours and modes as they get close to their peers
* Bluetooth client allows other hardware to interact and "do things" based on breaths
* Air input is through vents not a pipe, it both looks better and is safer
* Low cost compared to commercial bottles with minimal part count
* Other features and modes to be announced and released
* Uses easily sourced pressure sensor module and Bluetooth Arduino board
* Powered by small USB portable power pack, will run for tens of hours even on the small packs
* Open Source software and hardware design

## Background

After discussing how expensive clear bubbler bottlers were over a take-away meal, [@toydolly](https://github.com/toydolly) and [@jlatex](https://github.com/jlatex) figured we could 3D design and print something even better. Of course it would need little LED lights to make the liquid glow. And bluetooth.  And soon what was a simple "bottle, bottle cap, and tube", turned into an Arduino powered, bluetooth, breath detecting, co-operative and interactive project. [Everything is better with Bluetooth](https://youtu.be/0KXoBcQER_0?t=102). 

We really don't want to make and sell them. So why not build your own? Please do tag us if you build one, and if you make any improvements to the hardware or software we'd love to see them, drop an issue or PR here.

## Safety

This bubbler bottle is designed for your own personal use in a situation where you have the ability to disconnect your mask from it. It is not designed to restrict or block any ability to breathe. The bottle also works just fine without any liquid and will still light up and detect breathing .

## Build your own

* [mechanical construction detail and 3D files](mechanical/)
* [electronics construction details](electronics/)
* [arduino code](arduino/)
* [user guide](userguide/)
