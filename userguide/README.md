# User Guide for connecting to DG-Labs Coyote box

The bottle will look for DG-Labs Coyote bluetooth powerboxes. If it
finds one it will connect to it and send 5 'breath' waves to both A
and B channels at level '150' (defined in coyote.h, usually 2000
max). Then every breath sends a 'breath' wave to channels A and B. You
can alter the level up and down using the rocker switches on the box,
these are forgotten when you disconnect.  Holding a rocker switch will
reset the channel to zero and you'll need to disconnect or restart the
bottle to get back to the default.

# User Guide for connecting via bluetooth

You can connect to the device using any bluetooth serial (UART) application, but the easiest is the phone app ["Bluefruit Connect"](https://learn.adafruit.com/bluefruit-le-connect) from Adafruit.

## Setting the bottle number

Pick a bottle number between 1 and 19. Use an odd number (even numbers are reseved for bottles that act differently). To set the bottle ID to 5:

Select UART mode and type <pre>!S05</pre>

The bottle will respond "ok" and then reboot.  The bottle number is used when you have more than one bottle in use for proximity and also for the bluetooth ID.

## Display current bottle number

In UART mode type <pre>!H</pre>

## Set the rest colour

* In the app select "Controller" "Control Pad" and hit button 1.

* Go back and select "Colour Picker" and select the colour you want to use.

## Set the blow colour

* In the app select "Controller" "Control Pad" and hit button 2.

* Go back and select "Colour Picker" and select the colour you want to use.

## Go back to default of all colours

In UART mode type <pre>!B3</pre> or in the app select "Controller" "Control Pad" and hit button 3

## Reboot device

In UART mode type <pre>!B4</pre> or in the app select "Controller" "Control Pad" and hit button 4

## Debug mode: breath sensor

In UART mode type <pre>!D1</pre>.  You will start to receive the breath sensor data.  Go back and select "Plotter" to get a live graph of readings from the breath sensor. One line is the current value, the other the moving average. Each breath should cause a significant negative peak

## Debug mode: proximity sensor

In UART mode type <pre>!D2</pre>.  You will start to receive the proximity sensor data of any other bottles in range.  Go back and select "Plotter" to get a live graph of proximity (RSSI).