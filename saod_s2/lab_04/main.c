#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

typedef int Key;
typedef int Value;

int BTREE_CAPACITY = 5;

typedef struct BTreeNode {
		Key* keys;
		struct BTreeNode** C;
		int n;
		uint8_t leaf;
} BTreeNode;
	
BTreeNode* BTreeNode_new(uint8_t leaf) {
	BTreeNode* node = malloc(sizeof(BTreeNode));
	node->n = 0;
	node->leaf = leaf;
	node->keys = malloc(sizeof(Key) * (2*BTREE_CAPACITY-1));
	node->C = malloc(sizeof(BTreeNode*) * 2*BTREE_CAPACITY);
	memset(node->C, 0, sizeof(Key) * (2*BTREE_CAPACITY-1));
	return node;
}
	
// A function to borrow a key from C[idx-1] and insert it into C[idx]
void BTreeNode_borrowFromPrev(BTreeNode* node, int idx) {
	BTreeNode *child = node->C[idx];
	BTreeNode *sibling = node->C[idx-1];
	// The last key from C[idx-1] goes up to the parent and key[idx-1]
	// from parent is inserted as the first key in C[idx]. Thus, the  loses
	// sibling one key and child gains one key
	// Moving all key in C[idx] one step ahead
	for (int i = child->n-1; i >= 0; i--) {
		child->keys[i+1] = child->keys[i];
	}
	// If C[idx] is not a leaf, move all its child pointers one step ahead
	if (!child->leaf) {
		for(int i = child->n; i >= 0; i--) {
			child->C[i+1] = child->C[i];
		}
	}
	// Setting child's first key equal to keys[idx-1] from the current node
	child->keys[0] = node->keys[idx-1];
	// Moving sibling's last child as C[idx]'s first child
	if(!child->leaf) {
		child->C[0] = sibling->C[sibling->n];
	}
	// Moving the key from the sibling to the parent
	// This reduces the number of keys in the sibling
	node->keys[idx-1] = sibling->keys[sibling->n-1];
	child->n++;
	sibling->n--;
	return;
}
	
// A function to borrow a key from the C[idx+1] and place
// it in C[idx]
void BTreeNode_borrowFromNext(BTreeNode* node, int idx) {
	BTreeNode *child = node->C[idx];
	BTreeNode *sibling = node->C[idx+1];
	// keys[idx] is inserted as the last key in C[idx]
	child->keys[(child->n)] = node->keys[idx];
	// Sibling's first child is inserted as the last child
	// into C[idx]
	if (!(child->leaf)) {
		child->C[(child->n)+1] = sibling->C[0];
	}
	//The first key from sibling is inserted into keys[idx]
	node->keys[idx] = sibling->keys[0];
	// Moving all keys in sibling one step behind
	for (int i=1; i<sibling->n; i++) {
		sibling->keys[i-1] = sibling->keys[i];
	}
	// Moving the child pointers one step behind
	if (!sibling->leaf) {
		for(int i=0; i != sibling->n; i++) {
			sibling->C[i] = sibling->C[i+1];
		}
	}
	child->n++;
	sibling->n--;
}
	
// A function to fill child C[idx] which has less than t-1 keys
void BTreeNode_fill(BTreeNode* node, int idx) {
	if (idx != 0 && node->C[idx-1]->n >= BTREE_CAPACITY) {
		// If the previous child(C[idx-1]) has more than t-1 keys, borrow a key from that child
		BTreeNode_borrowFromPrev(node, idx);
	} else if (idx != node->n && node->C[idx+1]->n >= BTREE_CAPACITY) {
		// If the next child(C[idx+1]) has more than t-1 keys, borrow a key from that child
		BTreeNode_borrowFromNext(node, idx);
	} else {
		// Merge C[idx] with its sibling
		// If C[idx] is the last child, merge it with with its previous sibling
		// Otherwise merge it with its next sibling
		if (idx != node->n) {
			BTreeNode_merge(node, idx);
		} else {
			BTreeNode_merge(node, idx-1);
		}
	}
	return;
}

// A function to remove the idx-th key from this node - which is a leaf node
void BTreeNode_removeFromLeaf(BTreeNode* node, int idx) {
	for (int i = idx+1; i < node->n; i++) {
		node->keys[i-1] = node->keys[i];
	}
	node->n--;
}

