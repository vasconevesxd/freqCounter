/**
* @file main.c
* @name Daniel Soares Carreira
* @number 2191215
* @name Vasco Pedro Neves
* @number 2191752
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "args.h"
#include "auxFunc.h"
#include "functions.h"


int main(int argc, char *argv[]){
	//Inicialize the timer
	clock_t begin = clock();
	
	struct gengetopt_args_info args;
	
	//Validation of the input parameters
	if (!validOptions(argc, argv, &args)) {
		exit(1);
	}
	
	//Call the main function
	if (mainFunction(args, begin)) {
		exit(1);
	}
	
	//Write the error message to the stdout if --output is present
	if (args.output_given) {
		freopen ("/dev/tty", "a", stdout);
		printf("INFO:output written to \"%s\"\n",args.output_arg);
	}
	
	return 0;
}
