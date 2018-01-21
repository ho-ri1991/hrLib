#ifndef HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL
#define HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL

#include <utility>
#include <type_traits>

namespace hrlib::integer_sequence_util {
    template <typename>
    struct head;

    template <typename T, T I, T... J>
    struct head<std::integer_sequence<T, I, J...>> {
        static constexpr T value = I;
    };

    template <typename T>
    struct head<std::integer_sequence<T>>{};

    template <typename Seq>
    static constexpr auto head_v = head<Seq>::value;

    template <typename>
    struct tail;

    template <typename T, T I, T... J>
    struct tail<std::integer_sequence<T, I, J...>> {
        static constexpr T value = tail<std::integer_sequence<T, J...>>::value;
    };

    template <typename T, T I>
    struct tail<std::integer_sequence<T, I>> {
        static constexpr T value = I;
    };

    template <typename T>
    struct tail<std::integer_sequence<T>>{};

    template <typename Seq>
    static constexpr auto tail_v = tail<Seq>::value;

    template <typename, typename>
    struct concat;

    template <typename T, T... I1, T... I2>
    struct concat<std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>>  {
        using type = std::integer_sequence<T, I1..., I2...>;
    };

    template <typename IntegerSeq1, typename IntegerSeq2>
    using concat_t = typename concat<IntegerSeq1, IntegerSeq2>::type;

    template <typename T, T... I1, T... I2>
    constexpr std::integer_sequence<T, I1..., I2...> concat_fn(std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>) noexcept {
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

    template <typename>
    struct reverse;

    template <typename T, T I, T... J>
    struct reverse<std::integer_sequence<T, I, J...>> {
        using type = 
            concat_t<
                typename reverse<std::integer_sequence<T, J...>>::type, 
                std::integer_sequence<T, I>
            >;
    };

    template <typename T, T I>
    struct reverse<std::integer_sequence<T, I>> {
        using type = std::integer_sequence<T, I>;
    };

    template <typename Seq>
    using reverse_t = typename reverse<Seq>::type;

    template <typename T, T I, T ...J>
    constexpr auto reverse_fn(std::integer_sequence<T, I, J...>) noexcept {
        if constexpr (sizeof...(J) == 0) {
            return std::integer_sequence<T, I>{};
        } else {
            return concat_t<reverse_t<std::integer_sequence<T, J...>>, std::integer_sequence<T, I>>{};
        }
    }

}

#endif

