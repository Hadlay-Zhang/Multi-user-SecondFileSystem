#include <iostream>
//#include <sys/mman.h>
//#include <sys/types.h>
//#include <fcntl.h>
//#include <unistd.h>
#include <string.h>
//#include <stdio.h>
#include <sstream>

#include "../include/Kernel.h"
#include "../include/Interaction.h"

int main(int argc, char const *argv[])
{
    Interaction* interaction = new Interaction;
    interaction->Entrance();
    delete interaction;
    return 0;
}
