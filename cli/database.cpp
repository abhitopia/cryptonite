//
// Created by Abhishek Aggarwal on 23/12/2021.
//

#include "database.h"


StrategyGenConfig fromJson(json content) {
    return StrategyGenConfig{};
}

void Database::writeDB(json jsonDB) {
    std::ofstream o;
    o.open(absolute(dbPath).c_str());
    o << std::setw(4) << jsonDB << std::endl;
    if(o.bad())    //bad() function will check for badbit
    {
        o.close();
        throw std::system_error(errno, std::system_category(), "Failed to write: " + dbPath.string());
    }

}

json Database::readDB() {
    if(not fs::exists(dbPath)){
        return json::object({});
    }
    std::ifstream file; // Read
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(dbPath.c_str());
    // after open, check f and throw std::system_error with the errno
    if (!file){
        file.close();
        throw std::system_error(errno, std::system_category(), "Failed to read: " + dbPath.string());
    }
    json j;
    file >> j;
    file.close();
    return j;
}