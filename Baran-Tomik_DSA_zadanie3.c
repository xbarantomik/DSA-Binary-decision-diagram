/*

	Made by Adam Baran-Tomik, 06.05.2021
	GitHub: xbarantomik

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LEFT 0
#define RIGHT 1
#define NOT_REDUCED	 0
#define REDUCED 1

//#define POWER 13
int power;

typedef struct tree {
	struct tree* left;
	struct tree* right;
	struct tree* parent;
	struct bf* vector_part;
	int depth;
}TREE;

typedef struct bdd {	
	TREE* root;
	TREE* main_zero;
	TREE* main_one;
	int power;
	unsigned int count;
	unsigned int deleted_nodes_bdd;

}BDD;

typedef struct bf {
	char* value;
}BF;


void transfer_parameters(TREE* new_node, TREE* old_node) {

	new_node->depth = old_node->depth;
	new_node->left = NULL;
	new_node->right = NULL;
}


/*
Spocita dlzku vektora 
*/
int get_vector_len(char* vector) {
	int length = 0, i = 0;
	while (vector[i] == 1 || vector[i] == 0) {
		length++;
		i++;
	}
	return length;
}


char* create_vector() {

	int size = (int)(pow(2.0, (double)power));
	char* vector = (char*)malloc(size * sizeof(char));

	//srand(time(NULL));										// nieco typu randomize, NEPOUZIVAT pri tester.c

	//printf("vector: ");
	for (int i = 0; i < size; i++) {
		vector[i] = (rand() % 2);
		//printf("%d", vector[i]);
	}
	//printf(" [%d]\n", size);

	return vector;
}


BF* create_bf() {

	char* vector_numbers = create_vector();
	BF* vector = (BF*)malloc(sizeof(BF));
	vector->value = vector_numbers;

	return vector;
}


TREE* create_tree_root(BF* vector) {

	TREE* root = (TREE*)malloc(sizeof(TREE));
	root->vector_part = vector;
	root->parent = NULL;
	root->left = NULL;
	root->right = NULL;
	root->depth = 0;

	return root;
}


/*
Vytvorenie bud laveho alebo praveho potomka (podla child flag).
Podla child flag sa novemu uzlu prideli bud prva alebo druha polovica vektora.
*/
void create_child(TREE* parent, int len, int row_number, char child) {
	TREE* node = (TREE*)malloc(sizeof(TREE));
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;
	node->depth = row_number;

	int new_len = len / 2;
	char* parent_vector = parent->vector_part->value;
	char* child_vector = (char*)malloc(new_len * sizeof(char));

	if (!child) {										// left child
		for (int i = 0; i < new_len; i++){
			child_vector[i] = parent_vector[i];
		}
	}
	else {												// rigth child
		for (int i = new_len, j = 0; i < len; i++, j++) {
			child_vector[j] = parent_vector[i];
		}
	}

	BF* bf_vector = malloc(sizeof(BF));
	bf_vector->value = child_vector;
	node->vector_part = bf_vector;

	if (!child)											// left child
		parent->left = node;
	else												// rigth child
		parent->right = node;
}


/*
Prechadza diagram a na potrebnej urovni zavola create_child
na vytvorenie laveho a praveho potomka
*/
void traveller_for_creating_rows(TREE* node, int row_number) {	
	if (node != NULL) {
		if ((node->depth + 1) == row_number) {
			int vector_len = get_vector_len(node->vector_part->value);

			if (node->left == NULL && node->right == NULL && (node->depth + 1) == row_number && vector_len != 1) {
				create_child(node, vector_len, row_number, LEFT);
				create_child(node, vector_len, row_number, RIGHT);
			}
		}
		traveller_for_creating_rows(node->left, row_number);
		traveller_for_creating_rows(node->right, row_number);
	}
}


