**********************************************
	Como executar a versão sequencial
**********************************************
Para compilar o código, digite o comando a seguir no terminal:
	
	gcc jacobi_sequencial.c -o executavel_seq -O3 -lm

Para executar, digite:
	
	./executavel_seq <nome_do_arquivo.txt> <numero_de_execucoes>
	
O arquivo de saída é gerado automaticamente, com o nome "saida_sequencial_<nome_do_arquivo>.txt"
	
**********************************************
	Como executar a versão paralela
**********************************************
Para compilar o código, digite o comando a seguir no terminal:

	gcc jacobi_threads.c -o executavel_thread -O3 -lm -lpthread
	
Para executar, digite:

	./executavel_thread <nome_do_arquivo.txt> <numero_de threads> <numero_de_execucoes>
	
O arquivo de saída é gerado automaticamente, com o nome "saida_<numero_de_threads>_threads_<nome_do_arquivo>.txt"