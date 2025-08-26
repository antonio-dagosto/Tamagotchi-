Tomodachi – Arduino Virtual Pet
📖 Overview

Tomodachi is a Tamagotchi-style virtual pet built with an Arduino.
It uses an LCD screen, RGB LED, buzzer, button, and RTC (Real-Time Clock) module to simulate a digital pet that can eat, sleep, and react to the environment.

The pet must be fed daily, responds to light/dark cycles for sleep, and can disappear if neglected too long. It features playful animations, tunes, and memory persistence through EEPROM.

🛠️ Hardware Requirements

Arduino Mega 2560 (or compatible board)

LCD1602 display (connected via parallel interface)

DS1307 RTC module (with battery backup)

RGB LED (pins 6–8)

Buzzer (pin 9)

Push button (pin 10, with internal pull-up)

Light sensor (photoresistor) (A0 analog pin)

EEPROM (onboard for feeding time storage)

⚙️ Features

Feeding system

Pet must be fed daily using the button.

If not fed for more than 1 day, buzzer alarm activates.

If not fed for more than 2 days, pet disappears from LCD.

To revive, press the button 5 times.

Sleep & wake cycle

Detects darkness via light sensor.

Pet goes to sleep if dark for >10 seconds.

Plays a sleep tune when dozing off, and a wake tune in the morning.

Animations & display

Displays different faces ((^_^), (-_-), etc.) depending on mood/state.

Shows current time and status messages on the LCD.

Memory persistence

Feeding times saved in EEPROM so data survives resets.

📂 File Structure

Tomodachi.ino – Main Arduino sketch (handles LCD, RTC, feeding, animations, sleep/wake cycle, EEPROM storage, buzzer tunes).

🔧 Pin Connections
Component	Pin
LCD RS	12
LCD E	11
LCD D4–D7	5, 4, 3, 2
RGB LED (R, G, B)	6, 7, 8
Buzzer	9
Button	10
Light Sensor	A0
RTC (DS1307)	SDA/SCL (Wire library)
