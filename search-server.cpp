#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <execution>
#include <cmath>
#include <numeric>
#include <functional>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

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
    double relevance;
    int rating;
};

enum class DocumentStatus {
            ACTUAL,
            IRRELEVANT,
            BANNED,
            REMOVED,
        };

struct Query {
    set<string> plus_words;
    set<string> minus_words;
};

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& container) {
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

class SearchServer {
    public: 
        void SetStopWords(const string& text) {
                for (const string& word : SplitIntoWords(text)) {
                    stop_words_.insert(word);
                }
        }

        void AddDocument(int document_id, const string& document, 
                        const DocumentStatus& document_status, 
                        const vector<int>& doc_ratings) {
            document_data_[document_id] = {ComputeAverageRating(doc_ratings), document_status};
            vector<string> document_words = SplitIntoWordsNoStop(document);
            const double inv_words_count = 1.0 / document_words.size();
            for (const string& word : document_words) {
                word_in_document_freqs_[word][document_id] += inv_words_count;
                }
            ++document_count_;
        }

        vector<Document> FindTopDocuments(const string& query) const {
            
            const auto& filter = [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; };
            vector<Document> matched_documents = FindAllDocuments(query, filter);

            sort(execution::par, 
                 matched_documents.begin(),
                 matched_documents.end(),
                 [](const Document& lhs, const Document& rhs) {
				    return (abs(lhs.relevance - rhs.relevance) < EPSILON) && lhs.rating > rhs.rating
                    || lhs.relevance > rhs.relevance;
			});

            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }

            return matched_documents;
        }

        vector<Document> FindTopDocuments(const string& query, 
                                        function<bool(int, DocumentStatus, int)> filter) const {
                                            
            vector<Document> matched_documents = FindAllDocuments(query, filter);

            sort(execution::par, 
                 matched_documents.begin(),
                 matched_documents.end(),
                 [](const Document& lhs, const Document& rhs) {
				    return (abs(lhs.relevance - rhs.relevance) < EPSILON) && lhs.rating > rhs.rating
                    || lhs.relevance > rhs.relevance;
			});

            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }

            return matched_documents;
        }

    // PRIVATE //
    private:
        struct DocumentData {
            int rating;
            DocumentStatus status;
        };

        map<string, map<int, double>> word_in_document_freqs_;
        set<string> stop_words_;
        map<int, DocumentData> document_data_;
        int document_count_ = 0;
        
        static int ComputeAverageRating(const vector<int>& ratings) {
            if (ratings.empty()) {
                return 0;
            }
            return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
        }

        vector<int> SplitIntoNumbers(const string& text) const {
            vector<string> words = SplitIntoWords(text);
            vector<int> rating;
            for (int i=1;i<=stoi(words[0]);++i) {
                rating.push_back(stoi(words[i]));
            }
            return rating;
        }

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

        vector<Document> FindAllDocuments(const string& query, const function<bool(int, DocumentStatus, int)> filter) const {
            const Query query_words = ParseQuery(query);

            map<int, double> documents_relevance;
            for (const string& word : query_words.plus_words) {
                if (word_in_document_freqs_.count(word) != 0) {
                    const double inverse_document_freq = log(document_count_ / static_cast<double>(word_in_document_freqs_.at(word).size()));
                    for (const auto& [document_id, term_freq]: word_in_document_freqs_.at(word)) {
                        documents_relevance[document_id] += term_freq * inverse_document_freq;
                    }
                }
            }

            for (const string& word : query_words.minus_words) {
                if (word_in_document_freqs_.count(word) != 0) {
                    for (const auto& [document_id, _]: word_in_document_freqs_.at(word)) {
                        documents_relevance.erase(document_id);
                    }
                }
            }

            vector<Document> matched_documents;
                for (auto& [document_id, relevance]: documents_relevance) {
                    DocumentStatus status = document_data_.at(document_id).status;
                    int rating = document_data_.at(document_id).rating;
                    if (filter(document_id, status, rating)) {
                        matched_documents.push_back({document_id, relevance, rating});
                    } 
            }

            return matched_documents;
        }

        Query ParseQuery(const string& text) const {
            Query query;
            for (const string& word : SplitIntoWordsNoStop(text)) {
                if (word.find("-"s) != -1) {
                    query.minus_words.insert(word.substr(1));
                }
                else {
                    query.plus_words.insert(word);
                }
            }

            return query;
        }       
};

SearchServer CreateSearchServer() {
    SearchServer search_server;

    // const string stop_words_joined = ReadLine();
    // search_server.SetStopWords(stop_words_joined);

    // // Reads docs
    // const int document_count = ReadLineWithNumber();
    // for (int document_id = 0; document_id < document_count; ++document_id) {
    //     search_server.AddDocument(document_id, ReadLine());
    // }

    return search_server;
}

void PrintDocument(const Document& document) {
    cout << "{ "s
    << "document_id = "s << document.id << ", "s
    << "relevance = "s << document.relevance << ", "s
    << "rating = "s << document.rating
    << " }"s << endl;
}

int main() {
    // const SearchServer search_server = CreateSearchServer();

    // const string query = ReadLine();
    // for (auto& document : search_server.FindTopDocuments(query)) {
    //     PrintDocument(document);
    // }

    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; })) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }

    return 0;
}