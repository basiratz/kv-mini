#ifndef WAL_HPP
#define WAL_HPP

#include "file.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// create a struct to represent a single operation read fromt he log file

struct LogEntry
{
    std::string type; // like "put" or "delete"
    std::string key;
    std::string value;
};

class WAL {
private:
    File log_file_;
    std::string path_;

public:
    // open our log file in write only, create, and append modes
    explicit WAL(const std::string& path)
            : log_file_(path, O_WRONLY | O_CREAT | O_APPEND, 0644), path_(path){}
    
    // append a 'put' operation: PUT <key> <value>\n
    bool log_put(const std::string& key, const std::string& value){
        std::string record = "PUT "+key+" "+value+"\n";
        return log_file_.append(record);
    }

    //append a delete operation
    bool log_delete(const std::string& key){
        std::string record = "DELETE "+ key + "\n";
        return log_file_.append(record);
    }

    //replay the WAL from the disk to rebuild in-memory state after crash/restart
    std::vector<LogEntry> recover(){
        std::vector<LogEntry> entries;
        std::ifstream inFile(path_);
        if(!inFile.is_open()){
            return entries;
        }

        std::string line;
        while(std::getline(inFile, line)){
            if(line.empty()) continue;

            std::stringstream ss(line);
            std::string type, key, value;
            ss >> type >> key;
            if(type == "PUT"){
                ss >> value;
                entries.push_back({"PUT", key, value});
            } else if(type == "DELETE"){
                entries.push_back({"DELETE", key});
            }
        }
        return entries;
    }
};


#endif