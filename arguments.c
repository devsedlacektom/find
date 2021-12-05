#include "arguments.h"
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Check if an argument is a valid option
static inline bool isOpt(char *argument)
{
    return (argument[0] == '-');
}


// Parse options into an index for function pointer array
static int parseOpt(int opt)
{
    switch (opt) {
    case 'n':
        return 0;
    case 's':
        return 1;
    case 'm':
        return 2;
    case 'u':
        return 3;
    case 'f':
        return 4;
    case 't':
        return 5;
    case 'a':
        return 6;
    case '0':
        return 7;
    case 'h':
        return 8;
    default:
        return 9;
    }
}


// Check if a given argument is made of digits only
static bool isOnlyDigits(char *arg)
{
    // loop through string
    while (*arg != '\0') {
        // also incrementing the pointer
        if (!isdigit(*arg++)) {
            return false;
        }
    }
    return true;
}

// Check if the whole argument is made of only zero's
static bool isAllZero(char *arg)
{
    // loop through string
    while (*arg != '\0') {
        // also incrementing the pointer
        if (*arg++ != '0') {
            return false;
        }
    }
    return true;
}

// Check if the received mask is correct
static bool isCorrectMask(int mask)
{
    while (mask) {
        if ((mask % 10) > 7) {
            return false;
        }
        mask /= 10;
    }

    return true;
}

// Try to parse number from an argument, true on success, false on fail
static bool parseNumberFromArg(char *arg, int *num)
{
    int64_t parsed = strtol(arg, NULL, 10);

    if ((parsed < INT_MIN) || (parsed > INT_MAX)) {
        return false;
    // on 0, we need to check special cases (strtol specifics)
    } else if (parsed == 0) {
        if (!isAllZero(arg)) {
            return false;
        }
    } else {
        if (!isOnlyDigits(arg)) {
            return false;
        }
    }

    *num = parsed;
    return true;
}

// all of the set<*> functions defined below print help if parsing is not successful

// Set name into pArgs
static bool setName(ParsedArguments *pArgs, char *arg)
{
    if (isOpt(arg)) {
        fprintf(stderr, "\'-n\' takes a string as an argument and searches"
                        " through filesystem for files which names contain this string."
                        " The program will now terminate.\n");
        return false;
    }

    pArgs->setName = true;
    pArgs->nameArg = arg;
    return true;
}

// Set sort type in pArgs
static bool setSort(ParsedArguments *pArgs, char *arg)
{
    if (strcmp(arg, "f") == 0) {
        pArgs->sortType = 1;
        return true;
    } else if (strcmp(arg, "s") == 0) {
        pArgs->sortType = 2;
        return true;
    }

    fprintf(stderr, "\'-s\' takes \'f\' | \'s\' an argument and sorts"
                    " the results either by file name, file path (\'f\') or by size (\'s\')."
                    " The program will now terminate.\n");
    return false;
}

// Set mask in pArgs
static bool setMask(ParsedArguments *pArgs, char *arg)
{
    int maskNum = 0;
    if (!parseNumberFromArg(arg, &maskNum)) {
        fprintf(stderr, "\'-m\' expects a number as an argument. Terminating program.\n");
        return false;
    }

    if (!isCorrectMask(maskNum)) {
        fprintf(stderr, "\'-m\' expects a number as an argument. Number has to be formatted in octal."
                        " Terminating program.\n");
        return false;
    }

    pArgs->setMask = true;
    pArgs->mask = maskNum;
    return true;
}

// Set user in pArgs
static bool setUser(ParsedArguments *pArgs, char *arg)
{
    if (isOpt(arg)) {
        fprintf(stderr, "\'-u\' takes username as an argument and"
                        " filters results to only those owned by specific user."
                        " No username given!"
                        " The program will now terminate.\n");
        return false;
    }

    if (getpwnam(arg) == NULL) {
        fprintf(stderr, "User \'%s\' doesn't exist.\n", arg);
        return false;
    }

    pArgs->setUser = true;
    pArgs->usernameArg = arg;
    return true;
}

// Set mindepth in pArgs
static bool setMinDepth(ParsedArguments *pArgs, char *arg)
{
    int minDepth = 0;
    if (!parseNumberFromArg(arg, &minDepth)) {
        fprintf(stderr, "\'-f\' expects a number as an argument. Terminating program.\n");
        return false;
    }

    pArgs->setMinimalDepth = true;
    pArgs->minimalDepth = minDepth;
    return true;
}

// Set maxdepth in pArgs
static bool setMaxDepth(ParsedArguments *pArgs, char *arg)
{
    int maxDepth = 0;
    if (!parseNumberFromArg(arg, &maxDepth)) {
        fprintf(stderr, "\'-t\' expects a number as an argument. Terminating program.\n");
        return false;
    }

    pArgs->setMaximalDepth = true;
    pArgs->maximalDepth = maxDepth;
    return true;
}

// Set hidden file checking
static bool setHiddenFiles(ParsedArguments *pArgs, char *arg)
{
    pArgs->useless = arg;
    pArgs->setShowAll = true;
    return true;
}

// Set linebreak in pArgs to '\0'
static bool setNullCharTerminator(ParsedArguments *pArgs, char *arg)
{
    pArgs->useless = arg;
    pArgs->lineBreak = '\0';
    return true;
}

// Set option to display help in pArgs
static bool setHelp(ParsedArguments *pArgs, char *arg)
{
    pArgs->useless = arg;
    pArgs->showHelp = true;
    return true;
}

// Print info when argument is incorrect
static bool incorrectOpt(ParsedArguments *pArgs, char *arg)
{
    pArgs->useless = arg;
    fprintf(stderr, "Incorrect opt argument passed into the program, terminating.\n");
    return false;
}


/** \brief function fills ParsedArguments structure with info from
 *  received arguments. If arguments are incorrect the function returns false.
 * 
 *  @param pArgs - ParsedArguments structure that's filled with info
 *  @param argc - number of arguments
 *  @param arv - strings, arguments themselves.
 *  @return true if arguments were loaded successfully
 *          false otherwise
 */
bool parseArguments(ParsedArguments *pArgs, int argc, char *argv[])
{
    int optResult = 0;
    bool (*parseActions[])(ParsedArguments *, char *) = { setName, setSort, setMask, 
            setUser, setMinDepth, setMaxDepth, setHiddenFiles, setNullCharTerminator, setHelp, incorrectOpt };

    // loop through opts, parse them into pArgs structure
    while ((optResult = getopt(argc, argv, "n:s:m:u:f:t:a0h")) != -1 && optResult != '?') {
        if (!(*parseActions[parseOpt(optResult)])(pArgs, optarg)) {
            return false;
        }
    }

    // check getOpt
    if (optResult == '?' || optResult == ':') {
        return false;
    }

    // loop through argument, find first non opt and use it as a startDirectory path.
    for (int i = 1; i < argc; i++) {
        if (isOpt(argv[i])) {
            optResult = parseOpt(argv[i][1]);
            if ((optResult >= 0) && (optResult <= 5)) {
                i++;
            }
        } else {
            pArgs->startDirectory = argv[i];
            break;
        }
    }

    return true;
}
