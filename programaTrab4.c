/* Trabalho 4 - Organização de Arquivos
	Nomes da dupla:
	- FELIPE MOREIRA NEVES DE SOUZA - 10734651
	- BRUNO BALDISSERA CARLOTTO - 10724351
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "programaTrab4.h"
#include "funcoesMonitor.h"

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
void limpaRegistroIndice(iReg_Dados *irdados){
	irdados->chaveBusca[0] = '\0';
	irdados->byteOffset = -1;
	return;
}
int buscaBinariaIndice(iVetReg *vet, char *chave, int ini, int fim) {
	// 0 - caso base (busca sem sucesso)
	if (ini > fim) return -1;

	// 1 - calcula ponto central e verifica se chave foi encontrada
	int centro = (int)((ini+fim)/2.0);
	if (!strcmp(vet->v[centro]->chaveBusca, chave)){
		// retornar o primeiro (mais a esquerda) que da match com nomeServidor
		while(centro != 0 && !strcmp(vet->v[centro]->chaveBusca, chave))
			centro--;
		return centro;
	}

	// 2 - chamada recursiva para metade do espaco de busca
	if (strcmp(chave, vet->v[centro]->chaveBusca) < 0)
		// se chave eh menor, fim passa ser o centro-1
		return busca_binaria(vet, chave, ini, centro-1);

	if (strcmp(chave, vet->v[centro]->chaveBusca) > 0)
		// se a chave eh maior, inicio passa ser centro+1
		return busca_binaria(vet, chave, centro+1, fim);
	return -1;
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
int comparaRegistrosIndice(const void *A, const void *B) {
    iReg_Dados **rA, **rB;
    rA = (iReg_Dados**) A;
    rB = (iReg_Dados**) B;
	if (!strcmp((*rA)->chaveBusca, (*rB)->chaveBusca)){
		return ((*rB)->byteOffset - (*rA)->byteOffset);
	}
    return (strcmp((*rB)->chaveBusca, (*rA)->chaveBusca));
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

// FUNCIONALIDADE 10
void copiaIndiceRAM(FILE *arquivoBIN, iReg_Dados *irdados, iVetReg *vetRegIndice, int *erro){
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
	int i = 0;
	// lemos todo o arquivoBIN, registro por registro
	while (lerProxRegIndice(arquivoBIN, irdados)){
		// pula lixo
		while(fgetc(arquivoBIN) == '@'){
		}
		if(!feof(arquivoBIN))
			fseek(arquivoBIN, ftell(arquivoBIN)-1, SEEK_SET);
		pos_bin = ftell(arquivoBIN) - 128;
		// inserindo o registro atual no vetor
		strcpy(vetRegIndice->v[i]->chaveBusca, irdados->chaveBusca);
		vetRegIndice->v[i]->byteOffset = irdados->byteOffset;
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam reutiilizadas
		limpaRegistroIndice(irdados);
		i++;
	}
	// por fim, copiamos o ultimo registro presente no arquivo de indice
	strcpy(vetRegIndice->v[i]->chaveBusca, irdados->chaveBusca);
	vetRegIndice->v[i]->byteOffset = irdados->byteOffset;
	i++;
	vetRegIndice->tam = i;
	setStatus(arquivoBIN, '1');
	return;
}
int lerProxRegIndice(FILE *arquivoBIN, iReg_Dados *irdados){
	// le chaveBusca
	fread(irdados->chaveBusca, sizeof(char), 120, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;	
	/* se a chaveBusca nao tem '\0', habilita essa parte
	char c = 'a';
	int i = 0;
	while (c != '@' && i < 120)
		c = irdados->chaveBusca[i++];
	if (i < 120)
		irdados->chaveBusca[i] = '\0';
	*/
	// le byteOffset
	fread(&(irdados->byteOffset), sizeof(long), 1, arquivoBIN);
	if (feof(arquivoBIN))
		return 0;	
	return 1;
}
void escreveRAMIndice(FILE *arquivoBIN, iVetReg *vetRegIndice){
	// pulamos o cabecalho e escrevemos os registros de dados
	fseek(arquivoBIN, 0, SEEK_SET);
	char c = '1';
	fwrite(&c, sizeof(char), 1, arquivoBIN);
	int tam = vetRegIndice->tam;
	fwrite(&tam, sizeof(int), 1, arquivoBIN);
	char arroba[tam];
	for(int j = 0; j < tam; j++){
		arroba[j] = '@';
	}
	fseek(arquivoBIN, (pos+13), SEEK_SET);
	fwrite(arroba, sizeof(char), tam, arquivoBIN);
	for (int i = 0; i < vetRegIndice->tam; i++){
		fwrite(vetRegIndice->v[i]->chaveBusca, sizeof(char), 120, arquivoBIN);
		fwrite(&(vetRegIndice->v[i]->byteOffset), sizeof(long), 1, arquivoBIN);
	}
	return;
}
// FUNCIONALIDADE 12
void removeRegistroIndice(FILE *arquivoBIN, FILE *arquivoBINsaida, Reg_Dados *rdados, char *nomeCampo, char *valorCampo, iVetReg *vetRegIndice, int *erro){
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
			// remover do vetor de indice junto do arquivoBIN
			if (strlen(rdados->nomeServidor) > 0){
				// remove fazendo busca binaria no vetor indice
				int testeResBB = buscaBinariaIndice(vetRegIndice, rdados->nomeServidor, 0, vetRegIndice->tam);
				int resBB = -1;
				if (testeResBB != -1){
					// ir para frente ate dar match com o byte offset
					while(resBB == -1 && testeResBB < vetRegIndice->tam && !strcmp(vetRegIndice->v[testeResBB]->chaveBusca, rdados->nomeServidor){
						if (vetRegIndice->v[testeResBB]->byteOffset == (long) pos_bin)
							resBB = testeResBB;
						testeResBB++;
					}
				}
				if (resBB == -1)
					printf("Deu ruim");
				else{
					// ao encontrar, shiftar todos os proximos para pos-1
					for (int i = resBB; i < vetRegIndice->tam - 1; i++){
						vetRegIndice->v[i]->byteOffset = vetRegIndice->v[i+1]->byteOffset;
						strcpy(vetRegIndice->v[i]->chaveBusca, vetRegIndice->v[i+1]->chaveBusca);
					}
					// diminuir tam do vetor -1
					vetRegIndice->tam = vetRegIndice->tam - 1;
				}
			}
			// apos remover, ordenar novamente o vetor de indice
			MS_sort(vetRegIndice->v, vetRegIndice->tam, sizeof(iReg_Dados), comparaRegistrosIndice);
			fseek(arquivoBIN, pos_buffer, SEEK_SET);
			flag_encontrou = 0;
		}
		// os registros sao sempre limpados, a fim de que informacoes de registros antigos nao sejam reutiilizadas
		limpaRegistro(rdados);
	}
	setStatus(arquivoBIN, '1');
	return;
}