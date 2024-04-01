#include <stdlib.h>
#include <errno.h>
#include "connectionhandler.h"
#include "util.h"
#include "broadcastagent.h"

int main(int argc, char **argv)
{
    utilInit(argv[0]);
    broadcastAgentInit();
	infoPrint("Chat server, group 08");	//TODO: Add your group number!

    int check = 0;

	//TODO: evaluate command line arguments
    // /* If 1 argument is passed, argc is 1
    //     * argv[0] is the name of the program itself, argv[1] holds the first argument
    //     * */
    int port = 8111;
    if (argc == 2){
        for (int i=0;argv[1][i] != '\0';i++){
            int ascii = argv[1][i];
            if (ascii < 48 || ascii > 57){ //Ascii Code zwischen 48 - 57 (0-9)
                errorPrint("Bad Portnumber!");
                check = -1;
                break;
            }
        }
        if (check != -1){
            port = atoi(argv[1]);
            if (port < 1024) {             //Ports unter 1024 sind privilegierte Ports!
                check = -1;
                errorPrint("Bad Portnumber!");
            } else {
                infoPrint("Using Port %d", port);
            }
        }
    } else if(argc > 2) {
        infoPrint("Too many arguments. Using Standard-Port 8111");
    } else {
        infoPrint("Using Standard-Port 8111");
    }
	
	debugEnable();  

	if (check != -1){
        const int result = connectionHandler((in_port_t)port);
        if (result == 1) {
            //TODO: perform cleanup, if required by your implementation
            broadcastAgentCleanup();
        }
    return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
