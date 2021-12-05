#include "userStructures.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int RESULTS_REALLOCATION = 64;


/** \brief Return an initialized ParsedArguments structure,
 *  loaded with default values
 * 
 *  @return ParsedArguments structure
 */
ParsedArguments initParsedArguments()
{
    ParsedArguments pArgs;
    // the search with file's name and for file's owner
    // is turned off by default
    pArgs.setName = false;
    pArgs.setUser = false;

    // mask is off
    pArgs.setMask = false;
    pArgs.mask = 0;

    // sets sorting by path (default)
    pArgs.sortType = 0;

    // minimal and maximal depth are not set by default (basically 0 to whatever)
    pArgs.setMinimalDepth = false;
    pArgs.minimalDepth = 0;
    pArgs.setMaximalDepth = false;
    pArgs.maximalDepth = UINT32_MAX;

    // show hidden files is off
    pArgs.setShowAll = false;

    // default linebreak is \n
    pArgs.lineBreak = '\n';

    // show help is set to false
    pArgs.showHelp = false;

    // pointers set to NULL
    pArgs.nameArg = NULL;
    pArgs.usernameArg = NULL;
    pArgs.startDirectory = NULL;
    pArgs.useless = NULL;

    return pArgs;
}


/** \brief Return an initialized Results structure,
 *  loaded with default values
 * 
 *  @return Results structure
 */
Results initResults()
{
    Results res;
    res.arrayAllocatedSize = 0;
    res.arrayIndex = 0;
    res.resultsArray = NULL;
    return res;
}


/** \brief Free all of the resources used in Results structure
 *  
 *  @param res - Results structure
 */
void freeResults(Results *res)
{
    Result *current = NULL;

    // loop through all results
    for (size_t i = 0; i < res->arrayIndex; i++) {
        current = res->resultsArray + i;
        // if the result has been allocated, free it
        if (current->filePath != NULL)
            free(current->filePath);
    }

    // free the array of results
    if (res->resultsArray != NULL)
        free(res->resultsArray);
}


/** \brief Create a path string (either to file or to directory)
 * 
 *  @param previousPath - previous part of the path
 *  @param currentElement - name of a current element (file / directory)
 *  @param result - pointer to store the desired string
 *  @return true if successful
 */
bool createFilePath(char *previousPath, char *currentElement, char **result)
{
    // allocate room for a new string, all initialized to 0
    // (+2 because we add a backslash, and a nullchar to properly terminate a string)
    *result = calloc(strlen(previousPath) + strlen(currentElement) + 2, sizeof(char));

    // allocation unsuccessful
    if (*result == NULL) {
        return false;
    }

    // copy previous path
    strcpy(*result, previousPath);
    // add a backslash
    strcat(*result, "/");
    // add current element's name
    strcat(*result, currentElement);

    // allocation successful
    return true;
}


/** \brief Add a new Result into Results array. If memory is insufficient
 *         attempt to reallocate it and expands the array.
 * 
 *  @param res - Results structure
 *  @param filePath - pointer that's stored in a new Result
 *  @param fileSize - size of the file which's path is stored in a new Result
 *  @return true on success
 *          false on fail with memory allocation
 */
bool createResult(Results *res, char *filePath, size_t fileSize)
{
    // try to reallocate the array with new size (+ 64 results)
    if (res->arrayAllocatedSize <= res->arrayIndex) {
        // store new maximal size (current size is stored within arrayIndex)
        res->arrayAllocatedSize = res->arrayAllocatedSize + RESULTS_REALLOCATION;

        // reallocate current array with new size
        Result *reallocated = realloc(res->resultsArray, res->arrayAllocatedSize * sizeof(Result));
        
        // the reallocation is unsuccessful, finish with error
        if (reallocated == NULL) {
            return false;
        }

        // rewrite original pointer to new one
        res->resultsArray = reallocated;
    }

    // Point at the new result
    Result *newResult = res->resultsArray + res->arrayIndex;

    // Populate the new record
    newResult->filePath = filePath;
    newResult->fileSize = fileSize;

    // increment the next element index pointer
    res->arrayIndex++;

    // the operation was successful
    return true;
}
