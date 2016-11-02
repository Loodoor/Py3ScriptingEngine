#include <iostream>

#include "scripting/scripting.hpp"

int main(int argc, char *argv[])
{
    PyScripting::connect();  // initialize scripting engine
    PyScripting::run_all_modules();  // will run all modules loaded 1 time
    PyScripting::disconnect();  // quit

    return 0;
}
