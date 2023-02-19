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

struct Document {
    int id;
    int relevance;
};

class SearchServer {
    // PRIVATE //
    private:
        map<string, set<int>> word_in_documents_;
        set<string> stop_words_;

        vector<string> SplitIntoWords(const string& text) const {
            vector<string> words;
            string word;
            for (const char c : text) {
                if (c == ' ') {
                    if (!word.empty()) {
                        words.push_back(word);
                        word.clear();
                    }
                } 
                else {
                    word += c;
                }
            }

            if (!word.empty()) {
                words.push_back(word);
            }

        return words;
        }

        vector<string> SplitIntoWordsNoStop(const string& text) const {
            vector<string> words;
            for (const string& word : SplitIntoWords(text)) {
                if (stop_words_.count(word) == 0) {
                    words.push_back(word);
                }
            }
            return words;
        }

        vector<Document> FindAllDocuments(const string& query) const {
            const set<string> query_words = ParseQuery(query);

            map<int, int> documents_relevance;
            for (const string& word : query_words) {
                if (word_in_documents_.count(word) != 0) {
                    for (const int& document_id: word_in_documents_.at(word)) {
                        ++documents_relevance[document_id];
                    }
                }
            }

            vector<Document> matched_documents;
                for (auto& [document_id, relevance]: documents_relevance) {
                    matched_documents.push_back({document_id, relevance});
            }

            return matched_documents;
        }

        set<string> ParseQuery(const string& text) const {
            set<string> query_words;
            for (const string& word : SplitIntoWordsNoStop(text)) {
                query_words.insert(word);
            }
            return query_words;
        }

    // PUBLIC //
    public: 
        void SetStopWords(const string& text) {
            for (const string& word : SplitIntoWords(text)) {
                stop_words_.insert(word);
            }
        }

        void AddDocument(int document_id, const string& document) {
            for (const string& word : SplitIntoWordsNoStop(document)) {
                word_in_documents_[word].insert(document_id);
                }
        }

        vector<Document> FindTopDocuments(const string& query) const {
            vector<Document> matched_documents = FindAllDocuments(query);

            sort(execution::par, 
                 matched_documents.begin(),
                 matched_documents.end(),
                 [](const Document& lhs, const Document& rhs) {
				    return lhs.relevance > rhs.relevance;
			});

            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }

            return matched_documents;
        }
        
};

SearchServer CreateSearchServer() {
    SearchServer search_server;

    const string stop_words_joined = ReadLine();
    search_server.SetStopWords(stop_words_joined);

    // Reads docs
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
             << endl;
    }

    return 0;
}