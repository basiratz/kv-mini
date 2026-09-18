#ifndef FILE_HPP
#define FILE_HPP

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <utility>


class File{
private:
    int fd_ = -1;
public:
    // acquire rsource in constructor
    File(const std::string& path, int flags, mode_t mode = 0644){
        fd_ = ::open(path.c_str(), flags, mode);
        if(fd_ < 0){
            throw std::runtime_error("Failed to open file: "+path);
        }
    }

    //Release resource in destructor (RAII)
    ~File(){
        if(fd_ >=0){
            ::close(fd_);
        }
    }

    // disabling copying
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // Enable moving
    File(File&& other) noexcept : fd_(other.fd_){
        other.fd_ = -1;
    }
    
    File& operator=(File&& other){
        if(this != &other){
            if(fd_ >=0 ) ::close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    bool append(const std::string& data){
        ssize_t bytes_written = ::write(fd_, data.data(), data.size());
        return bytes_written == static_cast<ssize_t>(data.size());
    }
    
    int fd() const {
        return fd_;
    }

};


#endif
