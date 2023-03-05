#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

void AssertImpl(const bool& v, const string& v_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (v != true) {
        cerr << boolalpha;
        cerr << file << " (line: "s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << v_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << " (line: "s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& f, const string& f_str, const string& file,
                 const string& func, unsigned line) {
    f();
    cerr << f_str << " passed!" << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func, __FILE__, __FUNCTION__, __LINE__) // напишите недостающий код