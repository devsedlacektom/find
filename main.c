#include "arguments.h"
#include "find.h"
#include <stdio.h>


// Entry point -> all other code is in separate modules.
int main(int argc, char *argv[])
{
    ParsedArguments pArgs = initParsedArguments();

    // check if the arguments can be parsed
    if (!parseArguments(&pArgs, argc, argv)) {
        return EXIT_FAILURE;
    }

    // check if the find algorithm no errors (dynamic allocation)
    // also prints results / error messages.
    if (!find(&pArgs)) {
        return EXIT_FAILURE;
    }

    // no error occurred
    return EXIT_SUCCESS;
}
