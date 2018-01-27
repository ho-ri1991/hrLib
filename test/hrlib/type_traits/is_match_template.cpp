#include <hrlib/type_traits/is_match_template.hpp>
using namespace hrlib;

template <typename T>
struct X{};

template <typename T, typename U>
struct Y{};

template <typename ...Ts>
struct Z{};

struct W{};

int main(){
    static_assert(type_traits::is_match_template_v<X, X<int>>);
    static_assert(type_traits::is_match_template_v<Y, Y<bool, X<int>>>);
    static_assert(type_traits::is_match_template_v<Z, Z<int, bool, double>>);
    static_assert(type_traits::is_match_template_v<Z, Z<>>);
    static_assert(!type_traits::is_match_template_v<X, Y<int, int>>);
    static_assert(!type_traits::is_match_template_v<Z, Y<int, int>>);
    return 0;
}

