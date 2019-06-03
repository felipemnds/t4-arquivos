/* Trabalho 3 - Organização de Arquivos
	Nomes da dupla:
	- FELIPE MOREIRA NEVES DE SOUZA - 10734651
	- BRUNO BALDISSERA CARLOTTO - 10724351
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define PDISCO 20
#define DEBUG 0
/*
Pagina de Disco
	> 32paginas->kb de tamanho maximo fixo
	> pagina - conjunto de paginas de disco (PDISCO paginas com 32k bytes cada uma)
	> nPaginas - numero de paginas criadas
	> k - numero de bytes escritos na pagina atual
Caso os registros nao completem a pagina de disco, ela sera preenchida com @ nos espacos restantes
	Ao inves de copiarmos de fato valores para a pagina de disco, usaremos simbolos:
	> '@' - usado para completar paginas nao finalizadas
	> '1' - usado para sinalizar que o char em questao foi usado 
*/
typedef struct pagina_disco{
	char pagina[PDISCO][32000];
	int nPaginas;
	int k;
}Pagina_Disco;
/*
Registro de Cabecalho - tamanho total 214 bytes
	> status - armazena 0 se for inconsistente e 1 se consistente
	> topoLista - byte offset do registro removido ou -1
	> tags - valor resumido da tag para os campos correspondentes no registro de dados (i, s, t, n, c respectivamente)
	> campos - descricao completa dos campos correspondentes no registro de dados (recebe o valor lido do .csv)
*/
typedef struct reg_cabecalho{
	char status;
	long topoLista;
	char tags[5];
	char campos[5][40];
}Reg_Cabecalho;
/*
Registro de Dados
	> removido - char que armazena '*' para registros removidos e '-' para registros nao removidos
	> tamanhoRegistro - armazena o tamanho do registro
		> tamanho = encadeamentoLista + idServidor + salarioServidor + telefoneServidor + tamNomeServidor + nomeServidor + tamCargoServidor + cargoServidor
	> encadeamentoLista - armazena o byte offset do registro removido
	> idServidor - inteiro de 4 bytes
	> salarioServidor - numero de dupla precisao de 8 bytes
	> telefoneServidor - 14 bytes no formato (DD)NNNNN-NNNN
	> nomeServidor - campo variavel com indicador de tamanho (tamNomeServidor) do tipo string
	> cargoServidor - campo variavel com indicador de tamanho (tamCargoServidor) do tipo string
Strings de tamanho variavel devem ser finalizdas com '\0'
Definicoes em caso de valor nulo:
	> campos fixos
		> int ou dupla precisao - recebem -1
		> string - recebe '\0@@@@@@@@@'
	> campos variaveis
		> nao armazena nada (indicador de tamanho/tag/valor)
*/
typedef struct reg_dados{
	char removido;
	int tamanhoRegistro;
	long encadeamentoLista;
	int idServidor;
	double salarioServidor;
	char telefoneServidor[14];
	int tamNomeServidor;
	char nomeServidor[100];
	int tamCargoServidor;
	char cargoServidor[150];
}Reg_Dados;

// DECLARACAO DE FUNCOES (T1)
/* leCSVescreveBIN
	Funcao que le varios registros presentes em um arquivo.csv (campos separado por virgulas) e armazena em um arquivo binario de saida. 
*/
void leCSVescreveBIN (FILE*, FILE*, Reg_Dados*, Reg_Cabecalho*, Pagina_Disco*);
/* imprimirRegistrosBIN
	Funcao que le um arquivo binario e imprime os registros presentes, seguindo as seguintes regras:
	- cada campo eh separado por um espaco
	- temos no maximo um registro por linha
	- valores nulos devem ser tratados de maneiras especificas
*/
void imprimirRegistrosBIN(int*, FILE*, Reg_Dados*, int*);
/* buscaRegistrosBIN
	Funcao que le um arquivo binario e busca por campos com valores especificos, definidos pela entrada do usuario.
*/ 
void buscaRegistrosBIN(int*, FILE*, Reg_Dados*, char*, char*, Reg_Cabecalho*, int*);
/* lerCabecalhoCSV
	Funcao que le o registro de cabecalho de um arquivo do tipo .csv e armazena-o na primeira pagina de disco do programa.
	Alem disso, produz as primeiras impressoes dentro do arquivo binario que sera resultado da leCSVescreveBIN.
*/
void lerCabecalhoCSV (FILE*, FILE*, Reg_Cabecalho*, Pagina_Disco*);
/* lerDadosCSV
	Funcao que le os registros de dados de um arquivo do tipo .csv e armazena-os nas paginas de disco restantes.
	Alem disso, produz as impressoes finais dentro do arquivo binario que sera resultado da leCSVescreveBIN.
*/
int lerDadosCSV (FILE*, FILE*, Reg_Dados*, Reg_Cabecalho*, Pagina_Disco*);
/* fazLeituraImpressaoProxRegistroBIN
	Funcao que le o arquivo binario e imprime os campos presentes nele, de acordo com as predefinicoes definidas para a imprimirRegistrosBIN.
*/
int fazLeituraImpressaoProxRegistroBIN (FILE*, Reg_Dados*, int*, int*, int*);
/* fazLeituraProxRegistroBIN
	Funcao que le o proximo registro de um arquivo binario e imprime os campos presentes nele, de acordo com as predefinicoes definidas para a buscaRegistrosBIN.
*/
int fazLeituraProxRegistroBIN (FILE*, Reg_Dados*, int*, int*);
/* limpaRegistro
	Funcao que varre o registro de dados atual, limpando caracteres que sao possiveis ameacas para futuras escritas.
	Ex: '\0' que nao serao sobrescritos.
*/ 
void limpaRegistro(Reg_Dados*);
// DECLARACAO DE FUNCOES (T2)
/* testeEhConsistente
	Funcao que testa se o arquivo aberto eh consistente (status == 1)
*/
int testeEhConsistente (FILE *arquivoBIN);
/* setStatus
	Funcao que altera o status de um arquivo logo apos de comecar e logo antes de terminar algum processo
*/
void setStatus (FILE *arquivoBIN, char charConsistencia);
/* imprimeListaRemovidos
	Funcao que testa a remocao imprimindo todos os registros removidos 
*/
void imprimeListaRemovidos(FILE *arquivoBIN, Reg_Dados *rdados);
/* imprimeProxRegRemovido
	Funcao que imprime um registro da lista de removidos
*/
int imprimeProxRegRemovido (FILE *arquivoBIN, Reg_Dados *rdados, int *registroInexistente, int pos);
/* removeRegistro
	Funcao que busca pelos registros que satisfazem a busca do usuario e insere tais registros na lista de removidos 
*/
void removeRegistro(FILE *arquivoBIN, Reg_Dados *rdados, char *nomeCampo, char *valorCampo, int *erro);
/* lerRegistroPre
	Funcao auxiliar que le o proximo registro de um arquivo binario e armazena seus campos na estrutura Reg_Dados 
*/
int lerRegistroPre(FILE *arquivoBIN, Reg_Dados *rdados);
/* insereListaRemovidos
	Funcao que insere o registro com o byte offset "pos_bin" na lista de registros removidos 
*/
void insereListaRemovidos (int pos_bin, FILE *arquivoBIN);
/* insereRegistro
	Funcao que insere o registro com campos definidos em "rdados", considerando a presenca da lista de registros removidos
*/
void insereRegistro(FILE *arquivoBIN, Reg_Dados *rdados, int *erro);
/* insereRegistroPos
	Funcao que vai ate o byteoffset "pos" e insere o registro que possui os campos descritos em "rdados".
	Faz dois tipos de insercao:
		tipo "1" - insere um registro sem alterar o tamanho 
		tipo "2" - insere um registro escrevendo todos os campos (inclusive o tamanhoRegistro)
*/
void insereRegistroPos(FILE *arquivoBIN, Reg_Dados *rdados, int pos, int tipoInsercao);
/* atualizaRegistro
	Funcao que busca pelos registros que satisfacam a busca do usuario, atualizando todos esses com
	os valores definidos tambem pelo usuario
*/
void atualizaRegistro(FILE *arquivoBIN, Reg_Dados *rdados, char *campoBusca, char *valorBusca, char *campoAtualiza, char *valorAtualiza, int *erro);
/* atualizaRegistroPos
	Funcao que recebe um byteoffset "pos_bin" e atualiza o registro presente em tal posicao com os valores definidos pelo usuario
*/
void atualizaRegistroPos(int pos_bin, FILE *arquivoBIN, char *campoAtualiza, char *valorAtualiza, Reg_Dados *rdados);
/* tamanhoValidoAtualizacao
	Funcao que valida se o novo tamanho do registro apos a atualizacao sera:
		1) Valida (retorna 1) - basta sobrescrever o registro novo no local do atual
		2) Invalida (retorna 0) - precisamos remover o registro antigo para inserir o novo
*/
int tamanhoValidoAtualizacao(FILE *arquivoBIN, char *campoAtualiza, char *valorAtualiza, Reg_Dados *rdados);
/* getTopoLista
	Funcao que retorna o byteoffset do primeiro registro da lista de removidos (ou -1 caso ela esteja vazia)
*/
long getTopoLista(FILE *arquivoBIN);
/* setTopoLista
	Funcao que altera o byteoffset presente no topoLista, colocando o byteoffset do menor registro presente na lista de removidos
*/
void setTopoLista(FILE *arquivoBIN, long valor);
/* getEncadeamentoLista
	Funcao que retorna o byteoffset do registro que se encontra apos o registro na posicao "pos", dentro da lista de removidos
*/
long getEncadeamentoLista(FILE *arquivoBIN, int pos);
/* setEncadeamentoLista
	Funcao que altera o byteoffset do registro que se encontra apos o registro na posicao "pos", dentro da lista de removidos
*/
void setEncadeamentoLista(FILE *arquivoBIN, int pos, long valor);
/* getTamanhoRegistro
	Funcao que retorna o tamanho do registro presente na posicao "pos"
*/
int getTamanhoRegistro(FILE *arquivoBIN, int pos);
/* setRemovidoReg
	Funcao que altera o campo 'removido', definindo tal com '*'
*/
void setRemovidoReg(FILE *arquivoBIN, int pos);
/* preencheComArroba
	Funcao que vai ate o registro com byteoffset "pos" e preenche este com '@', apos uma remocao 
*/
void preencheComArroba (FILE *arquivoBIN, int pos);
/* ordenarRegistros
	Funcao que guarda registros em um vetor, ordena cada um com base no idServidor, e reescreve em um novo arquivoBIN
*/
void ordenarRegistros(FILE *arquivoBIN, Reg_Dados **vetReg, FILE *arquivoBINsaida, Reg_Dados *rdados, Reg_Cabecalho *Rcabecalho, Pagina_Disco *paginas, int *erro);
void MS_sort(void *vector, unsigned long n, size_t memsize, int (*fcmp)(const void *, const void *));
/* copiaCabecalhoBIN
	Funcao auxiliar à 'ordenarRegistros', que altera o topoLista de um arquivoBIN (considerando que a lista de removidos será ignorada),
	lê e copia os primeiros 32000 bytes para outro arquivoBIN novo 
*/
void copiaCabecalhoBIN(FILE *arquivoBIN, FILE *arquivoBINsaida);
/* escreveVetorBIN
	Funcao auxiliar à 'ordenarRegistros', que recebe uma struct do tipo Reg_Dados (vinda de um vetor de registros) e escreve os respectivos campos
	em um novo arquivoBIN, considerando quebras nas paginas de disco
*/
void escreveVetorBIN(FILE *arquivoBIN, Reg_Dados *rdados, Pagina_Disco *paginas, int boAnt);
/* comparaRegistros
	Funcao auxiliar à 'MS_sort', que recebe dois elementos do tipo Reg_Dados** e compara ambos através do campo idServidor
*/
int comparaRegistros(const void *A, const void *B);
void scan_quote_string(char *str);
void trim(char *str);
void binarioNaTela1(FILE *ponteiroArquivoBinario);
/* merging
	Funcao que recebe dois arquivos como entrada e produz um arquivo de saida. O arquivo de saida
	e composto por todos os registros nao repetidos de ambas as entradas, de forma a coalescer os dados.
*/
void merging(FILE* entradaMaior, FILE* entradaMenor, FILE* saida, int* entrada1);

