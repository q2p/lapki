#include <iostream>

// В качестве ключа - целое число.
typedef int Key;

// Класс ветви дерева.
class Node {
	public:
		Key key;
		Node* left = nullptr;
		Node* right = nullptr;
		// Высота под-дерева.
		int height = 1;

		// При создании, записываем в Node::key значение key.
		Node(Key key) : key { key } {}
		
		// Нужно рекурсивно удалить дочерние ветви.
		~Node() {
			delete left;
			delete right;
		}
};

// Класс полноценного AVL дерева.
class Tree {
	public:
		// При удалении, нужно удалить ветви, так как они создаются динамически.
		~Tree() {
			delete root;
		}

		// Выводит элементы по возрастанию.
		void printAscending() {
			std::cout << "Ascending: ";
			printAscendingInternal(root);
			std::cout << "\n";
		}

		// Вставка элемента.
		void insert(int key) {
			this->root = insertInternal(this->root, key);
			// Баланс корня после вставки.
			std::cout << "Balance: " << calcBalance(this->root) << "\n";
		}

		// Удаление элемента.
		void remove(int key) {
			this->root = removeInternal(this->root, key);
			// Баланс корня после удаления.
			std::cout << "Balance: " << calcBalance(this->root) << "\n";
		}

	private:
		// При создании ветвей нет.
		Node* root = nullptr;

		// Рекурсивно обходит дерево и выводит эл-ты по возрастанию.
		void printAscendingInternal(Node* node) {
			if (node != NULL) {
				printAscendingInternal(node->left);
				std::cout << node->key << " ";
				printAscendingInternal(node->right);
			}
		}

		// Находит наибольший элемент.
		static Key max(Key a, Key b) {
			return a > b ? a : b;
		}

		// Вычисляет вышину под-дерева.
		static int height(Node* node) {
			if (node == NULL) {
				return 0;
			}
			return node->height;
		}

		// Находит наименьший элемент в под-дереве.
		static Node* minValueNode(Node* node) {
			while(node->left != NULL) {
				node = node->left;
			}
			return node;
		}

		// Разворот влево.
		static Node* leftRotate(Node* x) {
			Node* y = x->right;
			Node* T2 = y->left;

			y->left = x;
			x->right = T2;

			// Пересчёт вышин ветвей.
			x->height = max(height(x->left), height(x->right))+1;
			y->height = max(height(y->left), height(y->right))+1;
			
			return y;
		}

		// Разворот вправо.
		static Node* rightRotate(Node* y) {
			Node* x = y->left;
			Node* T2 = x->right;

			x->right = y;
			y->left = T2;

			// Пересчёт вышин ветвей.
			x->height = max(height(x->left), height(x->right))+1;
			y->height = max(height(y->left), height(y->right))+1;
			
			return x;
		}

		// Баланс ветви, если < 0, то в правой ветви больше элементов, если > 0, то в левой, если = 0, то ветви одинаковой высоты.
		static int calcBalance(Node* node) {
			if (node == NULL) {
				return 0;
			}
			return height(node->left) - height(node->right);
		}

		// Рекурсивно вставляет элемент в под-дерево.
		static Node* insertInternal(Node* node, int key) {
			if (node == NULL) {
				return new Node(key);
			}

			if (key < node->key) {
				// Если нужно вставить в левое под-дерево.
				node->left = insertInternal(node->left, key);
			} else if (key > node->key) {
				// Если нужно вставить в правое под-дерево.
				node->right = insertInternal(node->right, key);
			} else {
				// Если элемент уже есть в дереве.
				return node;
			}

			// Обновляем высоту после вставки.
			node->height = 1 + max(height(node->left), height(node->right));

			int balance = calcBalance(node);

			// Ситуация левый, левый
			if (balance > 1 && key < node->left->key) {
				return rightRotate(node);
			}

			// Ситуация правый, правый
			if (balance < -1 && key > node->right->key) {
				return leftRotate(node);
			}

			// Ситуация левый, правый
			if (balance > 1 && key > node->left->key) {
				node->left = leftRotate(node->left);
				return rightRotate(node);
			}

			// Ситуация правый, левый
			if (balance < -1 && key < node->right->key) {
				node->right = rightRotate(node->right);
				return leftRotate(node);
			}

			// Дерево сбалансировано
			return node;
		}

		// Рекурсивно удаляет элемент из под-дерева. Возвращает новый корень под-дерева.
		static Node* removeInternal(Node* root, int key) {
			if(root == NULL) {
				return root;
			}
			
			if (key < root->key) {
				// Если эл-т в левом под-дереве.
				root->left = removeInternal(root->left, key);
			} else if (key > root->key) {
				// Если эл-т в правом под-дереве.
				root->right = removeInternal(root->right, key);
			} else {
				// Если элемент в данной ветке.
				if (root->left == NULL || root->right == NULL) {
					// Если есть только один лист или листьев нет.
					Node* temp = root->left ? root->left : root->right;

					if (temp == NULL) {
						// Если листьев нет, то удаляем корень
						delete root;
						return nullptr;
					} else {
						// Если лист есть, то меняем корень и лист местами
						*root = *temp;
						delete temp;
					}
				} else {
					// Если ветвей две, то ищем наименьший элемент в правой ветви и ставим на место удаляемого эл-та
					Node* temp = minValueNode(root->right);
					root->key = temp->key;
					root->right = removeInternal(root->right, temp->key);
				}
			}

			if (root == NULL) {
				return root;
			}

			// Пересчёт высоты.
			root->height = 1 + max(height(root->left), height(root->right));

			int balance = calcBalance(root);

			// Ситуация левый, левый
			if (balance > 1 && calcBalance(root->left) >= 0) {
				return rightRotate(root);
			}

			// Ситуация левый, правый
			if (balance > 1 && calcBalance(root->left) < 0) {
				root->left = leftRotate(root->left);
				return rightRotate(root);
			}

			// Ситуация правый, правый
			if (balance < -1 && calcBalance(root->right) <= 0) {
				return leftRotate(root);
			}

			// Ситуация правый, левый
			if (balance < -1 && calcBalance(root->right) > 0) {
				root->right = rightRotate(root->right);
				return leftRotate(root);
			}

			// Дерево сбалансировано
			return root;
		}
};

int main() {
	Tree tree;

	std::string line;

	while(true) {
		// Читаем команду
		std::cin >> line;
		if (line == "insert") { // Выполняем вставку
			Key key;
			std::cin >> key;
			tree.insert(key);
		} else if (line == "remove") { // Выполняем удаление
			Key key;
			std::cin >> key;
			tree.remove(key);
		} else if (line == "print") { // Выводим элементы по возрастанию
			tree.printAscending();
		} else if (line == "exit") { // Выход из программы
			break;
		} else { // Неизвестная команда
			std::cout << "Unknown command. Try \"insert\", \"remove\", \"print\" or \"exit\".\n";
		}
	}

	return 0;
}