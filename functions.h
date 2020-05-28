/**
* @file functions.h
* @name Daniel Soares Carreira
* @number 2191215
* @name Vasco Pedro Neves
* @number 2191752
*/

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

int validOptions(int argc, char *argv[], struct gengetopt_args_info *args);
int validMode(int mode);
int validSearch(char *hex);
int correctHexSize(char *hex, int tam);
int checkHex(char *hex, int tam);
int mainFunction(struct gengetopt_args_info args, clock_t begin);
int validDiscrete(char **args, unsigned int numArgs, unsigned long long tamMax);
void printSearch(char *searchArg, char *filename, unsigned int *offsets, unsigned int numOffsets);
void printInfo(unsigned int valor, unsigned int mode);
unsigned long freeSpaceSize(unsigned long max, unsigned int mode);

#endif
