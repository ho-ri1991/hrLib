#ifndef HRLIB_TYPE_TRAITS_IS_DETECTED
#define HRLIB_TYPE_TRAITS_IS_DETECTED

#include <type_traits>

namespace hrlib::type_traits {
    namespace detail {
        template <template <typename...> class Trait, typename Enable, typename... Args>
        struct is_detected_impl: std::false_type{};

        template <template <typename...> class Trait, typename... Args>
        struct is_detected_impl<Trait, std::void_t<Trait<Args...>>, Args...>: std::true_type{};
    }

    template <template <typename ...> class Trait, typename... Args>
    using is_detected = typename detail::is_detected_impl<Trait, void, Args...>::type;

    template <template <typename ...> class Trait, typename... Args>
    constexpr bool is_detected_v = is_detected<Trait, Args...>::value;
}

#endif

