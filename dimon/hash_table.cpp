#include <iostream>
#include <fstream>
#include <vector>
#include <list>

using namespace std;

class HashTable
{
	int size;
	int amount_of_elements = 0;
	const int p = 7;
	vector<vector<string>> data = vector<vector<string>>();
	public:
		HashTable(int hash_table_size)
		{
			size = hash_table_size;
			for (int i = 0; i != size; i++) {
				this->data.push_back(vector<string>());
			}
		}

	// Хэшируем по первой букве в слове.
	// Так можно будет легко осуществить поиск по первой букве.
	int hash_func(string value, int size, int p)
	{
		return (value[0] + p) % size;
	}

	bool insert_value(string value)
	{
		int bucket = hash_func(value, size, p);
		for(string item : data[bucket])
		{
			if (item == value)
			{
				return false;
			}
		}

		data[bucket].push_back(value);
		amount_of_elements++;
		return true;
	}

	bool contains_value(string value)
	{
		int bucket = hash_func(value, size, p);
		for(string item : this->data[bucket])
		{
			if (item == value)
			{
				return true;
			}
		}
		return false;
	}

	bool remove_value(string value)
	{
		int bucket = hash_func(value, size, p);
		for (int i = 0; i++; i != this->data[bucket].size())
		{
			string word = this->data[bucket][i];
			if (word == value) {
				this->data[bucket].erase(this->data[bucket].begin() + i);
				return true;
			}
		}
		return false;
	}

	int remove_starting_with(string letter)
	{
		int count = 0;
		int bucket = hash_func(letter, size, p);
		for (int i = 0; i != this->data[bucket].size();)
		{
			string word = this->data[bucket][i];
			if (word[0] == letter[0]) {
				this->data[bucket].erase(this->data[bucket].begin() + i);
				count++;
			} else {
				i++;
			}
		}
		return count;
	}

	void out()
	{
		ofstream file("output.txt");

		for (int i = 0; i != this->data.size(); i++)
		{
			file << (i+1) << ":";
			for (string word : this->data[i]) {
				file << " " << word;
			}
			file << "\n";
		}

		file.flush();
		file.close();
	}
};

vector<string> read_words(string file_name)
{
	vector<string> ret;

	ifstream file;
	file.open(file_name, ifstream::in);

	if (!file.is_open()) {
		cout << "Ошибка при открытии файла!" << endl;
		exit(1);
	}

	string word;
	word.clear();
	
	while (file >> word)
	{
		ret.push_back(word);
		word.clear();
	}

	return ret;
}

int main() {
	// Читаем все слова из файла
	vector<string> words = read_words("input.txt");

	int hash_table_size;

	cout << "Введите размер хэш-таблицы..." << endl;
	cin >> hash_table_size;

	// Создаём таблицу
	HashTable table = HashTable(hash_table_size);

	// Заполняем таблицу словами из файла
	for (string word : words) {
		table.insert_value(word);
	}

	// Сохраняем таблицу
	table.out();

	while(true) {
		cout << "Напишите слово, чтобы найти его в таблице или \"q\" чтобы прекратить поиск..." << endl;

		string word;
		cin >> word;

		if (word == "q") {
			break;
		}

		if (table.contains_value(word)) {
			cout << "Слово " << word << " есть в таблице" << endl;
		} else {
			cout << "Слова " << word << " нет в таблице" << endl;
		}
	}

	table.out();

	while(true) {
		cout << "Удалить слова начинающиеся с символа..." << endl;

		string symbol;
		cin >> symbol;

		// Если слово длиной в 1 символ
		if (symbol.size() == 1) {
			int removed = table.remove_starting_with(symbol);

			if (removed == 0) {
				cout << "Таких слов не найдено" << endl;
			} else {
				cout << "Удалено " << removed << " слов" << endl;
			}

			// Сохраняем таблицу
			table.out();

			break;
		}
	}

	return 0;
}