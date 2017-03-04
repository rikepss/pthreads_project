#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

/* Função que aloca dinamicamente uma matriz de ordem J_ORDEM e a retorna
    Variáveis:
    **matriz_A -> ponteiro de ponteiro que recebe a matriz alocada
    i-> auxilicar para o loop
*/
double **cria_matriz(int J_ORDER) {
    int i, j;

    double **matriz_A = (double **) malloc(J_ORDER*sizeof(double*));
	for(i = 0; i < J_ORDER; i++) {
		*(matriz_A + i) = (double *) malloc(J_ORDER*sizeof(double));
	}
    return matriz_A;
}

/*  Função que le da entrada do teclado a matriz matriz_A e o vetor matriz_B
    i,j -> auxiliares para rodar o for
    aux -> necessário pois o valor no arquivo de texto nao tem '.0' caracterizando o numero como inteiro.
    Então é necessário ler o valor como inteiro e depois fazer um cast para double.
*/
void le_matriz(int J_ORDER, double **matriz_A, double *matriz_B, FILE *f) {
    int i, j, aux;
	for(i = 0; i < J_ORDER; i++){
		for(j = 0; j < J_ORDER; j++) {
			fscanf(f, "%d", &aux);
			matriz_A[i][j] = (double)aux;
		}
	}

	for(i = 0; i < J_ORDER; i++) {
        fscanf(f, "%d", &aux);
        matriz_B[i] = (double)aux;
	}
}

/*  Função que calcula o valor máximo (em módulo) do elemento do vetor, e depois retorna esse valor
    (fazer o calculo do mh = critério de parada)
    i -> auxiliares para rodar o for
    tam-> tamanho do vetor vet passado como parâmetro
    *vet-> vetor com os valores calculadps nas iterações

    vet_aux -> vetor auxilia para nao substituir os valores originais do vetor passado como parâmetro
    max_val -> auxiliar para descobrir o valor máximo
*/
double maximum(double *vet, int tam) {
    int i;
    double *vet_aux = (double*)malloc(tam*sizeof(double));
    double max_val;
    if (vet[0] < 0)
        vet_aux[0] = - vet[0];
    else vet_aux[0] = vet[0];
    max_val = vet_aux[0];
    for(i = 1; i < tam; i++) {
        if (vet[i] < 0)
            vet_aux[i] = - vet[i];
        if(max_val < vet_aux[i]) {
            max_val = vet_aux[i];
        }
    }
    free(vet_aux);
    return max_val;
}

typedef struct _thread_data_t {
    double **matriz_A, *matriz_B, **matriz_A_aux, *matriz_B_aux;
    double *vet_aux, *diff;
    double soma_row_test;
    int ini, end, J_ORDER, J_ROW_TEST;
} thread_data_t;

void *iteracao_thread(void *arg) {
        int i,j;
        thread_data_t *thr_data = (thread_data_t *)arg;

     	for(i = thr_data->ini; i < thr_data->end; i++) {
            for(j = 0; j < thr_data->J_ORDER; j++) {
                if (i != j) {
                    thr_data->matriz_B[i] = thr_data->matriz_B[i] - thr_data->vet_aux[j]*thr_data->matriz_A[i][j];
                }
            }
            thr_data->diff[i] = thr_data->matriz_B[i] - thr_data->vet_aux[i];
            thr_data->soma_row_test += thr_data->matriz_A_aux[thr_data->J_ROW_TEST][i]*thr_data->vet_aux[i];
     	}
        pthread_exit(NULL);
}

