#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

double getLagrange(vector<double> arrayX, vector<double> arrayY, double x) {
    double t, y = 0.0;
    int i1, i2;
    for(i1 = 0; i1 < arrayX.size(); i1++) {
        t = 1.0;
        for(i2 = 0; i2 < arrayX.size(); i2++) {
            if (arrayX[i1] != arrayX[i2]) {
                t = t * (x - arrayX[i2]) / (arrayX[i1] - arrayX[i2]);
            }
        }
        y = y + t * arrayY[i1];
    }
    return y;
}

int main() {
    int size, i;
    double x, y;
    vector<double> arrayX, arrayY;

    cin >> size;
    for(int i = 0; i < size; i++) {
        cin >> x;
        cin >> y;
        arrayX.push_back(x);
        arrayY.push_back(y);
    }

    cin >> size;
    for(int i = 0; i < size; i++) {
        cin >> x;
        y = getLagrange(arrayX, arrayY, x);
        arrayX.push_back(x);
        arrayY.push_back(y);
    }

    ofstream out("graphic.txt");
    for(i = 0; i < arrayX.size(); i++) {
        out << arrayX[i] << " " << arrayY[i] << endl;
    }
    out.close();

    return 0;
}