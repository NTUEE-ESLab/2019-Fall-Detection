# 2019-Fall-Detection

## Motivation
Falls in the elderly have always been a serious medical and social problem. To detect and predict falls, and further more roughly determine what movement type currently is, a hidden Markov model (HMM)-based method using tri-axial accelerations of human body is proposed.
## Implementation
Using BSP_B-L475E-IOT01 library to read acceleration by the STM32L475 MCU. After that, collect data of movement types (standing, walking, runnung, fall, prediect) in order to feed in the HMM models we will train. To make the data cleaner, we calculated the root-mean-square of tri-axial acclerations instead of using the data straightforwardly. After trained, we get sets of a,b,pi parameters for each models. With the parameters, we can run viterbi algorithm on the MCU to determine which movement type has the highest probability at current time and therefore, determine what movement type the user is currently in. We also implement websocket to send the result from STM32 to Rpi. Wifi-ISM43362 library was added to STM32 to realize the utility.  When fall is detected, the LED connected to Rpi will turn red to indicate the fall is detected.
## Problems Enconutered
## References
https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6450028
