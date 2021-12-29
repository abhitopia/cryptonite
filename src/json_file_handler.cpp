//
// Created by Abhishek Aggarwal on 28/12/2021.
//

#include <iostream>
#include "json_file_handler.h"

void JsonFileHandler::write(std::string name, json& j) {
    auto file = fs::path(name);

    if(!fs::exists(file.parent_path())){
        fs::create_directory(file.parent_path());
    }
    auto tmpFile = file.parent_path() / ("." + file.stem().string() + ".tmp");
    auto bakFile = file.parent_path() / ("." + file.stem().string() + ".bak");

    // if original file exists, then create a backup
    if(exists(file)){
        fs::rename(file, bakFile);
    }

    // write to a temporary file
    std::ofstream o;
    o.open(absolute(tmpFile).c_str());
    o << std::setw(4) << j << std::endl;

    if(o.bad() | o.fail())    //bad() function will check for badbit
    {
        o.close();
        if(fs::exists(bakFile)){
            fs::rename(bakFile, file); // restore original file
        }
        throw std::system_error(errno, std::system_category(), "Failed to write: " + file.string());
    }

    o.close();

    // move temporary file to be the file to be written
    fs::rename(tmpFile, file);
    // when all is done, remove the backup file
    if(fs::exists(bakFile)){
        fs::remove(bakFile);
    }
}

json JsonFileHandler::read(std::string name, bool emptyIfNotExists) {
    auto file = fs::path(name);

    if(!fs::exists(file) and emptyIfNotExists){
        return json::object({});
    }
    std::ifstream i; // Read
    i.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    i.open(file.c_str());
    // after open, check f and throw std::system_error with the errno
    if (!i){
        i.close();
        throw std::system_error(errno, std::system_category(), "Failed to read: " + file.string());
    }
    json j;
    i >> j;
    i.close();
    return j;

}
