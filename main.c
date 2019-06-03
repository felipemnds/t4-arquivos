#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "programaTrab4.h"
#include "funcoesMonitor.h"

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
		
		if(!testeEhConsistente(arquivoBIN)){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		} 
		
		if(!testeEhConsistente(arquivoBIN2)){
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
		
		if(!testeEhConsistente(arquivoBIN)){
			printf("Falha no processamento do arquivo.\n");
			return 0;
		} 
		
		if(!testeEhConsistente(arquivoBIN2)){
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
