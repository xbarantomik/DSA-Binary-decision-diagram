/*

	Made by Adam Baran-Tomik, 06.05.2021
	GitHub: xbarantomik

*/


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define POWER 8

/*
Ked sa tato funkcia vola v cykle, tak urobi size (na konci cyklu) 
kombinacii 0 a 1, ktore budu bits dlhe. Tu je to pouzite na 
vytvorenie vsetkych moznych kombinacii bits dlheho vektora  
*/
void make_combination(char* vector_combination, unsigned size, unsigned bits){
	if (bits == 0) return;
	make_combination(vector_combination, size / 2, bits - 1);
	vector_combination[bits - 1] = size % 2;
}


/*
Testovanie diargramu vsetkymi moznymi kombinaciami bits dlheho vektora.
Redukcia diagramu.
Testovanie zredukovaneho diargramu vsetkymi moznymi kombinaciami bits dlheho vektora.
Porovnavanie vysledkov.
*/
int test(char* holder, unsigned size, double* test_times, unsigned bits) {

	char vector_combination[POWER], result;
	char* results_b4_reduce = (char*)malloc(size * sizeof(char));
	char* results_after_reduce = (char*)malloc(size * sizeof(char));
	clock_t start;


	start = clock();
	for (unsigned i = 0; i < size; i++) {							// testovane vsetkych moznych kombinacii vstupneho vektora
		make_combination(vector_combination, i, bits);	
		result = BDD_use(holder, vector_combination);
		if (result == -1) {
			printf("\nResults with index %d in BDD_use() before BDD_reduce() was -1 \n", i);
			return -1;
		}
		results_b4_reduce[i] = result;
	}
	clock_t use_before_time_t = clock() - start;


	start = clock();
	int deleted_nodes = BDD_reduce(holder);							// zredukovanie diagramu 
	if (deleted_nodes == -1) {
		printf("\nResults from BDD_reduce() was -1 \n");
		return -1;
	}
	clock_t reduce_time_t = clock() - start;


	start = clock();
	for (unsigned i = 0; i < size; i++) {							// testovane vsetkych moznych kombinacii vstupneho vektora po zredukovai diagramu
		make_combination(vector_combination, i, bits);
		result = BDD_use(holder, vector_combination);
		if (result == -1) {
			printf("\nResults with index %d in BDD_use() after BDD_reduce() was -1 \n", i);
			return -1;
		}
		results_after_reduce[i] = result;
	}
	clock_t use_after_time_t = clock() - start;


	start = clock();
	for (int i = 0; i < (int)size; i++) {							// porovnavanie vyslednych hodnot z BBD_use pred a po BDD_reduce(), ak za nezhoduju tak program skonci
		if (results_b4_reduce[i] != results_after_reduce[i]) {
			printf("\nResults with index %d from BDD_use() before and after BDD_reduce() weren't equal \n", i);
			return -1;
		}
	}
	clock_t comparing_time_t = clock() - start;

	free(results_b4_reduce);
	free(results_after_reduce);

	double use_before_time = ((double)use_before_time_t) / CLOCKS_PER_SEC;
	double reduce_time = ((double)reduce_time_t) / CLOCKS_PER_SEC;
	double use_after_time = ((double)use_after_time_t) / CLOCKS_PER_SEC;
	double comparing_time = ((double)comparing_time_t) / CLOCKS_PER_SEC;
	test_times[0] += use_before_time;
	test_times[1] += reduce_time;
	test_times[2] += use_after_time;
	test_times[3] += comparing_time;

	return deleted_nodes;
}


int main() {

	char* vector = NULL;
	char* holder = NULL;
	int power = POWER, bf_funcions = 100, deleted_nodes;
	int size = (int)(pow(2.0, (double)power));
	double test_times[7] = {0};		//test_times[0] = use_before_time
									//test_times[1] = reduce_time
									//test_times[2] = use_after_time
									//test_times[3] = comparing_time
									//test_times[4] = create_time
									//test_times[5] = delete_time
									//test_times[6] = whole_time								

	clock_t whole_start = clock();
	clock_t start;
		
	for (int i = 0; i < bf_funcions; i++){								// opakovanie vytvarania, testovanie a vymazavanie bf_funcions krat
		//printf("%d ", i);
		vector = (char*)init(power);

		start = clock();
		holder = (char*)BDD_create(vector);								// vyrvorenie diagramu
		clock_t create_time_t = clock() - start;

		start = clock();
		deleted_nodes = test(holder, size, test_times, power);			// testovanie
		if (deleted_nodes == -1)
			return 1;
		clock_t test_time_t = clock() - start;

		start = clock();
		delete_diagram(holder);											// vymazanie diagramu
		clock_t delete_time_t = clock() - start;

		double create_time = ((double)create_time_t) / CLOCKS_PER_SEC;
		double delete_time = ((double)delete_time_t) / CLOCKS_PER_SEC;
		test_times[4] += create_time;
		test_times[5] += delete_time;
	}
	clock_t whole_time_t = clock() - whole_start;
	double whole_time = ((double)whole_time_t) / CLOCKS_PER_SEC;
	test_times[6] = whole_time;

	for (int i = 0; i < 6; i++) {										// vypocitanie priemery z casov
		test_times[i] /= bf_funcions;
	}

	printf("Variables     : %d\n", power);
	printf("Vector length : %d (2^%d)\n\n", size, power);

	printf("[seconds] : Average times of functions in life of BDD\n");
	printf("%.4f : BDD_create()\n", test_times[4]);
	printf("%.4f : BDD_use() before reduction (%d times)\n", test_times[0], size);
	printf("%.4f : BDD_reduce()\n", test_times[1]);
	printf("%.4f : BDD_use() after reduction (%d times)\n", test_times[2], size);
	printf("%.4f : Comparing results from both BDD_use() results\n", test_times[3]);
	printf("%.4f : Deleting all BBD nodes\n\n", test_times[5]);
	printf("%.4f : Duration of %d RUNS\n", test_times[6], bf_funcions);

	return 0;
}