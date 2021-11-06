#include "stdio.h"
#include "malloc.h"

typedef struct Branch {
	int value;
	struct Branch* left;
	struct Branch* right;
	int height;
} Branch;

int max(int a, int b) {
	return a > b ? a : b;
}

int height(Branch* branch) {
	if (branch == NULL) {
		return 0;
	}
	return branch->height;
}

int calcBalance(Branch* branch) {
	if (branch == NULL) {
		return 0;
	}
	return height(branch->left) - height(branch->right);
}

Branch* minValueNode(Branch* branch) {
	while(branch->left != NULL) {
		branch = branch->left;
	}
	return branch;
}

void sortedSet(Branch* branch) {
	if (branch != NULL) {
		sortedSet(branch->left);
		printf("%d ", branch->value);
		sortedSet(branch->right);
	}
}

Branch* leftRotate(Branch* x) {
	Branch* y = x->right;
	Branch* T2 = y->left;

	y->left = x;
	x->right = T2;

	x->height = max(height(x->left), height(x->right))+1;
	y->height = max(height(y->left), height(y->right))+1;
	
	return y;
}

Branch* rightRotate(Branch* y) {
	Branch* x = y->left;
	Branch* T2 = x->right;

	x->right = y;
	y->left = T2;

	x->height = max(height(x->left), height(x->right))+1;
	y->height = max(height(y->left), height(y->right))+1;
	
	return x;
}

Branch* create(Branch* branch, int value) {
	if (branch == NULL) {
		branch = malloc(sizeof(Branch));
		branch->value = value;
		branch->height = 1;
		branch->left = NULL;
		branch->right = NULL;
		return branch;
	}

	if (value < branch->value) {
		branch->left = create(branch->left, value);
	} else if (value > branch->value) {
		branch->right = create(branch->right, value);
	} else {
		return branch;
	}

	branch->height = 1 + max(height(branch->left), height(branch->right));

	int balance = calcBalance(branch);

	if (balance > 1 && value < branch->left->value) {
		return rightRotate(branch);
	}

	if (balance < -1 && value > branch->right->value) {
		return leftRotate(branch);
	}

	if (balance > 1 && value > branch->left->value) {
		branch->left = leftRotate(branch->left);
		return rightRotate(branch);
	}

	if (balance < -1 && value < branch->right->value) {
		branch->right = rightRotate(branch->right);
		return leftRotate(branch);
	}

	return branch;
}

Branch* delete(Branch* branch, int value) {
	if(branch == NULL) {
		return branch;
	}
	
	if (value < branch->value) {
		branch->left = delete(branch->left, value);
	} else if (value > branch->value) {
		branch->right = delete(branch->right, value);
	} else {
		if (branch->left == NULL || branch->right == NULL) {
			Branch* temp = branch->left ? branch->left : branch->right;

			if (temp == NULL) {
				free(branch);
				return NULL;
			} else {
				*branch = *temp;
				free(temp);
			}
		} else {
			Branch* temp = minValueNode(branch->right);
			branch->value = temp->value;
			branch->right = delete(branch->right, temp->value);
		}
	}

	if (branch == NULL) {
		return NULL;
	}

	branch->height = 1 + max(height(branch->left), height(branch->right));

	int balance = calcBalance(branch);

	if (balance > 1 && calcBalance(branch->left) >= 0) {
		return rightRotate(branch);
	}

	if (balance > 1 && calcBalance(branch->left) < 0) {
		branch->left = leftRotate(branch->left);
		return rightRotate(branch);
	}

	if (balance < -1 && calcBalance(branch->right) <= 0) {
		return leftRotate(branch);
	}

	if (balance < -1 && calcBalance(branch->right) > 0) {
		branch->right = rightRotate(branch->right);
		return leftRotate(branch);
	}

	return branch;
}
		
void tree_delete(Branch* branch) {
	if(branch != NULL) {
		tree_delete(branch->left);
		tree_delete(branch->right);
		free(branch);
	}
}

int main(int argc, char** argv) {
	Branch* root = NULL;

	int input;

	printf("Actions: 0 - exit, 1 - create, 2 - delete, 3 - show sorted set.\n");

	while(1) {
		scanf("%d", &input);
		if (input == 0) {
			break;
		} else if (input == 1) {
			scanf("%d", &input);
			root = create(root, input);
			printf("Balanse: %d\n", calcBalance(root));
		} else if (input == 2) {
			scanf("%d", &input);
			root = delete(root, input);
			printf("Balanse: %d\n", calcBalance(root));
		} else if (input == 3) {
			printf("Sorted set: \n");
			sortedSet(root);
			printf("\n");
		} else {
			printf("Error.\n");
		}
	}
	
	tree_delete(root);

	return 0;
}