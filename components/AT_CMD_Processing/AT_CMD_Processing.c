#include "AT_CMD_Processing.h"

int getIndex(char* str, char key)
{
    char *e;
    e = strchr(str, key);
    return (int)(e - str);
}

enum input_type checkInput(char *buff, int buff_len, int* return_index)
{
    // printf("buffer = %s\n", buff);
    // printf("buff len = %d\n", buff_len);
    if(buff_len < 4)
        return Input_Error;

    //verificar se é comando AT
    char at_cmd[3];
    at_cmd[3] = '\0';
    strncpy(at_cmd ,buff, 3);
    at_cmd[3] = '\0';
    //printf("at_cmd = %s\n", at_cmd);
    int check = strcmp("AT+", at_cmd);
    if(check != 0) 
        return Input_Error; 

    printf("Verificado comando AT.\n");    
    int equalsIndex = getIndex(buff, '=');
    enum input_type num_code = Input_Error;
    if(equalsIndex < buff_len && equalsIndex > 3){
        int command_len = equalsIndex-3; 
        char command[equalsIndex-3];
        strncpy(command, buff+3, command_len);
        command[command_len] = '\0';
        num_code = checkNumCode(command);
        *return_index = equalsIndex;
    }
    return num_code;      
}

enum input_type checkNumCode(char *command)
{
    int reverse = strcmp(REVERSE_COMMAND, command);
    int size = strcmp(SIZE_COMMAND, command);
    int mult = strcmp(MULT_COMMAND, command);
    if(reverse == 0)
    {
        return Reverse_Cmd;
    }
    else if(size == 0)
    {
        return Size_Cmd;
    }
    else if(mult == 0)
    {
        return Mult_Cmd;
    }
    else
    {
        return Input_Error;
    }
}


void getString(char* buff, int equalsIndex, int len, char* returnStr, int* error){
    //verificar se string termina no =
    int quoteIndex = equalsIndex+1; 
    if(len == (quoteIndex) || buff[quoteIndex] != '"'){
        *error = 1;
    }else{
        int input_len = len-quoteIndex;
        if(input_len < 2){
            *error = 1;
        }else{
            char in[input_len];
            strncpy(in, buff+(quoteIndex+1), input_len); //elimina a primeira "
            in[input_len] = '\0';
            int secondQuoteIndex = getIndex(in, '"');
            if(secondQuoteIndex < 0 || secondQuoteIndex > input_len){
                *error = 1;
            }else{
                //Entrada ok
                strncpy(returnStr, in, secondQuoteIndex);
                returnStr[secondQuoteIndex] = '\0';
                *error = 0;
            }
        }
    }

}

void reverse_str(char* msg, int equalsIndex, int len, char* output, int* error)
{
    getString(msg, equalsIndex, len, output, error);
    if(*error == 1)
    {
        return;
    }
    else
    {
        int new_len = strlen(output);
        int i, temp;
        for(i = 0; i < new_len/2; i++)
        {
            temp = output[i];
            output[i] = output[new_len-i-1];
            output[new_len-i-1] = temp;
        }
    }
}

size_t size(char* msg, int equalsIndex, int len, int* error)
{
    size_t buffsize = len-8;
    char input[buffsize];
    getString(msg, equalsIndex, len, input, error);
    if(*error)
    {
        return 0;
    }
    else
    {
        return strlen(input);
    }
}

int getNumbers(char* buff, int startingIndex, int len, int* reachedComma, int* error){
    if(startingIndex == len){
        *error = 1;
        return -1;
    }else{
        int i;
        int empty = 1;
        for(i = startingIndex; i < len-1; i++){
            char character = buff[i];
            if(isdigit(character)){
                empty = 0;
                continue;
            }else if(isalpha(character)){
                *error = 1;
                return -1;
            }else{
                if(character == '.'){
                    *error = 1;
                    return -1;
                }else if(character == ',' && *reachedComma == 0 && empty == 0){
                    *reachedComma = i;
                    break;
                }else{
                    *error = 1;
                    return -1;
                }
            }
        }
        int num_len = i - startingIndex;
        char num_str[num_len];
        strncpy(num_str, buff+startingIndex, num_len);
        num_str[num_len] = '\0';
        return atoi(num_str);
    }
}

int mult(char* msg, int equalsIndex, int len, int* error)
{
    int reachedComma = 0;
    int a = getNumbers(msg, equalsIndex+1, len, &reachedComma, error);
    if(*error)
    {
        return -1;
    }
    else
    {
        int b = getNumbers(msg, reachedComma+1, len, &reachedComma, error);
        if(*error)
        {
            return -1;
        }
        else
        {
            return a * b;
        }
        
    }
}

// void writeToLog(char *log, int code){
//     FILE *fp;
//     time_t t = time(NULL);
//     struct tm tm = *localtime(&t);
//     char logfile_name[LOGFILE_NAME_SIZE];
//     sprintf(logfile_name, "log%02d-%02d-%d.log", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
//     if((fp = fopen(logfile_name , "a+")) == NULL) {
//         printf("Failed to open log file.\n");
//         return;
//     }else{
//         switch (code)
//         {
//         case REVERSE:
//             fprintf(fp, "[AT+REVERSE]\n");
//             fprintf(fp, "%02d:%02d:%02d:\t", tm.tm_hour, tm.tm_min, tm.tm_sec);
//             fprintf(fp, "%s", log);
//             break;
//         case SIZE:
//             fprintf(fp, "[AT+SIZE]\n");
//             fprintf(fp, "%02d:%02d:%02d:\t", tm.tm_hour, tm.tm_min, tm.tm_sec);
//             fprintf(fp, "%s", log);
//             break;    
//         case MULT:
//             fprintf(fp, "[AT+MULT]\n");
//             fprintf(fp, "%02d:%02d:%02d:\t", tm.tm_hour, tm.tm_min, tm.tm_sec);
//             fprintf(fp, "%s", log);
//             break;
//         default:
//             fprintf(fp, "[ERROR]\n");
//             fprintf(fp, "%02d:%02d:%02d:\t", tm.tm_hour, tm.tm_min, tm.tm_sec);
//             fprintf(fp, "%s", log);
//             break;
//         }
//     }
//     fclose(fp);   
// }