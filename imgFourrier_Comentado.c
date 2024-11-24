#include <stdio.h>     // Biblioteca para funcoes de entrada e saida padrao
#include <stdlib.h>    // Biblioteca para funcoes gerais
#include <string.h>    // Biblioteca para manipulacao de strings
#include <dirent.h>    // Biblioteca para manipulacao de diretorios
#include <sys/stat.h>  // Biblioteca para manipulacao de informacoes de arquivos
#include <stdint.h>    // Biblioteca para tipos de dados inteiros com tamanho definido (uint8_t, uintt16_t, uint32_t etc..), como estou programando no windows e vai rodar tambem no linux coloque aqui para portabilidade do codigo
#include <math.h>      // Biblioteca para funcoes matematicas
#include <errno.h>     // Biblioteca para manipulacao de erros

#define MAX_IMAGES 100                  // Define o numero maximo de imagens
#define MAX_FILENAME_LENGTH 254         // Define o tamanho maximo para nomes de arquivos
#define M_PI 3.14159265358979323846     // Define o valor de Pi

#pragma pack(push, 1)  // Alinha as estruturas em limites de 1 byte

// Estrutura para o cabecalho de FILE do BMP
typedef struct {
    uint16_t bfType;        // Tipo do arquivo , uint16_t = Unsigned Integer de 16 bits.
    uint32_t bfSize;        // Tamanho do arquivo em bytes, uint32_t = Unsigned Integer de 32 bits.
    uint16_t bfReserved1;   // Reservado
    uint16_t bfReserved2;   // Reservado
    uint32_t bfOffBits;     // Offset para os dados de pixel
} BITMAPFILEHEADER;

// Estrutura para o cabecalho de INFO do BMP
typedef struct {
    uint32_t biSize;           // Tamanho deste cabecalho
    int32_t  biWidth;          // Largura da imagem, int32_t = Signed Integer de 32 bits.
    int32_t  biHeight;         // Altura da imagem
    uint16_t biPlanes;         // Numero de planos de cor
    uint16_t biBitCount;       // Bits por pixel
    uint32_t biCompression;    // Tipo de compressao
    uint32_t biSizeImage;      // Tamanho da imagem em bytes
    int32_t  biXPelsPerMeter;  // Resolucao horizontal
    int32_t  biYPelsPerMeter;  // Resolucao vertical
    uint32_t biClrUsed;        // Numero de cores usadas
    uint32_t biClrImportant;   // Numero de cores importantes
} BITMAPINFOHEADER;

// Estrutura para pixel RGB
typedef struct {
    uint8_t blue;    // Componente azul
    uint8_t green;   // Componente verde
    uint8_t red;     // Componente vermelho
} RGB;

#pragma pack(pop)  // Termina o alinhamento de estruturas

// Estrutura para numeros complexos
typedef struct {
    double real;   // Parte real
    double imag;   // Parte imaginaria
} Complex;

// Declaracoes antecipadas das funcoes para o compilador nao enxer o saco
void save_fft_to_txt(const char *filename, Complex *fft_result, size_t size);
void node_process_fragment(RGB *fragment_pixels, size_t block_size, int width, int height, int fragment_index, uint8_t **red_channel_out, uint8_t **green_channel_out, uint8_t **blue_channel_out, const char *input_file);
void merge_fragment_files(const char *input_file);

