#include <cassert>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <set>

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

const map<string, QueryType> string_to_query_type_ = {{"ALL_BUSES", QueryType::AllBuses},
                                                {"NEW_BUS", QueryType::NewBus},
                                                {"BUSES_FOR_STOP", QueryType::BusesForStop},
                                                {"STOPS_FOR_BUS", QueryType::StopsForBus}}
                                                ;

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& container) {
    int len = container.size();
    for (const T& el: container) {
        if (el != container.at(len - 1)) {
            if (el.find("\n") != -1) {
                cout << el;
            }
            else {
                cout << el << " "s;
            }
        }
        else {
            cout << el;
        }
    }
    return out;
}

istream& operator>>(istream& is, Query& q) {
    string type;
    is >> type;
    q.type = string_to_query_type_.at(type);

    switch (q.type) {
    case QueryType::NewBus:
        int stop_count;
        is >> q.bus >> stop_count;
        
        for (int i=0;i<stop_count;++i) {
            string stop;
            is >> stop;
            q.stops.push_back(stop);
        }
        break;

    case QueryType::BusesForStop:
        is >> q.stop;
        break;

    case QueryType::StopsForBus:
        is >> q.bus;
        break;
    }
    return is;
}

struct BusesForStopResponse {
    vector<string> response_message;
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    os << r.response_message;
    return os;
}

struct StopsForBusResponse {
    vector<string> response_message;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    os << r.response_message;
    return os;
}

struct AllBusesResponse {
    vector<string> response_message;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    os << r.response_message;
    return os;
}

class BusManager {
public:
    void AddBus(const string& bus, const vector<string>& stops) {
        buses_to_stops_[bus] = stops;
        for (const string& stop: stops) {
            stops_to_buses_[stop].insert(bus);
        }
    }

    BusesForStopResponse GetBusesForStop(const string& stop) const {
        BusesForStopResponse response;
        vector<string> message;
        if (stops_to_buses_.count(stop) != 0) {
            int len = stops_to_buses_.at(stop).size();
            for (const string& bus: stops_to_buses_.at(stop)) {
                bus != *prev(stops_to_buses_.at(stop).end())
                ? message.push_back(bus)
                : message.push_back(bus + "\n"s);
            }
            response.response_message = message;
            return response;
        }
        else {
            response.response_message.push_back("No stop\n"s);
            return response;
        }
    }

    StopsForBusResponse GetStopsForBus(const string& bus) const {
        StopsForBusResponse response;
        vector<string> message;
        if (buses_to_stops_.count(bus) != 0) {
            for (const string& stop: buses_to_stops_.at(bus)){
                message.push_back("Stop"s);
                message.push_back(stop + ":"s);

                if (stops_to_buses_.at(stop).size() == 1) {
                    message.push_back("no interchange\n"s);
                }
                else {
                    int len = stops_to_buses_.at(stop).size();
                    for (const string& other_bus: stops_to_buses_.at(stop)) {
                        if (bus != other_bus) {
                            other_bus != *prev(stops_to_buses_.at(stop).end())
                            ? message.push_back(other_bus)
                            : message.push_back(other_bus + "\n"s);
                        }
                        else {
                            if (other_bus == *prev(stops_to_buses_.at(stop).end())) {
                                message.at(message.size() - 1) += "\n";
                            }
                        }
                    }
                }
            }
            response.response_message = message;
            return response;
        }
        else {
            message.push_back("No bus\n");
            response.response_message = message;
            return response;
        }
    }

    AllBusesResponse GetAllBuses() const {
        AllBusesResponse response;

        vector<string> message;
        if (buses_to_stops_.size() != 0) {
            for (auto& [bus, stops]: buses_to_stops_) {
                message.push_back("Bus");
                message.push_back(bus + ":");
                for (string stop: stops) {
                    stop != stops.at(stops.size() - 1)
                    ? message.push_back(stop)
                    : message.push_back(stop + "\n"s);
                }
            }
        }
        else {
            message.push_back("No buses\n"s);
        }

        response.response_message = message;
        return response;
    }

private:
    map<string, vector<string>> buses_to_stops_;
    map<string, set<string>> stops_to_buses_;
};

string ToString(const vector<string>& vector_string) {
    size_t len = vector_string.size();
    string message;
    for (const string& word: vector_string) {
        word != vector_string.at(len - 1) && word.find("\n"s) == -1
        ? message += (word + " "s)
        : message += (word);
    }
    return message;
}