/* matching
	Funcao que recebe dois arquivos binarios de entrada e produz um arquivo de saida. O arquivo de saida 
	e composto apenas pelas intersecoes de registros entre os dois arquivos de entrada, de forma que cada
	arquivo repetido e inserido apenas uma vez no arquivo final de saida.
*/
void matching(FILE* entradaMaior, FILE* entradaMenor, FILE* saida, int* entrada1);

int main(int argc, char const *argv[]){
	/* variaveis que serao definidas pelo usuario
		> func - numero da funcionalidade escolhida
		> filename1 - string que armazena nome do arquivo a ser aberto
		> n - numero de vezes que certas funcoes devem ser executadas
	*/
	int func;
	char *filename1 = malloc (40*sizeof(char));
	char *filename2 = malloc (40*sizeof(char));
	int n;
	scanf("%d", &func);
	/* definicao das estruturas usadas durante a aplicacao:
		- arquivoCSV - arquivo usado na leCSVescreveBIN para geracao de um arquivo binario
		- arquivoBIN - arquivo binario usado para receber valores de um .csv (leCSVescreveBIN) e fornecer registros (imprimirRegistrosBIN e buscaRegistros)
		- paginas - estrutura complementar que auxilia a contagem de paginas de disco
		- rcabecalho - estrutura que simula um registro de cabecalho
		- rdados - estrutura que simula um registro de dados
		- contpagdisco - contador que armazenara a quantidade de bytes acessados
		- pdisco_acessadas - contador que armazenara a quantidade de paginas de disco acessadas
		- erro - variavel que valida procedimentos que falharam por algum motivo, impedindo que a impressao do binario aconteca
	*/
	FILE *arquivoCSV;
	FILE *arquivoBIN;
	FILE *arquivoBINsaida;
	Pagina_Disco *paginas = malloc (sizeof(Pagina_Disco));
	Reg_Cabecalho *rcabecalho = malloc (sizeof(Reg_Cabecalho)); 
	Reg_Dados *rdados = malloc (sizeof(Reg_Dados));
	int contpagdisco = 0;
	int pdisco_acessadas = 0;
	int erro = 0;
	// definicao das variaveis exclusivas de algumas funcoes
	/* func4
		> nomeCampo - string que armazena nome do campo a ser buscado (usado somente na buscaRegistros)
		> valorCampo - string que armazena valor do campo a ser buscado(usado somente na buscaRegistros)
	*/
	char *nomeCampo = malloc (40*sizeof(char));
	char *valorCampo = malloc (40*sizeof(char));
	/* func5
		> campo_x - strings que armazenam os novos campos a serem inseridos no novo registro
	*/
	char *campo_id = malloc (40*sizeof(char));
	char *campo_salario = malloc (40*sizeof(char));
	char *campo_telefone = malloc (40*sizeof(char));
	char *campo_nome = malloc (40*sizeof(char));
	char *campo_cargo = malloc (40*sizeof(char));
	/* func6
		> campoBusca - nome do campo que sera usado como busca na atualizacao
		> valorBusca - valor do campo que sera usado como busca na atualizacao
		> campoAtualiza - nome do campo que sera atualizado
		> valorAtualiza - valor do campo que sera atualizado
	*/
	char *campoBusca = malloc (40*sizeof(char));
	char *valorBusca = malloc (40*sizeof(char));
	char *campoAtualiza = malloc (40*sizeof(char));
	char *valorAtualiza = malloc (150*sizeof(char));
	/* func7
		> vetorRegistros - vetor que armazena os registros durante a ordenacao
	*/
	Reg_Dados **vetReg = malloc (10000 * sizeof(Reg_Dados*));
	for (int i = 0; i < 10000; i++)
		vetReg[i] = malloc (sizeof(Reg_Dados));
	// bloco que encaminha o programa para a funcionalidade escolhida e abre os arquivos da maneira necessaria
	if (func == 1){
		scanf("%s", filename1);
		trim(filename1);
		arquivoCSV = fopen(filename1, "r");
		if (arquivoCSV == NULL){
			printf("Falha no carregamento do arquivo CSV.\n");
			return 0;
		}
		arquivoBIN = fopen("arquivoTrab1.bin", "wb");
		if (arquivoBIN == NULL){
			printf("Falha no carregamento do arquivo BIN.\n");
			return 0;
		}
		leCSVescreveBIN (arquivoCSV, arquivoBIN, rdados, rcabecalho, paginas);
		printf("arquivoTrab1.bin");
		fclose(arquivoCSV);
	}else if (func == 2){
		scanf("%s", filename1);
		trim(filename1);
		arquivoBIN = fopen(filename1, "rb");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		imprimirRegistrosBIN(&contpagdisco, arquivoBIN, rdados, &pdisco_acessadas);
	}else if (func == 3){
		scanf("%s %s %s", filename1, nomeCampo, valorCampo);
		trim(valorCampo);
		arquivoBIN = fopen(filename1, "rb");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		buscaRegistrosBIN(&contpagdisco, arquivoBIN, rdados, nomeCampo, valorCampo, rcabecalho, &pdisco_acessadas);
	}else if (func == 4){
		scanf("%s %d", filename1, &n);
		arquivoBIN = fopen(filename1, "rb+");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		for (int i = 0; (i < n && !erro); i++){
			scanf("%s", nomeCampo);
			scan_quote_string(valorCampo);
			removeRegistro(arquivoBIN, rdados, nomeCampo, valorCampo, &erro);
		}
		if (!erro) binarioNaTela1(arquivoBIN);
	}else if (func == 5){
		scanf("%s %d", filename1, &n);
		arquivoBIN = fopen(filename1, "rb+");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		for (int i = 0; (i < n && !erro); i++){
			limpaRegistro(rdados);
			// le removido
			rdados->removido = '-';
			// le encadeamento
			rdados->encadeamentoLista = -1;
			rdados->tamanhoRegistro += 8;
			// le idservidor
			scan_quote_string(campo_id);
			rdados->idServidor = atoi(campo_id);
			rdados->tamanhoRegistro += 4;
			// le salario
			scan_quote_string(campo_salario);
			if (strlen(campo_salario) == 0)
				rdados->salarioServidor = -1;
			else
				rdados->salarioServidor = atof(campo_salario);
			rdados->tamanhoRegistro += 8;
			// le telefone
			scan_quote_string(campo_telefone);
			if (strlen(campo_telefone) == 0){
				rdados->telefoneServidor[0] = '\0';
				for (int i = 1; i < 14; i++)
					rdados->telefoneServidor[i] = '@';
			}
			else
				strcpy(rdados->telefoneServidor, campo_telefone);
			rdados->tamanhoRegistro += 14;
			// le nome
			scan_quote_string(campo_nome);
			strcpy(rdados->nomeServidor, campo_nome);
			rdados->tamNomeServidor = strlen(rdados->nomeServidor) + 2;
			if (rdados->tamNomeServidor == 2)
				rdados->tamNomeServidor = 0;
			else
				rdados->tamanhoRegistro += 4 + rdados->tamNomeServidor;
			// le cargo
			scan_quote_string(campo_cargo);
			strcpy(rdados->cargoServidor, campo_cargo);
			rdados->tamCargoServidor = strlen(rdados->cargoServidor) + 2;
			if (rdados->tamCargoServidor == 2)
				rdados->tamCargoServidor = 0;
			else
				rdados->tamanhoRegistro += 4 + rdados->tamCargoServidor;
			insereRegistro(arquivoBIN, rdados, &erro);
		}
		if (!erro) binarioNaTela1(arquivoBIN);
	}else if(func == 6){
		scanf("%s %d", filename1, &n);
		arquivoBIN = fopen(filename1, "rb+");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		for (int i = 0; (i < n && !erro); i++){
			scanf("%s", campoBusca);
			scan_quote_string(valorBusca);
			scanf("%s", campoAtualiza);
			scan_quote_string(valorAtualiza);
			atualizaRegistro(arquivoBIN, rdados, campoBusca, valorBusca, campoAtualiza, valorAtualiza, &erro);
		}
		if (!erro) binarioNaTela1(arquivoBIN);
	}else if(func == 7){
		scanf("%s", filename1);
		scan_quote_string(filename2);
		arquivoBIN = fopen(filename1, "rb+");
		arquivoBINsaida = fopen(filename2, "wb+");
		if (arquivoBIN == NULL || arquivoBINsaida == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		ordenarRegistros(arquivoBIN, vetReg, arquivoBINsaida, rdados, rcabecalho, paginas, &erro);	
		if (!erro) binarioNaTela1(arquivoBINsaida);
	}else if (func == 8){
		scanf("%s", filename1);
		trim(filename1);
		arquivoBIN = fopen(filename1, "rb+");

		scanf(" %s", filename2);
		trim(filename2);
		FILE *arquivoBIN2 = fopen(filename2, "rb+");

		char *filename3 = malloc (40*sizeof(char));
		scanf(" %s", filename3);
		trim(filename3);
		arquivoBINsaida = fopen(filename3, "wb+");

		if (arquivoBIN == NULL || arquivoBIN2 == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		if (arquivoBINsaida == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		
		int entrada1 = 1;		
		
		fseek(arquivoBIN, 0, SEEK_END);
		int tam1 = ftell(arquivoBIN);
	
		fseek(arquivoBIN2, 0, SEEK_END);
		int tam2 = ftell(arquivoBIN2);

		if(tam1 < tam2){
			entrada1 = 2;
			FILE* aux = arquivoBIN;
			arquivoBIN = arquivoBIN2;
			arquivoBIN2 = aux;
		}	
		
		char teste_status;
		
		fseek(arquivoBIN, 0, SEEK_SET);
		fread(&teste_status, sizeof(char), 1, arquivoBIN);
		if (teste_status == '0'){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}		
	
		fseek(arquivoBIN2, 0, SEEK_SET);	
		fread(&teste_status, sizeof(char), 1, arquivoBIN2);
		if (teste_status == '0'){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}		
		
		merging(arquivoBIN, arquivoBIN2, arquivoBINsaida, &entrada1);

		binarioNaTela1(arquivoBINsaida);
		
		fclose(arquivoBIN2);
		fclose(arquivoBINsaida);
	}else if (func == 9){
		scanf("%s", filename1);
		trim(filename1);
		arquivoBIN = fopen(filename1, "rb+");

		scanf(" %s", filename2);
		trim(filename2);
		FILE *arquivoBIN2 = fopen(filename2, "rb+");

		char *filename3 = malloc (40*sizeof(char));
		scanf(" %s", filename3);
		trim(filename3);
		arquivoBINsaida = fopen(filename3, "wb+");

		if (arquivoBIN == NULL || arquivoBIN2 == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		if (arquivoBINsaida == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		
		int entrada1 = 1;		
		
		fseek(arquivoBIN, 0, SEEK_END);
		int tam1 = ftell(arquivoBIN);
	
		fseek(arquivoBIN2, 0, SEEK_END);
		int tam2 = ftell(arquivoBIN2);

		if(tam1 < tam2){
			entrada1 = 2;
			FILE* aux = arquivoBIN;
			arquivoBIN = arquivoBIN2;
			arquivoBIN2 = aux;
		}	
		
		char teste_status;
		
		fseek(arquivoBIN, 0, SEEK_SET);
		fread(&teste_status, sizeof(char), 1, arquivoBIN);
		if (teste_status == '0'){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}		
	
		fseek(arquivoBIN2, 0, SEEK_SET);	
		fread(&teste_status, sizeof(char), 1, arquivoBIN2);
		if (teste_status == '0'){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}		

		matching(arquivoBIN, arquivoBIN2, arquivoBINsaida, &entrada1);

		binarioNaTela1(arquivoBINsaida);
		
		fclose(arquivoBIN2);
		fclose(arquivoBINsaida);	
	}else if (func == 99){
		scanf("%s", filename1);
		trim(filename1);
		arquivoBIN = fopen(filename1, "rb");
		if (arquivoBIN == NULL){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		}
		imprimeListaRemovidos(arquivoBIN, rdados);
	}
	fclose(arquivoBIN);
	free(filename1);
	free(nomeCampo);
	free(valorCampo);
	free(rcabecalho);
	free(rdados);
	return 0;
}
// AUXILIARES
int testeEhConsistente (FILE *arquivoBIN){
	fseek(arquivoBIN, 0, SEEK_SET);
	// checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	char testeStatus = fgetc(arquivoBIN);
	if (testeStatus != '1')
		return 0;
	return 1;
}
void setStatus (FILE *arquivoBIN, char charConsistencia){
	fseek(arquivoBIN, 0, SEEK_SET);
	char cConsistencia = charConsistencia;
	fwrite(&(cConsistencia), sizeof(char), 1, arquivoBIN);
	return;
}
void limpaRegistro(Reg_Dados *rdados){
	rdados->tamanhoRegistro = 0;
	rdados->salarioServidor = 0;
	rdados->telefoneServidor[0] = '\0';
	rdados->tamNomeServidor = 0;
	rdados->nomeServidor[0] = '\0';
	rdados->tamCargoServidor = 0;
	rdados->cargoServidor[0] = '\0';
	return;
}
// FUNCIONALIDADE 1
void leCSVescreveBIN (FILE *arquivoCSV, FILE *arquivoBIN, Reg_Dados *rdados, Reg_Cabecalho *rcabecalho, Pagina_Disco *paginas){
	// primeiro, lemos o registro de cabecalho
	lerCabecalhoCSV(arquivoCSV, arquivoBIN, rcabecalho, paginas);
	// depois, lemos todos os registros de dados ate que o arquivo todo seja lido
	while (!feof(arquivoCSV))
		lerDadosCSV(arquivoCSV, arquivoBIN, rdados, rcabecalho, paginas);
	return;
}
int lerDadosCSV (FILE *arquivoCSV, FILE *arquivoBIN, Reg_Dados *rdados, Reg_Cabecalho *rcabecalho, Pagina_Disco *paginas){
	// colocar o ponteiro de arquivoCSV no lugar certo (apos ler '\n')
	fseek (arquivoCSV , ftell(arquivoCSV)-1, SEEK_SET); 
	// atualiza o reg de dados
	rdados->removido = '-';
	rdados->encadeamentoLista = -1;
	// leitura idServidor
	int i = 0;
	char sbuffer[100];
	char cbuffer = fgetc(arquivoCSV);
	while (cbuffer != ','){
		sbuffer[i++] = cbuffer;
		cbuffer = fgetc(arquivoCSV);
		if (feof(arquivoCSV))
			return 0;
	}
	if (i != 0){
		sbuffer[i] = '\0';
		rdados->idServidor = atoi(sbuffer);
	}
	else {
		rdados->idServidor = -1;
	}
	// leitura salarioServidor
	for (i = 0; i < 100; i++)
		sbuffer[i] = 0; // primeiro, limpamos a string buffer 'sbuffer'
	i = 0;
	cbuffer = fgetc(arquivoCSV);
	while (cbuffer != ','){
		sbuffer[i++] = cbuffer;
		cbuffer = fgetc(arquivoCSV);
	}
	if (i != 0){
		sbuffer[i] = '\0';
		rdados->salarioServidor = atof(sbuffer);
	}
	else {
		rdados->salarioServidor = -1;
	}
	// leitura telefoneServidor
	for (i = 0; i < 14; i++)
		rdados->telefoneServidor[i] = 0; // primeiro, limpamos a string buffer 'sbuffer'
	i = 0;
	cbuffer = fgetc(arquivoCSV);
	while (cbuffer != ','){
		rdados->telefoneServidor[i++] = cbuffer;
		cbuffer = fgetc(arquivoCSV);
	}
	if (i == 0){
		rdados->telefoneServidor[0] = '\0';
		for (int i = 1; i < 14; i++)
			rdados->telefoneServidor[i] = '@';
	}
	// leitura nomeServidor + tamNomeServidor
	for (i = 0; i < 100; i++)
		sbuffer[i] = 0; // primeiro, limpamos a string buffer 'sbuffer'
	i = 0;
	cbuffer = fgetc(arquivoCSV);
	while (cbuffer != ',' && cbuffer != '\n'){
		sbuffer[i++] = cbuffer;
		cbuffer = fgetc(arquivoCSV);
	}
	sbuffer[i] = '\0';
	strcpy(rdados->nomeServidor, sbuffer);
	rdados->tamNomeServidor = strlen(rdados->nomeServidor) + 2;
	// leitura cargoServidor + tamCargoServidor
	for (i = 0; i < 100; i++)
		sbuffer[i] = 0; // primeiro, limpamos a string buffer 'sbuffer'
	i = 0;
	cbuffer = fgetc(arquivoCSV);
	while (cbuffer != ',' && cbuffer != '\n' && cbuffer != '\r'){
		sbuffer[i++] = cbuffer;
		cbuffer = fgetc(arquivoCSV);
	}
	sbuffer[i] = '\0';
	strcpy(rdados->cargoServidor, sbuffer);
	rdados->tamCargoServidor = strlen(rdados->cargoServidor) + 2;
	/* tamamanhoRegistro =
		+8	: encadeamentoLista
		+4	: idServidor
		+8	: salarioServidor
		+14	: telefoneServidor
		+4	: tamNomeServidor
		+1	: tagCampo4
		+?	: nomeServidor + '\0'
		+4	: tamCargoServidor
		+1	: tagCampo5
		+?	: cargoServidor + '\0'
	*/
	rdados->tamanhoRegistro = 8 + 4 + 8 + 14 + 4 + rdados->tamNomeServidor + 4 + rdados->tamCargoServidor;
	// caso 1 - registro a ser inserido ultrapassa a pagina de disco atual
	if ((paginas->k + rdados->tamanhoRegistro + 1) >= 32000){
		int bytes_restantes = 32000 - paginas->k;
		for (int i = 0; i < bytes_restantes; i++){
			paginas->pagina[paginas->nPaginas][paginas->k++] = '@';
			fwrite(&(paginas->pagina[paginas->nPaginas][paginas->k-1]), sizeof(char), 1, arquivoBIN);
			rdados->tamanhoRegistro += 1;
		}
		paginas->k = 0;
		paginas->nPaginas++;

	}
	// caso 2 - registro a ser inserido nao ultrapassa a pagina de disco atual
	// atualiza removido 
	paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	// atualiza tamanhoRegistro
	for (int i = 0; i < 4; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	// atualiza encadeamentoLista
	for (int i = 0; i < 8; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	// atualiza idServidor
	for (int i = 0; i < 4; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	// atualiza salarioServidor
	for (int i = 0; i < 8; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	// atualiza telefoneServidor
	for (int i = 0; i < 14; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->telefoneServidor), sizeof(char), 14, arquivoBIN);
	if (strlen(rdados->nomeServidor) > 0){	
		// atualiza tamNomeServidor
		for (int i = 0; i < 4; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		// atualiza tagCampo4
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rcabecalho->tags[3]), sizeof(char), 1, arquivoBIN);
		// atualiza nomeServidor
		for (int i = 0; i < strlen(rdados->nomeServidor)+1; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(rdados->nomeServidor, sizeof(char), strlen(rdados->nomeServidor)+1, arquivoBIN);
	}
	if (strlen(rdados->cargoServidor) > 0){
		// atualiza tamCargoServidor
		for (int i = 0; i < 4; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		// atualiza tagCampo5
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rcabecalho->tags[4]), sizeof(char), 1, arquivoBIN);
		// atualiza cargoServidor
		for (int i = 0; i < strlen(rdados->cargoServidor)+1; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(rdados->cargoServidor, sizeof(char), strlen(rdados->cargoServidor)+1, arquivoBIN);
	}
	return 1;
}
void lerCabecalhoCSV (FILE *arquivoCSV, FILE *arquivoBIN, Reg_Cabecalho* rcabecalho, Pagina_Disco* paginas){
	// atualiza o reg de cabecalho
	rcabecalho->status = '0';
	rcabecalho->topoLista = -1;
	rcabecalho->tags[0] = 'i';
	rcabecalho->tags[1] = 's';
	rcabecalho->tags[2] = 't';
	rcabecalho->tags[3] = 'n';
	rcabecalho->tags[4] = 'c';
	char buffer = fgetc(arquivoCSV);
	int j = 0;
	for (int i = 0; i < 5; i++){
		while (buffer != ',' && buffer != '\n'){
			rcabecalho->campos[i][j] = buffer;
			buffer = fgetc(arquivoCSV);
			j++;
		}
		rcabecalho->campos[i][j] = '\0';
		j = 0;
		buffer = fgetc(arquivoCSV);	
	}
	rcabecalho->status = '1';
	// inicializa a pagina de disco
	paginas->nPaginas = 0;
	paginas->k = 0; // indice da pagina de disco atual (independente do valor dos loops)
	// ATUALIZA PAGINA DE DISCO	
	// atualiza status (pagina e arquivoBIN)
	paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rcabecalho->status), sizeof(char), 1, arquivoBIN);
	// atualiza topoLista (pagina e arquivoBIN)
	for (int i = 0; i < 8; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rcabecalho->topoLista), sizeof(long), 1, arquivoBIN);
	// atualiza tag e campo 1..5 (pagina e arquivoBIN)
	for (int i = 0; i < 5; i++){
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rcabecalho->tags[i]), sizeof(char), 1, arquivoBIN);
		for (int j = 0; j < strlen(rcabecalho->campos[i]); j++){
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		}
		fwrite(&(rcabecalho->campos[i]), sizeof(char), strlen(rcabecalho->campos[i]), arquivoBIN);
		for (int j = 0; j < (40-strlen(rcabecalho->campos[i])); j++){
			paginas->pagina[paginas->nPaginas][paginas->k++] = '@';
			fwrite(&(paginas->pagina[paginas->nPaginas][paginas->k-1]), sizeof(char), 1, arquivoBIN);
		}
	}
	int bytes_restantes = 32000 - paginas->k;
	for (int i = 0; i < bytes_restantes; i++){
		paginas->pagina[paginas->nPaginas][paginas->k++] = '@';
		fwrite(&(paginas->pagina[paginas->nPaginas][paginas->k-1]), sizeof(char), 1, arquivoBIN);
	}
	// se quantidade de bytes escritos eh igual a 32000, criamos uma nova pagina
	if (paginas->k >= 32000){
		paginas->nPaginas++;
		paginas->k = 0;
	}
	return;
}
// FUNCIONALIDADE 2
void imprimirRegistrosBIN(int *contpagdisco, FILE *arquivoBIN, Reg_Dados *rdados, int *pdisco_acessadas){
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		return;
	}
	/*
	feito isso, preparamos os contadores
	e posicionamos o ponteiro do arquivoBIN para a primeira posicao da pagina de disco 2
	pois a pagina de disco 1 guarda somente o registro de cabecalho, que nao nos interessa no momento
	*/ 
	*contpagdisco = 0;
	fseek(arquivoBIN, 32000, SEEK_SET);
	*contpagdisco += 32000;
	// separamos tambem uma flag que confere se existem realmente registros
	int registroInexistente = 1;
	int registroRemovido = 0;
	/*
	fazemos a leitura do arquivoBIN e imprimos (atraves da 'fazLeituraImpressaoProxRegistroBIN' registro por registro
	ate que aconteca algum erro ou o arquivoBIN chegue no fim
	apos cada leitura, uma linha eh pulada
	tambem conferimos se o proximo char na sequencia eh um '@', a fim de prever lixo usado para preencher uma pagina de disco
	caso um '@' seja encontrado, percorremos o arquivoBIN
	ate que o lixo acabe e o ponteiro alcance o inicio da proxima pagina de disco
	*/
	while (fazLeituraImpressaoProxRegistroBIN(arquivoBIN, rdados, contpagdisco, &registroInexistente, &registroRemovido)){
		if (!registroRemovido) printf("\n");
		if (fgetc(arquivoBIN) == '@'){
			*contpagdisco += 1;
			while(fgetc(arquivoBIN) == '@')
				*contpagdisco += 1;
		}
		fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		registroRemovido = 0;
	}
	// primeiro, testamos se o registroInexistente se manteve verdadeiro (1) ou nao
	if (registroInexistente){
		printf("Registro inexistente.\n");
		return;
	}
	/*
	por fim, calculamos a quantidade de paginas de disco acessadas
	e somamos 2 para contabilizar a pagina de disco 1 (que foi ignorada) e a pagina de disco atual (que nao necessariamente acabou)
	*/
	*pdisco_acessadas = (*contpagdisco)/32000;
	*pdisco_acessadas += 2;
	if ((*contpagdisco)%32000 > 0)
		*pdisco_acessadas += 1;
	printf("Número de páginas de disco acessadas: %d\n", *pdisco_acessadas);
	return;
}
int fazLeituraImpressaoProxRegistroBIN (FILE *arquivoBIN, Reg_Dados *rdados, int *contpagdisco, int *registroInexistente, int *registroRemovido){
	// le removido
	fread(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	*contpagdisco += sizeof(char)*1;
	if (feof(arquivoBIN))
		return 0;
	// le tamanhoRegistro
	fread(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	*contpagdisco += sizeof(int)*1;
	if (feof(arquivoBIN))
		return 0;
	// le encadeamentoLista
	fread(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	*contpagdisco += sizeof(long)*1;
	if (feof(arquivoBIN))
		return 0;	
	if(rdados->removido == '*') {
		*registroRemovido = 1;
		fseek(arquivoBIN, rdados->tamanhoRegistro - 8, SEEK_CUR);
		*contpagdisco += rdados->tamanhoRegistro - 8;
		return 1;
	}
	// le idServidor
	fread(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	*contpagdisco += sizeof(int)*1;
	printf("%d", rdados->idServidor);
	// le salarioServidor
	fread(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	*contpagdisco += sizeof(double)*1;
	if (rdados->salarioServidor == -1)
		printf("         ");
	else
		printf(" %.2lf", rdados->salarioServidor);
	// le telefoneServidor
	if(fgetc(arquivoBIN) == '\0'){
		fseek(arquivoBIN, ftell(arquivoBIN)+13, SEEK_SET);
		printf("               ");
	} else{
		fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		fread(rdados->telefoneServidor, sizeof(char), 14, arquivoBIN);
		printf(" %.14s", rdados->telefoneServidor);
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'n'){
		*registroInexistente = 0;
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamNomeServidor
		fread(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		*contpagdisco += sizeof(int)*1;
		printf(" %d", rdados->tamNomeServidor-2);
		// pula tagCampo4
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		*contpagdisco += 1;
		// le nomeServidor
		// rdados->nomeServidor[rdados->tamNomeServidor+1] = '\0';
		fread(rdados->nomeServidor, sizeof(char), rdados->tamNomeServidor-1, arquivoBIN);
		*contpagdisco += sizeof(char)*rdados->tamNomeServidor;
		printf(" %s", rdados->nomeServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'c'){
		*registroInexistente = 0;
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamCargoServidor
		fread(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		*contpagdisco += sizeof(int)*1;
		printf(" %d", rdados->tamCargoServidor-2);
		// pula tagCampo5
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		*contpagdisco += 1;
		// le cargoServidor
		// rdados->cargoServidor[rdados->tamCargoServidor+1] = '\0';
		fread(rdados->cargoServidor, sizeof(char), rdados->tamCargoServidor-1, arquivoBIN);
		*contpagdisco += sizeof(char)*rdados->tamCargoServidor;
		printf(" %s", rdados->cargoServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	return 1;
}
// FUNCIONALIDADE 3
void buscaRegistrosBIN(int *contpagdisco, FILE *arquivoBIN, Reg_Dados *rdados, char *nomeCampo, char *valorCampo, Reg_Cabecalho *rcabecalho, int *pdisco_acessadas){
	int flag_encontrou = 0;
	int flag_erro = 1;
	int buffer_pdiscos;
	*contpagdisco = 0;
	/*
	primeiro, atualizamos as tags e campos do registro de cabecalho atual
	ja que este sera usado posteriormente nas impressoes
	*/
	fread(&(rcabecalho->status), sizeof(char), 1, arquivoBIN);
	fread(&(rcabecalho->topoLista), sizeof(long), 1, arquivoBIN);
	fread(&(rcabecalho->tags[0]), sizeof(char), 1, arquivoBIN);
	fread(rcabecalho->campos[0], sizeof(char), 40, arquivoBIN);
	fread(&(rcabecalho->tags[1]), sizeof(char), 1, arquivoBIN);
	fread(rcabecalho->campos[1], sizeof(char), 40, arquivoBIN);
	fread(&(rcabecalho->tags[2]), sizeof(char), 1, arquivoBIN);
	fread(rcabecalho->campos[2], sizeof(char), 40, arquivoBIN);
	fread(&(rcabecalho->tags[3]), sizeof(char), 1, arquivoBIN);
	fread(rcabecalho->campos[3], sizeof(char), 40, arquivoBIN);
	fread(&(rcabecalho->tags[4]), sizeof(char), 1, arquivoBIN);
	fread(rcabecalho->campos[4], sizeof(char), 40, arquivoBIN);
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		return;
	}
	// ignoramos a primeira pagina de disco (que so possui o registro de cabecalho, ja lido)
	fseek(arquivoBIN, 32000, SEEK_SET);
	*contpagdisco += 32000;
	/*
	lemos todo o arquivoBIN, registro por registro
	a cada leitura, o registro resgatado eh testado
	e seus campos sao combinados com as entradas do usuario
	caso nenhuma seja encontrada, o proximo registro eh resgatado
	caso algum valor encontrado seja identico a entrada do usuario, este registro eh impresso 
	*/
	int foiRemovido = 0;
	while (fazLeituraProxRegistroBIN(arquivoBIN, rdados, contpagdisco, &foiRemovido)){
		if (fgetc(arquivoBIN) == '@'){
			*contpagdisco += 1;
			while(fgetc(arquivoBIN) == '@')
				*contpagdisco += 1;
		}
		fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		// compara o registro lido com a entrada do usuario
		if (!strcmp(nomeCampo, "idServidor")){
			if (atoi(valorCampo) == rdados->idServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "salarioServidor")){
			if (atof(valorCampo) == rdados->salarioServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "telefoneServidor")){
			if (!strcmp(valorCampo, rdados->telefoneServidor)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "nomeServidor")){
			if (!strcmp(valorCampo, rdados->nomeServidor)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "cargoServidor")){
			if (!strcmp(valorCampo, rdados->cargoServidor)){
				flag_encontrou = 1;
			}
		} 
		if (flag_encontrou && !foiRemovido){
			flag_erro = 0;
			buffer_pdiscos = *contpagdisco;
			printf("%s: %d\n", rcabecalho->campos[0], rdados->idServidor);
			if (rdados->salarioServidor >= 0)
				printf("%s: %.2lf\n", rcabecalho->campos[1], rdados->salarioServidor);
			else
				printf("%s: valor nao declarado\n", rcabecalho->campos[1]);
			if (rdados->telefoneServidor[0] != '\0')
				printf("%s: %.14s\n", rcabecalho->campos[2], rdados->telefoneServidor);
			else
				printf("%s: valor nao declarado\n", rcabecalho->campos[2]);
			if (strlen(rdados->nomeServidor) > 0)
				printf("%s: %s\n", rcabecalho->campos[3], rdados->nomeServidor);
			else
				printf("%s: valor nao declarado\n", rcabecalho->campos[3]);
			if (strlen(rdados->cargoServidor) > 0)
				printf("%s: %s\n", rcabecalho->campos[4], rdados->cargoServidor);
			else
				printf("%s: valor nao declarado\n", rcabecalho->campos[4]);
			printf("\n");
			flag_encontrou = 0;
			foiRemovido = 0;
		}
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam impressas
		limpaRegistro(rdados);
	}
	// se a flag erro nao foi alterada em nenhum momento, nada foi encontrado
	if (flag_erro){
		printf("Registro inexistente.\n");
		return;
	}
	else if (!strcmp(nomeCampo, "idServidor")){
		*pdisco_acessadas = buffer_pdiscos/32000;
		if (buffer_pdiscos%32000 > 0)
			*pdisco_acessadas += 1;
	}
	else{
		*pdisco_acessadas = (*contpagdisco)/32000;
		if ((*contpagdisco)%32000 > 0)
			*pdisco_acessadas += 1;
	}
	printf("Número de páginas de disco acessadas: %d\n", *pdisco_acessadas);
	return;
}
int fazLeituraProxRegistroBIN (FILE *arquivoBIN, Reg_Dados *rdados, int *contpagdisco, int *foiRemovido){
	char charRemovido = fgetc(arquivoBIN);
	if (charRemovido == '*'){
		*foiRemovido = 1;
		return 0;
	}
	// pula o campo 'tamanhoRegistro' e 'encadeamentoLista'
	for (int i = 0; i < 12; i++){
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		if (feof(arquivoBIN))
			return 0;
	}
	*contpagdisco += 13;
	// le idServidor
	fread(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	*contpagdisco += sizeof(int)*1;
	// printf("%d", rdados->idServidor);
	// le salarioServidor
	fread(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	*contpagdisco += sizeof(double)*1;
	// if (rdados->salarioServidor == -1)
	// 	printf("         ");
	// else
	// 	printf(" %.2lf", rdados->salarioServidor);
	// le telefoneServidor
	// if(fgetc(arquivoBIN) == '\0'){
		// fseek(arquivoBIN, ftell(arquivoBIN)+13, SEEK_SET);
		// printf("               ");
	// } else{
		// fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
	fread(rdados->telefoneServidor, sizeof(char), 14, arquivoBIN);
	*contpagdisco += sizeof(char)*14;
		// printf(" %.14s", rdados->telefoneServidor);
	// }
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'n'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamNomeServidor
		fread(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		*contpagdisco += sizeof(int)*1;
		// printf(" %d", rdados->tamNomeServidor-2);
		// pula tagCampo4
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le nomeServidor
		// rdados->nomeServidor[rdados->tamNomeServidor+1] = '\0';
		fread(rdados->nomeServidor, sizeof(char), rdados->tamNomeServidor-1, arquivoBIN);
		*contpagdisco += sizeof(char)*rdados->tamNomeServidor;
		// printf(" %s", rdados->nomeServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'c'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamCargoServidor
		fread(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		*contpagdisco += sizeof(int)*1;
		// printf(" %d", rdados->tamCargoServidor-2);
		// pula tagCampo5
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le cargoServidor
		// rdados->cargoServidor[rdados->tamCargoServidor+1] = '\0';
		fread(rdados->cargoServidor, sizeof(char), rdados->tamCargoServidor-1, arquivoBIN);
		*contpagdisco += sizeof(char)*rdados->tamCargoServidor;
		// printf(" %s", rdados->cargoServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	return 1;
}
// FUNCIONALIDADE 99
void imprimeListaRemovidos(FILE *arquivoBIN, Reg_Dados *rdados){
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		return;
	}
	/*
	feito isso, preparamos os contadores
	e posicionamos o ponteiro do arquivoBIN para a primeira posicao da pagina de disco 2
	pois a pagina de disco 1 guarda somente o registro de cabecalho, que nao nos interessa no momento
	*/ 
	fseek(arquivoBIN, 32000, SEEK_SET);
	// separamos tambem uma flag que confere se existem realmente registros
	int registroInexistente = 1;
	/*
	fazemos a leitura do arquivoBIN e imprimos
	ate que aconteca algum erro ou o arquivoBIN chegue no fim
	apos cada leitura, uma linha eh pulada
	tambem conferimos se o proximo char na sequencia eh um '@', a fim de prever lixo usado para preencher uma pagina de disco
	caso um '@' seja encontrado, percorremos o arquivoBIN
	ate que o lixo acabe e o ponteiro alcance o inicio da proxima pagina de disco
	*/
	int pos = (int) getTopoLista(arquivoBIN);
	if (pos == -1)
		printf("Lista de removidos vazia.\n");
	else{
		while (pos != -1 && imprimeProxRegRemovido(arquivoBIN, rdados, &registroInexistente, pos)){
			printf("\n");
			pos = (int) getEncadeamentoLista(arquivoBIN, pos);
		}
	}
	return;
}
int imprimeProxRegRemovido (FILE *arquivoBIN, Reg_Dados *rdados, int *registroInexistente, int pos){
	fseek(arquivoBIN, pos, SEEK_SET);
	// le removido
	fread(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	printf("%c", rdados->removido);
	// le tamanhoRegistro
	fread(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	printf(" %d", rdados->tamanhoRegistro);
	// le encadeamentoLista
	fread(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	printf(" %ld", rdados->encadeamentoLista);
	// le idServidor
	fread(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	printf(" %d", rdados->idServidor);
	// le salarioServidor
	fread(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	if (rdados->salarioServidor == -1)
		printf("         ");
	else
		printf(" %.2lf", rdados->salarioServidor);
	// le telefoneServidor
	if(fgetc(arquivoBIN) == '\0'){
		fseek(arquivoBIN, ftell(arquivoBIN)+13, SEEK_SET);
		printf("               ");
	} else{
		fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		fread(rdados->telefoneServidor, sizeof(char), 14, arquivoBIN);
		printf(" %.14s", rdados->telefoneServidor);
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'n'){
		*registroInexistente = 0;
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamNomeServidor
		fread(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		printf(" %d", rdados->tamNomeServidor-2);
		// pula tagCampo4
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le nomeServidor
		// rdados->nomeServidor[rdados->tamNomeServidor+1] = '\0';
		fread(rdados->nomeServidor, sizeof(char), rdados->tamNomeServidor-1, arquivoBIN);
		printf(" %s", rdados->nomeServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'c'){
		*registroInexistente = 0;
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamCargoServidor
		fread(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		printf(" %d", rdados->tamCargoServidor-2);
		// pula tagCampo5
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le cargoServidor
		// rdados->cargoServidor[rdados->tamCargoServidor+1] = '\0';
		fread(rdados->cargoServidor, sizeof(char), rdados->tamCargoServidor-1, arquivoBIN);
		printf(" %s", rdados->cargoServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
	}
	return 1;
}
// FUNCIONALIDADE 4
void removeRegistro(FILE *arquivoBIN, Reg_Dados *rdados, char *nomeCampo, char *valorCampo, int *erro){
	int flag_encontrou = 0;
	int pos_bin;
	int pos_buffer;
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		*erro = 1;
		return;
	}
	// ignoramos a primeira pagina de disco (que so possui o registro de cabecalho)
	setStatus(arquivoBIN, '0');
	fseek(arquivoBIN, 32000, SEEK_SET);
	// lemos todo o arquivoBIN, registro por registro
	// a cada leitura, o registro resgatado eh testado
	// e seus campos sao combinados com as entradas do usuario
	// caso nenhuma seja encontrada, o proximo registro eh resgatado
	// caso algum valor encontrado seja identico a entrada do usuario, este registro eh removido 
	while (lerRegistroPre(arquivoBIN, rdados)){
		// pula lixo
		while(fgetc(arquivoBIN) == '@'){
		}
		if(!feof(arquivoBIN))
			fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		pos_bin = ftell(arquivoBIN) - (rdados->tamanhoRegistro + 5);
		if (rdados->removido == '*')
			continue;
		// compara o registro lido com a entrada do usuario
		if (!strcmp(nomeCampo, "idServidor")){
			if (atoi(valorCampo) == rdados->idServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "salarioServidor")){
			if (strlen(valorCampo) == 0 && rdados->salarioServidor == -1){
				flag_encontrou = 1;
			}else if (atof(valorCampo) == rdados->salarioServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "telefoneServidor")){
			if (strlen(valorCampo) == 0 && strnlen(rdados->telefoneServidor,14) == 0){
				flag_encontrou = 1;
			}else if (strlen(valorCampo) != 0 && !strncmp(valorCampo, rdados->telefoneServidor,14)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "nomeServidor")){
			if (strlen(valorCampo) == 0 && strlen(rdados->nomeServidor) == 0){
				flag_encontrou = 1;
			}else if (!strcmp(valorCampo, rdados->nomeServidor)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(nomeCampo, "cargoServidor")){
			if (strlen(valorCampo) == 0 && strlen(rdados->cargoServidor) == 0){
				flag_encontrou = 1;
			}else if (!strcmp(valorCampo, rdados->cargoServidor)){
				flag_encontrou = 1;
			}
		} 
		if (flag_encontrou){
			pos_buffer = ftell(arquivoBIN);
			insereListaRemovidos(pos_bin, arquivoBIN);
			fseek(arquivoBIN, pos_buffer, SEEK_SET);
			flag_encontrou = 0;
		}
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam reutiilizadas
		limpaRegistro(rdados);
	}
	setStatus(arquivoBIN, '1');
	return;
}
int lerRegistroPre(FILE *arquivoBIN, Reg_Dados *rdados){
	// le removido
	fread(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// le tamanhoRegistro
	fread(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// le encadeamentoLista
	fread(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;	
	if(rdados->removido == '*') {
		fseek(arquivoBIN, rdados->tamanhoRegistro - 8, SEEK_CUR);
		return 1;
	}
	// le idServidor
	fread(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// printf("%d", rdados->idServidor);
	// le salarioServidor
	fread(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// le telefone
	fread(rdados->telefoneServidor, sizeof(char), 14, arquivoBIN);
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'n'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamNomeServidor
		fread(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		// pula tagCampo4
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le nomeServidor
		fread(rdados->nomeServidor, sizeof(char), rdados->tamNomeServidor-1, arquivoBIN);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// colocar /0
		rdados->nomeServidor[0] = '\0';
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'c'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamCargoServidor
		fread(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		// pula tagCampo5
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le cargoServidor
		fread(rdados->cargoServidor, sizeof(char), rdados->tamCargoServidor-1, arquivoBIN);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		rdados->cargoServidor[0] = '\0';
	}
	return 1;
}
void insereListaRemovidos (int pos_bin, FILE *arquivoBIN){
	// marca campo 'removido' com '*'
	setRemovidoReg(arquivoBIN, pos_bin);
	// caso 1) LISTA VAZIA - INSERE NO INICIO
	if (getTopoLista(arquivoBIN) == -1){
		setTopoLista(arquivoBIN, (long) pos_bin);	
		setEncadeamentoLista(arquivoBIN, pos_bin, -1);
	// caso 2) LISTA NAO VAZIA - INSERE COMUM
	} else {
		int ant = -1;
		int prox = (int) getTopoLista(arquivoBIN);
		int naoInserido = 1;
		while (naoInserido){
			// aqui, encontramos a posicao do registro removido na lista
			if (prox == -1 || getTamanhoRegistro(arquivoBIN, prox) >= getTamanhoRegistro(arquivoBIN, pos_bin)){
				// caso 2.1) o registro removido eh o menor da lista de removidos 
				if (ant == -1)
					setTopoLista(arquivoBIN, (long) pos_bin);
				// caso 2.2) o registro removido nao eh o menor da lista de removidos 
				else
					setEncadeamentoLista(arquivoBIN, ant, (long) pos_bin);
				setEncadeamentoLista(arquivoBIN, pos_bin, (long) prox);
				naoInserido = 0;
			}
			// aqui, ainda nao encontramos a posicao do registro removido na lista
			else{
				ant = prox;
				prox = (int) getEncadeamentoLista(arquivoBIN, prox);
			}
		}
	}
	// preenchemos os campos restantes no registro removido com arroba (exceto os primeiros campos)
	preencheComArroba(arquivoBIN, pos_bin);
	return;
}
// FUNCIONALIDADE 5
void insereRegistro(FILE *arquivoBIN, Reg_Dados *rdados, int *erro){
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		*erro = 1;
		return;
	}
	/*
	feito isso, preparamos os contadores
	e posicionamos o ponteiro do arquivoBIN para a primeira posicao da pagina de disco 2
	pois a pagina de disco 1 guarda somente o registro de cabecalho, que nao nos interessa no momento
	*/ 
	setStatus(arquivoBIN, '0');
	fseek(arquivoBIN, 32000, SEEK_SET);
	/*
	fazemos a leitura do arquivoBIN ate que o novo registro seja inserido
	ou ate que o fim da lista de removidos chegue ao fim, para que possamos seguramente inserir
	o novo registro no fim do arquivo
	*/
	int novaPosInsercao = (int) getTopoLista(arquivoBIN);
	int ant = -1;
	long prox;
	int naoInserido = 1;
	int tipoInsercao; // se for inserir num registro removido (1) se for inserido no fim do arquivo (2)
	while(novaPosInsercao != -1 && naoInserido){
		prox = getEncadeamentoLista(arquivoBIN, novaPosInsercao);
		// testa tamanho
		if (getTamanhoRegistro(arquivoBIN, novaPosInsercao) >= rdados->tamanhoRegistro){
			naoInserido = 0;
			if (ant == -1){
				setTopoLista(arquivoBIN, prox);
			}else{
				setEncadeamentoLista(arquivoBIN, ant, prox);
			}
			// nessa insercao, o tipo eh '1', pois o tamanho do registro
			// nao eh alterado caso este novo registro sobrescreva
			// um registro antigo removido
			insereRegistroPos(arquivoBIN, rdados, novaPosInsercao, 1);
		}
		else{
			ant = novaPosInsercao;
			novaPosInsercao = (int) getEncadeamentoLista(arquivoBIN, novaPosInsercao);
		}
	}
	// se ainda nao foi inserido, vamos inseri-lo no fim do arquivo
	if (naoInserido){
		fseek(arquivoBIN, 0, SEEK_END);
		novaPosInsercao = ftell(arquivoBIN);
		// ja aqui, o tipo da insercao eh '2', pois nao esta sobrescrevendo nenhum registro antigo
		insereRegistroPos(arquivoBIN, rdados, novaPosInsercao, 2);
	}
	setStatus(arquivoBIN, '1');
	return;
}
void insereRegistroPos(FILE *arquivoBIN, Reg_Dados *rdados, int pos, int tipoInsercao){
	fseek(arquivoBIN, pos, SEEK_SET);
	// escreve removido 
	fwrite(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	if (tipoInsercao == 1)
		fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET); // pula tamanhoRegistro
	else if (tipoInsercao == 2)
		fwrite(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN); // escreve tamanhoRegistro
	// escreve encadeamentoLista
	fwrite(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	// escreve idServidor
	fwrite(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	// escreve salarioServidor
	fwrite(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	// escreve telefoneServidor
	fwrite(&(rdados->telefoneServidor), sizeof(char), 14, arquivoBIN);
	if (strlen(rdados->nomeServidor) > 0){	
		// escreve tamNomeServidor
		fwrite(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		// escreve tagCampo4
		char tagNome = 'n';
		fwrite(&(tagNome), sizeof(char), 1, arquivoBIN);
		// escreve nomeServidor
		fwrite(rdados->nomeServidor, sizeof(char), strlen(rdados->nomeServidor)+1, arquivoBIN);
	}
	if (strlen(rdados->cargoServidor) > 0){
		// escreve tamCargoServidor
		fwrite(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		// escreve tagCampo5
		char tagCampo = 'c';
		fwrite(&(tagCampo), sizeof(char), 1, arquivoBIN);
		// escreve cargoServidor
		fwrite(rdados->cargoServidor, sizeof(char), strlen(rdados->cargoServidor)+1, arquivoBIN);
	}
	return;
}
// FUNCIONALIDADE 6
void atualizaRegistro(FILE *arquivoBIN, Reg_Dados *rdados, char *campoBusca, char *valorBusca, char *campoAtualiza, char *valorAtualiza, int *erro){
	int flag_encontrou = 0;
	int pos_bin;
	int pos_buffer;
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		*erro = 1;
		return;
	}
	// ignoramos a primeira pagina de disco (que so possui o registro de cabecalho)
	fseek(arquivoBIN, 32000, SEEK_SET);
	// lemos todo o arquivoBIN, registro por registro
	// a cada leitura, o registro resgatado eh testado
	// e seus campos sao combinados com as entradas do usuario
	// caso nenhuma seja encontrada, o proximo registro eh resgatado
	// caso algum valor encontrado seja identico a entrada do usuario, este registro eh atualizado
	while (lerRegistroPre(arquivoBIN, rdados)){
		// pula lixo
		while(fgetc(arquivoBIN) == '@'){
		}
		if(!feof(arquivoBIN))
			fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		pos_bin = ftell(arquivoBIN) - (rdados->tamanhoRegistro + 5);
		// ignoramos registros removidos
		if (rdados->removido == '*')
			continue;
		// compara o registro lido com a entrada do usuario
		if (!strcmp(campoBusca, "idServidor")){
			if (atoi(valorBusca) == rdados->idServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(campoBusca, "salarioServidor")){
			if (strlen(valorBusca) == 0 && rdados->salarioServidor == -1){
				flag_encontrou = 1;
			}else if (atof(valorBusca) == rdados->salarioServidor){
				flag_encontrou = 1;
			}
		}else if (!strcmp(campoBusca, "telefoneServidor")){
			if (strlen(valorBusca) == 0 && strnlen(rdados->telefoneServidor,14) == 0){
				flag_encontrou = 1;
			}else if (strlen(valorBusca) != 0 && !strncmp(valorBusca, rdados->telefoneServidor,14)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(campoBusca, "nomeServidor")){
			if (strlen(valorBusca) == 0 && strlen(rdados->nomeServidor) == 0){
				flag_encontrou = 1;
			}else if (!strcmp(valorBusca, rdados->nomeServidor)){
				flag_encontrou = 1;
			}
		}else if (!strcmp(campoBusca, "cargoServidor")){
			if (strlen(valorBusca) == 0 && strlen(rdados->cargoServidor) == 0){
				flag_encontrou = 1;
			}else if (!strcmp(valorBusca, rdados->cargoServidor)){
				flag_encontrou = 1;
			}
		} 
		if (flag_encontrou){
			pos_buffer = ftell(arquivoBIN);
			atualizaRegistroPos(pos_bin, arquivoBIN, campoAtualiza, valorAtualiza, rdados);
			fseek(arquivoBIN, pos_buffer, SEEK_SET);
			flag_encontrou = 0;
		}
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam impressas
		limpaRegistro(rdados);
	}
	return;
}
void atualizaRegistroPos(int pos_bin, FILE *arquivoBIN, char *campoAtualiza, char *valorAtualiza, Reg_Dados *rdados){
	// antes de atualizarmos o "rdados" com o valor novo, validamos o novo tamanho em comparacao com o tamanho antigo
	// a fim de que registros que nao encaixem no tamanho disponivel sejam nao sejam inseridos nessa mesma posicao
	int tamanhoValido = tamanhoValidoAtualizacao(arquivoBIN, campoAtualiza, valorAtualiza, rdados);
	// variavel auxiliar que impede atualizacoes repetidas
	int jaFoiAtualizado = 0;
	// primeiro, atualizamos nosso registro de dados com o valor novo
	// ao mesmo tempo, checamos se o novo campo realmente eh diferente do atual (caso contrario, jaFoiAtualizado = 1)
	if (!strcmp(campoAtualiza, "idServidor")){
		if(rdados->idServidor == atoi(valorAtualiza))
			jaFoiAtualizado = 1;
		rdados->idServidor = atoi(valorAtualiza);
	}else if (!strcmp(campoAtualiza, "salarioServidor")){
		if(rdados->salarioServidor == atof(valorAtualiza))
			jaFoiAtualizado = 1;
		rdados->salarioServidor = atof(valorAtualiza);
	}else if (!strcmp(campoAtualiza, "telefoneServidor")){
		if(!strncmp(rdados->telefoneServidor, valorAtualiza,14))
			jaFoiAtualizado = 1;
		strncpy(rdados->telefoneServidor, valorAtualiza,14);
	}else if (!strcmp(campoAtualiza, "nomeServidor")){
		if(!strcmp(rdados->nomeServidor, valorAtualiza))
			jaFoiAtualizado = 1;
		strcpy(rdados->nomeServidor, valorAtualiza);
		rdados->tamNomeServidor = strlen(rdados->nomeServidor)+2;
	}else if (!strcmp(campoAtualiza, "cargoServidor")){
		if(!strcmp(rdados->cargoServidor, valorAtualiza))
			jaFoiAtualizado = 1;
		strcpy(rdados->cargoServidor, valorAtualiza);
		rdados->tamCargoServidor = strlen(rdados->cargoServidor)+2;
	}
	// depois avaliamos a atualizacao
	if(jaFoiAtualizado){
		return;
	}
	else if (tamanhoValido){
		insereRegistroPos(arquivoBIN, rdados, pos_bin, 1);
	}else{
		// remove atual
		insereListaRemovidos(pos_bin, arquivoBIN);
		// insere novo
		int erro;
		insereRegistro(arquivoBIN, rdados, &erro);
	}
	return;
}
int tamanhoValidoAtualizacao(FILE *arquivoBIN, char *campoAtualiza, char *valorAtualiza, Reg_Dados *rdados){
	// variavel que armazena o delta (diferenca) entre o tamanho atual do registro e o novo tamanho apos a atualizacao
	int saldoAtualizacao;
	// variavel que armazena o espaco restante presente no registro atual (ou seja, a quantidade de lixo disponivel)
	int sobraRegistroAtual;
	// variaveis auxiliares que ajudarao no calculo do tamanho do registro
	int naoTemNome = 0, naoTemCargo = 0;
	int parcelaFixaTamanho = 8 + 4 + 8 + 14; // encadeamento + id + salario + telefone
	// calculo da sobra de espaco presente no registro (lixo)
	sobraRegistroAtual = rdados->tamanhoRegistro - parcelaFixaTamanho;
	if (strlen(rdados->nomeServidor) != 0)
		sobraRegistroAtual -= 4 + rdados->tamNomeServidor;
	if (strlen(rdados->cargoServidor) != 0)
		sobraRegistroAtual -= 4 + rdados->tamCargoServidor; 
	// calculo do saldo apos a atualizacao
	if (!strcmp(campoAtualiza, "nomeServidor")){
		saldoAtualizacao = strlen(valorAtualiza) - strlen(rdados->nomeServidor);
		if (strlen(rdados->nomeServidor) == 0)
			naoTemNome = 1;
	}
	else if (!strcmp(campoAtualiza, "cargoServidor")){
		saldoAtualizacao = strlen(valorAtualiza) - strlen(rdados->cargoServidor);
		if (strlen(rdados->cargoServidor) == 0)
			naoTemCargo = 1;
	}
	else{
		// em casos que nao envolvem nome e cargo, o saldo nunca mudara
		saldoAtualizacao = 0;
	}
	if (naoTemNome || naoTemCargo)
		rdados->tamanhoRegistro += 6; // adiciona a tag de "tamanho" + "tag" + "\0"
	rdados->tamanhoRegistro += saldoAtualizacao;
	// se o saldo sera suportado pela sobra presente no registro, retornamos 1
	if (sobraRegistroAtual >= saldoAtualizacao)
		return 1;
	return 0;
}
// SETTERS E GETTERS
long getTopoLista(FILE *arquivoBIN){
	// pula status
	fseek(arquivoBIN, 1, SEEK_SET);
	long topoLista;
	fread(&(topoLista), sizeof(long), 1, arquivoBIN);
	return topoLista;
}
void setTopoLista(FILE *arquivoBIN, long valor){
	// pula status
	fseek(arquivoBIN, 1, SEEK_SET);
	long topoLista = valor;
	fwrite(&(topoLista), sizeof(long), 1, arquivoBIN);
	return;
}
long getEncadeamentoLista(FILE *arquivoBIN, int pos){
	// vamos ate a pos, pulamos 1 (removido) + 4 (tamanhoRegistro)
	fseek(arquivoBIN, (pos+5), SEEK_SET);
	long encadeamentoLista;
	fread(&(encadeamentoLista), sizeof(long), 1, arquivoBIN);
	return encadeamentoLista;
}
void setEncadeamentoLista(FILE *arquivoBIN, int pos, long valor){
	// vamos ate a pos, pulamos 1 (removido) + 4 (tamanhoRegistro)
	fseek(arquivoBIN, (pos+5), SEEK_SET);
	long encadeamentoLista = valor;
	fwrite(&(encadeamentoLista), sizeof(long), 1, arquivoBIN);
	return;
}
int getTamanhoRegistro(FILE *arquivoBIN, int pos){
	// vamos ate a pos, pulamos 1 (removido)
	fseek(arquivoBIN, (pos+1), SEEK_SET);
	int tamanhoRegistro;
	fread(&(tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	return tamanhoRegistro;
}
void setTamanhoRegistro(FILE *arquivoBIN, int pos, int delta){
	// vamos ate a pos, pulamos 1 (removido)
	int tamanhoRegistro = getTamanhoRegistro(arquivoBIN, pos) + delta;
	fseek(arquivoBIN, (pos+1), SEEK_SET);
	fwrite(&(tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	return;
}
void setRemovidoReg(FILE *arquivoBIN, int pos){
	// vamos ate a pos
	fseek(arquivoBIN, pos, SEEK_SET);
	char removido = '*';
	fwrite(&(removido), sizeof(char), 1, arquivoBIN);
	return;
}
void preencheComArroba (FILE *arquivoBIN, int pos){
	// vamos ate a pos pulando 'removido', 'tamanho' e 'encadeamentoLista'
	int tam = getTamanhoRegistro(arquivoBIN, pos) - 8;
	fflush(stdout);
	char arroba[tam];
	for(int i = 0; i < tam; i++){
		arroba[i] = '@';
	}
	fseek(arquivoBIN, (pos+13), SEEK_SET);
	fwrite(arroba, sizeof(char), tam, arquivoBIN);
	return;
}
// FUNCIONALIDADE 7
void ordenarRegistros(FILE *arquivoBIN, Reg_Dados **vetReg, FILE *arquivoBINsaida, Reg_Dados *rdados, Reg_Cabecalho *rcabecalho, Pagina_Disco *paginas, int *erro){
	// antes de tudo, checamos se o arquivo binario pode ser lido (ou seja, esta consistente, status = 1)
	if (!testeEhConsistente(arquivoBIN)){
		printf("Falha no processamento do arquivo.\n");
		*erro = 1;
		return;
	}
	// ignoramos a primeira pagina de disco (que so possui o registro de cabecalho)
	fseek(arquivoBIN, 32000, SEEK_SET);
	// lemos todo o arquivoBIN, registro por registro
	// criamos, assim, a lista de todos os registros dentro da RAM
	int i = 0;
	while (lerRegistroPre(arquivoBIN, rdados)){
		// pula lixo
		while(fgetc(arquivoBIN) == '@'){
		}
		if(!feof(arquivoBIN))
			fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		// ignoramos registros removidos
		if (rdados->removido == '*')
			continue;
		// inserimos o registro no vetor de registros
		vetReg[i]->removido = rdados->removido;
		vetReg[i]->encadeamentoLista = rdados->encadeamentoLista;
		vetReg[i]->idServidor = rdados->idServidor;
		vetReg[i]->salarioServidor = rdados->salarioServidor;
		if (strlen(rdados->telefoneServidor) > 0)
			strncpy(vetReg[i]->telefoneServidor,rdados->telefoneServidor,14);				
		else{
			vetReg[i]->telefoneServidor[0] = '\0';
			for (int j = 1; j < 14; j++)
				vetReg[i]->telefoneServidor[j] = '@';
		}
		vetReg[i]->tamNomeServidor = rdados->tamNomeServidor;
		strcpy(vetReg[i]->nomeServidor,rdados->nomeServidor);
		vetReg[i]->tamCargoServidor = rdados->tamCargoServidor;
		strcpy(vetReg[i]->cargoServidor,rdados->cargoServidor);
		vetReg[i]->tamanhoRegistro = 8 + 4 + 8 + 14;
		if (strlen(vetReg[i]->nomeServidor) > 0){
			vetReg[i]->tamanhoRegistro += 4 + vetReg[i]->tamNomeServidor;
		}
		if (strlen(vetReg[i]->cargoServidor) > 0){
			vetReg[i]->tamanhoRegistro += 4 + vetReg[i]->tamCargoServidor;
		}
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam impressas
		limpaRegistro(rdados);
		i++;
	}
	// ordenamos o vetor de registros
	MS_sort(vetReg, i, sizeof(Reg_Dados*), comparaRegistros);
	// apos ordenar, inserimos estes registros em um novo arquivo binario
	copiaCabecalhoBIN(arquivoBIN, arquivoBINsaida);
	// inicializamos a pagina de disco (considerando que ja passamos pela primeira pagina de disco)
	paginas->nPaginas = 1;
	paginas->k = 0;
	// inicializamos os byteOffset's atual/anterior (variaveis que nos ajudarão na mudanca de tamanhoRegistro durante a quebra de paginas de disco)
	int boAtual = -1;
	int boAnt = -1;
	for (int k = 0; k < i; k++){
		boAtual = ftell(arquivoBINsaida);
		escreveVetorBIN(arquivoBINsaida, vetReg[k], paginas, boAnt);	
		boAnt = boAtual;
	}
}
void copiaCabecalhoBIN(FILE *arquivoBIN, FILE *arquivoBINsaida){
	setTopoLista(arquivoBIN, -1);
	fseek(arquivoBIN, 0, SEEK_SET);
	fseek(arquivoBINsaida, 0, SEEK_SET);
	char c;
	for (int i = 0; i < 32000; i++){
		fread(&c, sizeof(char), 1, arquivoBIN);
		fwrite(&c, sizeof(char), 1, arquivoBINsaida);
	}
	return;
}
void escreveVetorBIN(FILE *arquivoBIN, Reg_Dados *rdados, Pagina_Disco *paginas, int boAnt){
	int deltaNovoTamanho = 0;
	int bytes_restantes = 0;
	// caso 1 - registro a ser inserido ultrapassa a pagina de disco atual
	if ((paginas->k + rdados->tamanhoRegistro + 5) > 32000){
		bytes_restantes = 32000 - paginas->k;
		for (int i = 0; i < bytes_restantes; i++){
			paginas->pagina[paginas->nPaginas][paginas->k++] = '@';
			fwrite(&(paginas->pagina[paginas->nPaginas][paginas->k-1]), sizeof(char), 1, arquivoBIN);
			deltaNovoTamanho += 1;
		}
		paginas->k = 0;
		paginas->nPaginas++;
		int posBuffer = ftell(arquivoBIN);
		setTamanhoRegistro(arquivoBIN, boAnt, deltaNovoTamanho);
		fseek(arquivoBIN, posBuffer, SEEK_SET);
	}
	// caso 2 - registro a ser inserido nao ultrapassa a pagina de disco atual
	// atualiza removido 
	paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	// atualiza tamanhoRegistro
	for (int i = 0; i < 4; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	// atualiza encadeamentoLista
	for (int i = 0; i < 8; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	// atualiza idServidor
	for (int i = 0; i < 4; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	// atualiza salarioServidor
	for (int i = 0; i < 8; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	// atualiza telefoneServidor
	for (int i = 0; i < 14; i++)
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
	fwrite(&(rdados->telefoneServidor), sizeof(char), 14, arquivoBIN);
	if (strlen(rdados->nomeServidor) > 0){
		// atualiza tamNomeServidor
		for (int i = 0; i < 4; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		// atualiza tagCampo4
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		char tagNome = 'n';
		fwrite(&(tagNome), sizeof(char), 1, arquivoBIN);
		// atualiza nomeServidor
		for (int i = 0; i < strlen(rdados->nomeServidor)+1; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(rdados->nomeServidor, sizeof(char), strlen(rdados->nomeServidor)+1, arquivoBIN);
	}
	if (strlen(rdados->cargoServidor) > 0){
		// atualiza tamCargoServidor
		for (int i = 0; i < 4; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		// atualiza tagCampo5
		paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		char tagCargo = 'c';
		fwrite(&(tagCargo), sizeof(char), 1, arquivoBIN);
		// atualiza cargoServidor
		for (int i = 0; i < strlen(rdados->cargoServidor)+1; i++)
			paginas->pagina[paginas->nPaginas][paginas->k++] = '1';
		fwrite(rdados->cargoServidor, sizeof(char), strlen(rdados->cargoServidor)+1, arquivoBIN);
	}
	return;
}
int comparaRegistros(const void *A, const void *B) {
    Reg_Dados **rA, **rB;
    rA = (Reg_Dados**) A;
    rB = (Reg_Dados**) B;
    return ((*rB)->idServidor) - ((*rA)->idServidor);
}
// FUNCOES MATHEUS
void MS_sort(void *vector, unsigned long n, size_t memsize, int (*fcmp)(const void *, const void *)) {
	unsigned long middle, rN, j, k;
	void *aux, *r;

	if(n < 2) {
		return;
	}

	middle = (n / 2);
	r = vector + middle * memsize;
	rN = n - middle;
	MS_sort(vector, middle, memsize, fcmp);
	MS_sort(r, rN, memsize, fcmp);

	aux = (void *) malloc(memsize * n);
	j = k = 0;
	while(j + k < n) {
		if(k >= rN || (j < middle && fcmp(vector + memsize * j, r + memsize * k) >= 0)) {
			memcpy(aux + memsize * (j + k), vector + memsize * j, memsize);
			j++;
		} else {
			memcpy(aux + memsize * (j + k), r + memsize * k, memsize);
			k++;
		}
	}
	memcpy(vector, aux, memsize * n);
	free(aux);
}
// FUNCIONALIDADE 8
//funcao muito similar a "lerRegistroPre", porem com algumas modificacoes para atender melhor as funcionalidades 8 e 9
int lerProxRegistro(FILE *arquivoBIN, Reg_Dados *rdados, int* ultimoReg, int* pulaReg){
	// le removido
	fread(&(rdados->removido), sizeof(char), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// le tamanhoRegistro
	fread(&(rdados->tamanhoRegistro), sizeof(int), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;
	// le encadeamentoLista
	fread(&(rdados->encadeamentoLista), sizeof(long), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;	
	if(rdados->removido == '*') {
		fseek(arquivoBIN, rdados->tamanhoRegistro - 8, SEEK_CUR);
		return 1;
	}
	// le idServidor
	fread(&(rdados->idServidor), sizeof(int), 1, arquivoBIN);
	if (DEBUG) printf("id lido: %d\n", rdados->idServidor);
	if (feof(arquivoBIN))
		return 0;
	// le salarioServidor
	fread(&(rdados->salarioServidor), sizeof(double), 1, arquivoBIN);
	if (DEBUG) printf("salario lido: %lf\n", rdados->salarioServidor);
	if (feof(arquivoBIN))
		return 0;
	// le telefone
	fread(rdados->telefoneServidor, sizeof(char), 14, arquivoBIN);
	if (DEBUG) printf("telefone lido: %s\n", rdados->telefoneServidor);
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'n'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamNomeServidor
		fread(&(rdados->tamNomeServidor), sizeof(int), 1, arquivoBIN);
		// pula tagCampo4
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le nomeServidor
		fread(rdados->nomeServidor, sizeof(char), rdados->tamNomeServidor-1, arquivoBIN);
		if (DEBUG) printf("nome lido: %s\n", rdados->nomeServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// colocar /0
		rdados->nomeServidor[0] = '\0';
		if (DEBUG) printf("sem nome\n");
	}
	fseek(arquivoBIN, ftell(arquivoBIN)+4, SEEK_SET);
	if (fgetc(arquivoBIN) == 'c'){
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		// le tamCargoServidor
		fread(&(rdados->tamCargoServidor), sizeof(int), 1, arquivoBIN);
		// pula tagCampo5
		fseek(arquivoBIN, ftell(arquivoBIN)+1, SEEK_SET);
		// le cargoServidor
		fread(rdados->cargoServidor, sizeof(char), rdados->tamCargoServidor-1, arquivoBIN);
		if (DEBUG) printf("cargo lido: %s\n", rdados->cargoServidor);
	}
	else{
		fseek(arquivoBIN, ftell(arquivoBIN)-5, SEEK_SET);
		rdados->cargoServidor[0] = '\0';
		if (DEBUG) printf("sem nome\n");
	}
	return 1;
}

void escreveRegistro(Reg_Dados* reg, FILE* saida){
	fwrite(&reg->removido, sizeof(char), 1, saida);
	fwrite(&reg->tamanhoRegistro, sizeof(int), 1, saida);
	fwrite(&reg->encadeamentoLista, sizeof(long int), 1, saida);
	fwrite(&reg->idServidor, sizeof(int), 1, saida);
	fwrite(&reg->salarioServidor, sizeof(double), 1, saida);
	fwrite(reg->telefoneServidor, 14, 1, saida);
	if(strlen(reg->nomeServidor) > 1){
		fwrite(&reg->tamNomeServidor, sizeof(int), 1, saida);
		fwrite("n", sizeof(char), 1, saida);
		fwrite(reg->nomeServidor, strlen(reg->nomeServidor)+1, 1, saida);
	}		
	if(strlen(reg->cargoServidor) > 1){
		fwrite(&reg->tamCargoServidor, sizeof(int), 1, saida);
		fwrite("c", sizeof(char), 1, saida);
		fwrite(reg->cargoServidor, strlen(reg->cargoServidor)+1, 1, saida);
	}
}

/*	Esta funcao verifica se um registro lido e que esta elegivel para ser escrito contem lixo, por ser o final da pagina de disco do arquivo
	de origem. Caso afirmativo, o tamanho do registro e atualizado para nao levar mais em conta o lixo.
*/
void confereFinalPaginaEntrada(Reg_Dados* reg){
	int tam_reg = 34;
	if(reg->nomeServidor[0] != '\0'){
		tam_reg += reg->tamNomeServidor + 4;
	}	
	if(reg->cargoServidor[0] != '\0'){
		tam_reg += reg->tamCargoServidor + 4;
	}
	
	if(reg->tamanhoRegistro > tam_reg){
		if (DEBUG) printf("\tregistro precisa ter tamanho atualizado\n");		 
		reg->tamanhoRegistro = tam_reg;
	}
}

/*	Esta funcao confere se o que esta sendo escrito na saida necessita finalizar uma pagina de disco,
	e assim se tomam as medidas necessarias, atualizando sempre o tamanho atual de pagina, para futuras chamadas da mesma funcao.
*/
void confereFinalPaginaSaida(FILE* saida, Reg_Dados* reg, int ultimoReg, int* tamPag){
	if((*tamPag + reg->tamanhoRegistro+5) > 32000){
		int back = ftell(saida);	//guarda a posicao atual para voltar ao final das operacoes
		
		fseek(saida, ultimoReg + 1, SEEK_SET);	//volta para o inicio do ultimo registro, na posicao tamanhoRegistro
	
		int tam;
		fread(&tam, sizeof(int), 1, saida);	//le o tamanho do ultimo registro e guarda em tam	
			
		/*	este bloco de codigo a seguir volta o ponteiro do arquivo para o campo tamanhoRegistro
			e atualiza o tamanho do ultimo registro, com a quantidade de @s 	*/
		tam += 32000 - *tamPag;		
		fseek(saida, -sizeof(int), SEEK_CUR);
		fwrite(&tam, sizeof(int), 1, saida);
			
		/*	este bloco de codigo faz com que o ponteiro do arquivo va para o final do ultimo registro,
			 e se preenche o final da pagina de disco com @s	*/
		fseek(saida, back, SEEK_SET);	
		for(int i = 0; i < 32000 - *tamPag; i++){
			fwrite("@", sizeof(char), 1, saida);
		} 
			
		*tamPag = reg->tamanhoRegistro + 5;	//zeramos o tamanho da nova pagina de disco, e somamos com o valor do registro a ser inserido
	}
	else *tamPag += reg->tamanhoRegistro + 5;	//atualizamos aqui o tamanho atual da pagina de disco em questao
}

/*	Esta funcao le um registro do arquivo menor e o compara com o registro atual lido do arquivo maior, o registro com menor valor de id
	e retornado para depois ser escrito no arquivo final. O arquivo nao escolhido tem seu ponteiro reposicionado no inicio da ultima leitura feita, de forma a nao ser perdido.
	Caso o arquivo menor acabe, esta funcao apenas retorna o registro lido do arquivo maior.
*/
Reg_Dados* escolheRegistro(FILE* entradaMaior, Reg_Dados* reg1, FILE* entradaMenor, Reg_Dados* reg2, int* acabou, int* ultimoReg, int* pulaReg, int* entrada1){
	if (*acabou == 1) return reg1;
	
	if(lerProxRegistro(entradaMenor, reg2, ultimoReg, pulaReg) != 1){		
		*acabou = 1;
		return reg1;
	}

	//pula lixo
	while(fgetc(entradaMenor) == '@'){
	}
	if (!feof(entradaMenor)){
		fseek(entradaMenor, -1, SEEK_CUR);
	}
	
	if (DEBUG) printf("\t\tid1: %d, nome1: %s\n", reg1->idServidor, reg1->nomeServidor);
	if (DEBUG) printf("\t\tid2: %d, nome2: %s\n", reg2->idServidor, reg2->nomeServidor);
	
	if(reg1->idServidor == reg2->idServidor){		
		if (*entrada1 == 2) return reg2;
		else return reg1;	
	}
	if(reg1->idServidor < reg2->idServidor){
		fseek(entradaMenor, -(reg2->tamanhoRegistro+5), SEEK_CUR);
		return reg1;
	}
	else {
		fseek(entradaMaior, -(reg1->tamanhoRegistro+5), SEEK_CUR);
		return reg2;
	}
}

//	Esta funcao impede que lixo remanescente de leituras anteriores cause problemas de escrita relativos aos registros auxiliares
void limpaRegistros(Reg_Dados* r1, Reg_Dados* r2, Reg_Dados* r3){
	free(r1);
	r1 = (Reg_Dados*) malloc(sizeof(Reg_Dados));
	
	free(r2);
	r2 = (Reg_Dados*) malloc(sizeof(Reg_Dados));

	free(r3);
	r3 = (Reg_Dados*) malloc(sizeof(Reg_Dados));
}

/*	Esta funcao consiste em um loop que le registro por registro do maior dos arquivos de entrada da funcionalidade 8,
	e em seguida le o registro do menor. Assim, decide qual sera escrito no arquivo de saida, fazendo as operacoes necessarias
	para isso.
*/
void merging(FILE* entradaMaior, FILE* entradaMenor, FILE* saida, int* entrada1){
	fseek(entradaMaior, 0, SEEK_SET);
	fwrite("0", sizeof(char), 1, entradaMaior);
	fseek(entradaMenor, 0, SEEK_SET);		
	fwrite("0", sizeof(char), 1, entradaMenor);
	fseek(saida, 0, SEEK_SET);		
	fwrite("0", sizeof(char), 1, saida);

	copiaCabecalhoBIN(entradaMenor, saida);
	
	Reg_Dados* reg1 = (Reg_Dados*) malloc(sizeof(Reg_Dados));	
	Reg_Dados* reg2 = (Reg_Dados*) malloc(sizeof(Reg_Dados));
	Reg_Dados* escolhido;

	fseek(entradaMaior, 32000, SEEK_SET);
	fseek(entradaMenor, 32000, SEEK_SET);
	fseek(saida, 32000, SEEK_SET);

	int tamPag = 0;
	int ultimoReg = 0;
	int acabou = 0;
	int pulaReg = 0;

	while (lerProxRegistro(entradaMaior, reg1, &ultimoReg, &pulaReg)){
		//pula lixo
		while(fgetc(entradaMaior) == '@'){
		}
		if (!feof(entradaMaior)){
			fseek(entradaMaior, -1, SEEK_CUR);
		}
		//escolhe qual registro vai ser escrito, de forma a preservar a ordenacao por id
		escolhido = escolheRegistro(entradaMaior, reg1, entradaMenor, reg2, &acabou, &ultimoReg, &pulaReg, entrada1);
		
		confereFinalPaginaEntrada(escolhido);
		confereFinalPaginaSaida(saida, escolhido, ultimoReg, &tamPag);		
		ultimoReg = ftell(saida);
		
		escreveRegistro(escolhido, saida);
		
		limpaRegistros(reg1, reg2, escolhido);
	}

	limpaRegistros(reg1, reg2, escolhido);
	if (lerProxRegistro(entradaMenor, reg2, &ultimoReg, &pulaReg)){
		escreveRegistro(reg2, saida);
	}
	
	fseek(entradaMaior, 0, SEEK_SET);
	fwrite("1", sizeof(char), 1, entradaMaior);
	fseek(entradaMenor, 0, SEEK_SET);		
	fwrite("1", sizeof(char), 1, entradaMenor);
	fseek(saida, 0, SEEK_SET);		
	fwrite("1", sizeof(char), 1, saida);
}
//FUNCIONALIDADE 9

/*	Esta funcao e responsavel por retornar um registro nao nulo caso sejam detectados registros repetidos
	nos arquivos de origem.
	Alem disso ela percorre de forma correta os arquivos, a fim de nao perder uma possivel verificacao de repeticao. 
*/
Reg_Dados* detectaRegistrosIguais(FILE* entradaMaior, Reg_Dados* reg1, FILE* entradaMenor, Reg_Dados* reg2, int* acabou, int* ultimoReg, int* pulaReg, int* entrada1){
	if(lerProxRegistro(entradaMenor, reg2, ultimoReg, pulaReg) != 1){		
		*acabou = 1;
		return NULL;
	}
	//pula lixo
	while(fgetc(entradaMenor) == '@'){
	}
	if (!feof(entradaMenor)){
		fseek(entradaMenor, -1, SEEK_CUR);
	}
	
	if (DEBUG) printf("\t\tid1: %d, nome1: %s\n", reg1->idServidor, reg1->nomeServidor);
	if (DEBUG) printf("\t\tid2: %d, nome2: %s\n", reg2->idServidor, reg2->nomeServidor);
	
	if(reg1->idServidor < reg2->idServidor){
		fseek(entradaMenor, -(reg2->tamanhoRegistro+5), SEEK_CUR);
		return NULL;
	}
	else{
		if(reg1->idServidor == reg2->idServidor){		
			if (*entrada1 == 2) return reg2;
			else return reg1;	
		}
		else{
			fseek(entradaMaior, -(reg1->tamanhoRegistro+5), SEEK_CUR);
			return NULL;
		}
	} 
	return NULL;
}

/*	Esta funcao consiste em um loop que le registro por registro do maior dos arquivos de entrada da funcionalidade 9,
	e em seguida le o registro do menor. Assim percorrem-se paralelamente os dois arquivos de entrada em disco registro por registro,
	e quando dois registros de origens diferentes possuem o mesmo id, sao escritos no arquivo de saida,
	 com as verificacoes necessarias (paginas de disco, etc).
*/
void matching(FILE* entradaMaior, FILE* entradaMenor, FILE* saida, int* entrada1){
	fseek(entradaMaior, 0, SEEK_SET);
	fwrite("0", sizeof(char), 1, entradaMaior);
	fseek(entradaMenor, 0, SEEK_SET);		
	fwrite("0", sizeof(char), 1, entradaMenor);
	fseek(saida, 0, SEEK_SET);		
	fwrite("0", sizeof(char), 1, saida);

	copiaCabecalhoBIN(entradaMenor, saida);
	
	Reg_Dados* reg1 = (Reg_Dados*) malloc(sizeof(Reg_Dados));	
	Reg_Dados* reg2 = (Reg_Dados*) malloc(sizeof(Reg_Dados));
	Reg_Dados* pivo;

	fseek(entradaMaior, 32000, SEEK_SET);
	fseek(entradaMenor, 32000, SEEK_SET);
	fseek(saida, 32000, SEEK_SET);

	int tamPag = 0;
	int ultimoReg = 0;
	int acabou = 0;
	int pulaReg = 0;

	while (lerProxRegistro(entradaMaior, reg1, &ultimoReg, &pulaReg)){
		//pula lixo
		while(fgetc(entradaMaior) == '@'){
		}
		if (!feof(entradaMaior)){
			fseek(entradaMaior, -1, SEEK_CUR);
		}

		pivo = detectaRegistrosIguais(entradaMaior, reg1, entradaMenor, reg2, &acabou, &ultimoReg, &pulaReg, entrada1);
		if (acabou) break;		

		if (pivo != NULL){
			confereFinalPaginaEntrada(pivo);
			confereFinalPaginaSaida(saida, pivo, ultimoReg, &tamPag);		
			ultimoReg = ftell(saida);
		
			 escreveRegistro(pivo, saida);
		}
	
		limpaRegistros(reg1, reg2, pivo);
	}
	fseek(entradaMaior, 0, SEEK_SET);
	fwrite("1", sizeof(char), 1, entradaMaior);
	fseek(entradaMenor, 0, SEEK_SET);		
	fwrite("1", sizeof(char), 1, entradaMenor);
	fseek(saida, 0, SEEK_SET);		
	fwrite("1", sizeof(char), 1, saida);
}

// FUNCOES MATHEUS
void scan_quote_string(char *str) {

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}
void trim(char *str) {

	/*
	*	Essa função arruma uma string de entrada "str".
	*	Manda pra ela uma string que tem '\r' e ela retorna sem.
	*	Ela remove do início e do fim da string todo tipo de espaçamento (\r, \n, \t, espaço, ...).
	*	Por exemplo:
	*
	*	char minhaString[] = "    \t TESTE  DE STRING COM BARRA R     \t  \r\n ";
	*	trim(minhaString);
	*	printf("[%s]", minhaString); // vai imprimir "[TESTE  DE STRING COM BARRA R]"
	*
	*/

	size_t len;
	char *p;

	for(len = strlen(str); len > 0 && isspace(str[len - 1]); len--); // remove espaçamentos do fim
	str[len] = '\0';
	for(p = str; *p != '\0' && isspace(*p); p++); // remove espaçamentos do começo
	len = strlen(p);
	memmove(str, p, sizeof(char) * (len + 1));
}
void binarioNaTela1(FILE *ponteiroArquivoBinario) {

	/* Escolha essa função se você ainda tem o ponteiro de arquivo 'FILE *' aberto.
	*  Lembrando que você tem que ter aberto ele no fopen para leitura também pra funcionar (exemplo: rb, rb+, wb+, ...) */

	unsigned char *mb;
	unsigned long i;
	size_t fl;
	fseek(ponteiroArquivoBinario, 0, SEEK_END);
	fl = ftell(ponteiroArquivoBinario);
	fseek(ponteiroArquivoBinario, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, ponteiroArquivoBinario);
	for(i = 0; i < fl; i += sizeof(unsigned char)) {
		printf("%02X ", mb[i]);
		if((i + 1) % 16 == 0)	printf("\n");
	}
	free(mb);
}