// Funcao para criar os diretorios necessarios
void create_directories() {
    printf("Criando diretorios...\n");
    if (mkdir("TEMP"/*, 0755*/) == -1 && errno != EEXIST) { // tire o numero 0755 de comentario para sistemas linux
        perror("Erro ao criar diretorio TEMP"); // Tratamento de erro para criacao de diretorio
    } else {
        printf("Diretorio TEMP criado ou ja existe.\n"); // Mensagem de sucesso
    }
    if (mkdir("OUTPUT_BMP"/*, 0755*/) == -1 && errno != EEXIST) {
        perror("Erro ao criar diretorio OUTPUT_BMP");
    } else {
        printf("Diretorio OUTPUT_BMP criado ou ja existe.\n");
    }
    if (mkdir("OUTPUT"/*, 0755*/) == -1 && errno != EEXIST) {
        perror("Erro ao criar diretorio OUTPUT");
    } else {
        printf("Diretorio OUTPUT criado ou ja existe.\n");
    }
    if (mkdir("OUTPUT\\DAT"/*, 0755*/) == -1 && errno != EEXIST) {
        perror("Erro ao criar diretorio DAT");
    } else {
        printf("Diretorio DAT criado ou ja existe.\n");
    }
    if (mkdir("OUTPUT\\TXT"/*, 0755*/) == -1 && errno != EEXIST) {
        perror("Erro ao criar diretorio TXT");
    } else {
        printf("Diretorio TXT criado ou ja existe.\n");
    }
    
    
    
}
// Funcao para escanear a pasta "img" e listar todas as imagens BMP
void scan_images(char images[MAX_IMAGES][256], int *image_count) { //images[MAX_IMAGES][256] -> vetor image (ira receber os nomes das imagens), [MAX_IMAGES] quantidade de imagens maxima o codigo ira ler, [256] tamanho maximo dos nomes em bits dos arquivos
    DIR *d = opendir("img"); // Abre o diretorio "img"
    if (!d) { // tratamento de execao: Verifica se o diretorio foi aberto com sucesso
        perror("Erro ao abrir o diretorio de imagens"); // Mensagem de erro se falhou
        return;
    }
    *image_count = 0; // Inicializa a contagem de imagens
    struct dirent *dir; // Estrutura para entrada de diretorio, dirent.h
    while ((dir = readdir(d)) != NULL) { // Lê as entradas do diretorio
        if (strstr(dir->d_name, ".bmp") != NULL) { // Verifica se o arquivo e uma imagem BMP
            if (*image_count < MAX_IMAGES) { // se o limite maximo de imagens nao for alcancado:
                strcpy(images[*image_count], dir->d_name); // Salva o nome da imagem
                printf("Imagem encontrada: %s\n", dir->d_name); // Imprime o nome da imagem
                (*image_count)++; // Incrementa a contagem de imagens
            } else {
                fprintf(stderr, "Numero maximo de imagens (%d) alcancado.\n", MAX_IMAGES); // Erro se exceder o maximo
                break;
            }
        }
    }
    closedir(d); // Fecha o diretorio
}

// Funcao recursiva para calcular a Transformada Rapida de Fourier (FFT)
// Ainda estou confuso quanto a parte da recursividade desse treco, mas sei que e para divisao e consquista
// Referências:
// http://www.pads.ufrj.br/~mariane/Cap12_slides.pdf
// https://www.dspguide.com/ch12/2.htm
// https://fiscomp.if.ufrgs.br/index.php/Grupo4_-_FFT
void fft(Complex *x, size_t N) {
    if (N <= 1) return; // Caso base da recursao

    size_t half_N = N / 2;
    Complex *even = (Complex *)malloc(half_N * sizeof(Complex)); // Aloca memoria para indices pares (divisao e conquista)
    Complex *odd = (Complex *)malloc(half_N * sizeof(Complex));  // Aloca memoria para indices impares (divisao e conquista)
    if (!even || !odd) {
        perror("Erro ao alocar memoria para FFT");
        free(even);
        free(odd);
        return;
    }

    // Separa os indices pares e impares
    for (size_t i = 0; i < half_N; i++) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    fft(even, half_N); // FFT recursiva nos indices pares (divisao e conquista)
    fft(odd, half_N);  // FFT recursiva nos indices impares (divisao e conquista)

    for (size_t k = 0; k < half_N; k++) {
        double t = -2.0 * M_PI * k / N;
        Complex exp_val = {cos(t), sin(t)}; // Calcula o fator girante
        Complex temp = {exp_val.real * odd[k].real - exp_val.imag * odd[k].imag,
                        exp_val.real * odd[k].imag + exp_val.imag * odd[k].real}; // Multiplicacao complexa

        x[k].real = even[k].real + temp.real;         // Combina os resultados
        x[k].imag = even[k].imag + temp.imag;
        x[k + half_N].real = even[k].real - temp.real;
        x[k + half_N].imag = even[k].imag - temp.imag;
    }

    free(even); // Libera a memoria alocada
    free(odd);
}

// Funcao para aplicar a FFT em um canal e salvar os resultados
void apply_fft(uint8_t *channel_values, size_t size, const char *dat_filename, const char *txt_filename) {
    printf("Aplicando FFT no canal...\n");
    size_t N = size;
    Complex *fft_result = (Complex *)malloc(N * sizeof(Complex)); // Aloca memoria para os resultados da FFT
    if (!fft_result) {
        perror("Erro ao alocar memoria para resultados da FFT");
        return;
    }

    // Inicializa o array complexo com os valores do canal
    for (size_t i = 0; i < N; i++) {
        fft_result[i].real = channel_values[i];
        fft_result[i].imag = 0.0;
    }

    fft(fft_result, N); // Aplica a FFT

    FILE *fp = fopen(dat_filename, "wb"); // Abre arquivo como binario para escrever os resultados da FFT
    if (fp) {
        fwrite(fft_result, sizeof(Complex), N, fp); // Escreve os resultados da FFT
        fclose(fp);
        printf("Resultados da FFT salvos em: %s\n", dat_filename);
    } else {
        perror("Erro ao salvar arquivo DAT");
    }

    save_fft_to_txt(txt_filename, fft_result, N); // Salva os resultados em arquivo de texto
    free(fft_result); // Libera a memoria alocada
}