// A function to remove the key k from the sub-tree rooted with this node
void BTreeNode_remove(BTreeNode* node, Key k) {
	int idx = 0;
	while(idx != node->n) {
		if (node->keys[idx] == k) {
			if (node->leaf) {
				BTreeNode_removeFromLeaf(node, idx);
			} else {
				BTreeNode_removeFromNonLeaf(node, idx);
			}
			return;
		}
		if (node->keys[idx] > k) {
			break;
		}
		idx++;
	}
	if (node->leaf) {
		return;
	}
	
	uint8_t is_last_child = idx == node->n;

	if (node->C[idx]->n < BTREE_CAPACITY) {
		// If the child where the key is supposed to exist has less that t keys, we fill that child
		BTreeNode_fill(node, idx);
	}
	// If the last child has been merged, it must have merged with the previous
	// child and so we recurse on the (idx-1)th child. Else, we recurse on the
	// (idx)th child which now has atleast t keys
	if (is_last_child && node->n < idx) {
		BTreeNode_remove(node->C[idx-1], k);
	} else {
		BTreeNode_remove(node->C[idx], k);
	}
}
	
// A function to merge C[idx] with C[idx+1]
// C[idx+1] is freed after merging
void BTreeNode_merge(BTreeNode* node, int idx) {
	BTreeNode *child = node->C[idx];
	BTreeNode *sibling = node->C[idx+1];
	// Pulling a key from the current node and inserting it into (t-1)th
	// position of C[idx]
	child->keys[BTREE_CAPACITY-1] = node->keys[idx];
	// Copying the keys from C[idx+1] to C[idx] at the end
	for (size_t i = 0; i != sibling->n; i++)
		child->keys[i+BTREE_CAPACITY] = sibling->keys[i];
	// Copying the child pointers from C[idx+1] to C[idx]
	if (!child->leaf) {
		for(size_t i = 0; i <= sibling->n; i++)
			child->C[i+BTREE_CAPACITY] = sibling->C[i];
	}
	// Moving all keys after idx in the current node one step before -
	// to fill the gap created by moving keys[idx] to C[idx]
	for (size_t i = idx+1; i != node->n; i++)
		node->keys[i-1] = node->keys[i];
	// Moving the child pointers after (idx+1) in the current node one
	// step before
	for (size_t i = idx+2; i <= node->n; i++)
		node->C[i-1] = node->C[i];
	// Updating the key count of child and the current node
	child->n += sibling->n+1;
	node->n--;
	// Freeing the memory occupied by sibling
	free(sibling);
}
	
// A function to get predecessor of keys[idx]
// 		// A function to get the predecessor of the key- where the key
// 		// is present in the idx-th position in the node
Key BTreeNode_getPred(BTreeNode* node, int idx) {
		// Keep moving to the right most node until we reach a leaf
		node = node->C[idx];
		while (!node->leaf)
				node = node->C[node->n];
	
		// Return the last key of the leaf
		return node->keys[node->n-1];
}
	
Key BTreeNode_getSucc(BTreeNode* node, int idx) {
	// Keep moving the left most node starting from C[idx+1] until we reach a leaf
	node = node->C[idx+1];
	while (!node->leaf)
		node = node->C[0];
	
	// Return the first key of the leaf
	return node->keys[0];
}
	
// A function to remove the idx-th key from this node - which is a non-leaf node
void BTreeNode_removeFromNonLeaf(BTreeNode* node, int idx) {
	Key k = node->keys[idx];
	// If the child that precedes k (C[idx]) has atleast t keys,
	// find the predecessor 'pred' of k in the subtree rooted at
	// C[idx]. Replace k by pred. Recursively delete pred
	// in C[idx]
	if (node->C[idx]->n >= BTREE_CAPACITY) {
		Key pred = BTreeNode_getPred(node, idx);
		node->keys[idx] = pred;
		BTreeNode_remove(node->C[idx], pred);
	} else if (node->C[idx+1]->n >= BTREE_CAPACITY) {
		// If the child C[idx] has less that t keys, examine C[idx+1].
		// If C[idx+1] has atleast t keys, find the successor 'succ' of k in
		// the subtree rooted at C[idx+1]
		// Replace k by succ
		// Recursively delete succ in C[idx+1]
		Key succ = BTreeNode_getSucc(node, idx);
		node->keys[idx] = succ;
		BTreeNode_remove(node->C[idx+1], succ);
	} else {
		// If both C[idx] and C[idx+1] has less that t keys,merge k and all of C[idx+1]
		// into C[idx]
		// Now C[idx] contains 2t-1 keys
		// Free C[idx+1] and recursively delete k from C[idx]
		BTreeNode_merge(node, idx);
		BTreeNode_remove(node->C[idx], k);
	}
	return;
}
	
