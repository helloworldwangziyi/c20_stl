#include <iostream>
#include <type_traits>
#include <c20_stl/type_traits.hpp>


namespace c20_stl{


void test_traits(){
    // 嵌套类型要用 :: 访问(此处非模板上下文,不需要 typename)
    static_assert(std::is_same_v<c20_stl::true_type::type, c20_stl::true_type>);
    static_assert(c20_stl::true_type{});
    static_assert(!c20_stl::false_type{});

    // test is_same_v 
    static_assert(c20_stl::is_same_v<c20_stl::true_type::type, c20_stl::true_type>);
    static_assert(c20_stl::is_same_v<int, int>);
    static_assert(!c20_stl::is_same_v<int, bool>);
    static_assert(c20_stl::is_same_v<int,int> == std::is_same_v<int, int>);
}

void test_more_traits(){
    using namespace c20_stl;
    if(true_type{}.value == true){
        std::cout << "test is true" << std::endl;
    }else {
        std::cout << "test is false" << std::endl;
    }
    integral_constant<int, 5>::value_type c = 100;
    std::cout << c << std::endl;

    true_type::type ic{};
    std::cout << ic.value << std::endl;
    // true_type{}::type<true> ic;

    is_same<int, bool> ip{};
    std::cout << ip.value << std::endl;

    is_same<int, int> id {};
    std::cout << id.value << std::endl;

    is_same<int, int>::type test_id{};
    std::cout << test_id.value << std::endl;
}


}

int main(){
    using namespace c20_stl;
    test_traits();
    test_more_traits();
}
