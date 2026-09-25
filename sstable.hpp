#ifndef SSTABLE_HPP
#define SSTABLE_HPP

#include "bloom_filter.hpp"
#include <fstream>
#include <iostream>
#include <optional>
#include <map> 
#include <string>
#include <vector>
#include <optional>

struct IndexEntry {
    std::string key;
    std::streamoff offset;
};

class SSTable{
private:
    std::string filename_;
    size_t block_size_;
    std::vector<IndexEntry> sparse_index_;
    BloomFilter bloom_filter_;

    // Builds a sparse index holding only every K-th key off
    void build_sparse_index_and_filter(){
        sparse_index_.clear();
        std::ifstream in(filename_);
        if(!in.is_open()) return;

        size_t count = 0;
        if(!(in >> count)) return;

        std::string dummy;
        std::getline(in, dummy);

        std::string key, value;
        for(size_t i=0; i<count; ++i){
            std::streamoff current_offset = in.tellg();
            if(current_offset == -1) break;

            if(!std::getline(in, key) || !std::getline(in, value)){
                break;;
            }
            // 1. Add EVERY key to the filter
            bloom_filter_.add(key);

            // 2. Add key to Sparse Index only everyK-th item
            if(i % block_size_ == 0){
                sparse_index_.push_back({key, current_offset});
            }
        }

    }
   
public:
    explicit SSTable(const std::string& filename, size_t block_size = 4, size_t bloom_bits = 64) 
            : filename_(filename), block_size_(block_size),
                bloom_filter_(bloom_bits, 3){
                build_sparse_index_and_filter();
    }

    std::string filename () const{ return filename_;
    }
    size_t index_size() const { return sparse_index_.size(); }


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
        
        //Step 0: Check Bloom filter First!
        if(!bloom_filter_.contains(search_key)){
            std::cout << "  [BloomFilter] Key '" << search_key << "' rejected by " 
                      << filename_ << " (Disk read skipped!)\n";
            return std::nullopt; // Skip disk read entirely!
        }
        std::cout << "  [BloomFilter] Key '" << search_key << "' MAY exist in " 
                  << filename_ << ". Proceeding to disk search...\n";

        if(sparse_index_.empty()) return std::nullopt;

        // Step 1: Binary Search in-memory Sparse Index
        int low = 0;
        int high = static_cast<int>(sparse_index_.size()) - 1;
        int target_block_idx = -1;

        while(low<=high){
            int mid = low + (high - low) /2;

            if(sparse_index_[mid].key <= search_key){
                target_block_idx = mid; //candidate block start
                low = mid+1;
            } else{
                high = mid -1;
            }
        }
        // If search key is smaller than the smallest indexed key
        if(target_block_idx == -1) return std::nullopt;

        // Step 2: Jump to the block offset on the disk
        std::ifstream inFile(filename_);
        if(!inFile.is_open()){
            return std::nullopt;
        }
        inFile.seekg(sparse_index_[target_block_idx].offset);

        //Step 3: Scanat most block_size_ items for this offset
        std::string key, value;
        for (size_t i = 0; i < block_size_; ++i) {
            if (!std::getline(inFile, key) || !std::getline(inFile, value)) {
                break;
            }

            if (key == search_key) {
                return value; // Found match!
            }

            // Because keys are sorted, if we pass search_key, it doesn't exist
            if (key > search_key) {
                break;
            }
        }
        return std::nullopt; //Key not found in block
    }

};


#endif
