#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <execution>
#include <cmath>
#include <numeric>

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
    double relevance;
    int rating;
};

struct Query {
    set<string> plus_words;
    set<string> minus_words;
};

class SearchServer {
    // PRIVATE //
    private:
        map<string, map<int, double>> word_in_document_freqs_;
        set<string> stop_words_;
        map<int, int> doc_rating_;
        int document_count_ = 0;
        
        static int ComputeAverageRating(const vector<int>& ratings) {
            return accumulate(ratings.begin(), ratings.end(), 0) / ratings.size();
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

        vector<Document> FindAllDocuments(const string& query) const {
            const Query query_words = ParseQuery(query);
            
            map<int, double> documents_relevance;
            for (const string& word : query_words.plus_words) {
                if (word_in_document_freqs_.count(word) != 0) {
                    const double inverse_document_freq = log(document_count_ / (word_in_document_freqs_.at(word).size() * 1.0));
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
                    matched_documents.push_back({document_id, relevance, doc_rating_.at(document_id)});
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

    // PUBLIC //
    public: 
        void SetStopWords(const string& text) {
            for (const string& word : SplitIntoWords(text)) {
                stop_words_.insert(word);
            }
        }

        void AddDocument(int document_id, const string& document) {
            vector<int> doc_ratings = SplitIntoNumbers(ReadLine());
            doc_rating_[document_id] = ComputeAverageRating(doc_ratings);
            vector<string> document_words = SplitIntoWordsNoStop(document);
            const double inv_words_count = 1.0 / document_words.size();
            for (const string& word : document_words) {
                word_in_document_freqs_[word][document_id] += inv_words_count;
                }
            ++document_count_;
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
    for (auto [document_id, relevance, rating] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << 
                ", relevance = "s << relevance << 
                ", rating = "s << rating << " }"s
             << endl;
    }

    return 0;
}