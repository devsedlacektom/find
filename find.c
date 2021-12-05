#include "find.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/** \brief Print a problem that could have occurred within a directory
 * 
 *  @param baseDirectory path to directory in which the error occurred
 */
static void printDirectoryProblem(char *baseDirectory)
{
    // errors borrowed from documentation
    switch (errno) {
    case EACCES:
        fprintf(stderr, "Permission to open the directory \'%s\' was denied. Terminating program.\n", baseDirectory);
        break;
    case EMFILE:
        fprintf(stderr, "Too many open file descriptors. Terminating program.\n");
        break;
    case ENFILE:
        fprintf(stderr, "Too many open file descriptors. Terminating program.\n");
        break;
    case ENOENT:
        fprintf(stderr, "Directory \'%s\' doesn't exist. Terminating program.\n", baseDirectory);
        break;
    case ENOMEM:
        fprintf(stderr, "Program is out of memory. Terminating program.\n");
        break;
    case ENOTDIR:
        fprintf(stderr, "\'%s\' is not a valid directory. Terminating program.\n", baseDirectory);
        break;
    }
}


/** \brief Print what causes lstat function to fail (by reading errno)
 * 
 */
static void printFileProblem()
{
    // errors borrowed from documentation
    switch (errno) {
    case EACCES:
        fprintf(stderr, "Permission to view file stats denied.\n");
        break;
    case EIO:
        fprintf(stderr, "I/O Error while reading file stats.\n");
        break;
    case ELOOP:
        fprintf(stderr, "Loop in symbolic links found.\n");
        break;
    case ENAMETOOLONG:
        fprintf(stderr, "Path length too long.\n");
        break;
    case ENOTDIR:
        fprintf(stderr, "Element of a path leading to a file is not a directory.\n");
        break;
    case ENOENT:
        fprintf(stderr, "A component of path does not name an existing file or path is an empty string.\n");
        break;
    case EOVERFLOW:
        fprintf(stderr, "Overflow during file opening occurred.\n");
        break;
    case EBADF:
        fprintf(stderr, "The directory couldn't be read.\n");
        break;
    }
}


/** \brief Print help if "-h" opt occurs within arguments
 *
 */
static void printHelp()
{
    fprintf(stderr, "This program is a utility that finds files within a "
                    "POSIX compliant operating system.\nThe utility accepts these arguments:\n"
                    "    -n NAME -> Specify substring contained in the file name the utility will look for.\n"
                    "    -s s|f -> Set sorting the results by filename (f),"
                    " by file size (s). If the option is not set, files are sorted by their paths lexically.\n"
                    "    -u USER -> Only show files that are owned by USER.\n"
                    "    -m MASK -> Show files with desired file permissions.\n"
                    "    -f NUM -> Show files that in at least NUM level of directory (path) depth.\n"
                    "    -t NUM -> Show files that in maximum NUM level of directory (path) depth.\n"
                    "    -a -> Show all files, include hidden ones.\n"
                    "    -0 -> Set terminating character to be 'nullchar' (binary 0) instead of 'newline'.\n"
                    "    -h -> Print help on the screen and ends the program.\n"
                    "If there's a non opt argument, it's treated as a path to base directory. Only the first occurrence counts.\n");
}


/** \brief Parse file permissions from file stats
 * 
 *  @param statPtr pointer to stat structure
 *  @return (decimal) int representation of a mask
 */
static int getMask(struct stat *statPtr)
{
    int mask = 0;
    // the idea for shortening the code borrowed from:
    // http://codewiki.wikidot.com/c:system-calls:stat
    (statPtr->st_mode & S_IRUSR) ? (mask += 400) : (mask += 0);
    (statPtr->st_mode & S_IWUSR) ? (mask += 200) : (mask += 0);
    (statPtr->st_mode & S_IXUSR) ? (mask += 100) : (mask += 0);
    (statPtr->st_mode & S_IRGRP) ? (mask += 40) : (mask += 0);
    (statPtr->st_mode & S_IWGRP) ? (mask += 20) : (mask += 0);
    (statPtr->st_mode & S_IXGRP) ? (mask += 10) : (mask += 0);
    (statPtr->st_mode & S_IROTH) ? (mask += 4) : (mask += 0);
    (statPtr->st_mode & S_IWOTH) ? (mask += 2) : (mask += 0);
    (statPtr->st_mode & S_IXOTH) ? (mask += 1) : (mask += 0);
    return mask;
}


