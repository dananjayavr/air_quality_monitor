//
// Created by Dananjaya RAMANAYAKE on 18/11/2023.
//

#ifndef _DEBUG_H
#define _DEBUG_H

//Dependencies
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "config/config.h"

#define PRIX8        "hhX"
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

//Trace level definitions
#define TRACE_LEVEL_OFF      0
#define TRACE_LEVEL_FATAL    1
#define TRACE_LEVEL_ERROR    2
#define TRACE_LEVEL_WARNING  3
#define TRACE_LEVEL_INFO     4
#define TRACE_LEVEL_DEBUG    5
#define TRACE_LEVEL_VERBOSE  6

//Default trace level
#ifndef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_DEBUG
#endif

//Trace output redirection
#ifndef TRACE_PRINTF
#define TRACE_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef TRACE_ARRAY
#define TRACE_ARRAY(p, a, n) debugDisplayArray(stderr, p, a, n)
#endif

//Debugging macros
#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
#define TRACE_FATAL(...) TRACE_PRINTF(__VA_ARGS__)
#define TRACE_FATAL_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_FATAL(...)
   #define TRACE_FATAL_ARRAY(p, a, n)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
#define TRACE_ERROR(...) TRACE_PRINTF(__VA_ARGS__)
#define TRACE_ERROR_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_ERROR(...)
   #define TRACE_ERROR_ARRAY(p, a, n)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
#define TRACE_WARNING(...) TRACE_PRINTF(__VA_ARGS__)
#define TRACE_WARNING_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_WARNING(...)
   #define TRACE_WARNING_ARRAY(p, a, n)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
#define TRACE_INFO(...) TRACE_PRINTF(__VA_ARGS__)
#define TRACE_INFO_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_INFO(...)
   #define TRACE_INFO_ARRAY(p, a, n)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
#define TRACE_DEBUG(...) TRACE_PRINTF(__VA_ARGS__)
#define TRACE_DEBUG_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_DEBUG(...)
   #define TRACE_DEBUG_ARRAY(p, a, n)
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
#define TRACE_VERBOSE(...) TRACE_PRINTF(__VA_ARGS__)
   #define TRACE_VERBOSE_ARRAY(p, a, n) TRACE_ARRAY(p, a, n)
#else
#define TRACE_VERBOSE(...)
#define TRACE_VERBOSE_ARRAY(p, a, n)
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//Debug related functions
void debugInit(uint32_t baudrate);

void debugDisplayArray(FILE *stream,
                       const char *prepend, const void *data, size_t length);

//Deprecated definitions
#define TRACE_LEVEL_NO_TRACE TRACE_LEVEL_OFF

//C++ guard
#ifdef __cplusplus
}
#endif

#endif //_DEBUG_H
