#include <string>
#include <iostream>
using namespace std;

class MobilePhone {
	private:
		int storage_capacity;
		std::string manufacturer;
		std::string model;
	public:
		MobilePhone(std::string* manufacturer, std::string* model, int storage_capacity) {
			this->set_manufacturer(manufacturer);
			this->set_model(model);
			this->set_storage_capacity(storage_capacity);
		}
		std::string* get_manufacturer() {
			return &this->manufacturer;
		}
		void set_manufacturer(std::string* manufacturer) {
			this->manufacturer = *manufacturer;
		}
		std::string* get_model() {
			return &this->model;
		}
		void set_model(std::string* model) {
			this->model = *model;
		}
		int get_storage_capacity() {
			return this->storage_capacity;
		}
		void set_storage_capacity(int storage_capacity) {
			this->storage_capacity = storage_capacity;
		}
		void print() {
			std::cout << "Phone " << this->manufacturer << " " << this->model << " has " << this->storage_capacity << "MB storage capacity.\n";
		}
};


void main() {
	MobilePhone phone(&std::string("Nokia"), &std::string("3310"), 2);
	std::cout << "Phone's manufacturer:  " << *phone.get_manufacturer() << '\n';
	std::cout << "Phone's model: " << *phone.get_model() << '\n';
	std::cout << "Phone's storage capacity: " << phone.get_storage_capacity() << '\n';

	phone.set_storage_capacity(16);

	phone.print();
}