// g++ -Wall -pedantic ./main.cpp && ./a.out

#include <cstdint>
#include <iostream>
#include <list>

class WordNode {
	private:
		uint8_t pages_amount;
		uint16_t pages[10];

	public:
		WordNode* next_node;
		std::string word;

		WordNode(std::string* word, uint16_t page) {
			this->next_node = next_node;
			this->word = *word;
			this->pages_amount = 1;
			this->pages[0] = page;
		}

		void print() {
			std::cout << "Word " << word << " is found on pages ";
			for(int i = 0; i != this->pages_amount; i++) {
				if (i != 0) {
					std::cout << ", ";
				}
				std::cout << this->pages[i];
			}
			std::cout << '.' << std::endl;
		}

		void insert(uint16_t page) {
			for(int i = 0; i != this->pages_amount; i++) {
				if(this->pages[i] == page) {
					return;
				}
			}
			if(this->pages_amount == 10) {
				std::cout << "Can't append any more pages." << std::endl;
				return;
			}
			this->pages[this->pages_amount++] = page;
		}
};

class Table {
	private:
		std::list<WordNode> words;

	public:
		void look_up_word(std::string* word) {
			for (WordNode& i : this->words) {
				if(i.word == *word) {
					i.print();
				}
			}
		}

		void print_all_words() {
			for (WordNode& i : this->words) {
				i.print();
			}
		}

		void add_word(std::string* word, uint16_t page) {
			for (WordNode& i : this->words) {
				if(i.word == *word) {
					i.insert(page);
					return;
				}
			}
			words.emplace_back(word, page);
		}
};

int main(int argc, char** argv) {
	std::string action;
	std::string word;

	Table table;

	while(1) {
		std::cin >> action;
		if(action == "put") {
			uint16_t page;
			std::cin >> word >> page;
			table.add_word(&word, page);
		} else if(action == "search") {
			std::cin >> word;
			table.look_up_word(&word);
		} else if(action == "dump") {
			table.print_all_words();
		} else {
			std::cout << "Unknown command, try 'put', 'search' or 'dump'." << std::endl;
		}
	}

	return 0;
}