/** \brief Determine if given file/directory name is a hidden object
 * 
 *  @param fileName - name of the file / directory
 *  @return true if object name starts with '.'
 *          false otherwise
 */
static inline bool isHidden(char *fileName)
{
    return (fileName[0] == '.');
}


/** \brief If "-n" opt occurs in arguments, check if the desired
 *  substring is present in the name of the file 
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param fileName - name of the file (correct string)
 *  @return -true if the "-n" opt occurred in the arguments and desired
 *           substring is present within fileName, OR true if "-n" is not present
 *          -false only when "-n" is present and desired substring is not 
 */
static bool checkName(ParsedArguments *pArgs, char *fileName)
{
    // if the name checking is desired it checks the name
    if (pArgs->setName) {
        if (strstr(fileName, pArgs->nameArg) == NULL) {
            return false;
        }
    }

    // otherwise the file is suitable whatever its name is
    return true;
}


/** \brief If "-m" opt occurs in arguments, check if the file has
 *  desired permissions
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param permissions - file permissions (obtained by calling getMask() function)
 *  @return -true if the "-m" opt occurred in the arguments and file has 
 *           desired permission OR true if "-m" is not present
 *          -false only when permissions don't match 
 */
static bool checkPermissions(ParsedArguments *pArgs, int permissions)
{
    // if mask checking is set to true it checks the mask
    if (pArgs->setMask) {
        return (permissions == pArgs->mask);
    }

    // otherwise the file is suitable whatever its mask is
    return true;
}

/** \brief If "-u" opt occurs in arguments, check if the file
 *  is owned by our desired owner. 
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param statPtr - stat structure
 *  @return -true if the "-u" opt occurred in the arguments and file is owned
 *           by our desired owner, OR true if "-u" is not present
 *          -false only when "-u" is present and file's owner differs 
 */
static bool checkUser(ParsedArguments *pArgs, struct stat *statPtr)
{
    struct passwd *pwd = NULL;
    errno = 0;

    // checking for user set.
    if (pArgs->setUser) {
        pwd = getpwnam(pArgs->usernameArg);
        // compare user id's
        return (pwd->pw_uid == statPtr->st_uid);
    }

    // not searching for user, file suitable
    return true;
}


/** \brief If "-f" opt occurs in arguments, check if the desired
 *  recursive directory depth occurs
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param depth - obtained from findRecursive parameters
 *  @return -true if the "-f opt occurred in the arguments and desired
 *           depth is reached OR true if "-f" is not present
 *          -false only when "-f" is present and minimal depth is not reached
 */
static bool checkMinDepth(ParsedArguments *pArgs, size_t depth)
{
    // if checking for minimal depth is set, check it
    if (pArgs->setMinimalDepth) {
        return (pArgs->minimalDepth <= depth);
    }

    // otherwise the file is suitable whatever its depth is
    return true;
}


/** \brief If "-t" opt occurs in arguments, check if the desired
 *  recursive directory depth occurs
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param depth - obtained from findRecursive parameters
 *  @return -true if the "-f opt occurred in the arguments and desired
 *           depth has not been reached OR true if "-t" is not present
 *          -false only when "-t" is present and maximal depth has been surpassed
 */
static bool checkMaxDepth(ParsedArguments *pArgs, size_t depth)
{
    // if checking for maximal depth is set, it checks it
    if (pArgs->setMaximalDepth) {
        return (pArgs->maximalDepth >= depth);
    }

    // otherwise the file is suitable whatever its depth is
    return true;
}


/** \brief If "-a" opt doesn't occur in arguments, check if the current file
 *  is not hidden
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param fileName - name of the file
 *  @return -true if the "-a" opt occurred in the arguments 
 *           OR true if "-a" is not present and file is NOT hidden
 *          -false when "-a" is not present and file is hidden
 */
static bool checkHidden(ParsedArguments *pArgs, char *fileName)
{
    // if checking for all files is set, it checks hidden files as well
    if (pArgs->setShowAll) {
        return true;
    }

    // it only allows non hidden files
    return (!isHidden(fileName));
}


/** \brief Search through filesystem and recursively try to find
 *  desired files
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param baseDirectory - directory that's opened in function call
 *  @param res - Results structure containing Result array
 *               used to store filenames and file sizes
 *  @param depth - recursive depth (length from the first directory)
 *  @return -true if recursion is successful
 *          -false if first directory cannot be opened OR any malloc (/calloc)
 *           fail occurs. In that case any recursion stops immediately.
 */
