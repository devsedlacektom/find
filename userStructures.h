#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// fix multiple imports
#ifndef USER_STRUCTURES_DEFINED
#define USER_STRUCTURES_DEFINED

// structure stores necessary info for find algorithm
typedef struct
{
    // start directory in which to start
    char *startDirectory;

    // if true, the files have to contain the specified string in their name
    bool setName;
    char *nameArg;

    // default => sorts by path, if set to false sorts by the file size
    uint8_t sortType;

    bool setMask;
    int mask;

    // if true, the owner of the files is USER (get from argument)
    bool setUser;
    char *usernameArg;

    // sets minimal depth of files
    bool setMinimalDepth;
    uint32_t minimalDepth;

    // sets maximal depth of given files
    bool setMaximalDepth;
    uint32_t maximalDepth;

    // sets algorithm to look for hidden objects
    bool setShowAll;

    // sets line breaks to Nullchar instead
    char lineBreak;

    // sets the program to display depth, then stops
    bool showHelp;

    // used to get rid of warnings
    void *useless;
} ParsedArguments;


// structure stores info about one result
typedef struct
{
    // string of file path
    char *filePath;

    // size
    size_t fileSize;
} Result;


// structure stores array of results, plus currently allocated size and number of
// elements stored in the array so far
typedef struct
{
    // array of results
    Result *resultsArray;
    // index of next item (last item is index - 1)
    size_t arrayIndex;
    // maximum allocated size, used in realloc
    size_t arrayAllocatedSize;
} Results;


/** \brief Create a ParsedArguments structure with default values
 * 
 *  @return ParsedArguments structure
 */
ParsedArguments initParsedArguments();


/** \brief Create a Results structure
 *
 *  @return new Results structure
 */
Results initResults();


/** \brief Add a file into a results array, if memory is exceeded it reallocates the whole array with additional memory
 * 
 *  @param res - Results structure containing array of Result structure
 *  @param filePath - path to file
 *  @param fileSize - size of file
 *  @return true if a new result could be created (no allocation errors)
 *          false if an allocation error occurred
 */
bool createResult(Results *res, char *filePath, size_t fileSize);


/** \brief Create a new filePath, used either with directories, or with files themselves
 * 
 *  @param previousPath - path so far
 *  @param currentElement - name of a current object (file / directory)
 *  @param result - stores a new string
 *  @return true if a new result could be created (no allocation errors)
 *          false if an allocation error occurred (result will not get initialized)
 */
bool createFilePath(char *previousPath, char *currentElement, char **result);


/** \brief Free heap memory used by Results array
 * 
 *  @param res - Results structure containing array of Result structure
 */
void freeResults(Results *res);

#endif