// A utility function to split the child y of this node
// Note that y must be full when this function is called
void BTreeNode_splitChild(BTreeNode* node, int i, BTreeNode *y) {
	// Create a new node which is going to store (t-1) keys of y
	BTreeNode* z = BTreeNode_new(y->leaf);
	z->n = BTREE_CAPACITY-1;
	// Copy the last (t-1) keys of y to z
	for (int j = 0; j < BTREE_CAPACITY-1; j++)
		z->keys[j] = y->keys[j+BTREE_CAPACITY];
	// Copy the last t children of y to z
	if (!y->leaf) {
		for (int j = 0; j < BTREE_CAPACITY; j++)
			z->C[j] = y->C[j+BTREE_CAPACITY];
	}
	// Reduce the number of keys in y
	y->n = BTREE_CAPACITY - 1;
	// Since this node is going to have a new child,
	// create space of new child
	for (int j = node->n; j >= i+1; j--)
		node->C[j+1] = node->C[j];
	// Link the new child to this node
	node->C[i+1] = z;
	// A key of y will move to this node. Find location of
	// new key and move all greater keys one space ahead
	for (int j = node->n-1; j >= i; j--)
		node->keys[j+1] = node->keys[j];
	// Copy the middle key of y to this node
	node->keys[i] = y->keys[BTREE_CAPACITY-1];
	// Increment count of keys in this node
	node->n++;
}
	
// A utility function to insert a new key in this node
// The assumption is, the node must be non-full when this
// function is called
void BTreeNode_insertNonFull(BTreeNode* node, Key k) {
	// Initialize index as index of rightmost element
	int i = node->n-1; // TODO: size_t causes undefined behaviour
	// If this is a leaf node
	if (node->leaf) {
		// The following loop does two things
		// a) Finds the location of new key to be inserted
		// b) Moves all greater keys to one place ahead
		while (i >= 0 && node->keys[i] > k) {
			node->keys[i+1] = node->keys[i];
			i--;
		}
		// Insert the new key at found location
		node->keys[i+1] = k;
		node->n++;
	} else {
		// If this node is not leaf
		// Find the child which is going to have the new key
		while (i >= 0 && node->keys[i] > k)
			i--;
		// See if the found child is full
		if (node->C[i+1]->n == 2*BTREE_CAPACITY-1) {
			// If the child is full, then split it
			BTreeNode_splitChild(node, i+1, node->C[i+1]);
			// After split, the middle key of C[i] goes up and
			// C[i] is splitted into two.  See which of the two
			// is going to have the new key
			if (node->keys[i+1] < k)
				i++;
		}
		BTreeNode_insertNonFull(node->C[i+1], k);
	}
}

// Function to search key k in subtree rooted with this node
uint8_t BTreeNode_search(BTreeNode* node, Key k) {
	size_t i = 0;
	while (i < node->n && node->keys[i] < k) 
		i++;
	
	if (node->keys[i] == k)
		return 1;

	if (node->leaf)
		return 0;

	return BTreeNode_search(node->C[i], k);
}

typedef struct BTree {
		BTreeNode* root;
} BTree;

BTree BTree_new() {
	BTree ret = { .root = NULL };
	return ret;
}

uint8_t BTree_contains(BTree* tree, Key k) {
	return (tree->root == NULL) ? 0 : BTreeNode_search(tree->root, k);
}

// The main function that inserts a new key in this B-Tree
void BTree_insert(BTree* tree, Key k) {
	// If tree is empty
	if (tree->root == NULL) {
		// Allocate memory for root
		tree->root = BTreeNode_new(1);
		tree->root->keys[0] = k;  // Insert key
		tree->root->n = 1;  // Update number of keys in root
	} else { // If tree is not empty
		// If root is full, then tree grows in height
		if (tree->root->n == 2*BTREE_CAPACITY-1) {
			// Allocate memory for new root
			BTreeNode *s = BTreeNode_new(0);
			// Make old root as child of new root
			s->C[0] = tree->root;
			// Split the old root and move 1 key to the new root
			BTreeNode_splitChild(s, 0, tree->root);
			// New root has two children now.  Decide which of the
			// two children is going to have new key
			int i = 0;
			if (s->keys[0] < k)
				i++;
			BTreeNode_insertNonFull(s->C[i], k);
			// Change root
			tree->root = s;
		} else { // If root is not full, call insertNonFull for root
			BTreeNode_insertNonFull(tree->root, k);
		}
	}
}