static bool findRecursive(ParsedArguments *pArgs,
        char *baseDirectory,
        Results *res,
        size_t depth)
{
    // Combine values from multiple recursion depths, the first one will be true implicitly
    // if we encounter an error, crash the whole program

    // used for directory access.
    errno = 0;

    DIR *currentDirectory = opendir(baseDirectory);
    bool resultRec = true;

    // if directory fails at level 0 (base directory) the recursion ends
    // and false is returned. otherwise true is returned (error message is shown
    // both times)
    if (currentDirectory == NULL) {
        printDirectoryProblem(baseDirectory);
        if (depth == 0)
            return false;
        return true;
    }

    // depth 0 = basedirectory, everthing has an increased depth
    depth++;
    
    // used to access files in directory
    struct dirent *directoryElement = NULL;

    // used file statistics
    struct stat buf;

    // path to file is stored here
    char *currentPath = NULL;

    // loop through content of a directory
    while ((directoryElement = readdir(currentDirectory)) != NULL) {
        // reset errno just in case
        errno = 0;
        // error occurred somewhere, breaks the rest of the cycle
        if (!resultRec) {
            break;
        }
    
        // skip current and parent folder
        if ((strcmp(directoryElement->d_name, ".") == 0)
                    || (strcmp(directoryElement->d_name, "..") == 0)) {
            continue;
        }

        // create a filePath for our file. after we don't need it anymore
        // we free it (if we do we keep it in result, free it at the end of the code run)
        if (!createFilePath(baseDirectory, 
                    directoryElement->d_name,
                    &currentPath)) {
            fprintf(stderr, "Couldn't allocate file path.\n");
            closedir(currentDirectory);
            return false;
        }

        // get stats for the file, if unsuccessful it proceeds
        if (lstat(currentPath, &buf) != 0) {
            printFileProblem();
            free(currentPath);
            continue;
        }

        // check for directory = recursion with "currentPath" as base directory.
        if (S_ISDIR(buf.st_mode)) {
            // if the directory is not hidden or we want to search through all files
            // we enter the directory, else only free the path and continue
            if (!isHidden(directoryElement->d_name) || pArgs->setShowAll) {
                resultRec = findRecursive(pArgs, currentPath, res, depth);
            }

            // do not put directory into results
            free(currentPath);
            continue;
        } else if (S_ISREG(buf.st_mode)) {
            // is regular file. if one of the conditions fail, free allocated string and
            // continue with the cycle

            // if a condition fails the file is skipped
            if (!(checkName(pArgs, directoryElement->d_name) &&
                        checkPermissions(pArgs, getMask(&buf)) &&
                        checkUser(pArgs, &buf) &&
                        checkMinDepth(pArgs, depth) &&
                        checkHidden(pArgs, directoryElement->d_name))) {
                free(currentPath);
                continue;
            }

            // cutting the recursion branch
            if (!checkMaxDepth(pArgs, depth)) {
                free(currentPath);
                closedir(currentDirectory);
                return (true && resultRec);
            }

            // ADD RESULT, all memory allocation problems return false
            if (!createResult(res, currentPath, buf.st_size)) {
                free(currentPath);
                closedir(currentDirectory);
                return false;
            }
        } else {
            // is not regular file -> skip
            free(currentPath);
            continue;
        }

    }

    // after readdir was done, the directory is closed and result value is returned
    closedir(currentDirectory);
    return (true && resultRec);
}


/** \brief Retrieve fileName from any path
 * 
 *  @param filePath - the whole path to the file
 *  @return char pointer at part of the path that's the name of the file
 */
char *getFileName(char *filePath)
{
    char *iter = filePath;
    while (*iter != '\0') {
        if (*iter++ == '/') {
            filePath = iter;
        }
    }

    return filePath;
}


/** \brief Compare strings case insensitive
 * 
 *  @param strOnePtr - pointer to the first string
 *  @param strTwoPtr - pointer to the second string
 *  @return positive num if first string is bigger
 *          negative num if first string is smaller
 *          0 if strings are equal
 */
int strCmpCI(char *strOnePtr, char *strTwoPtr)
{
    int chOne = 0;
    int chTwo = 0;

    // until we get to the end of string
    while (*strOnePtr != '\0' && *strTwoPtr != '\0') {
        // makes all chars lowercase + pointer iteration
        chOne = tolower(*(strOnePtr++));
        chTwo = tolower(*(strTwoPtr++));

        // if the result is not zero it returns the value
        if (chOne - chTwo != 0) {
            return chOne - chTwo;
        }
    }

    // if one string is shorter then it returns correct value for qsort
    if (*strOnePtr == '\0' && *strTwoPtr != '\0') {
        return -1;
    } else if (*strOnePtr != '\0' && *strTwoPtr == '\0') {
        return 1;
    }

    return 0;
}


