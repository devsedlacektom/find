#include "userStructures.h"
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef FIND_DEFINED
#define FIND_DEFINED

/** \brief Find all suitable files within a POSIX system
 *  if there's a problem with base directory or memory allocation
 *  false is returned and no files are printed as a result.
 *
 *  @param pArgs - ParsedArguments structure
 *  @return true if successful
 *          false if base directory doesn't exist, or memory 
 *          allocation failed during execution
 */
bool find(ParsedArguments *pArgs);

#endif
