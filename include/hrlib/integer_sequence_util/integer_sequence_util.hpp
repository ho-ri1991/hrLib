#ifndef HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL
#define HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL

#include <utility>
#include <type_traits>

namespace hrlib::integer_sequence_util {
    template <typename, typename>
    struct integer_sequence_cat;

    template <typename T, T... I1, T... I2>
    struct integer_sequence_cat<std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>>  {
        using type = std::integer_sequence<T, I1..., I2...>;
    };

    template <typename IntegerSeq1, typename IntegerSeq2>
    using integer_sequence_cat_t = typename integer_sequence_cat<IntegerSeq1, IntegerSeq2>::type;

    template <typename T, T... I1, T... I2>
    constexpr std::integer_sequence<T, I1..., I2...> integer_sequence_cat_fn(std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>) noexcept {
        return std::integer_sequence<T, I1..., I2...>{};
    }

    template <typename sequence, template <auto> class Fn>
//  template <typename sequence, template <typename sequence::value_type> class Fn> // C++14
    struct transform;

    template <typename T, T... I, template <T> class Fn>
    struct transform<std::integer_sequence<T, I...>, Fn>{
        using type = std::integer_sequence<T, Fn<I>::value...>;
    };

    template <typename T, T... I, typename Fn>
    constexpr auto transform_fn(std::integer_sequence<T, I...>, Fn fn) noexcept(std::is_nothrow_invocable_r_v<T, Fn, T>) {
        return std::integer_sequence<T, fn(I)...>{};
    }
}

#endif

