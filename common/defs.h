#ifndef __DEFS_H
#define __DEFS_H

#ifdef _WIN32
    #include <windows.h>
#elif linux
    #include <linux/limits.h>
#else
    #include <limits.h>
#endif

#ifndef PATH_MAX
    #ifdef MAX_PATH
        #define PATH_MAX    MAX_PATH
    #else
        #define PATH_MAX    4096
    #endif
#endif

#endif