// Funcao para salvar os resultados da FFT em um arquivo de texto
void save_fft_to_txt(const char *filename, Complex *fft_result, size_t size) {
    FILE *fp = fopen(filename, "w"); // Abre o arquivo como texto
    if (!fp) {
        perror("Erro ao abrir arquivo TXT");
        printf("Arquivo: %s\n", filename);
        return;
    }

    for (size_t i = 0; i < size; i++) {
        fprintf(fp, "%lf %lf\n", fft_result[i].real, fft_result[i].imag); // Escreve os resultados da FFT no arquivo
    }

    fclose(fp);
}
void send_fragment_to_node(uint8_t *fragment_data, size_t fragment_size, const char *channel, int fragment_index) {
    // Funcao para enviar dados para o no (nao implementada)
}

void receive_processed_fragment_from_node(uint8_t *processed_data, size_t data_size, const char *channel, int fragment_index) {
    // Funcao para receber dados do no (nao implementada)
}

// Funcao para o servidor ler e fragmentar a imagem
void server_process_image(const char *input_file) {
    printf("Servidor: Processando imagem %s\n", input_file);

    int width = 0, height = 0;
    char full_path[MAX_FILENAME_LENGTH];
    snprintf(full_path, sizeof(full_path), "img/%s", input_file); // Monta o caminho completo

    FILE *fp = fopen(full_path, "rb"); // Abre o arquivo de imagem
    if (!fp) {
        perror("Erro ao abrir o arquivo BMP");
        return;
    }

    BITMAPFILEHEADER bfh; // Cabecalho de arquivo BMP
    BITMAPINFOHEADER bih; // Cabecalho de informacoes BMP

    fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp); // Lê o cabecalho de arquivo
    fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp); // Lê o cabecalho de informacoes

    if (bfh.bfType != 0x4D42) { // Verifica se e um arquivo BMP
        fprintf(stderr, "Erro: O arquivo nao e um BMP valido.\n");
        fclose(fp);
        return;
    }

    width = bih.biWidth;
    height = abs(bih.biHeight); // Usa o valor absoluto em caso de altura negativa, nao sei como acontece, mas acontece
    int row_padded = (width * 3 + 3) & (~3); // Calcula o tamanho da linha com padding

    // Calcula o numero de linhas que correspondem a aproximadamente 16 MB
    size_t fragment_size_bytes = 16 * 1024 * 1024; // 16 MB
    int lines_per_fragment = fragment_size_bytes / row_padded;
    if (lines_per_fragment == 0) lines_per_fragment = 1;

    printf("Servidor: Fragmentando a imagem em pedacos de %d linhas (~16MB)\n", lines_per_fragment);

    // Preparar para escrever as imagens de saida dos canais
    // Abrir os arquivos de saida agora e escrever os cabecalhos, depois anexar os dados dos pixels 
    FILE *fp_red = NULL, *fp_green = NULL, *fp_blue = NULL;
    unsigned char *row_red = NULL, *row_green = NULL, *row_blue = NULL;

    // Atualizar os cabecalhos para imagem RGB de 24 bits
    BITMAPFILEHEADER out_bfh = bfh;
    BITMAPINFOHEADER out_bih = bih;
    out_bih.biBitCount = 24;
    out_bih.biSizeImage = row_padded * height;
    out_bfh.bfSize = out_bih.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    char output_filename[MAX_FILENAME_LENGTH];

    // Abrir arquivos de saida para os canais
    snprintf(output_filename, sizeof(output_filename), "OUTPUT_BMP\\%s_red.bmp", input_file);
    fp_red = fopen(output_filename, "wb"); // Abre o arquivo de saida do canal vermelho
    if (!fp_red) {
        perror("Erro ao criar arquivo BMP para canal vermelho");
        fclose(fp);
        return;
    }
    fwrite(&out_bfh, sizeof(BITMAPFILEHEADER), 1, fp_red); // Escreve o cabecalho de arquivo
    fwrite(&out_bih, sizeof(BITMAPINFOHEADER), 1, fp_red); // Escreve o cabecalho de informacoes

    snprintf(output_filename, sizeof(output_filename), "OUTPUT_BMP\\%s_green.bmp", input_file);
    fp_green = fopen(output_filename, "wb"); // Abre o arquivo de saida do canal verde
    if (!fp_green) {
        perror("Erro ao criar arquivo BMP para canal verde");
        fclose(fp);
        fclose(fp_red);
        return;
    }
    fwrite(&out_bfh, sizeof(BITMAPFILEHEADER), 1, fp_green); // Escreve o cabecalho de arquivo
    fwrite(&out_bih, sizeof(BITMAPINFOHEADER), 1, fp_green); // Escreve o cabecalho de informacoes

    snprintf(output_filename, sizeof(output_filename), "OUTPUT_BMP\\%s_blue.bmp", input_file);
    fp_blue = fopen(output_filename, "wb"); // Abre o arquivo de saida do canal azul
    if (!fp_blue) {
        perror("Erro ao criar arquivo BMP para canal azul");
        fclose(fp);
        fclose(fp_red);
        fclose(fp_green);
        return;
    }
    fwrite(&out_bfh, sizeof(BITMAPFILEHEADER), 1, fp_blue); // Escreve o cabecalho de arquivo
    fwrite(&out_bih, sizeof(BITMAPINFOHEADER), 1, fp_blue); // Escreve o cabecalho de informacoes

    row_red = (unsigned char *)malloc(row_padded); // Aloca memoria para a linha do canal vermelho
    row_green = (unsigned char *)malloc(row_padded); // Aloca memoria para a linha do canal verde
    row_blue = (unsigned char *)malloc(row_padded); // Aloca memoria para a linha do canal azul

    if (!row_red || !row_green || !row_blue) {
        perror("Erro ao alocar memoria para buffer de linha de saida");
        fclose(fp);
        fclose(fp_red);
        fclose(fp_green);
        fclose(fp_blue);
        if (row_red) free(row_red);
        if (row_green) free(row_green);
        if (row_blue) free(row_blue);
        return;
    }

    // Processar cada fragmento
    int fragment_index = 0;
    for (int y_start = 0; y_start < height; y_start += lines_per_fragment) {
        int current_block_height = (y_start + lines_per_fragment > height) ? (height - y_start) : lines_per_fragment;
        size_t block_size = (size_t)width * current_block_height;

        // Ler o fragmento do arquivo de imagem
        fseek(fp, bfh.bfOffBits + (height - y_start - current_block_height) * row_padded, SEEK_SET);
        unsigned char *row = (unsigned char *)malloc(row_padded);
        if (!row) {
            perror("Erro ao alocar memoria para o buffer de linha");
            fclose(fp);
            fclose(fp_red);
            fclose(fp_green);
            fclose(fp_blue);
            free(row_red);
            free(row_green);
            free(row_blue);
            return;
        }

        // Buffer para armazenar o fragmento
        RGB *fragment_pixels = (RGB *)malloc(block_size * sizeof(RGB));
        if (!fragment_pixels) {
            perror("Erro ao alocar memoria para o fragmento de pixels");
            free(row);
            fclose(fp);
            fclose(fp_red);
            fclose(fp_green);
            fclose(fp_blue);
            free(row_red);
            free(row_green);
            free(row_blue);
            return;
        }

        for (int y = 0; y < current_block_height; y++) {
            fread(row, 1, row_padded, fp); // Lê uma linha
            int row_idx = current_block_height - 1 - y;
            for (int x = 0; x < width; x++) {
                size_t idx = (size_t)row_idx * width + x;
                fragment_pixels[idx].blue = row[x * 3];
                fragment_pixels[idx].green = row[x * 3 + 1];
                fragment_pixels[idx].red = row[x * 3 + 2];
            }
        }
        free(row);

        // Simular envio do fragmento para o no (apagar depois de implementar o MPI)
        printf("Servidor: Enviando fragmento %d para o no\n", fragment_index);
        // Aqui, chamar uma funcao MPI_Send (depois de implementar o MPI)

        // Simular processamento no no (mudar depois de implementar o MPI)
        uint8_t *red_channel_values = NULL;
        uint8_t *green_channel_values = NULL;
        uint8_t *blue_channel_values = NULL;
        node_process_fragment(fragment_pixels, block_size, width, current_block_height, fragment_index,
                              &red_channel_values, &green_channel_values, &blue_channel_values, input_file);

        // Simular recebimento dos dados processados do no (apagar depois de implementar o MPI)
        printf("Servidor: Recebendo fragmento processado %d do no\n", fragment_index);
        // Aqui, chamar uma funcao MPI_Recv (depois de implementar o MPI)

        // Escrever os dados dos canais separados nos arquivos BMP de saida
        for (int y = current_block_height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                size_t idx = (size_t)y * width + x;
                uint8_t red_value = red_channel_values[idx];
                uint8_t green_value = green_channel_values[idx];
                uint8_t blue_value = blue_channel_values[idx];

                // Para imagem do canal vermelho
                row_red[x * 3] = 0;               // Blue
                row_red[x * 3 + 1] = 0;           // Green
                row_red[x * 3 + 2] = red_value;   // Red

                // Para imagem do canal verde
                row_green[x * 3] = 0;             // Blue
                row_green[x * 3 + 1] = green_value; // Green
                row_green[x * 3 + 2] = 0;         // Red

                // Para imagem do canal azul
                row_blue[x * 3] = blue_value;     // Blue
                row_blue[x * 3 + 1] = 0;          // Green
                row_blue[x * 3 + 2] = 0;          // Red
            }
            fwrite(row_red, 1, row_padded, fp_red);     // Escreve a linha no arquivo do canal vermelho
            fwrite(row_green, 1, row_padded, fp_green); // Escreve a linha no arquivo do canal verde
            fwrite(row_blue, 1, row_padded, fp_blue);   // Escreve a linha no arquivo do canal azul
        }

        // Liberar memoria
        free(red_channel_values);
        free(green_channel_values);
        free(blue_channel_values);
        free(fragment_pixels);

        fragment_index++;
    }

    fclose(fp);
    fclose(fp_red);
    fclose(fp_green);
    fclose(fp_blue);
    free(row_red);
    free(row_green);
    free(row_blue);
}

