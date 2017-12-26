#ifndef HRLIB_TYPE_TRAITS_IS_MATCH_TEMPLATE 
#define HRLIB_TYPE_TRAITS_IS_MATCH_TEMPLATE 

#include <type_traits>

namespace hrlib::type_traits {
    template <template <typename...> class Class, typename T>
    struct is_match_template: std::false_type{};

    template <template <typename...> class Class, typename ...Args>
    struct is_match_template<Class, Class<Args...>>: std::true_type{};

    template <template <typename...> class Class, typename T>
    constexpr bool is_match_template_v = is_match_template<Class, T>::value;
}

#endif

