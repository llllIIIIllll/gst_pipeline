#ifndef _COMMON_HPP_
#define _COMMON_HPP_
#ifdef _WIN32
#include <process.h>
#define GETPID _getpid
#else
#include <unistd.h>
#define GETPID getpid
#endif

#define RESERVE(x) ( (void)x )

#endif  // _COMMON_HPP_