#ifndef SSTABLE_HPP
#define SSTABLE_HPP

#include "bloom_filter.hpp"
#include <cstdio> 
#include <fstream>
#include <iostream>
#include <optional>
#include <map> 
#include <string>
#include <vector>
#include <optional>

const std::string TOMBSTONE = "__DELETED_TOMBSTONE__";

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
    static bool write_from_memtable(const std::string& filename, const std::map<std::string, std::string>& data){
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

    // Two-Pointer Merge Compaction of Two SSTable files
    static bool compact(const SSTable& sst_old, const SSTable& sst_new, const std::string& output_filename){
        std::ifstream in_old(sst_old.filename());
        std::ifstream in_new(sst_new.filename());

        if( !in_old.is_open() || !in_new.is_open()) return false;

        size_t count_old = 0, count_new = 0;
        in_old >> count_old;
        in_new >> count_new;

        std::string dummy;
        std::getline(in_old, dummy);
        std::getline(in_new, dummy);

        std::string k_old, v_old, k_new, v_new;
        bool has_old = static_cast<bool>(std::getline(in_old, k_old) && std::getline(in_old, v_old));
        bool has_new = static_cast<bool>(std::getline(in_new, k_new) && std::getline(in_new, v_new));

        // Temporary map to collct merged entries before wrting count hader

        std::map<std::string, std::string> merged_data;

        while (has_old && has_new)
        {
            if(k_old < k_new){
                if(v_old != TOMBSTONE ){
                    merged_data[k_old] = v_old;
                }
                has_old = static_cast<bool>(std::getline(in_old, k_old) && std::getline(in_old, v_old));
            } else if(k_new < k_old){
                if(v_new != TOMBSTONE){
                    merged_data[k_new] = v_new;
                }
                has_new = static_cast<bool>(std::getline(in_new, k_new) && std::getline(in_new, v_new));
            } else{
                // key exists in both: Newer SSTable value takes priority
                if(v_new!=TOMBSTONE){
                    merged_data[k_new] = v_new;
                }
                // Advance both readers past the duplicate key
                has_old = static_cast<bool>(std::getline(in_old, k_old) && std::getline(in_old, v_old));
                has_new = static_cast<bool>(std::getline(in_new, k_new) && std::getline(in_new, v_new));

            }
        }
        // Drain remaining entries from the old sstable
        while (has_old)
        {
            if(v_old != TOMBSTONE) merged_data[k_old] = v_old;
                has_old = static_cast<bool>(std::getline(in_old, k_old) && std::getline(in_old, v_old));
        }
        // Drain remaining entries from the new sstable
        while (has_new)
        {
            if(v_new != TOMBSTONE) merged_data[k_new] = v_new;
                has_new = static_cast<bool>(std::getline(in_new, k_new) && std::getline(in_new, v_new));
        }

        // Write the merged map into the new sstable fle
        return write_from_memtable(output_filename, merged_data);
        
        
        
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
