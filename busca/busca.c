#include <stdio.h>
#include <stdlib.h>

#define FAIL -404
#define DEBUG 1

/* Funcao que gera um vetor de inteiros aleatorios
	em um determinado intervalo de valores
   Parametros:
	int tam - tamanho do vetor a ser gerado
	int min - valor minimo
	int max - valor maximo
   Retorno:
	int*    - endereco do vetor na heap
*/
int* gera_vetor (int tam, int min, int max) {

	// verifica problemas
	if (tam < 1 || min > max) return NULL;

	int* vet = (int *) malloc(sizeof(int)*tam);

	// verifica se houve problema na alocacao
	if (vet == NULL) return NULL;

	for (int i = 0; i < tam; i++) {
		// ex. min = 0, max = 5, [0, 4] = [0,4+1] = [0, 5]
		//     min = 3, max = 8, [0, 5] + 3 = [3, 8]
		int val = (rand()%(max-min+1)) + min;
		vet[i] = val;
	}
	return vet;
}

/* Funcao que gera um vetor de inteiros ordenados
   Parametros:
	int tam - tamanho do vetor a ser gerado
	int min - valor minimo
	int max - valor maximo
   Retorno:
	int*    - endereco do vetor na heap
*/
int* gera_vetor_ordenado(int tam, int min, int max) {

	// verifica problemas
	if (tam < 1 || min > max) return NULL;

	int* vet = (int *) malloc(sizeof(int)*tam);

	// verifica se houve problema na alocacao
	if (vet == NULL) return NULL;

	// TODO: melhorar isso pra ficar uniformemente distribuido
	int val = min;
	vet[0] = 0;
	for (int i = 0; i < tam; i++) {
		vet[i] = val;
		val = vet[i]+rand()%3;
		if (val > max) val = max;
	}
	return vet;
}


/* Imprime vetor de inteiros
   Parametros
	int* v - vetor
	int  N - tamanho do vetor
*/
void imprime_vetor(int*v, int N) {
	for (int i = 0; i < N; i++)
		printf("%d ", v[i]);
	printf("\n");
}

/* Busca sequencial
   Parametros:
      int* - vetor de inteiros para realizar a busca
      int chave - chave a ser buscada
      int N - tamanho do vetor
   Retorno:
      int - posicao em que a chave foi encontrada, ou FAIL caso contrario
*/
int busca_sequencial(int* v, int chave, int N) {
	for (int i = 0; i < N; i++) {
		if (v[i] == chave)
			return i;
	}	
	return FAIL;
}


/* Busca sequencial (recursiva)
	OBS: NAO UTILIZAR NA PRATICA, apenas para fins didaticos
   Parametros:
      int* - vetor de inteiros para realizar a busca
      int chave - chave a ser buscada
      int N - tamanho do vetor
      int i - posicao a ser verificada (inicializada com 0)
   Retorno:
      int - posicao em que a chave foi encontrada, ou FAIL caso contrario
*/
int busca_sequencial_recursiva(int* v, int chave, int N, int i) {

	// caso base (1)
	if (i > N) return FAIL;
	// caso base (2)
	if (v[i] == chave) return i;

	// passo recursivo
	return busca_sequencial_recursiva(v, chave, N, i+1);

	// TODO: verificar o que acontece
	// return busca_sequencial_recursiva(v, chave, N, i++); n.funciona
	// return busca_sequencial_recursiva(v, chave, N, ++i); ok
	// i sera modificado
}


/* Busca binaria
	OBS: assume que o vetor esta ordenado
	TODO: analisar o algoritmo usando f(n) = c + f(n/2)
   Parametros:
      int* - vetor de inteiros para realizar a busca
      int chave - chave a ser buscada
      int ini   - posicao inicial do espaco de busca
      int fim   - posicao final do espaco de busca
   Retorno:
      int - posicao em que a chave foi encontrada, ou FAIL caso contrario
*/
int busca_binaria(int* v, int chave, int ini, int fim) {

	// 0 - caso base (busca sem sucesso)
	if (ini > fim) return FAIL;

	// 1 - calcula ponto central e verifica se chave foi encontrada
	int centro = (int)((ini+fim)/2.0);
	if (v[centro] == chave)
		return centro;

	// 2 - chamada recursiva para metade do espaco de busca
	if (chave < v[centro])
		// se chave eh menor, fim passa ser o centro-1
		return busca_binaria(v, chave, ini, centro-1);

	if (chave > v[centro])
		// se a chave eh maior, inicio passa ser centro+1
		return busca_binaria(v, chave, centro+1, fim);
	
}


int main (int argc, char* argv[]) {

	// pegar valores N, min, max da linha de comando
	// argv[0] - programa, argv[1] - N, argv[2] - min, argv[3] - max
	if (argc < 4) {
		printf("Uso do programa: \n");
		printf(" %s <tam vetor> <val min> <val max>\n", argv[0]);
		return 1;
	}
	int N = atoi(argv[1]);	
	int min = atoi(argv[2]);	
	int max = atoi(argv[3]);	
	if (N < 1 || max <= min) {
		printf("Verifique valores informados\n");
		printf(" - tam deve ser maior do que 0\n");
		printf(" - maximo deve ser maior do que o minimo\n");
		return 2;
	}

	int* v = gera_vetor(N, min, max);
	int* v_ordenado = gera_vetor_ordenado(N, min, max);
	
	if (DEBUG) imprime_vetor(v, N);
	if (DEBUG) imprime_vetor(v_ordenado, N);

	// Busca repetidamente
	int chave;
	printf("Entre com chave no intervalo [%d, %d]\n", min, max);
	printf("(ou chave fora do intervalo para sair)\n");
	do {
		scanf("%d", &chave);

		// realiza busca
		int pos = busca_sequencial(v, chave, N);
		//int pos = busca_sequencial_recursiva(v, chave, N, 0);

		printf("Busca sequencial:\n\t");
		if (pos >= 0) {
			printf("Chave encontrada em %d\n", pos);
		} else {
			printf("Busca sem sucesso\n");
		}

		// realiza busca
		pos = busca_binaria(v_ordenado, chave, 0, N-1);

		printf("Busca binaria:\n\t");
		if (pos >= 0) {
			printf("Chave encontrada em %d\n", pos);
		} else {
			printf("Busca sem sucesso\n");
		}

	} while (chave >= min && chave <= max);




	free(v);

	return 0;
}
