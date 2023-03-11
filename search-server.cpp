#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <execution>
#include <cmath>
#include <numeric>
#include <optional>
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
    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    Document() = default;

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

template <typename StringCollection>
set<string> MakeSetStopWords(const StringCollection& collection) {
    set<string> set_words(collection.begin(), collection.end());
    return set_words;
}

class SearchServer {
public:
    inline static constexpr int INVALID_DOCUMENT_ID = -1;

    SearchServer() = default;

    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words)
        : stop_words_(MakeSetStopWords(stop_words))
    {}

    explicit SearchServer(const string& stop_words_text)
        : stop_words_(MakeSetStopWords(SplitIntoWords(stop_words_text)))
    {}

    void SetStopWords(const string& text) {
            for (const string& word : SplitIntoWords(text)) {
                stop_words_.insert(word);
            }
    }

    [[nodiscard]] bool AddDocument(int document_id, const string& document, 
                    const DocumentStatus& document_status, 
                    const vector<int>& doc_ratings) {
        if (!CheckId(document_id)){
            return false;
        }

        vector<string> document_words = SplitIntoWordsNoStop(document);

        for (string word: document_words) {
            if (!IsValidWord(word)) {
                return false;
            }
        }

        document_data_[document_id] = {ComputeAverageRating(doc_ratings), document_status};
        const double inv_words_count = 1.0 / document_words.size();
        for (const string& word : document_words) {
            word_in_document_freqs_[word][document_id] += inv_words_count;
        }
        ++document_count_;
        return true;
    }

    template <typename DocumentPredicate>
    optional<vector<Document>> FindTopDocuments(const string& query_text, DocumentPredicate filter) const {
        
        const Query query = ParseQuery(query_text);

        for (const string& word: query.minus_words) {
            if (count(word.begin(), word.end(), '-') > 0 || word.empty()) {
                return nullopt;
            }
        }

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

    optional<vector<Document>> FindTopDocuments(const string& query, DocumentStatus status) const {
        return FindTopDocuments(query, [status](int document_id, DocumentStatus doc_status, int rating) { return doc_status == status; });
    }

    optional<vector<Document>> FindTopDocuments(const string& query) const {
        return FindTopDocuments(query, DocumentStatus::ACTUAL);
    }

    function<int(vector<int>)> GetComputeAverageRatingFunc() {
        auto func = ComputeAverageRating;
        return func;
    }

    int GetDocumentId(int index) const {
        if (index >= 0 && index < document_count_) {
            return added_ids_.at(index);
        }
        return INVALID_DOCUMENT_ID; 
    }

    optional<tuple<vector<string>, DocumentStatus>> MatchDocument(const string& raw_query, int document_id, tuple<vector<string>, DocumentStatus>& result) const {
        const Query query = ParseQuery(raw_query);

        for (const string& word: query.minus_words) {
            if (count(word.begin(), word.end(), '-') > 0 || word.empty()) {
                return nullopt;
            }
        }

        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_in_document_freqs_.count(word) == 0) {
                continue;
            }

            if (word_in_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_in_document_freqs_.count(word) == 0) {
                continue;
            }

            if (word_in_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return make_tuple(matched_words, document_data_.at(document_id).status);
    }

// PRIVATE //
private:
    static bool IsValidWord(const string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';});
    }

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    map<string, map<int, double>> word_in_document_freqs_;
    set<string> stop_words_;
    map<int, DocumentData> document_data_;
    vector<int> added_ids_;
    int document_count_ = 0;

    bool CheckId(int id) {
        if (id < 0 || count(added_ids_.begin(), added_ids_.end(), id) == 1) {
            return false;
        }
        else {
            added_ids_.push_back(id);
            return true;
        }
    }
    
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
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate filter) const {

        map<int, double> documents_relevance;
        for (const string& word : query.plus_words) {
            if (word_in_document_freqs_.count(word) != 0) {
                const double inverse_document_freq = log(document_count_ / static_cast<double>(word_in_document_freqs_.at(word).size()));
                for (const auto& [document_id, term_freq]: word_in_document_freqs_.at(word)) {
                    documents_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
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
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.value().size() == 1);
        const Document& doc0 = found_docs.value()[0];
        ASSERT(doc0.id == doc_id);
    }

    // Then we make sure that the search for the same 
    // word included in the list of stop words returns an empty result
    {
        SearchServer server;
        (void) server.SetStopWords("in the"s);
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.value().empty());
    }

    // Then we make sure that the search for a minus word 
    // returns an empty result
    {
        SearchServer server;
        auto found_docs = server.FindTopDocuments("in -cat"s);
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(found_docs.value().empty());
    }
}

