//dataset/data.h
#include "dataset/data.h"
#include <limits>

int main(){

    auto vec = generate_struct_records(177016); //Mi limite
    for(auto& v: vec){
        v.showData_line();
    }

    return 0;

}