// Funcao para o no processar o fragmento recebido
void node_process_fragment(RGB *fragment_pixels, size_t block_size, int width, int height, int fragment_index,
                           uint8_t **red_channel_out, uint8_t **green_channel_out, uint8_t **blue_channel_out,
                           const char *input_file) {
    printf("No: Processando fragmento %d\n", fragment_index);

    // Separar os canais
    uint8_t *red_channel_values = (uint8_t *)malloc(block_size * sizeof(uint8_t));
    uint8_t *green_channel_values = (uint8_t *)malloc(block_size * sizeof(uint8_t));
    uint8_t *blue_channel_values = (uint8_t *)malloc(block_size * sizeof(uint8_t));

    if (!red_channel_values || !green_channel_values || !blue_channel_values) {
        perror("Erro ao alocar memoria para canais do fragmento");
        free(red_channel_values);
        free(green_channel_values);
        free(blue_channel_values);
        return;
    }

    for (size_t i = 0; i < block_size; i++) {
        red_channel_values[i] = fragment_pixels[i].red;
        green_channel_values[i] = fragment_pixels[i].green;
        blue_channel_values[i] = fragment_pixels[i].blue;
    }

    // Aplicar FFT em cada canal
    char dat_filename[MAX_FILENAME_LENGTH];
    char txt_filename[MAX_FILENAME_LENGTH];

    snprintf(dat_filename, sizeof(dat_filename), "TEMP\\%s_red_fft_fragment_%d.dat", input_file, fragment_index);
    snprintf(txt_filename, sizeof(txt_filename), "TEMP\\%s_red_fft_fragment_%d.txt", input_file, fragment_index);
    apply_fft(red_channel_values, block_size, dat_filename, txt_filename);

    snprintf(dat_filename, sizeof(dat_filename), "TEMP\\%s_green_fft_fragment_%d.dat", input_file, fragment_index);
    snprintf(txt_filename, sizeof(txt_filename), "TEMP\\%s_green_fft_fragment_%d.txt", input_file, fragment_index);
    apply_fft(green_channel_values, block_size, dat_filename, txt_filename);

    snprintf(dat_filename, sizeof(dat_filename), "TEMP\\%s_blue_fft_fragment_%d.dat", input_file, fragment_index);
    snprintf(txt_filename, sizeof(txt_filename), "TEMP\\%s_blue_fft_fragment_%d.txt", input_file, fragment_index);
    apply_fft(blue_channel_values, block_size, dat_filename, txt_filename);

    // Retornar os canais separados para o servidor
    *red_channel_out = red_channel_values;
    *green_channel_out = green_channel_values;
    *blue_channel_out = blue_channel_values;
}

