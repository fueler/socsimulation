/**
 * @author Wayne Moorefield
 * @brief Main entry point for application
 */

#include <stdio.h>
#include "socbasic.h"


/**
 * @brief Program Entry Point
 * @param argc number of arguments passed
 * @param argv ptr to arguments
 * @return 0 if success, otherwise error
 */
int main(int argc, char **argv)
{
    Memory mem;
    CPUContext cpuctx;

    if (resetSoC(cpuctx, mem)) {
        if (loadProgram(mem)) {
            if (runProgram(cpuctx, mem)) {
                debugDumpSocStatus(cpuctx, mem);
            } else {
                printf("ERROR: Unable to run program\n");
            }
        } else {
            printf("ERROR: Unable to load program\n");
        }
    } else {
        printf("ERROR: Unable to reset SoC\n");
    }

	return 0;
}