/*
Vytvori koren diagramu a BDD strukturu kde ho ulozi
*/
BDD* BDD_create(BF* vector) {

	TREE* root = create_tree_root(vector);
	TREE* main_zero = NULL;
	TREE* main_one = NULL;

	for (int i = 1; i <= power; i++)
		traveller_for_creating_rows(root, i);
	
	BDD* holder = (BDD*)malloc(sizeof(BDD));
	holder->root = root;
	holder->main_zero = NULL;
	holder->main_one = NULL;
	holder->power = power;
	holder->count = node_counter(holder, NOT_REDUCED);
	holder->deleted_nodes_bdd = 0;

	return holder;
}


/*
Prechadza strom a hlada prve konecne uzly s hodnotou 0 a 1 podla nich nastavi 
main_zero a main_one. Nenastavi vsak len pointre na ne, pretoze je mozne ze sa 
vymazu, tak vytvara novy uzol a aj BF funkciu (strukturu), na ktoru v uzli ukazuje. 
*/
void traveller_for_finding_main_final_nodes(BDD* bdd, TREE* root, int vector_len) {									
	if (root != NULL) {
		if (vector_len == 2) {
			if (bdd->main_zero == NULL && (root->left->vector_part->value[0] == 0 || root->right->vector_part->value[0] == 0)) {
				if (root->left->vector_part->value[0] == 0) {
					bdd->main_zero = (TREE*)malloc(sizeof(TREE));
					transfer_parameters(bdd->main_zero, root->left);

					BF* zero_bf = (BF*)malloc(sizeof(BF));
					zero_bf->value = root->left->vector_part->value;
					bdd->main_zero->vector_part = zero_bf;
				}
				else {
					bdd->main_zero = (TREE*)malloc(sizeof(TREE));
					transfer_parameters(bdd->main_zero, root->right);

					BF* zero_bf = (BF*)malloc(sizeof(BF));
					zero_bf->value = root->right->vector_part->value;
					bdd->main_zero->vector_part = zero_bf;
				}
			}
			if (bdd->main_one == NULL && (root->left->vector_part->value[0] == 1 || root->right->vector_part->value[0] == 1)) {
				if (root->left->vector_part->value[0] == 1) {
					bdd->main_one = (TREE*)malloc(sizeof(TREE));
					transfer_parameters(bdd->main_one, root->left);

					BF* one_bf = (BF*)malloc(sizeof(BF));
					one_bf->value = root->left->vector_part->value;
					bdd->main_one->vector_part = one_bf;
				}
				else{
					bdd->main_one = (TREE*)malloc(sizeof(TREE));
					transfer_parameters(bdd->main_one, root->right);

					BF* one_bf = (BF*)malloc(sizeof(BF));
					one_bf->value = root->right->vector_part->value;
					bdd->main_one->vector_part = one_bf;
				}
			}
		}
		if (bdd->main_zero != NULL && bdd->main_one != NULL) {
			bdd->main_zero->parent = NULL;
			bdd->main_one->parent = NULL;
			return;
		}
		traveller_for_finding_main_final_nodes(bdd, root->left, vector_len / 2);
		traveller_for_finding_main_final_nodes(bdd, root->right, vector_len / 2);
	}
}


/*
Prechada strom, a posledne uzly vymazava a pointre nastavuje na
main_zero a main_one aby boli len dva posledne uzly.
Pri vymazavani vymaze aj BF funkciu, ktora je samostatna struktura,
main_zero / main_one maju vlastu BF funkciu (strukturu).
*/
void traveller_to_reduce_leafs(BDD* bdd, TREE* root, int row_number) {
	if (root != NULL) {
		if ((root->depth + 1) == row_number) {
			if (root->left != NULL) {
				if (root->left->vector_part->value[0] == 0) {
					free(root->left->vector_part);
					free(root->left);
					root->left = bdd->main_zero;
					bdd->deleted_nodes_bdd++;
				}
				else {
					free(root->left->vector_part);
					free(root->left);
					root->left = bdd->main_one;
					bdd->deleted_nodes_bdd++;
				}
			}
			if (root->right != NULL) {
				if (root->right->vector_part->value[0] == 0) {
					free(root->right->vector_part);
					free(root->right);
					root->right = bdd->main_zero;
					bdd->deleted_nodes_bdd++;
				}
				else {
					free(root->right->vector_part);
					free(root->right);
					root->right = bdd->main_one;
					bdd->deleted_nodes_bdd++;
				}
			}
		}
		if ((root->depth + 1) < row_number) {
			traveller_to_reduce_leafs(bdd, root->left, row_number);
			traveller_to_reduce_leafs(bdd, root->right, row_number);
		}
	}
}


