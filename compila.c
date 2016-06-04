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
	unsigned char *cod; 		/* vetor com as instrucoes de maquina */

	while ((c = fgetc(f)) != EOF) {
    switch (c) {
      case 'r': {  				/* retorno - 'ret' varpc */
        int idx;
        char var;
        if (fscanf(f, "et %c%d", &var, &idx) != 2) 
          error("comando invalido", line);
        if (var != '$') 
        	checkVarP(var, idx, line);
        
        /* implementação parte do retorno */

        break;
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

  return (funcp)cod;
}

void libera (void *p);