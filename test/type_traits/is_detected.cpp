#include <utility>
#include <hrlib/type_traits/is_detected.hpp>

using namespace hrlib;

struct X{
    int x;
    void f();
};

struct Y{
    using type = int;
    void g(int, int);
};

template <typename T>
using support_f = decltype(std::declval<T>().f());

template <typename T>
using support_f_int = decltype(std::declval<T>().f(int{}));

template <typename T>
using has_x = decltype(std::declval<T>().x);

template <typename T, typename U>
using support_g = decltype(std::declval<T>().g(std::declval<U>(), std::declval<U>()));

template <typename T>
using has_type = typename T::type;

int main(){
    static_assert(type_traits::is_detected_v<support_f, X>);
    static_assert(!type_traits::is_detected_v<support_f, Y>);
    static_assert(!type_traits::is_detected_v<support_f_int, X>);
    static_assert(type_traits::is_detected_v<has_x, X>);
    static_assert(!type_traits::is_detected_v<has_x, Y>);
    static_assert(type_traits::is_detected_v<support_g, Y, int>);
    static_assert(!type_traits::is_detected_v<support_g, Y, X>);
    static_assert(type_traits::is_detected_v<has_type, Y>);
    static_assert(!type_traits::is_detected_v<has_type, X>);
    return 0;
}

