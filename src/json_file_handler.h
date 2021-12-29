//
// Created by Abhishek Aggarwal on 28/12/2021.
//

#ifndef CRYPTONITE_JSON_FILE_HANDLER_H
#define CRYPTONITE_JSON_FILE_HANDLER_H


#include <string>
#include <fstream>
#include "../include/json.h"

using json = nlohmann::json;
namespace fs = std::filesystem;


class JsonFileHandler {
public:
    static void write(std::string name, json& j);
    static json read(std::string name, bool emptyIfNotExists=false);
};


#endif //CRYPTONITE_JSON_FILE_HANDLER_H
