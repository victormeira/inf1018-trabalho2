#include<stdio.h>
#include<stdlib.h>

typedef int (*funcp) ();

static void error (const char *msg, int line) {
	printf("erro %s na linha %d\n", msg, line);
	exit(EXIT_FAILURE);
}

void checkVar(char var, int idx, int line) {
	switch (var) {
		case 'v':
			if ((idx < 0) || (idx > 19))
			 error("operando invalido", line);
			break;
		default:
			 error("operando invalido", line);
	 }
}
		 
void checkVarP(char var, int idx, int line) {
	switch (var) {
		case 'v':
			if ((idx < 0) || (idx > 19))
			 error("operando invalido", line);
			break;
		case 'p':
			if ((idx < 0) || (idx > 2))
			 error("operando invalido", line);
			break;
		default:
			 error("operando invalido", line);
	 }
}

funcp compila (FILE *f){
	int line = 1;
	int c;
	int i;
	int pos = 0;         /* conta a ultima pos no vetor cod preenchida */					
	unsigned char *cod; 	/* vetor com as instrucoes de maquina */
	unsigned char *aux;  /* vetor auxiliar */
	unsigned char prep[] = {0x55, 0x48, 0x89, 0xE5, 0xC9, 0xC3};  /* equivalente a prepara pilha no (4 prim bytes), leave(4) e ret(5) */

	cod = (unsigned char*)malloc(sizeof(unsigned char)*4);

	if(cod==NULL)
	{
		printf("Erro alocacao de memoria");
		exit(-1);
	}

	for(i=0;i<4;i++,pos++)
		cod[i]=prep[i];     /* inicia vetor com pushq %rbp e movq %rsp, %rbp */

	while ((c = fgetc(f)) != EOF) {
		switch (c) {
			case 'r': {  				/* retorno - 'ret' varpc */
				int idx;
				char var;
				if (fscanf(f, "et %c%d", &var, &idx) != 2) 
					error("comando invalido", line);
				if (var != '$')
				{
					checkVarP(var, idx, line);

					if(var == 'p')  /* retorna um parametro */
					{
						cod = (unsigned char*)realloc(cod,((pos+1)+2)*sizeof(unsigned char));  /* adiciona mais 2 pos no vetor */
						if(cod==NULL)
						{
						 printf("Erro realocacao de memoria");
						 exit(-2);
						}

						cod[pos+1]= 0x89;
						pos++;

						if(idx == 0)
							cod[pos+1]= 0xF8;
						else
							if(idx == 1)
								cod[pos+1]= 0xF0;
							else
								cod[pos+1]= 0xD0;
						pos++;
					}
					else            /* retorna uma var. local */
					{
						cod = (unsigned char*)realloc(cod,((pos+1)+3)*sizeof(unsigned char));  /* adiciona mais 3 pos no vetor */
						if(cod==NULL)
						{
						 printf("Erro realocacao de memoria");
						 exit(-2);
						}

						cod[pos+1]= 0x8B;
						cod[pos+2]= 0x45;
						pos = pos + 2;

						cod[pos+1]= (idx*(-4)); /* pos na pilha como signed */
						pos++;

					}

				} 
				else              /* retorna uma constante */
				{
					cod = (unsigned char*)realloc(cod,((pos+1)+5)*sizeof(unsigned char));  /* adiciona mais 5 pos no vetor */
					if(cod==NULL)
					{
						printf("Erro realocacao de memoria");
						exit(-2);
					}

					cod[pos+1] = 0xB8;
					pos++;

					for(i=0;i<4;i++,pos++)
						cod[pos+1]=(unsigned char) idx >> 8*i;  /* preenche em Little Endian */
				}
				/* implementação parte do retorno */

				cod = (unsigned char*)realloc(cod,((pos+1)+2)*sizeof(unsigned char));  /* adiciona mais 2 pos no vetor */
				if(cod==NULL)
				{
					printf("Erro realocacao de memoria");
					exit(-2);
				}

				for(i=4;i<6;i++,pos++)
					cod[pos+1]=prep[i];   /* finaliza o vetor com leave e ret */

				return (funcp)cod;
			}
			case 'i': {  				/* if - 'if' var n1 n2 n3 */
				int idx, n1, n2, n3;
				char var;
				if (fscanf(f, "f %c%d %d %d %d", &var, &idx, &n1, &n2, &n3) != 5)
					error("comando invalido", line);
				if (var != '$') 
					checkVar(var, idx, line);
				
				/* implementação parte do if */

				break;
			}
			case 'v': {  				/* atribuicao - var '=' varpc op varpc */
				int idx0, idx1, idx2;
				char var0 = c, var1, var2;
				char op;
				if (fscanf(f, "%d = %c%d %c %c%d",
									 &idx0, &var1, &idx1, &op, &var2, &idx2) != 6)
					error("comando invalido", line);
				checkVar(var0, idx0, line);
				if (var1 != '$') 
					checkVarP(var1, idx1, line);
				if (var2 != '$') 
					checkVarP(var2, idx2, line);
				
				/* implementação parte da atribuição */

				break;
			}
			default: error("comando desconhecido", line);
		}
		line ++;
		fscanf(f, " ");
	}

	return NULL; /* se vier aqui, algo deu errado */
}

void libera (void *p){
	int i;
	unsigned char *cod;
	cod = (unsigned char*) p;

	for(i=0;cod[i]!=0xC3;i++) /* enquanto não é o código de retorno */
		free(p);

	free(p);                /* da free no último */

	return;
}