/*
Vymaze vsetky listy diagramu a ich rodicia budu ukazovat len na 
main_zero alebo main_one uzly (podla hodnoty vektora v liste samozrejme) 
*/
void reduce_leafs(BDD* bdd) {

	int vector_len = get_vector_len(bdd->root->vector_part->value);
	char zero = 0, one = 0;

	for (int i = 0; i < vector_len; i++){									// zisti ci sa vo vectore nachadza 0 a 1
		if (zero && one)
			break;
		else if (bdd->root->vector_part->value[i] == 0)
			zero = 1;
		else if (bdd->root->vector_part->value[i] == 1)
			one = 1;
	}

	if (!(zero && one)) {													// pokial je vo vectore len jedno cislo
		traveller_for_finding_main_final_nodes(bdd, bdd->root, vector_len);
		traveller_to_reduce_leafs(bdd, bdd->root, bdd->power);
		bdd->deleted_nodes_bdd -= 1;										// lebo ako keby sa bud main_zero alebo main_one nevymazali									

	}
	else {		
		traveller_for_finding_main_final_nodes(bdd, bdd->root, vector_len);
		traveller_to_reduce_leafs(bdd, bdd->root, bdd->power);
		bdd->deleted_nodes_bdd -= 2;										// lebo ako keby sa main_zero a main_one nevymazali
	}
}


/*
Zredukuje diagram
*/
int BDD_reduce(BDD* bdd) {

	if (bdd == NULL || bdd->root == NULL)
		return -1;

	TREE* root = bdd->root;
	reduce_leafs(bdd);
	bdd->count -= bdd->deleted_nodes_bdd;									// aktualizovanie poctu uzlov po redukovani

	return (int)bdd->deleted_nodes_bdd;
}


/*
Podla vektora a jeho hodnot (lavy potomok ak 0 a pravy potomok ak 1)
prechadza diagram od korena po list 
*/
char BDD_use(BDD* bdd, char* vector) {

	if (bdd == NULL || bdd->root == NULL)
		return -1;

	TREE* curr_node = bdd->root;
	int i = 0;
	char fin_vector = NULL;

	while (curr_node != NULL) {
		fin_vector = curr_node->vector_part->value[0];
		if (vector[i] == 0) {
			if(curr_node->left != NULL)
				curr_node = curr_node->left;
			else
				curr_node = curr_node->right;
		}
		else {
			if (curr_node->right != NULL)
				curr_node = curr_node->right;
			else
				curr_node = curr_node->left;
		}
		i++;
	}
	return fin_vector;
}


/*
Spocita koncove uzly (listy)
*/
unsigned leaf_node_counter(BDD* bdd, TREE* root, char is_after_reduce){
	if (root == NULL)
		return 0;
	if (root->left == NULL && root->right == NULL) {
		if (is_after_reduce)
			if (bdd->main_zero == root || bdd->main_one == root)
				return 0;
		return 1;
	} else
		return leaf_node_counter(bdd, root->left, is_after_reduce) + leaf_node_counter(bdd, root->right, is_after_reduce);
}


/*
Spocita uzly ktore nie su koncove (listy)
*/
unsigned int node_counter_except_leafs(TREE* root){
	if (root == NULL)
		return 0;

	int count = 0;
	if (root->left && root->right)
		count++;

	count += (node_counter_except_leafs(root->left) + node_counter_except_leafs(root->right));
	return count;
}


