#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>

class Point {
  public:
    double x;
    double y;
    bool is_interpolated;
};

double interpolate(std::vector<Point>& points, double x) {
  double ret = 0.0;
  for (auto& i : points) {
    if (i.is_interpolated) {
      continue;
    }
    double u = i.y;
    double l = 1.0;
    for (auto& j : points) {
      if (!j.is_interpolated && i.x != j.x) {
        u *= x - j.x;
        l *= i.x - j.x;
      }
    }
    ret += u / l;
  }
  return ret;
}

void insert_point(std::vector<Point>& points, Point& new_point) {
  for(auto i = points.begin(); i != points.end(); i++) {
    if (new_point.x < i->x) {
      points.insert(i, new_point);
      return;
    }
  }
  points.push_back(new_point);
}

void write_points(std::string path, std::vector<Point>& points) {
  std::ofstream file(path);
  for(auto& p : points) {
    file << p.x << ' ' << p.y << '\n';
  }
  file.flush();
  file.close();
}

int main() {
  std::vector<Point> points;
  Point point;
  size_t size;

  std::cout << "Enter the number of initial points: ";
  std::cin >> size;
  std::cout << "Enter X and Y coordinates for initial points:\n";
  point.is_interpolated = false;
  while(size-- != 0) {
    std::cin >> point.x >> point.y;
    insert_point(points, point);
  }
  write_points("initial.txt", points);

  std::cout << "Enter the number of points to interpolate: ";
  std::cin >> size;
  std::cout << "Enter X coordinates for points to interpolate:\n";
  point.is_interpolated = true;
  while(size-- != 0) {
    std::cin >> point.x;
    point.y = interpolate(points, point.x);
    insert_point(points, point);
  }
  write_points("interpolated.txt", points);

  return 0;
}