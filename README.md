# 802.11-WLAN-simulation

In this project, we need to implement the ad hoc version of 802.11 Wireless Lan. An ad hoc
mode is a network with no central control and with no connections to the “outside world.” In this
model, there are N hosts and all hosts can transmit packet with each other. Each host generates
frames with a mean arrival rate which follows the negative exponential distribution. Also, each
host has an infinite buffer queue for the waiting frames. During simulation, each host first choose
a random receiver, if there is no collision then the transmission will begin, otherwise it won’t.
This model uses CSMA/CA protocol to deal with the collision avoidance.