void BTree_remove(BTree* tree, Key k) {
	if (tree->root == NULL) {
		return;
	}
	// Call the remove function for root
	BTreeNode_remove(tree->root, k);
	// If the root node has 0 keys, make its first child as the new root
	// if it has a child, otherwise set root as NULL
	if (tree->root->n == 0) {
		BTreeNode* temp = tree->root;
		if (tree->root->leaf) {
			tree->root = NULL;
		} else {
			tree->root = tree->root->C[0];
		}
		free(temp);
	}
}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////// RBTree ///////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct RBNode {
  Key val;
  uint8_t is_red;
	struct RBNode* parent;
  struct RBNode* left;
	struct RBNode* right;
} RBNode;
 
typedef struct RBTree {
	RBNode *root;
} RBTree;

RBNode* rbtree_new_node(Key val) {
	RBNode* ret = malloc(sizeof(RBNode));
	ret->val = val;
	ret->parent = NULL;
	ret->left = NULL;
	ret->right = NULL;
	ret->is_red = 1;
	return ret;
}
 
// check if node is left child of parent
uint8_t rbtree_is_on_left(RBNode* node) {
	return node == node->parent->left;
}
 
RBNode* rbtree_parent_sibling(RBNode* node) {
  if (node->parent == NULL || node->parent->parent == NULL) {
    return NULL;
	} else if (rbtree_is_on_left(node->parent)) {
    return node->parent->parent->right;
	} else {
  	return node->parent->parent->left;
	}
}

RBNode* rbtree_sibling(RBNode* node) {
  if (node->parent == NULL) {
    return NULL;
	} else if (rbtree_is_on_left(node)) {
    return node->parent->right;
	} else {
  	return node->parent->left;
	}
}

void rbtree_move_down(RBNode* node, RBNode* nParent) {
  if (node->parent != NULL) {
    if (rbtree_is_on_left(node)) {
      node->parent->left = nParent;
    } else {
      node->parent->right = nParent;
    }
  }
  nParent->parent = node->parent;
  node->parent = nParent;
}

// left rotates the given node
void rbtree_rotate_left(RBNode** root, RBNode* x) {
  RBNode *new_parent = x->right;

  if (x == *root) {
    *root = new_parent;
	}

  rbtree_move_down(x, new_parent);

  x->right = new_parent->left;

  if (new_parent->left != NULL) {
    new_parent->left->parent = x;
	}

  new_parent->left = x;
}

void rbtree_rotate_right(RBNode** root, RBNode* x) {
  RBNode *new_parent = x->left;

  if (x == *root) {
    *root = new_parent;
	}

  rbtree_move_down(x, new_parent);

  x->left = new_parent->right;

  if (new_parent->right != NULL) {
    new_parent->right->parent = x;
	}

  new_parent->right = x;
}

void rbtree_swap_colors(RBNode *n1, RBNode *n2) {
  uint8_t temp = n1->is_red;
  n1->is_red = n2->is_red;
  n2->is_red = temp;
}

// fix red red at given node
void rbtree_fix_red_red(RBNode** root, RBNode* x) {
  // if x is root color it black and return
  if (x == *root) {
    x->is_red = 0;
    return;
  }

  // initialize parent, grandparent, uncle
  RBNode* parent = x->parent;
	RBNode* grandparent = parent->parent;

  RBNode* uncle;
  if (parent == NULL || grandparent == NULL)
    uncle = NULL;
  else if (rbtree_is_on_left(parent))
    uncle = grandparent->right;
  else
    uncle = grandparent->left;

  if (parent->is_red) {
    if (uncle != NULL && uncle->is_red) {
      // uncle red, perform recoloring and recurse
      parent->is_red = 0;
      uncle->is_red = 0;
      grandparent->is_red = 1;
      rbtree_fix_red_red(root, grandparent);
    } else {
      // Else perform LR, LL, RL, RR
      if (rbtree_is_on_left(parent)) {
        if (rbtree_is_on_left(x)) {
          // for left right
          rbtree_swap_colors(parent, grandparent);
        } else {
          rbtree_rotate_left(root, parent);
          rbtree_swap_colors(x, grandparent);
        }
        // for left left and left right
        rbtree_rotate_right(root, grandparent);
      } else {
        if (rbtree_is_on_left(x)) {
          // for right left
          rbtree_rotate_right(root, parent);
          rbtree_swap_colors(x, grandparent);
        } else {
          rbtree_swap_colors(parent, grandparent);
        }

        // for right right and right left
        rbtree_rotate_left(root, grandparent);
      }
    }
  }
}

