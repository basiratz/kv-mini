#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <string>
#include <vector>
#include <functional>

class BloomFilter {
private:
    std::vector<bool> bit_array_;
    size_t num_hashes_;

    //helper: generate k different hash values for a key using seed variation
    std::vector<size_t> get_hashes(const std::string& key) const {
        std::vector<size_t> hashes;
        hashes.reserve(num_hashes_);

        std::hash<std::string> hasher;
        for(size_t i=0; i<num_hashes_; ++i){
            //salt the key  with index "i" to produce distinct hash values
            std::string salted_key = key + "_" + std::to_string(i);
            size_t hash_val = hasher(salted_key) % bit_array_.size();
            hashes.push_back(hash_val);
        }
        return hashes;
    }
public:
    //Default: 64 bits with 3 hash functions
    explicit BloomFilter(size_t size = 64, size_t num_hashes = 3)
            : bit_array_(size, false), num_hashes_(num_hashes) {}
    
    //add key to filter: flip bit position to 1
    void add(const std::string& key){
        for(size_t bit_index: get_hashes(key)){
            bit_array_[bit_index] = true;
        }
    }

    // Check if key may exist in filter
    //return false = Definetly NOT in filter (100%)
    // Returns true = Probably in filter
    bool contains(const std::string& key) const {
        for(size_t bit_index: get_hashes(key)){
            if(!bit_array_[bit_index]){
                return false;
            }
        }
        return true;
    }
    size_t size() const { return bit_array_.size(); }

};


#endif