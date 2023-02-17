#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <execution>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

set<string> ParseStopWords(const string& text) {
    set<string> stop_words;
    for (const string& word : SplitIntoWords(text)) {
        stop_words.insert(word);
    }
    return stop_words;
}

vector<string> SplitIntoWordsNoStop(const string& text, const set<string>& stop_words) {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (stop_words.count(word) == 0) {
            words.push_back(word);
        }
    }
    return words;
}

void AddDocument(map<string, set<int>>& word_in_documents,
                 const set<string>& stop_words,
                 int document_id,
                 const string& document) {
    for (const string& word : SplitIntoWordsNoStop(document, stop_words)) {
        word_in_documents[word].insert(document_id);
    }
}

set<string> ParseQuery(const string& text, const set<string>& stop_words) {
    set<string> query_words;
    for (const string& word : SplitIntoWordsNoStop(text, stop_words)) {
        query_words.insert(word);
    }
    return query_words;
}

vector<pair<int, int>> FindDocuments(const map<string, set<int>>& word_in_documents,
                                     const set<string>& stop_words,
                                     const string& query,
                                     const bool& get_all_results) {
    const set<string> query_words = ParseQuery(query, stop_words);

    map<int, int> documents_relevance;
    for (const string& word : query_words) {
        if (word_in_documents.count(word) != 0) {
            for (const int& document_id: word_in_documents.at(word)) {
                ++documents_relevance[document_id];
            }
        }
    }

    vector<pair<int, int>> documents_relevance_reversed;
    for (auto& [document_id, relevance]: documents_relevance) {
        documents_relevance_reversed.push_back({relevance, document_id});
    }

    sort(execution::par, documents_relevance_reversed.begin(), documents_relevance_reversed.end());
    // sort(documents_relevance_reversed.begin(), documents_relevance_reversed.end());
    reverse(documents_relevance_reversed.begin(), documents_relevance_reversed.end());

    if (!get_all_results && documents_relevance_reversed.size() > MAX_RESULT_DOCUMENT_COUNT) {
            documents_relevance_reversed.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

    vector<pair<int, int>> matched_documents;
    for (auto& [relevance, document_id]: documents_relevance_reversed) {
        matched_documents.push_back({document_id, relevance});
    }
    return matched_documents;
}

int main() {
    const string stop_words_joined = ReadLine();
    const set<string> stop_words = ParseStopWords(stop_words_joined);

    // Reads docs
    map<string, set<int>> documents;
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        AddDocument(documents, stop_words, document_id, ReadLine());
    }

    // const string query = ReadLine();
    // Print all results on query
    // for (auto [document_id, relevance] : FindDocuments(documents, stop_words, query, true)) {
    //     cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
    //          << endl;
    // }

    const string query = ReadLine();
    // Prints the specified number of results
    for (auto [document_id, relevance] : FindDocuments(documents, stop_words, query, false)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
             << endl;
    }
}