#ifndef OPENCOMMAND
#define OPENCOMMAND
#include <stdio.h>
#include "../StruttureDati.h"
#include <stdlib.h> 
#include <string.h>
#include "Utils.h"

void handleOpenCommand(char* command, int socket);

void getFileName(char* command, char* nomefile);



#endif
