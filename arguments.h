#include "userStructures.h"
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ARGUMENTS_PARSING_DEFINED
#define ARGUMENTS_PARSING_DEFINED

/** \brief Fill ParsedArguments structure with info from
 *  commandline. If arguments are incorrect the function returns false.
 * 
 *  @param pArgs ParsedArguments structure that's filled with info
 *  @param argc number of arguments
 *  @param arv strings, arguments themselves.
 *  @return true if arguments were loaded successfully
 *          false otherwise
 */
bool parseArguments(ParsedArguments *pArgs, int argc, char *argv[]);

#endif
