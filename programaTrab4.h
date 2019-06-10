#ifndef PROGRAMA_TRAB4_H
#define PROGRAMA_TRAB4_H

#include <stdio.h>

#include "funcoesMonitor.h"

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
/*
Registro de cabecalho (indice)
*/
typedef struct reg_cabecalho_indice{
	char status;
	int nroRegistros;
}iReg_Cabecalho;
/*
Registro de dados (indice)
*/
typedef struct reg_dados_indice{
	char chaveBusca[120];
	long byteOffset;
}iReg_Dados;
/*
Vetor de registros de dados do indice
*/
typedef struct vet_reg_dados_indice{
	iReg_Dados **v;
	int tam;
}iVetReg;

typedef struct no_lista_indice{
	struct no_lista_indice* prox;
	iReg_Dados* dados;
}No;

typedef struct lista_{
	No* inicio;
	No* fim;
	int tam;
}Lista;

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
int buscaRegistrosBIN(int*, FILE*, Reg_Dados*, char*, char*, Reg_Cabecalho*, int*);
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

/* removeRegistroIndice
	Funcao que busca pelos registros que satisfazem a busca do usuario e insere tais registros na lista de removidos 
	Alem disso, atualiza o arquivo de indice enviado como um dos parametros da funcao
*/
void removeRegistroIndice(FILE *arquivoBIN, FILE *arquivoBINsaida, Reg_Dados *rdados, char *nomeCampo, char *valorCampo, iVetReg *vetRegIndice, int *erro);

//FUNCIONALIDADE 10

void escreveIndice(No* indice, FILE* saida);

void transfereLista(Lista* indices, FILE* saida);

void criaArquivoIndice(FILE* entrada, FILE* saida, int* erro);

//FUNCIONALIDADE 11
void guardaDescricoesCabecalho(FILE* entrada, Reg_Cabecalho* cab);

void imprime_registro_encontrado(FILE* bin, iReg_Dados* indiceAux, Reg_Cabecalho* cab);

void verificaRecuperacao(FILE* entradaIndices, FILE* entradaBin, iReg_Dados* indiceAux, char campo[], char valor[], Reg_Cabecalho* cab, int* encontrado, int* acessosDados);

int lerProxIndice(FILE* entrada, iReg_Dados* indice);

int recuperaDados(FILE* entradaIndices, FILE* entradaBin, char campo[], char valor[], int origemChamada);/*
*/
int lerProxRegIndice(FILE *arquivoBIN, iReg_Dados *irdados);
/*
*/
void copiaIndiceRAM(FILE *arquivoBIN, iReg_Dados *irdados, iVetReg *vetRegIndice, int *erro);
/*
*/
void escreveRAMIndice(FILE *arquivoBIN, iVetReg *vetRegIndice);
/*
*/
void insereRegistroIndice(FILE *arquivoBIN, FILE *arquivoBINsaida, Reg_Dados *rdados, iVetReg *vetRegIndice, int *erro);
#endif