/** \brief Determine which file name is bigger 
 *  (case insensitive, unless fileNames are equal)
 * 
 *  @param resultOne - pointer to the first Result element in array
 *  @param resultTwo - pointer to the second Result element in array
 *  @return positive num if resultOne's filename is larger
 *          negative num if resultOne's filename is smaller
 *          0 if names are equal
 */
int sortByFileName(const void *resultOne, const void *resultTwo)
{
    char *fileNameOne = getFileName(((Result *) resultOne)->filePath);
    char *fileNameTwo = getFileName(((Result *) resultTwo)->filePath);

    int result = 0;

    // case insensitive difference
    if ((result = strCmpCI(fileNameOne, fileNameTwo)) != 0) {
        return result;
    }

    // case sensitive difference
    return (strcmp(((Result *) resultOne)->filePath, ((Result *) resultTwo)->filePath));
}


/** \brief Determine which file size is larger 
 *  (reversed, because we want to display the files from largest
 *   to smallest)
 * 
 *  @param resultOne - pointer to the first Result element in array
 *  @param resultTwo - pointer to the second Result element in array
 *  @return -1 if resultOne's filesize is larger
 *          1 if resultOne's filesize is smaller
 *          if sizes are equal, sorting by fileName happens
 */
int sortByFileSize(const void *resultOne, const void *resultTwo)
{
    size_t sizeOne = ((Result *) resultOne)->fileSize;
    size_t sizeTwo = ((Result *) resultTwo)->fileSize;
    
    // this inverts how the qsort sorts the files
    if (sizeOne > sizeTwo) {
        return -1;
    } else if (sizeOne < sizeTwo) {
        return 1;
    }

    return sortByFileName(resultOne, resultTwo);
}


/** \brief Determine which file path is larger (case sensitive)
 * 
 *  @param resultOne - pointer to the first Result element in array
 *  @param resultTwo - pointer to the second Result element in array
 *  @return positive num if resultOne's filepath is larger
 *          negative num if resultOne's filepath is smaller
 *          0 if names are equal
 */
int sortByFilePath(const void *resultOne, const void *resultTwo)
{
    // case sensitive
    return (strcmp(((Result *) resultOne)->filePath, ((Result *) resultTwo)->filePath));
}


/** \brief Sort the results, according to received
 *  opts from console, in situ.
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param res - Results structure, containing Result array and additional info
 */
static void sortResults(ParsedArguments *pArgs, Results *res)
{
    int (*sortType[]) (const void *, const void *) =
            { sortByFileName, sortByFilePath, sortByFileSize };
    qsort(res->resultsArray, res->arrayIndex, sizeof(Result), sortType[pArgs->sortType]);
}


/** \brief Print results on stdout
 * 
 *  @param pArgs - ParsedArguments structure
 *  @param res - Results structure, containing Result array and additional info
 */
static void printResults(ParsedArguments *pArgs, Results *res)
{
    Result *current = NULL;
    for (size_t i = 0; i < res->arrayIndex; i++) {
        // points at element of array
        current = res->resultsArray + i;
        printf("%s", current->filePath);
        putchar(pArgs->lineBreak);
    }
}


/** \brief Find files from desired directory and
 *  sorts the results set by opt arguments
 *
 *  @param pArgs - ParsedArguments structure 
 *         containing info from commandline / impilicit settings if no info is retrieved
 *  @return -true if operation was successful
 *          -false if first directory cannot be opened OR any malloc (/calloc)
 *           fail occurs. In that case any recursion stops immediately.
 */
bool find(ParsedArguments *pArgs)
{
    // show help if desired and return true
    if (pArgs->showHelp) {
        printHelp();
        return true;
    }

    Results results = initResults();
    bool resultOfRecursion = false;

    // base dir not set, using current working dir
    if (pArgs->startDirectory == NULL) {
        resultOfRecursion = findRecursive(pArgs, ".", &results, 0);
    } else {
        // base dir set
        resultOfRecursion = findRecursive(pArgs, pArgs->startDirectory, &results, 0);
    }

    // if recursion succeeds, print sorted results
    if (resultOfRecursion) {
        sortResults(pArgs, &results);
        printResults(pArgs, &results);
    }

    // release memory
    freeResults(&results);
    // return result
    return resultOfRecursion;
}
