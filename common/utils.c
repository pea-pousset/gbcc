/**
 * \addtogroup Commons
 * \{
 * \defgroup Utils
 * \addtogroup Utils
 * \{
 */

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#elif linux
    #include <unistd.h>
    #include <linux/limits.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

#ifndef PATH_MAX
    #ifdef MAX_PATH
        #define PATH_MAX    MAX_PATH
    #else
        #define PATH_MAX    4096
    #endif
#endif

#include "errors.h"


/*========================================================================*//**
 * Allocates a block of memory or throws a fatal error in case of failure
 *
 * \param size: Size of the memory block to allocate
 * \return NULL in case of failure, a pointer to the memory block allocated
 * otherwise
 *//*=========================================================================*/
 void* mmalloc(size_t size)
 {
    void* new = malloc(size);
    if (new == NULL)
        ccerr(F, "unable to allocate memory");
    return new;
 }

/*========================================================================*//**
 * Changes the size of a memory block or throws a fatal error in case of
 * failure
 *
 * \param ptr: Pointer to the memory block to resize
 * \param size: New size for the memory block
 * \return NULL in case of failure, a pointer to the memory block allocated
 * otherwise
 *//*=========================================================================*/
 void* mrealloc(void* ptr, size_t size)
 {
    void* new = realloc(ptr, size);
    if (new == NULL)
        ccerr(F, "unable to allocate memory");
    return new;
 }

/*========================================================================*//**
 * Generates a temporary file name
 *
 * \return A temporary file name and path
 * \todo error handling
 *//*=========================================================================*/
char* m_tmpnam()
{
    static char name[PATH_MAX * 2 + 1];
#if !defined(_WIN32) && !defined(__APPLE__)
    char cwd[PATH_MAX + 1];
#endif
    name[0] = 0;
#ifdef _WIN32
    GetTempPath(PATH_MAX, name);
    GetCurrentDirectory(PATH_MAX, cwd);
    SetCurrentDirectory(name);
    strcat(name, tmpnam(NULL));
    SetCurrentDirectory(cwd);
#elif defined(__APPLE__)
    strcpy(name, tmpnam(NULL));
#else
    char* tmp_path = NULL;
    tmp_path = getenv("TMPDIR");
    if (!tmp_path)
        tmp_path = getenv("TMP");
    if (!tmp_path)
        tmp_path = getenv("TEMP");
    if (!tmp_path)
        tmp_path = getenv("TEMPDIR");
    if (!tmp_path)
        tmp_path = "";
    
    strcpy(name, tmp_path);
    getcwd(cwd, PATH_MAX);
    chdir(tmp_path);
    strcat(name, tmpnam(NULL));
    chdir(cwd);
#endif
    return name;
}

/*=======================================================================*//**
 * Copies the content of a file to another
 *
 * \param src: name of the file to be copied
 * \param dst: name of the file where the content is to be copied
 *
 * \return 1 in case of success, 0 otherwise
 *//*========================================================================*/
int copy_file(const char* src, const char* dst)
{
#ifdef _WIN32
    return CopyFile(src, dst, FALSE);
#else
    size_t nread, nwrite;
    unsigned char buf[4096];
    FILE* i = fopen(src, "rb");
    FILE* o = fopen(dst, "wb");

    if (i == NULL || o == NULL)
        return 0;

    do
    {
        nread = fread(buf, 1, 4096, i);
        if (nread)
            nwrite = fwrite(buf, 1, nread, o);
        else
            nwrite = 0;
    } while (nread > 0 && nread == nwrite);

    fclose(i);
    fclose(o);

    if (nwrite)
        return 0;

    return 1;
#endif
}

/**
 * \} Utils
 * \} Commons
 */
