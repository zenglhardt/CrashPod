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
| -----------  | ---------------- |                    | ----------- |
| Metro M0 Express                            | Microcontroller| https://www.adafruit.com/product/3505      |
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


