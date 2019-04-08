/**
 * \addtogroup Commons
 * \{
 * \addtogroup Utils
 * \{
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

char* m_tmpnam();
void* mmalloc(size_t size);
void* mrealloc(void* ptr, size_t size);
int copy_file(const char* src, const char* dst);

#endif

/**
 * \} Utils
 * \} Commons
 */
