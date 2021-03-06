#include<stdio.h>
#include<stdlib.h>

typedef int (*funcp) ();

typedef struct retorno {
	char var;
	int idx;
} Retorno;

typedef struct atribuicao {
	char var1, var2, op;
	int idx0, idx1, idx2;
} Atribuicao;

typedef struct condicao {
	char var;
	int idx;
	int Lmenor, Ligual, Lmaior;
} Condicao;

static void error(const char *msg, int line);

void checkVar(char var, int idx, int line);

void checkVarP(char var, int idx, int line);

int escreveRet(unsigned char *cod, int posicao, int linha, Retorno ret);

int escreveIf (unsigned char *cod, int posicao, Condicao cond);

int escreveAtr (unsigned char *cod, int posicao, int linha, Atribuicao atr);

void preencheIf (unsigned char *cod, int linhas, char *ifcontrol, int *linecontrol);

funcp compila (FILE *f){
	int c, i;
	int line = 1;			/* guarda o valor da linha no arquivo */
	int pos = 0;         	/* conta a ultima pos no vetor cod preenchida */					
	unsigned char *cod; 	/* vetor com as instrucoes de maquina */
	char ifcontrol[50];		/* vetor auxiliar onde se ifcontrol[i] == 1, tem if na linha (i+1) */
	int linecontrol[50];	/* vetor auxiliar que diz a pos no vetor cod da (i-esima+1) linha */
	unsigned char prep[] = {0x55, 0x48, 0x89, 0xE5, 0x48, 0x83, 0xEC, 0x50};  
							/* equivalente a prepara pilha */
							/* FAZER subq $80, %rsp */
	Retorno ret;
	Atribuicao atr;
	Condicao cond;

	cod = (unsigned char*)malloc(sizeof(unsigned char)*1024);

	if(cod==NULL){
		printf("Erro alocacao de memoria.\n");
		exit(-1);
	}

	for(i=0;i<8;i++)
		cod[i]=prep[i];     /* inicia vetor com pushq %rbp e movq %rsp, %rbp */
	pos = i-1;

	for(i=0;i<50;i++)
		ifcontrol[i] = 0;
	
	while ((c = fgetc(f)) != EOF){
		linecontrol[line-1] = (pos+1); 	/* linha comeca pos seguinte a ultima preenchida */
		switch (c){
			case 'r': {  				/* retorno - 'ret' varpc */
				if (fscanf(f, "et %c%d", &ret.var, &ret.idx) != 2) 
					error("comando invalido", line);
				
				pos = escreveRet(cod, pos, line, ret);

				break;
			}

			case 'i': {  				/* if - 'if' var n1 n2 n3 */
				if (fscanf(f, "f %c%d %d %d %d", &cond.var, &cond.idx, &cond.Lmenor, &cond.Ligual, &cond.Lmaior) != 5)
					error("comando invalido", line);
				if (cond.var != '$'){
					checkVar(cond.var, cond.idx, line);
					
					pos = escreveIf(cod, pos, cond);

					ifcontrol[line-1] = 1; /* seta o ifcontrol a 1 para dizer que tem if na linha line */
				}
				break;
			}

			case 'v': {					/* atribuicao - var '=' varpc op varpc */
				if (fscanf(f, "%d = %c%d %c %c%d", &atr.idx0, &atr.var1, &atr.idx1, &atr.op, &atr.var2, &atr.idx2) != 6)
					error("comando invalido", line);

				pos = escreveAtr(cod, pos, line, atr);

				break;
			}
			default: error("comando desconhecido", line);
		}
		line ++;
		fscanf(f, " ");
	}

	preencheIf(cod,line,ifcontrol,linecontrol);

	return (funcp) cod; 
}

void libera (void *p){
	free(p);
	return;
}

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

int escreveRet(unsigned char *cod, int posicao, int linha, Retorno ret) {
	int pos = posicao;
	int i;

	if (ret.var != '$'){
		checkVarP(ret.var, ret.idx, linha);

		if(ret.var == 'p')  /* retorna um parametro */{
			cod[pos+1]= 0x89;
			pos++;

			if(ret.idx == 0)
				cod[pos+1]= 0xF8;
			else if (ret.idx == 1)
				cod[pos+1]= 0xF0;
			else
				cod[pos+1]= 0xD0;

			pos++;
		}
		else {        /* retorna uma var. local */
			cod[pos+1]= 0x8B;
			cod[pos+2]= 0x45;
			pos += 2;

			cod[pos+1]= ((ret.idx+1)*(-4)); /* pos na pilha como signed */
			pos++;
		}
	} 
	else {           /* retorna uma constante */
		cod[pos+1] = 0xB8;
		pos++;

		for(i=0;i<4;i++){
			cod[pos+1]=(unsigned char) (ret.idx >> (8*i));  /* preenche em Little Endian */
			pos++;
		}
	}

	
	/* finaliza a instrucao com leave e ret */
	cod[pos+1] = 0xC9;
	cod[pos+2] = 0xC3;
	
	pos += 2;

	return pos;
}