void TestGetAllBuses() {
    BusManager bm;

    // Empty check
    assert(ToString(bm.GetAllBuses().response_message) == ("No buses\n"s));
    
    // Add and get all buses
    vector<string> lines = {"NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s};

    for (string& line: lines) {
        istringstream stream(line);

        // Query q;
        string q;
        stream >> q;

        string bus_name;
        stream >> bus_name;

        int stops_count;
        stream >> stops_count;

        vector<string> stop_names;
        for (int i=0;i<stops_count;++i) {
            string stop_name;
            stream >> stop_name;
            stop_names.push_back(stop_name);
        }

        bm.AddBus(bus_name, stop_names);
    }

    cout << ToString(bm.GetAllBuses().response_message) << endl;
    assert(ToString(bm.GetAllBuses().response_message) == "Bus 32: Tolstopaltsevo Marushkino Vnukovo\n"s);

    cout << "TestGetAllBuses is OK"s << endl;
}

void TestGetStopsForBus() {
     BusManager bm;

    // Empty check
    assert(ToString(bm.GetStopsForBus("32"s).response_message) == ("No bus\n"s));
    
    // Add and get all buses
    vector<string> lines = {"NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s};

    for (string& line: lines) {
        istringstream stream(line);

        // Query q;
        string q;
        stream >> q;

        string bus_name;
        stream >> bus_name;

        int stops_count;
        stream >> stops_count;

        vector<string> stop_names;
        for (int i=0;i<stops_count;++i) {
            string stop_name;
            stream >> stop_name;
            stop_names.push_back(stop_name);
        }

        bm.AddBus(bus_name, stop_names);
    }

    assert(ToString(bm.GetStopsForBus("32"s).response_message) == ("Stop Tolstopaltsevo: no interchange\n" \
                                                                   "Stop Marushkino: no interchange\n" \   
                                                                   "Stop Vnukovo: no interchange\n"));

    vector<string> stops;
    stops = {"Tolstopaltsevo", "Marushkino", "Vnukovo", "Peredelkino", "Solntsevo", "Skolkovo"};
    bm.AddBus("32K", stops);
    stops.clear();

    stops = {"Kokoshkino", "Marushkino", "Vnukovo", "Peredelkino", "Solntsevo", "Troparyovo"};
    bm.AddBus("950", stops);
    stops.clear();

    stops = {"Vnukovo", "Moskovsky", "Rumyantsevo", "Troparyovo"};
    bm.AddBus("272", stops);

    assert(ToString(bm.GetStopsForBus("272"s).response_message) == ("Stop Vnukovo: 32 32K 950\n" \
                                                                   "Stop Moskovsky: no interchange\n"
                                                                   "Stop Rumyantsevo: no interchange\n"
                                                                   "Stop Troparyovo: 950\n"));

    cout << "TestGetStopsForBus is OK"s << endl;
}

void TestGetBusesForStop() {
    BusManager bm;

    assert(ToString(bm.GetBusesForStop("Vnukovo"s).response_message) == "No stop\n"s);

    vector<string> lines = {"NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s,
                            "NEW_BUS 32K 6 Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo"s};

    for (string& line: lines) {
        istringstream stream(line);

        // Query q;
        string q;
        stream >> q;

        string bus_name;
        stream >> bus_name;

        int stops_count;
        stream >> stops_count;

        vector<string> stop_names;
        for (int i=0;i<stops_count;++i) {
            string stop_name;
            stream >> stop_name;
            stop_names.push_back(stop_name);
        }

        bm.AddBus(bus_name, stop_names);
    }

    assert(ToString(bm.GetBusesForStop("Vnukovo"s).response_message) == "32 32K\n"s);
    
    cout << "TestGetBusesForStop is OK"s << endl;
}

void TestBusManager() {
    TestGetAllBuses();
    TestGetStopsForBus();
    TestGetBusesForStop();
}

// int main() {
//     TestBusManager();
// }

int main() {
    int query_count;
    cin >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        Query q;
        cin >> q;
        switch (q.type) {
            case QueryType::NewBus:
                bm.AddBus(q.bus, q.stops);
                cout << endl;
                break;
            case QueryType::BusesForStop:
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:
                cout << bm.GetStopsForBus(q.bus) << endl;
                break;
            case QueryType::AllBuses:
                cout << bm.GetAllBuses() << endl;
                break;
        }
    }
}