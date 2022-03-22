#ifndef AT_CMD_PROCESSING_H
#define AT_CMD_PROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum input_type
{
    Reverse_Cmd,
    Size_Cmd,
    Mult_Cmd,
    Input_Error 
};

static const char* REVERSE_COMMAND = "REVERSE";
static const char* SIZE_COMMAND    = "SIZE";
static const char* MULT_COMMAND    = "MULT";

enum input_type checkInput(char *buff, int buff_len, int* return_index);
enum input_type checkNumCode(char *command);
void            reverse_str(char* buff, int equalsIndex, int len, char* output, int* error);
size_t          size(char* msg, int equalsIndex, int len, int* error);
int             mult(char* msg, int equalsIndex, int len, int* error);

#endif
