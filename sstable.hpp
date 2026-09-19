#ifndef SSTABLE_HPP
#define SSTABLE_HPP

#include <fstream>
#include <iostream>
#include <optional>
#include <map> 
#include <string>
#include <vector>

class SSTable{
private:
    std::string filename_;

public:
    explicit SSTable(const std::string& filename) : filename_(filename){}

    std::string filename () const{
        return filename_;
    }

    // static helper fun() to write a sorted map to a new file (SSTable) on disk
    static bool write_from_memtable(std::string& filename, const std::map<std::string, std::string>& data){
        std::ofstream outFile(filename);
        if(!outFile.is_open()){
            return false;
        }

        //write entry count first
        outFile << data.size()<<"\n";

        //write keys and values sequentially
        for(const auto& [key, value] : data){
            outFile << key <<"\n" << value <<"\n";
        }
        return outFile.good();
    }

    //Linear search inside the SSTable file for a key
    std::optional<std::string> get(const std::string& search_key) const {
        std::ifstream inFile(filename_);
        if(!inFile.is_open()){
            return std::nullopt;
        }
        size_t count = 0;
        if(!(inFile >> count)) return std::nullopt;

        std::string dummy; 
        std::getline(inFile, dummy);

        std::string key, value;
        for(size_t i=0; i<count; ++i){
            if(!std::getline(inFile, key) || !std::getline(inFile, value)){
                break;
            }
            if(key == search_key){
                return value; //found the match
            }

        }
        return std::nullopt; //not found in this file
    }

};




#endif
