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
	int c, i, j, k, offset;
	int line = 1;			/* guarda o valor da linha no arquivo */
	int ultvar = -1;		/* guarda o indice da ult var local criada */
	int pos = 0;         	/* conta a ultima pos no vetor cod preenchida */
	int posifaux = 0;		/* inteiro auxiliar para calculo do offset if */					
	unsigned char *cod; 	/* vetor com as instrucoes de maquina */
	char ifcontrol[50];		/* vetor auxiliar onde se ifcontrol[i] == 1, tem if na linha (i+1) */
	int linecontrol[50];	/* vetor auxiliar que diz a pos no vetor cod da (i-esima+1) linha */
	unsigned char prep[] = {0x55, 0x48, 0x89, 0xE5, 0xC9, 0xC3};  
							/* equivalente a prepara pilha no (4 prim bytes), leave(4) e ret(5) */

	cod = (unsigned char*)malloc(sizeof(unsigned char)*1024);

	if(cod==NULL){
		printf("Erro alocacao de memoria.\n");
		exit(-1);
	}

	for(i=0;i<4;i++)
		cod[i]=prep[i];     /* inicia vetor com pushq %rbp e movq %rsp, %rbp */
	pos = i-1;

	for(i=0;i<50;i++)
		ifcontrol[i] = 0;
	
	while ((c = fgetc(f)) != EOF){
		linecontrol[line-1] = (pos+1); 	/* linha comeca pos seguinte a ultima preenchida */
		switch (c){
			case 'r': {  				/* retorno - 'ret' varpc */
				int idx;
				char var;
				if (fscanf(f, "et %c%d", &var, &idx) != 2) 
					error("comando invalido", line);
				if (var != '$'){
					checkVarP(var, idx, line);

					if(var == 'p')  /* retorna um parametro */{
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
					else{        /* retorna uma var. local */
						cod[pos+1]= 0x8B;
						cod[pos+2]= 0x45;
						pos += 2;

						cod[pos+1]= ((idx+1)*(-4)); /* pos na pilha como signed */
						pos++;

					}

				} 
				else{           /* retorna uma constante */
					cod[pos+1] = 0xB8;
					pos++;

					for(i=0;i<4;i++){
						cod[pos+1]=(unsigned char) (idx >> (8*i));  /* preenche em Little Endian */
						pos++;
					}
				}
				
				for(i=4;i<6;i++){
					cod[pos+1]=prep[i];   /* finaliza o vetor com leave e ret */
					pos++;
				}
				
				break;
			}

			case 'i': {  				/* if - 'if' var n1 n2 n3 */
				int idx, n1, n2, n3;
				char var;
				if (fscanf(f, "f %c%d %d %d %d", &var, &idx, &n1, &n2, &n3) != 5)
					error("comando invalido", line);
				if (var != '$'){
					checkVar(var, idx, line);
					
					/* adiciona o cmpl $0, var */
					cod[pos+1] = 0x83;
					cod[pos+2] = 0x7D;
					cod[pos+3] = ((idx+1)*(-4)); /* pos na pilha como signed */
					cod[pos+4] = 0x00;
					pos+=4;

					/* adiciona jl e coloca onde ficara o offset a linha para onde deve pular */
					cod[pos+1]= 0x0F;
					cod[pos+2]= 0x8C;
					cod[pos+3]= n1;
					pos+=6;

					/* adiciona je e coloca onde ficara o offset a linha para onde deve pular */
					cod[pos+1]= 0x0F;
					cod[pos+2]= 0x84;
					cod[pos+3]= n2;
					pos+=6;

					/* adiciona jl e coloca onde ficara o offset a linha para onde deve pular */
					cod[pos+1]= 0x0F;
					cod[pos+2]= 0x8F;
					cod[pos+3]= n3;
					pos+=6;

					ifcontrol[line-1] = 1; /* seta o ifcontrol a 1 para dizer que tem if na linha line */
				}
				break;
			}

			case 'v': {					/* atribuicao - var '=' varpc op varpc */
				int idx0, idx1, idx2;
				char var0 = c, var1, var2;
				char op;
				if (fscanf(f, "%d = %c%d %c %c%d", &idx0, &var1, &idx1, &op, &var2, &idx2) != 6)
					error("comando invalido", line);
				checkVar(var0, idx0, line);
				if (var1 != '$'){
					checkVarP(var1, idx1, line);
					if(var1 == 'p'){	/* 1o eh parametro */
						cod[pos+1] = 0x89;
						pos++;
						switch (idx1) {
							case 0: {
								cod[pos+1] = 0xF8;
								break;
							}
							case 1: {
								cod[pos+1] = 0xF0;
								break;
							}
							case 2: {
								cod[pos+1] = 0xD0;
								break;
							}
						}
						pos++;
					}
					else{					/* 1o eh varlocal */
						cod[pos+1] = 0x8B;
						cod[pos+2] = 0x45;
						cod[pos+3] = ((idx1+1)*(-4));
						pos += 3;
					}
				}
				else{ 						/* 1o eh constante */
					cod[pos+1] = 0xB8;
					pos++;
					
					for (i=0; i<4; i++){
						cod[pos+1] = (unsigned char) (idx1 >> (8*i)); /* preenche em Little Endian */
						pos++;
					}
				}

				if (var2 != '$'){
					checkVarP(var2, idx2, line);
					if(var2 == 'p'){		/* 2o eh parametro */
						cod[pos+1] = 0x89;
						pos++;
						switch (idx2){
							case 0:{
								cod[pos+1] = 0xF9;
								break;
							}
							case 1:{
								cod[pos+1] = 0xF1;
								break;
							}
							case 2:{
								cod[pos+1] = 0xD1;
								break;
							}
						}
						pos++;
					}
					else{					/* 2o eh varlocal */
						cod[pos+1] = 0x8B;
						cod[pos+2] = 0x4D;
						cod[pos+3] = ((idx2+1)*(-4));
						pos += 3;
					}
				}
				else{						/* 2o eh constante */
					cod[pos+1] = 0xB9;
					pos++;
					
					for (i=0; i<4; i++){
						cod[pos+1] = (unsigned char) (idx2 >> (8*i)); /* preenche em Little Endian */
						pos++;
					}
				}

				/* implementacao dos casos de operacao */
				switch (op){
					case '+':{
						cod[pos+1] = 0x01;
						cod[pos+2] = 0xC8;
						pos += 2;
						break;
					}
					case '-':{
						cod[pos+1] = 0x29;
						cod[pos+2] = 0xC8;
						pos += 2;
						break;
					}
					case '*':{
						cod[pos+1] = 0x0F;
						cod[pos+2] = 0xAF;
						cod[pos+3] = 0xC1;
						pos += 3;
						break;
					}
				}

				if(idx0 > ultvar){ 		/* se for uma nova variavel */
					if (idx0%4 == 0){ 	/* se passar for multiplo de 4, quer dizer que tem que tirar mais 16 da pilha */
						/* subq $16, %rsp - {0x48, 0x83, 0xEC, 0x10} */
						cod[pos+1] = 0x48;
						cod[pos+2] = 0x83;
						cod[pos+3] = 0xEC;
						cod[pos+4] = 0x10;
						pos += 4;
					}
					ultvar++;
				}

				/* implementacao de mover %eax para var local lembrando que pospilha = (idx+1)*(-4) */
				cod[pos+1] = 0x89;
				cod[pos+2] = 0x45;
				cod[pos+3] = ((idx0+1)*(-4));
				pos += 3;
				break;
			}
			default: error("comando desconhecido", line);
		}
		line ++;
		fscanf(f, " ");
	}

	for(i=0;i<line;i++){
		if(ifcontrol[i]){
			posifaux = linecontrol[i]; /* posicao no vetor cod que comeca o if*/
			posifaux += 4;

			for(k=0;k<3;k++){
				posifaux+=2; /* pula para prox buraco do if */
				offset = (linecontrol[cod[posifaux]-1] - posifaux)-1;
				if(offset<0)
					offset = offset-4;
							/* posicao da linha que esta no cod atual menos a pos atual */
							/* -1 pois a atual nao conta no offset (apenas distancia)   */
				for (j=0; j<4; j++){
						cod[posifaux] = (unsigned char) (offset >> (8*j)); /* preenche em Little Endian */
						posifaux++;
				}
			}
		}
	} 

	printf("Vetor Final: ");
	for(i=0;i<pos+1;i++)
		printf("%0x ",cod[i]);
	printf("\n");

	return (funcp) cod; 
}

void libera (void *p){
	free(p);

	return;
}
