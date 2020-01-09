# 2019-Fall-Detection

## Motivation
Falls in the elderly have always been a serious medical and social problem. To detect and predict falls, and further more roughly determine what movement type currently is, a hidden Markov model (HMM)-based method using tri-axial accelerations of human body is proposed.
## Implementation
We used `BSP_B-L475E-IOT01` library to read acceleration by the STM32L475 MCU. After that, we collected data of movement types (standing, walking, runnung, fall, prediect) in order to feed in the HMM models. In convenience, we calculated the magenitude of tri-axial accelerations instead of using the data straightforwardly. After training, we got sets of alpha, beta, pi parameters for each model. With the parameters, we could run viterbi algorithm on the MCU to determine which movement type has the highest probability at current time. Consequently, find out the user's movement type. We also implemented websocket to send the result from STM32 to Rpi. `Wifi-ISM43362` library was added to STM32 to realize the utility.  When fall is detected, the LED connected to Rpi will turn red to indicate the fall is detected.
## How to Run 
Replace the corresponding code in the STM32 with the code on our repository and add the library metioned above to the STM32 as well. Remember to do it under the default program `mbed-os-example-wifi` in Mbed Studio. After setting up your wifi environment, run the `socket_server.py` on Rpi or PC and the result will output on the terminate.
## Training
Run the hmm.py to fit the dataset (remember to change the tri-axial acceleration in to magenitude). Also, rewrite the file name you desire too input and output. The output file will be the csv file with the model parameter.
## Result
The result is as we expected. It could predeict and detect fall with good precision. However, there is still room for improvement. One we failed to accomplish is that when determining the running states, the output often will have false alarm before it. For example, while we start running, it will first output predict and falling for a while. We belief it's mainly because the data we collected while starting running is too similar to the running data, therefore, leading to false alarm. 
## Problems Enconutered
At the beginning, we was worried about the MCU is not able to support the computing power we desired. Fortunately, with little technique of thread management, it could perform flawlessly as we looked for.
## Future Work
Since the movement detection is not entirely prefect now, the future work will be focused on pursuing better precision of the result. Implementing Kalman filter or re-train the model with better dataset or with resived window size are efforts we can be working on. Furthermore, although we have made the prediction, we can only predict but couldn't do anything after that. Hence, developing a protection gear to take action after prediction is also a subject that we concern.
## References
https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6450028
