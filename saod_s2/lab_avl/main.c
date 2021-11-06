#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>

typedef int Key;

typedef struct Node {
	Key key;
	struct Node* left;
	struct Node* right;
	size_t height;
} Node;

size_t height(Node* node) {
	return node ? node->height : 0;
}

int8_t avlBalance(Node* node) {
	return node ? (int8_t)((ssize_t) height(node->left) - (ssize_t) height(node->right)) : 0;
}

Key minKey(Node* node) {
	while(node->left != NULL) {
		node = node->left;
	}
	return node->key;
}

void avlPrintAscending(Node* node) {
	if (node != NULL) {
		avlPrintAscending(node->left);
		printf("%d ", node->key);
		avlPrintAscending(node->right);
	}
}

void updateHeight(Node* node) {
	size_t lh = height(node->left);
	size_t rh = height(node->right);
	node->height = 1 + (lh > rh ? lh : rh);
}

Node* rotateLeft(Node* x) {
	Node* y = x->right;
	Node* T2 = y->left;

	y->left = x;
	x->right = T2;

	updateHeight(x);
	updateHeight(y);
	
	return y;
}

Node* rotateRight(Node* y) {
	Node* x = y->left;
	Node* T2 = x->right;

	x->right = y;
	y->left = T2;


	updateHeight(x);
	updateHeight(y);
	
	return x;
}

Node* avlInsert(Node* branch, Key key) {
	if (branch == NULL) {
		branch = malloc(sizeof(Node));
		branch->key = key;
		branch->height = 1;
		branch->left = NULL;
		branch->right = NULL;
		return branch;
	}

	if (key < branch->key) {
		branch->left = avlInsert(branch->left, key);
	} else if (key > branch->key) {
		branch->right = avlInsert(branch->right, key);
	} else {
		return branch;
	}

	updateHeight(branch);

	ssize_t balance = avlBalance(branch);

	if (balance > 1 && key < branch->left->key) {
		return rotateRight(branch);
	}

	if (balance < -1 && key > branch->right->key) {
		return rotateLeft(branch);
	}

	if (balance > 1 && key > branch->left->key) {
		branch->left = rotateLeft(branch->left);
		return rotateRight(branch);
	}

	if (balance < -1 && key < branch->right->key) {
		branch->right = rotateRight(branch->right);
		return rotateLeft(branch);
	}

	return branch;
}

Node* avlRemove(Node* branch, Key key) {
	if(branch == NULL) {
		return branch;
	}
	
	if (key < branch->key) {
		branch->left = avlRemove(branch->left, key);
	} else if (key > branch->key) {
		branch->right = avlRemove(branch->right, key);
	} else {
		if (branch->left == NULL || branch->right == NULL) {
			Node* temp = branch->left ? branch->left : branch->right;

			if (temp == NULL) {
				free(branch);
				return NULL;
			} else {
				*branch = *temp;
				free(temp);
			}
		} else {
			Key temp = minKey(branch->right);
			branch->key = temp;
			branch->right = avlRemove(branch->right, temp);
		}
	}

	if (branch == NULL) {
		return NULL;
	}

	updateHeight(branch);

	ssize_t balance = avlBalance(branch);

	if (balance > 1 && avlBalance(branch->left) >= 0) {
		return rotateRight(branch);
	}

	if (balance > 1 && avlBalance(branch->left) < 0) {
		branch->left = rotateLeft(branch->left);
		return rotateRight(branch);
	}

	if (balance < -1 && avlBalance(branch->right) <= 0) {
		return rotateLeft(branch);
	}

	if (balance < -1 && avlBalance(branch->right) > 0) {
		branch->right = rotateRight(branch->right);
		return rotateLeft(branch);
	}

	return branch;
}
		
void avlDestroyTree(Node* branch) {
	if(branch != NULL) {
		avlDestroyTree(branch->left);
		avlDestroyTree(branch->right);
		free(branch);
	}
}

int main(int argc, char** argv) {
	Node* root = NULL;

	char line[80];

	puts("Commands: \"insert\", \"remove\", \"ascending\", \"exit\".");

	while(1) {
		scanf("%s", line);
		if (strcmp("insert", line) == 0) {
			int key;
			scanf("%d", &key);
			root = avlInsert(root, key);
			printf("Balance after addition: %d\n", avlBalance(root));
		} else if (strcmp("remove", line) == 0) {
			int key;
			scanf("%d", &key);
			root = avlRemove(root, key);
			printf("Balance after removal: %d\n", avlBalance(root));
		} else if (strcmp("ascending", line) == 0) {
			printf("Numbers in ascending order: ");
			avlPrintAscending(root);
			printf("\n");
		} else if (strcmp("exit", line) == 0) {
			break;
		} else {
			puts("Unknown command. Try \"insert\", \"remove\", \"ascending\" or \"exit\".");
		}
	}
	
	avlDestroyTree(root);

	return 0;
}