// find node that replaces a deleted node in BST
RBNode* BSTreplace(RBNode* x) {
  if (x->left != NULL && x->right != NULL) {
		RBNode* temp = x->right;

		while (temp->left != NULL)
			temp = temp->left;

		return temp;
	}

  if (x->left != NULL) {
    return x->left;
	} else if (x->right != NULL) {
    return x->right;
	}
  return NULL;
}

void rbtree_delete_node(RBTree* tree, RBNode *node) {
  RBNode *u = BSTreplace(node);

  uint8_t uvBlack = (u == NULL || !u->is_red) && !node->is_red;
  RBNode *parent = node->parent;

  if (u == NULL) {
    if (node == tree->root) {
      tree->root = NULL;
    } else {
      if (uvBlack) {
        rbtree_fix_black_black(tree, node);
      } else {
        if (rbtree_sibling(node) != NULL) {
          rbtree_sibling(node)->is_red = 1;
				}
      }

      if (rbtree_is_on_left(node)) {
        parent->left = NULL;
      } else {
        parent->right = NULL;
      }
    }
    free(node);
		return;
  }

  if (node->left == NULL || node->right == NULL) {
    // v has 1 child
    if (node == tree->root) {
      // v is root, assign the value of u to v, and delete u
      node->val = u->val;
      node->left = node->right = NULL;
      free(u);
    } else {
      // Detach v from tree and move u up
      if (rbtree_is_on_left(node)) {
        parent->left = u;
      } else {
        parent->right = u;
      }
      free(node);
      u->parent = parent;
      if (uvBlack) {
        // u and v both black, fix double black at u
        rbtree_fix_black_black(tree, u);
      } else {
        // u or v red, color u black
        u->is_red = 0;
      }
    }
    return;
  }

  Key swap = u->val;
  u->val = node->val;
  node->val = swap;

  rbtree_delete_node(tree, u);
}

void rbtree_fix_black_black(RBTree* tree, RBNode* x) {
  if (x == tree->root) {
    return;
	}

  RBNode *sibling = rbtree_sibling(x), *parent = x->parent;
  if (sibling == NULL) {
    rbtree_fix_black_black(tree, parent);
		return;
  }

  if (sibling->is_red) {
    parent->is_red = 1;
    sibling->is_red = 0;
    if (rbtree_is_on_left(sibling)) {
      rbtree_rotate_right(tree, parent);
    } else {
      rbtree_rotate_left(tree, parent);
    }
    rbtree_fix_black_black(tree, x);
		return;
  }
	
  if (sibling->left != NULL && sibling->left->is_red) {
    if (rbtree_is_on_left(sibling)) {
      sibling->left->is_red = sibling->is_red;
      sibling->is_red = parent->is_red;
      rbtree_rotate_right(tree, parent);
    } else {
      sibling->left->is_red = parent->is_red;
      rbtree_rotate_right(tree, sibling);
      rbtree_rotate_left(tree, parent);
    }
 		parent->is_red = 0;
  } else if (sibling->right != NULL && sibling->right->is_red) {
    if (rbtree_is_on_left(sibling)) {
      sibling->right->is_red = parent->is_red;
      rbtree_rotate_left(tree, sibling);
      rbtree_rotate_right(tree, parent);
    } else {
      sibling->right->is_red = sibling->is_red;
      sibling->is_red = parent->is_red;
      rbtree_rotate_left(tree, parent);
    }
 		parent->is_red = 0;
  } else {
    sibling->is_red = 1;
    if (!parent->is_red) {
      rbtree_fix_black_black(tree, parent);
		} else {
      parent->is_red = 0;
		}
  }
}
 
RBTree rbtree_new() {
	RBTree ret = { .root = NULL };
	return ret;
}

RBNode* rbtree_search(RBNode* node, Key key) {
  while (1) {
		if (key == node->val) {
  		return node;
    }
		RBNode* next;
    if (key < node->val) {
			next = node->left;
    } else {
      next = node->right;
    }
    if (next == NULL) {
  		return node;
		}
		node = next;
  }
}

