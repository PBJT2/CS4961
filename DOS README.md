
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



