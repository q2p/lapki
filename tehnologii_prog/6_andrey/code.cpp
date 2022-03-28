#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>

class Square {
	public:
		double side;
	
		Square (double side) {
			this->side = side;
		}
	
		double diagonal() {
			return sqrt(side*side);
		}
	
		double perimeter() {
			return 4*side;
		}
	
		double area() {
			return side*side;
		}

		void print_info() {
			std::cout << "Square: "                    << std::endl;
			std::cout << " Side: "      << side        << std::endl;
			std::cout << " Diagonal: "  << diagonal()  << std::endl;
			std::cout << " Perimeter: " << perimeter() << std::endl; 
			std::cout << " Area: "      << area()      << std::endl; 
		}
};
 
class Pyramid : public Square {
	public:
		double apopheme;
		
		Pyramid(double base, double apopheme) : Square(base) {
			this->apopheme = apopheme;
		}
	
		double height() {
			return sqrt(apopheme*apopheme - side*side/4);
		}
	
		double volume() {
			return side*side * height() / 3;
		}
	
		double area() {
			return side*side + 2 * apopheme * side;
		}

		void print_info() {
			std::cout << "Pyramid: "                << std::endl;
			std::cout << " Base: "      << side     << std::endl;
			std::cout << " Height: "    << height() << std::endl;
			std::cout << " Apopheme: "  << apopheme << std::endl;
			std::cout << " Area: "      << area()   << std::endl;
			std::cout << " Volume: "    << volume() << std::endl; 
			std::cout << " Area: "      << area()   << std::endl; 
		}
};

double random_value() {
	return (double)(rand() % 100 + 1) / 10;
}
 
int main(void) {
	int squares_amount;
	int pyramids_amount;
	double a;
	std::cin >> squares_amount >> pyramids_amount >> a;

	std::vector<Square> squares;
	std::vector<Pyramid> pyramids;

	for(int i = 0; i != squares_amount; i++) {
		squares.push_back(Square(random_value()));
	}
	for(int i = 0; i != pyramids_amount; i++) {
		pyramids.push_back(Pyramid(random_value(), random_value()));
	}
	
	for(int i = 0; i != squares.size(); i++) {
		squares[i].print_info();
	}
	for(int i = 0; i != pyramids.size(); i++) {
		pyramids[i].print_info();
	}

	Square* min_area = &squares[0];
	for(int i = 0; i != squares.size(); i++) {
		if (squares[i].area() < min_area->area()) {
			min_area = &squares[i];
		}
	}
	
	int matches = 0;
	for(int i = 0; i != pyramids.size(); i++) {
		if (pyramids[i].height() > a) {
			matches++;
		}
	}

	std::cout << "Smallest area within squares is: " << min_area->area() << std::endl;
	std::cout << "There is " << matches << " pyramids higher than " << a << std::endl;
	return 0;
}