int escreveIf (unsigned char *cod, int posicao, Condicao cond) {
	int pos = posicao;

	/* adiciona o cmpl $0, var */
	cod[pos+1] = 0x83;
	cod[pos+2] = 0x7D;
	cod[pos+3] = ((cond.idx+1)*(-4)); /* pos na pilha como signed */
	cod[pos+4] = 0x00;
	pos+=4;

	/* adiciona jl e coloca onde ficara o offset a linha para onde deve pular */
	cod[pos+1]= 0x0F;
	cod[pos+2]= 0x8C;
	cod[pos+3]= cond.Lmenor;
	pos+=6;

	/* adiciona je e coloca onde ficara o offset a linha para onde deve pular */
	cod[pos+1]= 0x0F;
	cod[pos+2]= 0x84;
	cod[pos+3]= cond.Ligual;
	pos+=6;

	/* adiciona jl e coloca onde ficara o offset a linha para onde deve pular */
	cod[pos+1]= 0x0F;
	cod[pos+2]= 0x8F;
	cod[pos+3]= cond.Lmaior;
	pos+=6;

	return pos;
}

int escreveAtr (unsigned char *cod, int posicao, int linha, Atribuicao atr) {
	int pos = posicao;
	int i;

	checkVar('v', atr.idx0, linha);
	if (atr.var1 != '$'){
		checkVarP(atr.var1, atr.idx1, linha);
		if(atr.var1 == 'p'){	/* 1o eh parametro */
			cod[pos+1] = 0x89;
			pos++;
			switch (atr.idx1) {
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
			cod[pos+3] = ((atr.idx1+1)*(-4));
			pos += 3;
		}
	}
	else{ 						/* 1o eh constante */
		cod[pos+1] = 0xB8;
		pos++;
		
		for (i=0; i<4; i++){
			cod[pos+1] = (unsigned char) (atr.idx1 >> (8*i)); /* preenche em Little Endian */
			pos++;
		}
	}

	if (atr.var2 != '$'){
		checkVarP(atr.var2, atr.idx2, linha);
		if(atr.var2 == 'p'){		/* 2o eh parametro */
			cod[pos+1] = 0x89;
			pos++;
			switch (atr.idx2){
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
			cod[pos+3] = ((atr.idx2+1)*(-4));
			pos += 3;
		}
	}
	else{						/* 2o eh constante */
		cod[pos+1] = 0xB9;
		pos++;
		
		for (i=0; i<4; i++){
			cod[pos+1] = (unsigned char) (atr.idx2 >> (8*i)); /* preenche em Little Endian */
			pos++;
		}
	}

	/* implementacao dos casos de operacao */
	switch (atr.op){
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

	/* implementacao de mover %eax para var local lembrando que pospilha = (idx+1)*(-4) */
	cod[pos+1] = 0x89;
	cod[pos+2] = 0x45;
	cod[pos+3] = ((atr.idx0+1)*(-4));
	pos += 3;

	return pos;
}

void preencheIf (unsigned char *cod, int linhas, char *ifcontrol, int *linecontrol){
	int pos;
	int offset;
	int i, j, k;

	for(i=0;i<linhas-1;i++){
		if(ifcontrol[i]){
			pos = linecontrol[i]; /* posicao no vetor cod que comeca o if*/
			pos += 4;

			for(k=0;k<3;k++){
				pos+=2; /* pula para prox buraco do if */
				offset = (linecontrol[cod[pos]-1] - pos)-4;
							/* posicao da linha que esta no cod atual menos a pos atual */
							/* -4 pois o ultimo pedaco do offset esta 4 bytes depois */
				for (j=0; j<4; j++){
						cod[pos] = (unsigned char) (offset >> (8*j)); /* preenche em Little Endian */
						pos++;
				}
			}
		}
	} 
	return;
}