/*  Função principal que calcula o método de Jacobi-Richardison
    **matriz_A -> ponteiro de ponteiro que contem a matriz A
    *matriz_B -> ponteiro que contem a matriz B
    J_ORDER ->  ordem da matriz
    J_ROW_TEST -> valor de teste
    J_ERROR -> ordem de erro maxima
    J_ITE_MAX -> numero máximo de iterações
    *out -> variavel tipo FILE * usada para gravar a saída da execucao do programa
    Mr -> variável que recebe o valor máximo de cada linha
    cont -> Calcula as iterações
    soma_row_test -> soma de test do J_ROW_TEST
*/
void calcula_jacobi_richardson(double **matriz_A, double *matriz_B, int J_ORDER, int J_ROW_TEST, double J_ERROR, int J_ITE_MAX, int NUM_THREADS, FILE *out) {
	int i,j;
    //Vetor das diferenças xi^(k+1) - xi^(k)
    double *diff = (double*)malloc(J_ORDER*sizeof(double));
    //Vetor auxiliar para o calculo das iterações
    double *vet_aux = (double*)malloc(J_ORDER*sizeof(double));
    //Vetor B*
    double *vet_B = (double*)malloc(J_ORDER*sizeof(double));
    //Matriz auxiliar que armazena os valores iniciais da matriz A
    double **matriz_A_aux = cria_matriz(J_ORDER);
    //Vetor auxiliar que armazena os valores iniciais da matriz B
    double *matriz_B_aux = (double*)malloc(J_ORDER*sizeof(double));

    /* - esse laço duplo faz a divisao das linhas de matriz_A e matriz_B
    pelo seus respectivos elementos da diagonal principal de A
       - além disso ele já coloca zero nas posições da diagonal principal da matriz_A
    */
	for(i = 0; i < J_ORDER; i++) {
        matriz_B_aux[i] = matriz_B[i];
		for(j = 0; j < J_ORDER; j++) {
            matriz_A_aux[i][j] = matriz_A[i][j];
			if (i != j) {
                matriz_A[i][j] = matriz_A[i][j]/matriz_A[i][i];
			}
		}
		matriz_B[i] = matriz_B[i]/matriz_A[i][i];
		matriz_A[i][i] = 0;
	}

	//Faz a soma de cada linha da matriz para checar a convergencia
	double *sum_matriz = (double *) calloc(J_ORDER,sizeof(double));

	for(i = 0; i < J_ORDER; i++) {
		for(j = 0; j < J_ORDER; j++) {
			sum_matriz[i] = sum_matriz[i] + matriz_A[i][j];
		}
		//caso nao seja convergente o programa não continua executando o método
		if (sum_matriz[i] >= 1) {
            printf("Nao converge. \n");
            return;
		}
		vet_B[i] = matriz_B[i];
	}
	//Calcula as iterações
	int cont = 0;
	double Mr; //variável que recebe o valor máximo de cada linha
	double soma_row_test = 0;
	//auxiliar para verificação da criação da thread
  int n = J_ORDER/NUM_THREADS;
  //variáve que recebe o retorno da funcao de criação de uma thread,
  // verificando se foi possivel ou nao
  int ret;
  thread_data_t thr_data[NUM_THREADS];
		//passa as informações  importantes para calcular o método em
  	//uma thread para que posso ser passada na função que cria uma thread
    for (i = 0 ; i < NUM_THREADS ; i++) {
        thr_data[i].J_ORDER = J_ORDER;
        thr_data[i].J_ROW_TEST = J_ROW_TEST;
        thr_data[i].matriz_A = matriz_A;
        thr_data[i].matriz_A_aux = matriz_A_aux;
        thr_data[i].matriz_B_aux = matriz_B_aux;
    }
   //cria NUM_THREADS threads
    pthread_t threads[NUM_THREADS];
	//while que roda as iterações do método
	while (cont < J_ITE_MAX) {
        for (i = 0 ; i < J_ORDER ; i++) {
            vet_aux[i] = matriz_B[i];
            matriz_B[i] = vet_B[i];
            diff[i] = 0;
        }
        Mr = 0;
        soma_row_test = 0;
				//passa as informações importantes
    		//(que precisam ser constantemente atualizadas) para calcular o método em
       //uma thread para que posso ser passada na função que cria uma thread
        for(i = 0; i < NUM_THREADS; i++) {
            thr_data[i].vet_aux = vet_aux;
            thr_data[i].matriz_B = matriz_B;
            thr_data[i].diff = diff;
            thr_data[i].soma_row_test = soma_row_test;
            thr_data[i].ini = n*i;
            thr_data[i].end = n*(i + 1);
            if (i == NUM_THREADS - 1) {
								thr_data[i].end = J_ORDER;
						}
            ret = pthread_create(&threads[i], NULL, iteracao_thread, (void*)&thr_data[i]);

            if (ret){
                printf("ERROR! Return code: %d\n", ret);
                return;
            }
        }
    		//função que sincroniza as threads, só deixa continuar o a executar o
    		//código depois que todas as threads estiverem terminadas
        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        for (i = 0 ; i < NUM_THREADS ; i++) {
            soma_row_test += thr_data[i].soma_row_test;
        }
        //verifica se o erro Mr passou do determinado pelo usuário
        Mr = maximum(diff, J_ORDER) / maximum(vet_aux, J_ORDER);
        if (Mr <= J_ERROR) {
            break;
        }
        cont++;
	}
	//imprime as informações pedidas na tela
	fprintf(out, "-----------------------------------------\n");
    fprintf(out, "Iterations: %d\n", cont);
    fprintf(out, "RowTest: %d => [%lf] =? %lf\n", J_ROW_TEST, soma_row_test, matriz_B_aux[J_ROW_TEST]);

    for(i = 0; i < J_ORDER; i++) {
        free(matriz_A_aux[i]);
    }
    free(matriz_A_aux);
    free(sum_matriz);
    free(diff);
    free(vet_aux);
    free(vet_B);
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 4) {
        printf("./exec <file_name.txt> <n_threads> <n_executions>\n");
        return 0;
    }
	int i, k, J_ORDER, J_ROW_TEST, J_ITE_MAX, NUM_THREADS, NUM_EXEC;
	double J_ERROR;
	clock_t itime, ftime;
	FILE *f, *out;
	double **matriz_A;
	double *matriz_B;
  char saida[25];

  char *n_threads = (char*)malloc(sizeof(int));
  n_threads = argv[2];
  char *n_exec = (char*)malloc(sizeof(int));
  n_exec = argv[3];
  NUM_THREADS = atoi(n_threads);
  NUM_EXEC = atoi(n_exec);

  double tempo_medio = 0, desvio_padrao = 0;
  double vet_media[NUM_EXEC];

	for(k = 0; k < NUM_EXEC; k++) {
		if (fopen(argv[1], "r") == NULL) {
            printf("File .txt not found\n");
            return -1;
		}
    //abre o arquivo de leitura e le do arquivo os valores necessários
    //para calcular o método
    f = fopen(argv[1], "r");
		fscanf(f, "%d", &J_ORDER);
		fscanf(f, "%d", &J_ROW_TEST);
		fscanf(f, "%lf", &J_ERROR);
		fscanf(f, "%d", &J_ITE_MAX);
		//cria o nome do arquivo de saida
    strcpy(saida, "saida_");
    strcat(saida, n_threads);
    strcat(saida, "_threads_");
    strcat(saida, argv[1]);
    out = fopen(saida, "a");

		matriz_B = (double*)malloc(J_ORDER*sizeof(double));
		matriz_A = cria_matriz(J_ORDER);
		le_matriz(J_ORDER, matriz_A, matriz_B, f);
		//o tempo de execução é gerado pegando o tempo depois de executar e subtraindo pelo tempo depois.
    //esses tempos são pegos pela função clock() encontrada na biblioteca time.h
		itime = clock();
		calcula_jacobi_richardson(matriz_A, matriz_B, J_ORDER, J_ROW_TEST, J_ERROR, J_ITE_MAX, NUM_THREADS, out);
		ftime = clock();
		//printa o tempo de execução
		fprintf(out, "\nExecution time: %lf seconds\n",(ftime-itime) / (CLOCKS_PER_SEC * 1.0));
        fprintf(out, "-----------------------------------------\n");
        vet_media[k] = (ftime-itime) / (CLOCKS_PER_SEC * 1.0);
        tempo_medio += vet_media[k];

		for(i = 0; i < J_ORDER; i++) {
			free(matriz_A[i]);
		}
		free(matriz_A);
		free(matriz_B);
		fclose(f);
		fclose(out);
	}

	tempo_medio = tempo_medio / NUM_EXEC;
	for (i = 0 ; i < NUM_EXEC ; i++) {
        desvio_padrao += (vet_media[i] - tempo_medio)*(vet_media[i] - tempo_medio);
	}
  //printa o tempo médio depois de executar as 10 vezes o método Jacobi-Richardson
	printf("Average time of %d executions: %lf seconds\n", NUM_EXEC, tempo_medio);
    if (NUM_EXEC < 2)
         printf("Standard deviation: 0.000000\n");
    else printf("Standard deviation: %lf\n", sqrt(desvio_padrao / (NUM_EXEC-1)));

	return 0;
}
