#include <hrlib/type_traits/type_list_util.hpp>
#include <boost/type_index.hpp>
#include <iostream>

template <typename... Args>
struct TypeList {};

using namespace hrlib::type_traits;

int main(){
    static_assert(std::is_same_v<head_t<TypeList<int, bool, double>>, int>);
    static_assert(std::is_same_v<head_t<TypeList<int>>, int>);
    head<TypeList<>> h;
    
    static_assert(std::is_same_v<tail_t<TypeList<int, bool, double>>, TypeList<bool, double>>);
    static_assert(std::is_same_v<tail_t<TypeList<int>>, TypeList<>>);
    tail<TypeList<>> t;

    static_assert(std::is_same_v<concat_t<TypeList<int, bool, double>, TypeList<int, float>>, TypeList<int, bool, double, int, float>>);
    static_assert(std::is_same_v<concat_t<TypeList<>, TypeList<int, bool>>, TypeList<int, bool>>);

    static_assert(std::is_same_v<push_back_t<TypeList<>, int>, TypeList<int>>);
    static_assert(std::is_same_v<push_back_t<TypeList<int, bool>, bool>, TypeList<int, bool, bool>>);

    static_assert(std::is_same_v<push_front_t<TypeList<>, int>, TypeList<int>>);
    static_assert(std::is_same_v<push_front_t<TypeList<int, bool>, int>, TypeList<int, int, bool>>);

    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 0>, TypeList<bool, double>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 1>, TypeList<int, double>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 2>, TypeList<int, bool>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 3>, TypeList<int, bool, double>>);

    static_assert(find_v<TypeList<int, bool, int, double>, int> == 0);
    static_assert(find_v<TypeList<int, bool, int, double>, bool> == 1);
    static_assert(find_v<TypeList<int, bool, int, double>, double> == 3);
    static_assert(find_v<TypeList<int, bool, int, double>, float> == 4);

    return 0;
}

