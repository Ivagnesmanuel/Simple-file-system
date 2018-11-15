#ifndef __ERROR_H__ // accorgimento per evitare inclusioni multiple di un header
#define __ERROR_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)


#endif

// this is used for hardware/software failure to close the progam (or bad parametres)
// in case of errors on bitmap and similar, to check on simple fs => return -1