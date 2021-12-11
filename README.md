# CrashPod: a bicycle helmet with an integrated crash detection and alert system
Project by Zachary Englhardt

Prepared for Comp_Eng 495 Wearable and Mobile Computing at Northwestern University (Fall 2021)

## Summary:
Cycling is both an excellent form of exercise and an environmentally-friendly mode of transportation. Unfortunately, a serious bicycle crash can leave riders incapacitated and unable to call for help. The CrashPod integrates additional safety features into the traditional bicycle helmet, adding an accelerometer to detect high-impact crash events and GPS connectivity to keep track of the rider's location. An on-board cellular radio then automatically sends a SMS text to an emergency contact with a Google Maps link to the crash site. The system is powered by solar cells coupled with a rechargeable battery, enabling the rider to add the CrashPod to their daily cycling kit without requiring nightly charging or battery replacements.

## Visuals:
![C6BFD13C-1EC8-416F-9048-90A719B6D07A_1_105_c](https://user-images.githubusercontent.com/39633184/145655452-df967cb7-c651-4bac-8a55-3746430d3016.jpeg)
![0F308E7D-FD70-4FE4-9206-8C661D549FB7_1_105_c](https://user-images.githubusercontent.com/39633184/145655469-c6ef9d2a-7564-489c-af41-37fd077d3cd4.jpeg)
![CrashPod Demo](https://user-images.githubusercontent.com/39633184/145655501-79357b8e-c386-4670-94ed-5e7fc7536c58.jpg)

## Components: 

| Component                                   | Description | Link | 
| -----------  | ---------------- | ----------- |
| Metro M0 Express                          | Microcontroller| https://www.adafruit.com/product/3505      |
| SARA-R4 LTE Breakout                        | Cell connectivity | https://www.sparkfun.com/products/14997 |
| SAM-M8Q GPS Breakout                        | GPS module | https://www.sparkfun.com/products/15210 |
| MPU6050 | Gyro + Accel                      | https://www.sparkfun.com/products/11028 |
| Rocker switch | Power on/off                | https://www.sparkfun.com/products/8837 |
| Speaker | Alert when crash detected         | https://www.sparkfun.com/products/15350 |
| LED Button | Cancel text alert after crash  | https://www.sparkfun.com/products/15350 |
| Lithium-Ion Battery | rechargable battery   | https://www.sparkfun.com/products/13854 |
| Quiic cable | saves some soldering for i2c  | https://www.sparkfun.com/products/17259 |
| Solar Panels (x2) | energy harvesting       | https://www.sparkfun.com/products/16356 |
| Solar battery charger                       | charges LiPo battery from solar panels | https://www.dfrobot.com/product-1781.html |
| Miscellanous transistor | Drive the speaker off higher battery voltage | parts bin |

## Implementation
#### Hardware
Hardware assembly was dictated mostly by the form-factor of the specific bike helmet I used to make the prototype. You may need to use diffrent pins dpending on the shape of the dev boards and helmet. 
I chose to connect the LTE modem to the microcontroller via hardware serial (Pins 0,1). The GPS and accelerometer were connected via Quiic connectors. The Metro M0 does not have a Quiic socket, so I broke out one of the cables and soldered it to the corresponding i2c pins (since Quiic is just i2c over a specific cable shape). The Quiic cable provides power to both of these components from the Metro M0 as well. 

The SARA-R4 module and the solar power charger both have the capability to charge LiPo batteries. I chose to connect the battery to the solar power charger, and power the SARA-R4 and Metro M0 via the regulated 3.3v output of the solar charger. This power line is hard-wired to the power switch in the back of the helmet. Thus, all the sensors and radios are disabled when the power switch is off. 

The LED button is connected to a digital output pin to drive the LED and a digital input pin with INPUT_PULLUP to read the button state. 

The speaker is controlled via a transistor. The speaker is driven with the full battery voltage through a transistor, making it much louder than connecting through a PWM pin. 

Components were assembled and the code validated on a breadboard before soldering everything together and fitting it inside the helmet. I had to hollow out portions of the back of the helmet with a razor blade to make room to slot in some of the larger boards and make room for the button, speaker, and power switch. Hot glue was used liberally to hold everything together. 

### Software
The software consists of a single Arduino sketch with three major library dependencies. The raw code for these libraries are not included in this repo - you will have to add them manually through the arduino IDE library manager. Links to the libraries are below: 

SARA-R4 Library: https://github.com/sparkfun/SparkFun_LTE_Shield_Arduino_Library
GPS Library: https://github.com/sparkfun/SparkFun_u-blox_GNSS_Arduino_Library
MPU_6050 Library: https://github.com/jarzebski/Arduino-MPU6050

The code progresses through two stages. On power-on, the Setup function attempts to connect the LTE modem to a network. I used the Hologram IoT sim provided with the device. Once a connection is established (this can take up to 3 minutes) the speaker plays two tones. Then the device enters the main loop. The GPS module is polled every 5 seconds to update the rider location. There are two interrupts of note - first is the fall detection interrupt, connected to the interrupt pin of the MPU6050. When this is triggered, a flag variable is set causing the system to enter the "alert" stage. 

In the alert stage, the speaker plays a constant tone on the speaker and the LED button is illuminated. The interrupt flag variable for the button press is checked. If the button is pressed, the alert is cancelled and we will assume the rider is ok. If the button is not pressed within 30 seconds, we proceed to send a text. 

When sending a text, we include the hard-coded name and base portion of a google maps link. We then fill in the latitude and longitude from the last recorded gps location into the appropriate portions of the link so it generates a google maps pin at the location when opened. The SARA-R4 module sends the text. 

Once the text is sent, we enter a second alert stage where the speaker and button LED pulse on and off. This stays on until the battery runs out or the device is powered off. 

## Lessons Learned / Post-Mortem

I was originally planning on using the Sparkfun Artemis board for this project, since it has on-board BLE. This would have enabled configuring emergency contacts via Bluetooth. Unfortunately, it seems that the Serial/Uart drivers have significant issues to date, which prevented me from establishing communication with the LTE modem. The Arduino libraries are still in beta, so I would love to give it another try with the Artemis platform once there is a stable release. It took me quite a while to identify this issue as it only occurred when trying to send long strings via serial (i.e. sending a text). The shorter configuration and status commands for the LTE module still worked. Fortunately I happened to have a Metro M0 laying around from a previous project, so I subbed that in for this prototype. 

My inclination when working on these projects is to try and build everything all at once and hope it works. However, I was aware it would be nearly impossible to fix wiring or update firmware once everything was hot-glued inside a bike helmet. This forced me to be more disciplined in my testing as I went about assembling the device, basic "hello world" code with each component after installing it to make sure all connections were sound before continuing. 

I was lucky to dodge many of the parts shortages many of my classmates experienced. I made a point to part out the entire system on paper (every wire, connector, etc.) before ordering to avoid a last-minute scramble waiting for parts in shipping. Even so, it took about 4 weeks for everything to arrive despite ~1 week shipping estimates. 

There are a few future improvements I would like to make to the CrashPod. First, I want to add Bluetooth for emergency contact configuration. Requiring a firmware update to change the string for the emergency contact phone number is hardly user-friendly! The entire system is also quite a bit larger than it needs to be. Most of the pins on the dev board and breakout boards are unused, so making a custom PCB for all of the components would reduce the overall size dramatically. This might make it possible to mount the entire system on top of the helmet without cutting out portions to fit the boards. Sticking boards inside the foam of the helmet surely voids any sort of crash safety rating of the helmet, so this is required if I want to make a prototype I can safely use every day. 


