#include <iostream>
#include <c20_stl/type_traits.hpp>

namespace c20_stl{
    
}

int main(){
    std::cout << c20_stl::integral_constant<int, 42>::value << '\n';
    std::cout << "heollo world" << std::endl;
    return 0;
}