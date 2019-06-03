# Arquivo de índice
## REGISTRO DE CABECALHO
Deve completar uma página de disco inteira
- status (char)
- nroRegistros (int)
  - qtdd de registros de dados do arquivo de índice
## REGISTRO DE DADOS
Campos e registros de tamanho fixo
- chaveBusca (string de 120 bytes)
  - chave de busca do índice
- byteOffset (long)
  - byte offset da chave correspondente

# Funções
## 10 - criaNovoIndice (leEntradaEscreveIndice, copiaIndiceRAM, MS_sort(comparaIndices), escreveRAMIndice)
- criar um arquivo de indice secundário fortemente ligado
  - criar um registro de cabecalho aux
  - criar um registro de dados aux
  - criar um vetor de iReg_Dados
  - entrar na funcao 10 (copiaIndiceRAM)
    - levar junto a estrutura de pagina de discos e um outro vetor (de iReg_Dados)
    - preencher o registro de cabecalho
      - status = 0
      - nroRegistros começa com 0
    - armazena o byte offset (acho que a funcao "remove" faz isso)
    - preencher os registros de dados, lendo um por um (lerRegistroPre) com um Reg_Dados comum
    - coloca o nome no "chaveBusca" do iReg_Dados
    - adapta a funcao insereRegistro (pois ela trata fim de paginas de disco)
- inserir tudo seco do arquivo no vetor
- ordena registros adaptando a funcao fcmp (em ordem alfabetica, se empatar nome, ordena por byteoffset) 
- ignorar removidos e nomes nulos
- fechar com binarioNaTela1

``` 10 arquivoEntrada.bin arquivoIndiceNomeServidor.bin ```

## 11 - copiaIndiceRAM, buscaRegistrosIndice (buscaBinariaIndice, <trecho que imprime buscas>, <trecho que calcula paginas de disco acessadas>), escreveRAMIndice
- ler uma busca do usuário por nomeServidor
- carregar o arquivo de índice para a RAM num vetor (pra fazer buscas binárias eficientes)
- buscar no índice (busca binaria)
  - ao encontrar
    - ir para o primeiro
      - ou fica no registro encontrado (se o anterior for diferente)
      - ou vai infinito para os anteriores até achar o primeiro diferente
    - a partir do primeiro, vai retornando até o fim (retornar 0, 1 ou vários registros)
- imprimir da seguinte forma ("valor nao declarado" para nulos)
  - desCampox: [valor declarado]
  - 1 reg por linha
  - ao fim, mostrar [n de acessos a disco na operacao de carregamento] + [numero de acessos a disco para acessar o arquivo de dados]
    - carregamento:
      - calcula paginas de disco normalmente, mas no arquivo de indice
    - leitura arquivo de dados:
      - cria uma variavel "pagina atual" que comeca com -1
      - criar os casos ao acessar um byte offset que deu match
        - if (pag atual == -1)
          - pag atual = bo / 32000;
        - else if (bo / 32000 != pag atual)
          - pag atual = bo / 32000;
          - pag acessadas++;
```
11 arquivo.bin nomeServidor CRISTIANO ANDRE DA SILVA
numero de identificacao do servidor: 6202963
salario do servidor: 4097.91
telefone celular do servidor: (60)99435-1564
nome do servidor: CRISTIANO ANDRE DA SILVA
cargo do servidor: AGENTE ADMINISTRATIVO
---pular uma linha em branco ----
Número de páginas de disco para carregar o arquivo de índice: 5
Número de páginas de disco para acessar o arquivo de dados: 1
```

## 12 - copiaIndiceRAM, removeRegistroIndice ({removeRegistro}, buscaRemocaoIndice (buscaBinariaIndice, <trecho que valida byte offset>, shiftIndice)), escreveRAMIndice
- estender a func4 (remocao)
- carregar o arquivo de índice para a RAM num vetor (pra fazer buscas binárias eficientes)
  - usar a copiaIndiceRAM criada na func10
- enquanto as n remoções acontecerem, atualizar o índice só na RAM
  - criar outra funcao pra buscar 
    - internamente, busca pela chave
    - se busca tiver o mesmo byte offset
      - shifta os proximos para pos-1
      - diminui o tamVetor em 1 (ou seja, talvez eh uma boa criar uma struct com vet e int tamVetor)
- no fim, operação de reescrita do índice (wb pra escrever no arquivo)
> saída: listar o arquivo de índice

## 13 - copiaIndiceRAM, insereRegistroRAM ({insereRegistro}, insereIndice, MS_sort(comparaIndices)), escreveRAMIndice
- estender a func5 (insercao)
- fazer as mesmas coisas que a func12
  - carregar o arquivo de índice para a RAM num vetor (pra fazer buscas binárias eficientes)
    - usar a copiaIndiceRAM criada na func10
  - enquanto as n insercoes acontecerem, atualizar o índice só na RAM (só inserir no fim do vetor)
  - ao fim, ordenar o vetor na RAM com os indices
  - no fim, operação de reescrita do índice (wb pra escrever no arquivo)
> saída: listar o arquivo de índice

## 14
- executar uma busca com a func 3 e armazenar as paginas de disco
- executar uma busca com a func 11 e armazenar as paginas de disco (ignorando as usadas no carregamento)
```
14 arquivo.bin arquivoIndiceServidor.bin nomeServidor CRISTIANO ANDRE DA SILVA
*** Realizando a busca sem o auxílio de índice
numero de identificacao do servidor: 6202963
salario do servidor: 4097.91
telefone celular do servidor: (60)99435-1564
nome do servidor: CRISTIANO ANDRE DA SILVA
cargo do servidor: AGENTE ADMINISTRATIVO
---pular uma linha em branco----
Número de páginas de disco acessadas: 2
***  Realizando  a  busca com  o  auxílio  de  um  índice  secundário fortemente ligado
numero de identificacao do servidor: 6202963
salario do servidor: 4097.91
telefone celular do servidor: (60)99435-1564
nome do servidor: CRISTIANO ANDRE DA SILVA
cargo do servidor: AGENTE ADMINISTRATIVO
---pular uma linha em branco ----
Número de páginas de disco para carregar o arquivo de índice: x
Número de páginas de disco para acessar o arquivo de dados: 1
---pular uma linha em branco ----
Diferença no número de páginas de disco acessadas: 1
```