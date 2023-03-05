#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include "unit-tests-lib.cpp"

using namespace std;

class Synonyms {
public:
    void Add(const string& first_word, const string& second_word) {
        synonyms_[first_word].insert(second_word);
        synonyms_[second_word].insert(first_word);
    }

    size_t GetSynonymCount(const string& word) const {
        if (synonyms_.count(word) != 0) {
            return synonyms_.at(word).size();
        }
        return 0;
    }

    bool AreSynonyms(const string& first_word, const string& second_word) const {
        if (synonyms_.count(first_word) != 0) {
            if (synonyms_.at(first_word).count(second_word) != 0) {
                return true;
            }
            else {
                return false;
            }
        } 
        else {
            return false;
        }
    }

private:
    map<string, set<string>> synonyms_;
};

void TestAddingSynonymsIncreasesTheirCount() {
    Synonyms synonyms;
    ASSERT_EQUAL(synonyms.GetSynonymCount("music"s), 0);
    ASSERT_EQUAL(synonyms.GetSynonymCount("melody"s), 0);

    synonyms.Add("music"s, "melody"s);
    ASSERT_EQUAL(synonyms.GetSynonymCount("music"s), 1);
    ASSERT_EQUAL(synonyms.GetSynonymCount("melody"s), 1);

    synonyms.Add("music"s, "tune"s);
    ASSERT_EQUAL(synonyms.GetSynonymCount("music"s), 2);
    ASSERT_EQUAL(synonyms.GetSynonymCount("tune"s), 1);
    ASSERT_EQUAL(synonyms.GetSynonymCount("melody"s), 1);
    cout << "Test AddingSynonymsIncreasesTheirCount passed!"s << endl;
}

void TestAreSynonyms() {
    Synonyms synonyms;
    synonyms.Add("music"s, "melody"s);
    synonyms.Add("music"s, "tune"s);
    ASSERT(synonyms.AreSynonyms("music"s, "tune"s) == true);
    ASSERT(synonyms.AreSynonyms("music"s, "melody"s) == true);

    ASSERT(synonyms.AreSynonyms("melody"s, "music"s) == true);
    ASSERT(synonyms.AreSynonyms("tune"s, "music"s) == true);

    ASSERT(synonyms.AreSynonyms("mass"s, "music"s) == false);
    ASSERT(synonyms.AreSynonyms("dance"s, "music"s) == false);
    cout << "Test AreSynonyms passed!"s << endl;
}

void TestSynonyms() {
    TestAddingSynonymsIncreasesTheirCount();
    TestAreSynonyms();
    cout << "Test Synonyms passed!"s << endl;
}

int main() {
    TestSynonyms();

    Synonyms synonyms;

    string line;
    while (getline(cin, line)) {
        istringstream command(line);
        string action;
        command >> action;

        if (action == "ADD"s) {
            string first_word, second_word;
            command >> first_word >> second_word;
            synonyms.Add(first_word, second_word);
        } else if (action == "COUNT"s) {
            string word;
            command >> word;
            cout << synonyms.GetSynonymCount(word) << endl;
        } else if (action == "CHECK"s) {
            string first_word, second_word;
            command >> first_word >> second_word;
            if (synonyms.AreSynonyms(first_word, second_word)) {
                cout << "YES"s << endl;
            } else {
                cout << "NO"s << endl;
            }
        } else if (action == "EXIT"s) {
            break;
        }
    }
}