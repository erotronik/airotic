# Airotic

The next generation of bubbler bottles, with RGB lighting and bluetooth.

## Features

* Borosilicate glass bottle, easy to clean, harder to break
* Customizable RGB lighting which works with or without liquid
* Breath sensor detects each breath and causes the colours to change and fade
* Bluetooth server allows bottles to be customised from your phone
* Proximity sensing lets bottles change colours and modes as they get close to their friends
* Bluetooth client allows other hardware to interact and "do things" based on breaths
* Air input is through vents not a pipe, for aesthetics and safety
* Low cost compared to commercial bottles
* Uses easily sourced parts including bottle, pressure sensor module, Bluetooth Arduino board
* Powered by small USB portable power pack, will run for a day even on the smallest packs
* Can connect to Coyote DG-Labs e-stim box
* Other features and modes to be announced and released
* Open Source software and hardware design

## Background

After discussing how expensive and poorly designed clear bubbler bottles were over a take-away meal, [@toydolly](https://github.com/toydolly) and [@jlatex](https://github.com/jlatex) figured that we could design and make something even better.

Of course it would need little LED lights to make the liquid glow. And bluetooth.  And soon what was a simple "bottle, 3d printed bottle cap, and tube", turned into an Arduino powered, bluetooth, breath detecting, co-operative and interactive project. [Everything is better with Bluetooth](https://youtu.be/0KXoBcQER_0?t=102). 

We really didn't want to make and sell them. So why not build your own? Please do tag and show us if you build one. If you make any improvements to the hardware or software we'd love to see them too, drop an issue or PR here.

## Safety

This bubbler bottle is designed for your own personal use in a situation where you have the ability to disconnect your mask. It is not designed to restrict or block any ability to breathe. The bottle also works just fine without any liquid and will still light up and detect breathing. If you plan to use them at an event please check with an event organiser first.

## Battery Safety

We have designed the bubbler bottle to use an external USB Powerbank. This is deliberate and we did not want to use a LIPO battery. We have personally witnessed several LIPO failures over the years, some while charging, but not always. The last thing you want is your LIPO battery to explode or start venting lithium gasses into your breathing path. 

## Build your own

To make your own bottle you need to buy a few things, 3D print a few things, solder a few things, and use a PC to upload the program.

* [mechanical construction detail and 3D files](mechanical/)
* [electronics construction details](electronics/)
* [programming details](arduino/)
* [user guide](userguide/)
