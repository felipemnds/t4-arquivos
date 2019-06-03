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
## 10
- criar um arquivo de indice secundário fortemente ligado
- indexar o nomeServidor
- não usar lista invertida (mostrar dados repetidos no próprio índice)
- se tá removido no arquivo de entrada, não deve ter chave no índice
- se não tem nome, não deve ter chave no índice também
- ordem alfabética
- se nome == igual, ordenar por byte offset
- fechar com binarioNaTela1

``` 10 arquivoEntrada.bin arquivoIndiceNomeServidor.bin ```

## 11
- receber uma busca do usuário por nomeServidor
- carregar o arquivo de índice para a RAM
- buscar no índice e retornar 0, 1 ou vários registros
- imprimir da seguinte forma ("valor nao declarado" para nulos)
  - desCampox: [valor declarado]
  - 1 reg por linha
  - ao fim, mostrar [n de acessos a disco na operacao de carregamento] + [numero de aceesos a disco para acessar o arquivo de dados]

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

## 12
- estender a func4
- realizar operacao de carregamento do índice para a RAM
- enquanto as n remoções acontecerem, atualizar o índice só na RAM
- no fim, operação de reescrita do índice (wb pra escrever no arquivo)
> saída: listar o arquivo de índice

## 13
- estender a func5
- fazer as mesmas coisas que a func12
> saída: listar o arquivo de índice

## 14
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