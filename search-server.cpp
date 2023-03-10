#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <execution>
#include <cmath>
#include <numeric>
#include "print-templates.cpp"
#include "unit-tests-lib.cpp"

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

    Document()
        : id(0)
        , relevance(0)
        , rating(0)
    {}

    Document(int _id)
        : id(_id)
        , relevance(0)
        , rating(0)
    {}

    Document(int _id, double _relevance)
        : id(_id)
        , relevance(_relevance)
        , rating(0)
    {}

    Document(int _id, double _relevance, int _rating)
        : id(_id)
        , relevance(_relevance)
        , rating(_rating)
    {}
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

class SearchServer {
public: 
    SearchServer() = default;

    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words) {
        set<string> set_words(stop_words.begin(), stop_words.end());
        swap(stop_words_, set_words);
    }

    explicit SearchServer(const string& text) {
        SetStopWords(text);
    }

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

    

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& query, DocumentPredicate filter) const {
                                        
        vector<Document> matched_documents = FindAllDocuments(query, filter);

        sort(execution::par, 
                matched_documents.begin(),
                matched_documents.end(),
                [](const Document& lhs, const Document& rhs) {
                return (abs(lhs.relevance - rhs.relevance) < EPSILON) 
                        && lhs.rating > rhs.rating
                        || lhs.relevance > rhs.relevance;
        });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& query, DocumentStatus status) const {
        return FindTopDocuments(query, [status](int document_id, DocumentStatus doc_status, int rating) { return doc_status == status; });
    }

    vector<Document> FindTopDocuments(const string& query) const {
        return FindTopDocuments(query, DocumentStatus::ACTUAL);
    }

    function<int(vector<int>)> GetComputeAverageRatingFunc() {
        auto& func = ComputeAverageRating;
        return func;
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

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const string& query, DocumentPredicate filter) const {
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
                    matched_documents.push_back(Document{document_id, relevance, rating});
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

void PrintDocument(const Document& document) {
    cout << "{ "s
    << "document_id = "s << document.id << ", "s
    << "relevance = "s << document.relevance << ", "s
    << "rating = "s << document.rating
    << " }"s << endl;
}

// -------- Starting Search Engine Unit Tests ----------

// The test checks that the search engine excludes stop words when adding documents
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // First, we make sure that the search for a word that is not included 
    // in the list of stop words finds the right document.
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id);
    }

    // Then we make sure that the search for the same 
    // word included in the list of stop words returns an empty result
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }

    // Then we make sure that the search for a minus word 
    // returns an empty result
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in -cat"s).empty());
    }
}

void TestMatchingDocuments() {
    const vector<int> doc_id = {1, 2, 0};
    // Check sort by relevance
    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
        server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
        server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
        ASSERT(found_docs.size() == 3);
        ASSERT(found_docs.size() == doc_id.size());

        int index = 0;
        bool test_passed = true;
        for (auto& doc: found_docs) {
            if (doc.id != doc_id.at(index)) {
                test_passed = false;
                break;
            }
            ++index;
        }
        ASSERT(test_passed);
    }

    // Get documents with a predicate
    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        server.AddDocument(2, "белый кот и модный ошейник"s,  DocumentStatus::ACTUAL, {8, -3});
        server.AddDocument(3, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, 
                                                        [](int document_id, DocumentStatus status, int rating) 
                                                        { return document_id % 2 == 0; });
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == 2);
    }

    // Get documents with a determinate status
    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        server.AddDocument(2, "белый кот и модный ошейник"s,  DocumentStatus::ACTUAL, {8, -3});
        server.AddDocument(3, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        server.AddDocument(6, "ухоженный скворец евгений кот"s,   DocumentStatus::BANNED, {9});
        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, 
                                                        [](int document_id, DocumentStatus status, int rating) 
                                                        { return status == DocumentStatus::BANNED; });
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == 6);
    }
}

void TestCalculations() {
    
    const vector<vector<int>> ratings = {{5, 3, -1}, {2, 9, 0, 1}, {2, 4, 6}, {1, 2, 5}, {0, 0, -4}};
    const vector<int> mean_rating = {2, 3, 4, 2, -1};
    // Test mean rating calculation
    {
        SearchServer server;
        vector<int> res_means;
        auto ComputeAverageRating = server.GetComputeAverageRatingFunc();
        for (auto& doc_r:ratings) {
            res_means.push_back(ComputeAverageRating(doc_r));
        }
        ASSERT(res_means == mean_rating);
    }

    // Test correct calc of relevance
    const vector<double> relevance = {0.650672, 0.274653, 0.101366};
    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
        server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
        server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {9});
        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
        vector<double> res_relevance;
        for (auto& doc: found_docs) {
            res_relevance.push_back(doc.relevance);
        }

        ASSERT(res_relevance.size() == relevance.size());
        
        int counter = 0;
        for (size_t i=0; i<res_relevance.size();++i) {
            if (abs(res_relevance.at(i) - relevance.at(i)) < EPSILON) {
                ++counter;
            }
        }
        ASSERT(counter == 3);
    }
}

// The entry point for running tests
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMatchingDocuments);
    RUN_TEST(TestCalculations);
}

// --------- End of search engine unit tests -----------

int main() {
    RUN_TEST(TestSearchServer);

    vector<string> stop_words = {"и"s, "в"s, "на"s};
    // SearchServer search_server("и в на"s);
    SearchServer search_server(stop_words);
    Document doc{10, 3.5};

    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }

    return 0;
}