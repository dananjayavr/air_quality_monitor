//
// Created by Dananjaya RAMANAYAKE on 18/11/2023.
//

//Dependencies
#include <stdio.h>
#include "debug.h"


//Debug related functions
void debugInit(uint32_t baudrate) {

    // NOT IMPLEMENTED
    UNUSED(baudrate);
}

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(FILE *stream,
                       const char *prepend, const void *data, size_t length)
{
    UNUSED(stream);

    unsigned int i;

    //Dump the contents of the array
    for(i = 0; i < length; i++)
    {
        //Beginning of a new line?
        if((i % 16) == 0)
        {
            TRACE_PRINTF("%s", prepend);
        }

        //Display current data byte
        TRACE_PRINTF("%02" PRIX8 " ", *((const uint8_t *) data + i));

        //End of current line?
        if((i % 16) == 15 || i == (length - 1))
        {
            TRACE_PRINTF("\r\n");
        }
    }
}
