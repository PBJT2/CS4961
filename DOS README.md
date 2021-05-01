Denial of service occurs when network communication services are interrupted or shut down, making them inaccessible to the intended users. This is often an attack carried out by hackers who aim to overhelm the netowrk with more traffic than it can handle. Thus, a network intended to transfer incoming and outgoing data and requests is being flooded with fakes. The motive behind these attacks can range from simply disrupting service to even extortion. 

The goal is to simulate a denial of service by injecting the flood attack on the COSMOS-CFS network, and develop a program that will continously monitor the network traffic, Once an increase in traffic is detected, the user will be notified.

We have implemented this anomaly both as ground and satellite-based. However, to better control our injection and detection, we will present this anomaly as satellite-based. Meaning we will be executing the injection and detection commands through the Core Flight System. 

Two CFS applications has been created. The Denial of Service Injection (DOSI) and Denial of Service Detection (DOSD) app. 

We use a toolbox called Netwox to help us carry out the flooding attack. Once the injection command is executed through the DOSI app, it will start spamming the network with data packets.

For detection, the command will be executed through the DOSD app which will begin monitoring the network traffic. We do this by continously checking the system's rx_bytes file. This file contains the total data packets received. We then calculate the rate in kbps. If this rate exceeds a certain rate, the app will send a message to the CFS terminal and notify the user of unusual amount of traffic. 


*Launching OpenSat Kit *
*************************
1) Launch the Command Terminal pressing "ctrl+t"
2) In the terminal type in "cd OpenSatKit-master/cosmos"
3) Then Type in "ruby Launcher"
4) Press Ok
5) Click OpenSatKit
6) On the "Open Sat Kit" Window click on Start cFS
7) Enter your password used to log on to your computer

*******************************************
*Sending Denial of Service Detection (DOSD)*
*******************************************
While the cFS is running

1) On the "cFS Command and Telemetry Server" Window click on "Cmd Packets" Tab
2) Click on the "View in Command Sender"
3) On the "Command Sender" window for "Target": select: "DOSD"
6) For "Command:" select "DETECT"
7) For "App_State": 0 to disconnect cFS-COSMOS connection, 1 to remain connected after detection
8) Click "Send"

*******************************************
*Sending Denial of Service Injection (DOSI)*
*******************************************
While the cFS is running

1) On the "cFS Command and Telemetry Server" Window click on "Cmd Packets" Tab
2) Click on the "View in Command Sender"
3) On the "Command Sender" window for "Target": select: "DOSI"
6) For "Command:" select "INJECTION"
7) Click "Send"

**NOTE: Execute the detection command before the injection. When injection command is executed, a terminal will appear. To cancel the injection, simply exit the terminal. Otherwise, it will terminate after 10 seconds. When APP_State is set to 0, the TFTP App will disconnect. Users will no longer be able to send files to the ground. 

***************
*File Location*
***************
Main Files

Files used to do the detection is located at:
../OpenSatKit-master/cfs/apps/DOSD

Files used to do the injection is located at:
../OpenSatKit-master/cfs/apps/DOSI



