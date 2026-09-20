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
    //In-memory list of byt offsets where each key begins in the sstable file
    std::vector<std::streamoff> offsets_;

    // Helper to scan the file once and populate the byte offset array

    void build_offset_index(){
        offsets_.clear();
        std::ifstream in(filename_);
        if(!in.is_open()) return;

        size_t count = 0;
        if(!(in >> count)) return;

        std::string dummy;
        std::getline(in, dummy);

        std::string key, value;
        for(size_t i=0; i<count; ++i){
            //Save exact byte position where "key" starts
            std::streamoff current_offset_ = in.tellg();
            if(current_offset_ = -1) break;

            if(!std::getline(in, key) || !std::getline(in, value)){
                break;
            }
            offsets_.push_back(current_offset_);
        }
    }

public:
    explicit SSTable(const std::string& filename) : filename_(filename){}

    std::string filename () const{
        return filename_;
    }
    size_t size() const { return offsets_.size(); }


    // static helper to write a sorted map to a new file (SSTable) on disk
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

// Binary Search on disk file using byte offsets: O(log n)
    std::optional<std::string> get(const std::string& search_key) const {

        if(offsets_.empty()) return std::nullopt;

        std::ifstream inFile(filename_);
        if(!inFile.is_open()){
            return std::nullopt;
        }
        int low = 0;
        int high = static_cast<int>(offsets_.size())-1;

        while (low <= high)
        {
            int mid = low + (high - low) / 2;
            
            // seek directly to the byte offset of the middle entry
            inFile.seekg(offsets_[mid]);

            std::string key, value;
            if(!std::getline(inFile, key) || !std::getline(inFile, value)){
                break;
            }
            if(key == search_key){
                return value;
            } else if( key <search_key){
                low = mid + 1;
            } else{
                high = mid - 1;
            }
        }

        return std::nullopt;
    }

};


#endif
