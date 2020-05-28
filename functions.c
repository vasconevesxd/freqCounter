/**
* @file functions.c
* @name Daniel Soares Carreira
* @number 2191215
* @name Vasco Pedro Neves
* @number 2191752
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>

#include "memory.h"
#include "args.h"
#include "functions.h"
#include "auxFunc.h"

int validOptions(int argc, char *argv[], struct gengetopt_args_info *args){
	struct stat stats;
	
	//Verify if the received arguments are valid
	if (cmdline_parser(argc, argv, args) != 0){
		return 0;
	}
	
	//Verify if the option '--file' or '--dir' was received
	if (!args->file_given && !args->dir_given) {
		fprintf(stderr,"ERROR: the program must receive -f/--file or -d/--dir.\n");
		return 0;
	}
	
	//Validate the '--file' arguments
	if (args->file_given) {
		for (unsigned int i = 0 ; i < args->file_given ; ++i) {
			//If the argument is a non existing file
			if (access(args->file_arg[i], F_OK) == -1){
				fprintf(stderr, "ERROR: the file '%s' do not exist.\n", args->file_arg[i]);
				return 0;
			}
			stat(args->file_arg[i], &stats);
			//If the argument is a directory.
			if (S_ISDIR(stats.st_mode)) {
				fprintf(stderr,"ERROR: '%s' is a directory.\n",args->file_arg[i]);
				return 0;
			}
		}
	}
	else {
		args->file_arg = MALLOC(sizeof(char *));
	}
	
	//Validate is the option '--compact' was combined with '--discrete' and/or '--search'
	if(args->compact_given && (args->discrete_given != 0 || args->search_given != 0)) {
		fprintf(stderr,"ERROR: the option -c/--compact is not compatible with \"--discrete\" neither with \"-s/--search\".\n");
		return 0;
	}
	
	//Validate the argument of the option '--mode'
	if (args->mode_given) {
		if (!validMode(args->mode_arg))
			return 0;
	}
	else {
		//If mode is not define, set default value of 1
		args->mode_arg = 1;
	}
	
	//Validate the argument of '--dir'
	if (args->dir_given) {
		stat(args->dir_arg, &stats);
		
		if (!S_ISDIR(stats.st_mode)) {
			fprintf(stderr,"ERROR: directory ‘%s’ do not exist.\n",args->dir_arg);
			return 0;
		}
	}
	
	//Validate the argument of '--search'
	if (args->search_given) {
		if (args->compact_given || args->discrete_given || args->mode_given) {
			fprintf(stderr,"ERROR: the option -s/--search is not compatible with \"--discrete\", \"-c/--compact\" and \"-m/--mode\".\n");
			return 0;
		}
		if (!validSearch(args->search_arg))
			return 0;
	}
	
	//Validate the arguments of '--discrete'
	if (args->discrete_given) {
		unsigned long long tamMax = maxMode(args->mode_arg);
		//Verify if the argumets are valid
		if (!validDiscrete(args->discrete_arg, args->discrete_given, tamMax)) {
			return 0;
		}
		
		//Make the vector sorted
		BubbleSort(args->discrete_arg, args->discrete_given);
	}
	return 1;
}

int validDiscrete(char **args, unsigned int numArgs, unsigned long long tamMax) {
	for (unsigned int i=0 ; i < numArgs ; i++) {
		//Verify if it is a numeber
		if (!validNumber(args[i])) {
			return 0;
		}
		
		//Verify if the number is in range
		unsigned long long val = strtoull(args[i], NULL, 10);
		if (val > tamMax) {
			fprintf(stderr,"ERROR: '%llu' is not in range [0, %llu].\n",val,tamMax);
			return 0;
		}	
	}
	return 1;
}

int validMode(int mode) {
	if (mode != 1 && mode != 2 && mode != 4) {
		fprintf(stderr,"ERROR: invalid value ‘%d’ for -m/--mode.\n",mode);
		return 0;
	}
	return 1;
}

int validSearch(char *hex) {
	int tam = strlen(hex);
	
	//Convert all of the caracters to lowercase
	for (int i = 1 ; i < tam ; ++i) {
		hex[i] = tolower(hex[i]);
	}
	
	//Verify if the argument have '0x'
	if(hex[0] != '0' || hex[1] != 'x') {
		fprintf(stderr,"freqCounter:invalid value ‘%s’ for -s/--search (needs to be specified in HEX format)\n", hex);
		return 0;
	}
	
	return correctHexSize(hex, tam);
}

int correctHexSize(char *hex, int tam) {
	//Verify if the argument have the correct size of bytes
	if(tam < 4 || tam >  66 || tam % 2 == 1) {
		fprintf(stderr,"freqCounter:invalid value ‘%s’ for -s/--search (needs to be an integer value of bytes)\n", hex);
		return 0;
	}
	return checkHex(hex, tam);
}

int checkHex(char *hex, int tam) {
	//Verify if all the caracters are hexadecimal
	for (int i = 2 ; i < tam ; ++i) {
		if (!isxdigit(hex[i])) {
			fprintf(stderr,"ERROR: '%s' is not a hexadecimal number.\n", hex);
			return 0;
		}
	}
	return 1;
}

int mainFunction(struct gengetopt_args_info args, clock_t begin) {
	FILE *fd = NULL, *output = NULL;
	unsigned int *bytesMode = NULL;
	size_t size_to_read, tam_bytes = 0;
	unsigned int valDec, sum, numOffsets, numBytesSearch, off, numFilesDir = 0, *offsets = NULL, *numSearch = NULL;
	char aux[2];
	long long int min = 0, max = 0, size_vetor;
	
	//Redirect the stdout to the file descriptor of the output
	if (args.output_given){
		output = freopen(args.output_arg, "w", stdout);
		if (output == NULL) {
			fprintf(stderr,"ERROR: data could not be saved in '%s'.\n",args.output_arg);
			return 1;
		}
	}
	
	//Fill the vector 'numSearch' with the decimal value of each byte of the search argument
	if (args.search_given) {
		//Number bytes of the search argument
		numBytesSearch = (strlen(args.search_arg)-2)/2;
		
		numSearch = MALLOC(sizeof(unsigned int)*numBytesSearch);
		
		for (unsigned int n = 0; n < numBytesSearch; ++n) {
			aux[0] = args.search_arg[n*2+2];
			aux[1] = args.search_arg[n*2+3];
			//Save in position 'n' of the vector 'numSearch', the decimal value of the pair of hexadecimal (Ex: 0xA2 = 162)
			numSearch[n] = (unsigned int)strtol(aux, NULL, 16);
		}
	}
	if (args.dir_given){
		struct stat stats;
		struct dirent *files;
		size_t size;
		char filePath[150];
		
		//Opens the directory received in dir argument
		DIR *folder = opendir(args.dir_arg);
		if(folder == NULL) {
			fprintf(stderr,"ERROR: cannot access directory ‘%s’.\n",args.dir_arg);
			return 1;
		}
		
		//For each entity in directory
		while ((files = readdir(folder))) {
			//If the entity is different from '.'(current directory), '..'(parent directory) and '~' in the end (temporary file)
			if (files->d_name[0] != '.' && files->d_name[1] != '.' && files->d_name[strlen(files->d_name)-1] != '~') {
				strcpy(filePath,args.dir_arg);
				if (args.dir_arg[strlen(args.dir_arg)-1] != '/')
					strncat(filePath,"/",1);
				strcat(filePath,files->d_name);
				stat(filePath, &stats);
				//If it is a directory skip to the next itenaration
				if (S_ISDIR(stats.st_mode))
					continue;
				numFilesDir++;
				//Reserve memory to insert the file
				args.file_arg = realloc(args.file_arg,(numFilesDir + args.file_given) * sizeof(char *));
				if (args.file_arg != NULL){
					//Calcule and reserve the necessary space for file name
					size = strlen(filePath) + 1;
					args.file_arg[numFilesDir+args.file_given-1] = MALLOC(size);
					if (args.file_arg[numFilesDir+args.file_given-1] == NULL){
						return 1;
					}
					strcpy(args.file_arg[numFilesDir+args.file_given-1],filePath);
				}
			}
		}
		closedir(folder);
		//Update the file counter
		args.file_given = args.file_given + numFilesDir;
	}
	//For each file
	for (unsigned int i = 0 ; i < args.file_given ; ){
		//Will save in size_vetor the amount to reserve
		size_vetor = freeSpaceSize(max,args.mode_arg);
		//max is the maximum value to read from file
		max += size_vetor;
		
		//Will only reset sum and numOffsets one time per file
		if (min == 0) {
			sum = 0;
			numOffsets = 0;
		}
		//Opens the file on read mode
		fd = fopen(args.file_arg[i],"rb");
		
		if (fd == NULL){
			printf("ERROR:'%s': CANNOT PROCESS FILE\n",args.file_arg[i]);
			printf("----------\n");
			continue;
		}
		if (!args.search_given) {
			bytesMode = calloc(size_vetor,sizeof(unsigned int));
			if (bytesMode == NULL) return 1;
		}
		
		//Will only inicialize the variables one time per file
		if (min == 0) {
			offsets = MALLOC(sizeof(unsigned int));
			if (offsets == NULL) return 1;
			
			//Calcule the size of the file
			fseek(fd, 0, SEEK_END);
			tam_bytes = (unsigned int)ftell(fd);
			fseek(fd, 0, SEEK_SET);
			
			//Calcule the size of the file to read
			size_to_read = tam_bytes - (tam_bytes % args.mode_arg);
		}
		
		//While the reading of the file is not finished
		for (unsigned int n = 0, j = 0 ; j < size_to_read ; j += args.mode_arg) {
			valDec = 0;
			if (n == 0)
				off = ftell(fd);
			
			//Read the byte(s) from file
			fread(&valDec, args.mode_arg, 1, fd);
			
			//If the option '--search' if given
			if (args.search_given) {
				// If the 'n' byte of the search argument is equal to the decimal value read then it will check the next one, otherwise it will return to the  
				//initial byte of the search
				n = (numSearch[n] == valDec) ? n + 1 : 0 ;
				
				//Then the search sequence is checked
				if (n == numBytesSearch) {
					n = 0;
					offsets = realloc(offsets, (numOffsets + 1) * sizeof(unsigned int));
					if (offsets == NULL) {
						fprintf(stderr,"ERROR: not enough memory.\n");
						return 1;
					}
					offsets[numOffsets] = off;
					numOffsets++;
				}
				continue;
			}
			//Save on vector the values between min and max
			if ((valDec >= min) & (valDec < max)) {
				bytesMode[valDec-min]++;
			}
			
		}
		
		//Shows the information related to the search and continues to the file to follow if it exists
		if (args.search_given) {
			printSearch(args.search_arg, args.file_arg[i], offsets, numOffsets);
			FREE(offsets);
			printf("----------\n");
			i++;
			fclose(fd);
			continue;
		}
		
		if (min == 0) {
			if (!args.compact_given) {
				//Show the message after all the files in the '--file' option, if any
				if (i == (args.file_given-numFilesDir)) {
					printf("DIR:'%s'\n",args.dir_arg);
				}
				printf("freqCounter:'%s':%u bytes\n",args.file_arg[i],tam_bytes);
			}
			else {
				printf("%s:%ubytes:",args.file_arg[i],tam_bytes);
			}
		}
		
		if (!args.discrete_given) {
			for (unsigned int f = 0 ; f < size_vetor ; ++f) {
				//Shows the results of the '--compact' option
				if (bytesMode[f] != 0) {
					//Calculates the sum by adding all counters
					sum = sum + bytesMode[f];
					if (args.compact_given){
						printf("%u",bytesMode[f]);
						continue;
					}
					
					//Shows the results of the remaining options
					printInfo(f+min, args.mode_arg);
					printf("%u\n",bytesMode[f]);
				}
			}
			//Show the sum
			if (max == pow(2, 8 * args.mode_arg)) {
				if (!args.compact_given) {
					printf("sum:%u\n", sum);
				}
				else {
					printf(":%u\n", sum);
				}
			}
		}
		else {
			//Displays information about the '--discrete' option
			for (unsigned int l = 0 ; (unsigned int) l < args.discrete_given ; ++l) {
				unsigned int discreteArg = (unsigned int)strtoul(args.discrete_arg[l], NULL, 10);
				if ((discreteArg >= min) & (discreteArg < max)) {
					printInfo(discreteArg, args.mode_arg);
					printf("%u\n",bytesMode[discreteArg-min]);
				}
			}
		}
		FREE(bytesMode);
		fclose(fd);
		//If the data was all showed
		if (max == pow(2, 8 * args.mode_arg)) {
			//Skip to the next file
			i++;
			//Reset min and max value
			max = 0;
			printf("----------\n");
		}
		min = max;
	}
	//Free the allocated memory for the search option
	if (args.search_given)
		FREE(numSearch);
		
	//End of timer, and presentation of the same
	if (args.time_given){
		clock_t end = clock();
		float time_spent = (float)(end - begin) / CLOCKS_PER_SEC;
		printf("time: %.6f seconds\n", time_spent);
	}
	
	//Closes the output discretion, if given
	if (args.output_given)
		fclose(output);
		
	return 0;
}

void printSearch(char *searchArg, char *filename, unsigned int *offsets, unsigned int numOffsets) {
	printf("freqCounter:looking for '%s' in '%s'\n", searchArg, filename);
			
	if (numOffsets == 0)
		printf("0 results found\n");
			
	for (unsigned int k = 0; k < numOffsets; ++k)
		printf("#%d: offset: 0x%x\n",k+1,offsets[k]);
}

void printInfo(unsigned int valor, unsigned int mode) {
	if (mode==1){
		printf("byte %03u:",valor);
	}
	else if (mode==2) {
		printf("bi-byte %05u:",valor);
	}
	else {
		printf("quad-byte %010u:",valor);
	}
}

unsigned long freeSpaceSize(unsigned long max, unsigned int mode) {
	int i;
	unsigned long exp = 0;
	unsigned int *aux = NULL;
	//While exp value is lower than the max value for the mode
	for (i = 2 ; exp < pow(2, 8 * mode) ; ++i) {
		//Will reserve 'exp' blocks of sizeof(unsigned int)
		exp = pow(2, i);
		aux = malloc(exp * sizeof(unsigned int));
		if (aux == NULL) {
			//If the available amount of memory is bigger than the necessary
			if (pow(2, i-1) < (pow(2, 8 * mode) - max))
				//Return the amount that's available
				return pow(2, i-1);
			//Return the amount that's necessary
			return (pow(2, 8 * mode) - max);
		}
		FREE(aux);
	}
	//Return the amount that's necessary
	return (pow(2, 8 * mode) - max);
}
