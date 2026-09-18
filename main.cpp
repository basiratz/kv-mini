#include <iostream>
#include "file.hpp"

void test_raii(){
     // open the file for writing (we will create if mising, append mode)
    File f("test_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    f.append("Log entry 1\n");
    f.append("Log entry 2\n");
    //destructor will run auto
}
int main() {
    std::cout<<"Testing RAII file wrapper....\n";
    test_raii();
    std::cout<<"File auto closed with out manual close() call... \n";
    return 0;
}