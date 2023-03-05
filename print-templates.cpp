#include <iostream>
#include <vector>
#include <set>
#include <map>

using namespace std;

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& container) {
    out << "["s;
    int counter = 0;
    for (const T& element: container) {
        out << element;
        if (counter != (container.size() - 1)) {
            out << ", "s;
        }
        ++counter;
    }
    out << "]"s;
    return out;
}

template <typename T>
ostream& operator<<(ostream& out, const set<T>& container) {
    out << "{"s;
    int counter = 0;
    for (const T& element: container) {
        out << element;
        if (counter != (container.size() - 1)) {
            out << ", "s;
        }
        ++counter;
    }
    out << "}"s;
    return out;
}

template <typename T, typename U>
ostream& operator<<(ostream& out, const map<T, U>& container) {
    out << "{"s;
    int counter = 0;
    for (auto& [key, value]: container) {
        out << key << ": "s << value;
        if (counter != (container.size() - 1)) {
            out << ", "s;
        }
        ++counter;
    }
    out << "}"s;
    return out;
}