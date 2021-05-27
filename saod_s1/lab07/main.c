#include <malloc.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef int32_t Data;

typedef struct Tree {
  Data data;
  struct Tree* left;
  struct Tree* right;
} Tree;

Tree* make_node(Data data) {
  Tree* node = (Tree*)malloc(sizeof(Tree));
  node->data  = data;
  node->left  = NULL;
  node->right = NULL;
  return node;
}

void insert(Tree** root, Data data){
  while(1) {
    if(*root == NULL) {
      *root = make_node(data);
      return;
    }
    
    if (data == (*root)->data) {
      return;
    }

    if(data < (*root)->data) {
      root = &((*root)->left);
    } else {
      root = &((*root)->right);
    }
  }
}

uint8_t contains(Tree* root, Data data){
  while(root != NULL) {
    if (data == root->data) {
      return 1;
    }

    if(data < root->data) {
      root = root->left;
    } else {
      root = root->right;
    }
  }
  return 0;
}

void freeTree(Tree* elem) {
  if (elem != NULL) {
    freeTree(elem->left);
    freeTree(elem->right);
    free(elem);
  }
}

int main(int argc, char** argv){
  Data to_add[] = { 6, 2, 9, 3, 8, 7 };
  Data to_check[] = { 6, 2, 9, 3, 8, 7, 228, 9, 13, 37 };
  Tree* root = NULL;
  printf("Added:");
  for(size_t i = 0; i != sizeof(to_add) / sizeof(Data); i++) {
    printf(" %d", to_add[i]);
    insert(&root, to_add[i]);
  }
  printf("\n Checking:");
  for(size_t i = 0; i != sizeof(to_check) / sizeof(Data); i++) {
    printf(" (%d is %s)", to_check[i], contains(root, to_check[i]) ? "present" : "absent");
  }
  freeTree(root);
  printf("\n");
  return 0;
}
