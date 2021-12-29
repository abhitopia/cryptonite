//
// Created by Abhishek Aggarwal on 21/12/2021.
//

#ifndef CRYPTONITE_DATABASE_H
#define CRYPTONITE_DATABASE_H

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "../include/json.h"
#include "../src/config.h"

using json = nlohmann::json;

//using namespace std;
namespace fs = std::filesystem;

class Database {
    fs::path dbPath{};
public:
    Database(std::string path){
        dbPath = fs::path(path);
    }
    void writeDB(json jsonDB);
    json readDB();
};

#endif //CRYPTONITE_DATABASE_H
