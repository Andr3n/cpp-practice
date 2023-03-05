#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

void AssertImpl(const bool& v, const string& v_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (v != true) {
        cout << boolalpha;
        cout << file << " (line: "s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << v_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << " (line: "s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void PrintDiagnosticInformation() {
    cout << "Function name: "s << __FUNCTION__ << endl;
    cout << "File name: "s << __FILE__ << endl;
    cout << "Line number: "s << __LINE__ << endl;
    cout << "Line number: "s << __LINE__ << endl;
}