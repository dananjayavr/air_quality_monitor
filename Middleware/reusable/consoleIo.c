// Console IO is a wrapper between the actual in and output and the console code
// In an embedded system, this might interface to a UART driver.

#include "consoleIo.h"

extern UART_HandleTypeDef huart2;

uint8_t ch = 0; // received character
uint8_t rx_buffer[256]; // buffer for received commands
uint8_t rx_counter = 0; // counter to keep track of buffer position
uint8_t buffer_ready = 0; // flag to determine if a complete command has been received

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Transmit(huart,&ch,1,HAL_MAX_DELAY); // echo

    //TODO: handle cases of buffer overflow gracefully
    rx_buffer[rx_counter++] = ch; // load each character to buffer
    if(ch == '\r' || ch == '\n')
        buffer_ready = 1; // if a new-line character or a carriage return character is received, set the relevant flag

    HAL_UART_Receive_IT(huart, &ch, 1); // continue receiving character(s)
}

eConsoleError ConsoleIoInit(void)
{
    HAL_UART_Receive_IT(&huart2, &ch, 1); // initiate reception
    return CONSOLE_SUCCESS;
}
eConsoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength, uint32_t *readLength)
{
    UNUSED(bufferLength);

    if(buffer_ready) { // if a complete command is received
        buffer_ready = 0; // unset flag
        memcpy(buffer,rx_buffer,rx_counter); // copy the received command to library buffer
        *readLength = rx_counter; // set appropriate length

        memset(rx_buffer,0,256); // clear receive buffer
        rx_counter = 0; // clear pointer so we can start from the beginning next time around
    } else {
        buffer[0] = '\n'; // if no command is received, send the '>'
        *readLength = 0;
    }

    return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSendString(const char *buffer)
{
    uint8_t c = 0;
    while (*buffer != '\0') {
        c = *buffer;
        HAL_UART_Transmit(&huart2,&c,1,HAL_MAX_DELAY);
        c = 0;
        buffer++;
    }

	return CONSOLE_SUCCESS;
}

