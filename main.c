#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compila.c"

int main (int argc, char*argv[]) {
	FILE *input;
	funcp SB;
	int retorno;
	
	if (argc < 2) {
		printf ("%s filename\n", argv[0]);
    	exit(1);
	}
	input = fopen(argv[1], "r");
	if (input == NULL) {
		printf("Erro ao abrir arquivo!\n");
		exit(2);
	}
	
	SB = compila(input);
	
	switch(argc) {
		case 2:{
			retorno = SB();
			break;
		}
		case 3:{
			retorno = SB(atoi(argv[2]));
			break;
		}
		case 4:{
			retorno = SB(atoi(argv[2]),atoi(argv[3]));
			break;
		}
		case 5:{
			retorno = SB(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
			break;
		}
	} 

	printf("Valor de Retorno = %d\n", retorno);
	
	libera(SB);
	
	return 0;
}