void TestMatchingDocuments() {
    const vector<int> doc_id = {1, 2, 0};
    // Check sort by relevance
    {
        SearchServer server;
        (void) server.SetStopWords("и в на"s);
        (void) server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
        (void) server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
        (void) server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
        auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
        ASSERT(found_docs.value().size() == 3);
        ASSERT(found_docs.value().size() == doc_id.size());

        int index = 0;
        bool test_passed = true;
        for (auto& doc: found_docs.value()) {
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
        (void) server.AddDocument(2, "белый кот и модный ошейник"s,  DocumentStatus::ACTUAL, {8, -3});
        (void) server.AddDocument(3, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, 
                                                            [](int document_id, DocumentStatus status, int rating) 
                                                            { return document_id % 2 == 0; });
        ASSERT(found_docs.value().size() == 1);
        const Document& doc0 = found_docs.value()[0];
        ASSERT(doc0.id == 2);
    }

    // Get documents with a determinate status
    {
        SearchServer server;
        (void) server.SetStopWords("и в на"s);
        (void) server.AddDocument(2, "белый кот и модный ошейник"s,  DocumentStatus::ACTUAL, {8, -3});
        (void) server.AddDocument(3, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        (void) server.AddDocument(6, "ухоженный скворец евгений кот"s,   DocumentStatus::BANNED, {9});
        auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, 
                                                            [](int document_id, DocumentStatus status, int rating) 
                                                            { return status == DocumentStatus::BANNED; });
        ASSERT(found_docs.value().size() == 1);
        const Document& doc0 = found_docs.value()[0];
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
        (void) server.SetStopWords("и в на"s);
        (void) server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
        (void) server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
        (void) server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {9});
        auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
        vector<double> res_relevance;
        for (auto& doc: found_docs.value()) {
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

// int main() {
//     RUN_TEST(TestSearchServer);

//     vector<string> stop_words = {"и"s, "в"s, "на"s};
//     // SearchServer search_server("и в на"s);
//     SearchServer search_server(stop_words);
//     Document doc{10, 3.5};

//     search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
//     search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
//     search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
//     search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

//     cout << "ACTUAL by default:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
//         PrintDocument(document);
//     }

//     cout << "BANNED:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
//         PrintDocument(document);
//     }

//     cout << "Even ids:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//         PrintDocument(document);
//     }

//     return 0;
// }

int main() {
    SearchServer search_server("и в на"s);

    (void) search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});

    if (!search_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2})) {
        cout << "Документ не был добавлен, так как его id совпадает с уже имеющимся"s << endl;
    }

    if (!search_server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2})) {
        cout << "Документ не был добавлен, так как его id отрицательный"s << endl;
    }

    if (!search_server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2})) {
        cout << "Документ не был добавлен, так как содержит спецсимволы"s << endl;
    }

    if (!search_server.AddDocument(2, "большой пёс той-терьер"s, DocumentStatus::ACTUAL, {1, 3, 2})) {
        cout << "Документ не был добавлен, так как содержит спецсимволы"s << endl;
    }

    // if (const auto documents = search_server.FindTopDocuments("--пушистый"s, documents)) {
    if (const auto documents = search_server.FindTopDocuments("кот -"s)) {
        for (const Document& document : documents.value()) {
            PrintDocument(document);
        }
    } else {
        cout << "Ошибка в поисковом запросе"s << endl;
    }
}