/*
leaf_node_counter() + node_counter_except_leafs() spocita 
a ak PO REDUCE tak pripocita main_zero a main_one ak nie su NULL
*/
unsigned int node_counter(BDD* bdd, char is_after_reduce) {
	if (is_after_reduce) {
		unsigned int real_leaf_nodes = 0;
		if (bdd->main_zero != NULL)
			real_leaf_nodes++;
		if (bdd->main_one != NULL)
			real_leaf_nodes++;

		return leaf_node_counter(bdd, bdd->root, is_after_reduce) + node_counter_except_leafs(bdd->root) + real_leaf_nodes;
	}else
		return leaf_node_counter(bdd, bdd->root, is_after_reduce) + node_counter_except_leafs(bdd->root);
}

//------------------------------------------------------
//---------------pridane kvoli tester.c------------------
//------------------------------------------------------

BF* init(int p) {
	power = p;
	BF* vector = create_bf();
	return vector;
}

/*
Prechadzanie diagramu a vymazavanie jeho uzlo post order.
Ale ak by s isiel vymazat uzol main_zero alebo main_one
ho nevymaze.
*/
void delete_diagram_vol(BDD* bdd, TREE* node) {

	if (node == NULL) return;
	delete_diagram_vol(bdd, node->left);
	delete_diagram_vol(bdd, node->right);

	if (node != bdd->main_zero && node != bdd->main_one) {
		free(node->vector_part);
		free(node);
	}
}

void delete_diagram(BDD* bdd) {
	delete_diagram_vol(bdd, bdd->root);
	bdd->count = NULL;
	bdd->deleted_nodes_bdd = NULL;
	bdd->power = NULL;
	free(bdd->main_zero);
	free(bdd->main_one);
	free(bdd);
}



/*
main - nepouzivat ak sa pouziva tester.c
*/
/*
int main() {

	BF* vector = create_bf();
	BDD* holder = BDD_create(vector);
	unsigned int count1 = node_counter(holder, NOT_REDUCED);
	//my_little_tester(holder, vector);
	int deleted_nodes_ret = BDD_reduce(holder);
	unsigned int count2 = node_counter(holder, REDUCED);


	//my_little_tester(holder, vector);
	printf("\nCount before reduce: %lu\nCount after reduce:  %lu", count1, count2);
	printf("\nNodes deleted in BDD_reduce() [bdd]: %d", deleted_nodes_ret);
	printf("\nNodes left after BDD_reduce() [bdd]: %d", holder->count);
	printf("\nNodes left + deleted (same as before reduce) : %d\n", (int)holder->count + deleted_nodes_ret);

	return 0;
	 
}

void my_little_tester(BDD* holder, BF* vector) {

	//2^5 aka 32
	char in1[] = { 0, 1, 0, 1, 0 };	//10
	char in2[] = { 1, 1, 0, 0, 1 };	//25
	char in3[] = { 1, 0, 0, 1, 0 };	//18
	char in4[] = { 0, 0, 1, 0, 0 };	//4
	char in5[] = { 1, 1, 1, 1, 0 };	//30
	char in6[] = { 0, 0, 0, 0, 1 };	//1
	char m1 = BDD_use(holder, in1);
	char m2 = BDD_use(holder, in2);
	char m3 = BDD_use(holder, in3);
	char m4 = BDD_use(holder, in4);
	char m5 = BDD_use(holder, in5);
	char m6 = BDD_use(holder, in6);
	if (vector->value[10] == m1 && vector->value[25] == m2 && vector->value[18] == m3 &&
		vector->value[4] == m4 && vector->value[30] == m5 && vector->value[1] == m6) {

		printf("\nAll OK\n");
		printf("\n%d == %d", vector->value[10], m1);
		printf("\n%d == %d", vector->value[25], m2);
		printf("\n%d == %d", vector->value[18], m3);
		printf("\n%d == %d", vector->value[4], m4);
		printf("\n%d == %d", vector->value[30], m5);
		printf("\n%d == %d", vector->value[1], m6);
	}
	else {
		printf("\nNOT OK");
	}
}
*/