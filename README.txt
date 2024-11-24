===========================================================
README.txt
===========================================================

# Projeto: Processamento e Análise de Imagens BMP com FFT

Este projeto foi desenvolvido para processar imagens no formato BMP, realizar fragmentação, separação de canais (RGB), e aplicar a Transformada Rápida de Fourier (FFT) em cada canal para análise de frequências. Os resultados são salvos em formatos binários (.dat) e texto (.txt) para visualização e pós-processamento.

## Principais Funcionalidades
1. **Criação de Diretórios**: Automatiza a criação de pastas necessárias para o processamento e armazenamento dos resultados.
2. **Escaneamento de Imagens**: Identifica imagens BMP no diretório `img` e processa cada uma delas.
3. **Fragmentação**: Divide imagens grandes em fragmentos de aproximadamente 16 MB para facilitar o processamento.
4. **Separação de Canais**: Extrai os canais de cor (vermelho, verde e azul) de cada imagem.
5. **Aplicação da FFT**: Realiza análise de frequências usando a Transformada Rápida de Fourier (FFT) em cada canal.
6. **Armazenamento dos Resultados**: Salva os resultados da FFT em arquivos binários e texto, organizados por canal e fragmento.
7. **Mesclagem**: Consolida fragmentos processados em arquivos únicos por canal.

## Estrutura de Diretórios
- **img/**: Diretório de entrada contendo imagens BMP.
- **TEMP/**: Diretório temporário para armazenar fragmentos e resultados intermediários.
- **OUTPUT_BMP/**: Diretório para imagens BMP separadas por canal (R, G, B).
- **OUTPUT/**:
  - **DAT/**: Diretório para resultados binários da FFT.
  - **TXT/**: Diretório para resultados da FFT em formato texto.

## Como Usar
1. **Pré-requisitos**:
   - Instale um compilador C (como GCC ou Clang).
   - Certifique-se de que as bibliotecas padrão C (stdio.h, stdlib.h, etc.) estão disponíveis.
   - Crie um diretório `img/` na raiz do projeto e insira as imagens BMP que deseja processar.

2. **Compilação**:
   - Para compilar o código, use o comando:
     ```bash
     gcc -o image_processor main.c -lm
     ```
   - A flag `-lm` é necessária para vincular a biblioteca matemática.

3. **Execução**:
   - Execute o programa usando:
     ```bash
     ./image_processor
     ```

4. **Saída**:
   - Os arquivos processados estarão nos diretórios `OUTPUT_BMP`, `OUTPUT/DAT` e `OUTPUT/TXT`.

## Limitações e Cuidados
- O programa processa no máximo **100 imagens** (ajustável com a constante `MAX_IMAGES`).
- O tamanho máximo do nome de arquivos é limitado a **254 caracteres**.
- Imagens com largura muito grande podem exigir ajustes no tamanho do fragmento.
- O programa foi desenvolvido para plataformas Windows e Linux. Algumas funcionalidades (como criação de diretórios) possuem compatibilidade condicional.

## Estruturas Principais
1. **BITMAPFILEHEADER**: Armazena informações básicas do cabeçalho BMP.
2. **BITMAPINFOHEADER**: Contém metadados detalhados sobre a imagem BMP.
3. **RGB**: Representa um pixel com componentes azul, verde e vermelho.
4. **Complex**: Usado para armazenar números complexos durante a FFT.

## Recursos Adicionais
- O programa utiliza **divisão e conquista** para implementar a FFT de forma recursiva.
- Para aprender mais sobre FFT, veja as referências:
  - [DSP Guide](https://www.dspguide.com/ch12/2.htm)
  - [Slides da UFRJ](http://www.pads.ufrj.br/~mariane/Cap12_slides.pdf)

## Dependências
Este projeto utiliza apenas bibliotecas padrão da linguagem C, garantindo portabilidade entre sistemas Windows e Linux.

## Possíveis Melhorias
- Implementação de MPI para processar fragmentos em múltiplos nós em paralelo.
- Adicionar suporte a outros formatos de imagem além de BMP.
- Melhorar o manuseio de erros em operações críticas.

===========================================================
