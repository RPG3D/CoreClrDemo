
#include <iostream>
#include "ClrManager.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n";

    ClrManager clrManager;

    clrManager.LoadHostFxr();
    
    return 0;
}