// Funcao para mesclar os arquivos de fragmentos em um unico arquivo por canal
void merge_fragment_files(const char *input_file) {
    printf("Mesclando arquivos de fragmentos para a imagem: %s\n", input_file);

    const char *channels[] = {"red", "green", "blue"};
    const char *channel_initials[] = {"R", "G", "B"};

    // Para ambos os arquivos .dat e .txt
    for (int ch = 0; ch < 3; ch++) {
        // Abrir arquivos de saida
        char output_dat_filename[MAX_FILENAME_LENGTH];
        char output_txt_filename[MAX_FILENAME_LENGTH];

        snprintf(output_dat_filename, sizeof(output_dat_filename), "OUTPUT\\DAT\\%s_%s.dat", input_file, channel_initials[ch]);
        snprintf(output_txt_filename, sizeof(output_txt_filename), "OUTPUT\\TXT\\%s_%s.txt", input_file, channel_initials[ch]);

        FILE *fp_out_dat = fopen(output_dat_filename, "wb");
        if (!fp_out_dat) {
            perror("Erro ao abrir arquivo DAT de saida");
            continue;
        }

        FILE *fp_out_txt = fopen(output_txt_filename, "w");
        if (!fp_out_txt) {
            perror("Erro ao abrir arquivo TXT de saida");
            fclose(fp_out_dat);
            continue;
        }

        int fragment_index = 0;
        while (1) {
            // Construir nomes dos arquivos de fragmentos
            char fragment_dat_filename[MAX_FILENAME_LENGTH];
            char fragment_txt_filename[MAX_FILENAME_LENGTH];

            snprintf(fragment_dat_filename, sizeof(fragment_dat_filename), "TEMP\\%s_%s_fft_fragment_%d.dat", input_file, channels[ch], fragment_index);
            snprintf(fragment_txt_filename, sizeof(fragment_txt_filename), "TEMP\\%s_%s_fft_fragment_%d.txt", input_file, channels[ch], fragment_index);

            // Verificar se o arquivo de fragmento existe
            FILE *fp_fragment_dat = fopen(fragment_dat_filename, "rb");
            if (!fp_fragment_dat) {
                // Sem mais fragmentos
                break;
            }

            FILE *fp_fragment_txt = fopen(fragment_txt_filename, "r");
            if (!fp_fragment_txt) {
                // Fechar o arquivo de fragmento dat
                fclose(fp_fragment_dat);
                // Sem mais fragmentos
                break;
            }

            // Ler o arquivo dat do fragmento e escrever no arquivo dat de saida
            // Obter o tamanho do arquivo de fragmento
            fseek(fp_fragment_dat, 0, SEEK_END);
            long fragment_size = ftell(fp_fragment_dat);
            fseek(fp_fragment_dat, 0, SEEK_SET);

            // Alocar buffer
            char *buffer = (char *)malloc(fragment_size);
            if (buffer) {
                fread(buffer, 1, fragment_size, fp_fragment_dat);
                fwrite(buffer, 1, fragment_size, fp_out_dat);
                free(buffer);
            } else {
                perror("Erro ao alocar memoria para o buffer do fragmento");
            }
            fclose(fp_fragment_dat);
            remove(fragment_dat_filename); // Excluir o arquivo de fragmento dat

            // Ler o arquivo txt do fragmento e escrever no arquivo txt de saida
            char line[1024];
            while (fgets(line, sizeof(line), fp_fragment_txt)) {
                fputs(line, fp_out_txt);
            }
            fclose(fp_fragment_txt);
            remove(fragment_txt_filename); // Excluir o arquivo de fragmento txt

            fragment_index++;
        }

        fclose(fp_out_dat);
        fclose(fp_out_txt);
    }

    printf("Mesclagem concluida para a imagem: %s\n", input_file);
}

// Funcao principal
int main(int argc, char *argv[]) {
    create_directories(); // Cria os diretorios necessarios
    char images[MAX_IMAGES][256]; // Array para armazenar os nomes dos arquivos de imagem
    int image_count = 0; // Inicializa a contagem de imagens
    scan_images(images, &image_count); // Escaneia por imagens

    for (int i = 0; i < image_count; i++) {
        server_process_image(images[i]); // Processa cada imagem
        merge_fragment_files(images[i]);  // Mescla os arquivos de fragmentos em um unico arquivo por canal
    }
    
    return 0;
}