void rbtree_insert(RBTree* tree, Key key) {
  if (tree->root == NULL) {
  	RBNode* new_node = rbtree_new_node(key);
    new_node->is_red = 0;
    tree->root = new_node;
		return;
  }

  RBNode *parent = rbtree_search(tree->root, key);

  if (parent->val == key) {
  	return;
	}

  RBNode* new_node = rbtree_new_node(key);
  new_node->parent = parent;
  if (key < parent->val) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
	}

  rbtree_fix_red_red(tree, new_node);
}

void rbtree_remove(RBTree* tree, Key key) {
  if (tree->root == NULL) {
    return;
	}

  RBNode* node = rbtree_search(tree->root, key);

  if (node->val == key) {
  	rbtree_delete_node(tree, node);
  }
}

void BTreeNode_print(BTreeNode* node) {
  for (size_t i = 0; i < node->n+1; i++) {
    if (!node->leaf) {
      BTreeNode_print(node->C[i]);
		}
		if (i < node->n) {
			printf("%d ", node->keys[i]);
		}
  }
}

void BTree_print(BTree* tree) {
	printf("BTree contents: ");
  if (tree->root != NULL) {
		BTreeNode_print(tree->root);
	}
	printf("\n");
}

void rbtree_print(RBNode* node) {
  if (node != NULL) {
		rbtree_print(node->left);
		printf("%d ", node->val);
		rbtree_print(node->right);
	}
}

// void btree_destroy(BTreeNode* node) {
//   if (node != NULL) {
// 		for(size_t i = 0; i != node->n+1; i++) {
// 			btree_destroy(node->C[i]);
// 		}
// 		free(node);
// 	}
// }

// void rbtree_destroy(RBNode* node) {
//   if (node != NULL) {
// 		rbtree_destroy(node->left);
// 		rbtree_destroy(node->right);
// 		free(node);
// 	}
// }

// int main(int argc, char** argv) {
// 	BTree btree = BTree_new();
// 	RBTree rbtree = rbtree_new();

// 	char line[80];

// 	puts("Commands: \"insert\", \"remove\", \"ascending\", \"exit\".");

// 	while(1) {
// 		scanf("%s", line);
// 		if (strcmp("insert", line) == 0) {
// 			Key key;
// 			scanf("%d", &key);
// 			BTree_insert(&btree, key);
// 			rbtree_insert(&rbtree, key);
// 		} else if (strcmp("remove", line) == 0) {
// 			Key key;
// 			scanf("%d", &key);
// 			BTree_remove(&btree, key);
// 			rbtree_remove(&rbtree, key);
// 		} else if (strcmp("ascending", line) == 0) {
// 			BTree_print(&btree);
// 			printf("RBTree contents: ");
// 			rbtree_print(rbtree.root);
// 			printf("\n");
// 		} else if (strcmp("exit", line) == 0) {
// 			break;
// 		} else {
// 			puts("Unknown command. Try \"insert\", \"remove\", \"ascending\" or \"exit\".");
// 		}
// 	}
	
// 	rbtree_destroy(rbtree.root);

// 	return 0;
// }


void insertKey(BTree* btree, RBTree* rbtree, Key key) {
	printf("Inserting %d", key);
	BTree_insert(btree, key);
	rbtree_insert(rbtree, key);
}

void removeKey(BTree* btree, RBTree* rbtree, Key key) {
	printf("Removing %d", key);
	BTree_insert(btree, key);
	rbtree_insert(rbtree, key);
}

void print(BTree* btree, RBTree* rbtree) {
	printf("B-Tree: ");
	if (btree->root != NULL) {
		BTreeNode_print(btree->root);
	}
	printf("\n");
	printf("Red-Black-Tree: ");
	rbtree_print(rbtree->root);
	printf("\n");
}

int main(int argc, char** argv) {
	BTree btree = BTree_new();
	RBTree rbtree = rbtree_new();

	insertKey(&btree, &rbtree, 7);
	insertKey(&btree, &rbtree, 5);
	insertKey(&btree, &rbtree, 2);
	insertKey(&btree, &rbtree, 3);
	insertKey(&btree, &rbtree, 4);
	insertKey(&btree, &rbtree, 1);
	insertKey(&btree, &rbtree, 6);
	print(&btree, &rbtree);
	removeKey(&btree, &rbtree, 4);
	print(&btree, &rbtree);
	removeKey(&btree, &rbtree, 1);
	print(&btree, &rbtree);
	removeKey(&btree, &rbtree, 12);
	print(&btree, &rbtree);

	return 0;
}