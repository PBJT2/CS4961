Single Bit Error Injection READ ME

*************************
*Launching Open Sat Kit *
*************************
1) Launch the Command Terminal pressing "ctrl+t"
2) In the terminal type in "cd OpenSatKit-master/cosmos"
3) Then Type in "ruby Launcher"
4) Press Ok
5) Click OpenSatKit
6) On the "Open Sat Kit" Window click on Start cFS
7) Enter your password used to log on to your computer

*******************************************
*Sending Single Bit Error Injection (SBEI)*
*******************************************
While the cFS is running

1) On the "cFS Command and Telemetry Server" Window click on "Cmd Packets" Tab
2) Click on the "View in Command Sender"
3) On the "Command Sender" window for "Target:" select: "MM"
6) For "Command:" select "SBEI_INJECT"
7) Click "Send"

In the Terminal Window it should write out the following:
"SBEI COMMAND: Bit Flipped = X Data = Y"
"X" is the bit being flipped counting from 0-7
"Y" is the resultant value of the flip 

NOTE: The memory being flipped always starts with 0.

***************
*File Location*
***************
Main Files
Files used to do the injection is located in the following
.c File is located in: "../OpenSatKit-master/cfs/apps/mm/fsw/src/mm_sbei.c"
.h File is located in :"../OpenSatKit-master/cfs/apps/mm/fsw/src/mm_sbei.h"

The Dummy Application Folder (SBEI)
Files for the dummy application is located in the following:
"../OpenSatKit-master/cfs/apps/sbei/fsw/src"