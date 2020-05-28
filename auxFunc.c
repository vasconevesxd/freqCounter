/**
* @file auxFunc.c
* @name Daniel Soares Carreira
* @number 2191215
* @name Vasco Pedro Neves
* @number 2191752
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "args.h"
#include "auxFunc.h"

//Function returns the maximum value that can be read depending on the mode received
unsigned int maxMode(int mode) {
	if (mode == 1)
		return (UMBYTE-1);
	if (mode == 2)
		return (DOISBYTES-1);
	
	return (QUATROBYTES-1);
}

//Function to go through the entire string and check if it consists only of decimal characters
int validNumber(char *strNum) {
	for (unsigned int i = 0; i < strlen(strNum); ++i) {
		if (!isdigit(strNum[i])) {
			fprintf(stderr,"ERROR: '%s' is not a integer.\n",strNum);
			return 0;
		}
	}
	return 1;
}

//Function of the Bubble Sort sorting algorithm but with some changes to be compatible with the project
void BubbleSort(char **a, unsigned int array_size){
    unsigned int i, j;
    char *aux = NULL;
    for (i = 0; i < (array_size - 1); ++i)
    {
        for (j = 0; j < array_size - 1 - i; ++j )
        {
            if (strtoul(a[j], NULL, 10) > strtoul(a[j+1], NULL, 10))
            {
                aux = a[j+1];
                a[j+1] = a[j];
                a[j] = aux;
            }
        }
    }
}
