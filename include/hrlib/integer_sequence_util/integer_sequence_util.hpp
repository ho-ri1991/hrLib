#ifndef HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL
#define HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL

#include <utility>
#include <type_traits>
#include <hrlib/type_traits/type_traits.hpp>

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
    struct last;

    template <typename T, T I, T... J>
    struct last<std::integer_sequence<T, I, J...>> {
        static constexpr T value = last<std::integer_sequence<T, J...>>::value;
    };

    template <typename T, T I>
    struct last<std::integer_sequence<T, I>> {
        static constexpr T value = I;
    };

    template <typename T>
    struct last<std::integer_sequence<T>>{};

    template <typename Seq>
    static constexpr auto last_v = last<Seq>::value;

    template <typename>
    struct tail;

    template <typename T, T I, T... J>
    struct tail<std::integer_sequence<T, I, J...>> {
        using type = std::integer_sequence<T, J...>;
    };

    template <typename T, T I>
    struct tail<std::integer_sequence<T, I>> {
        using type = std::integer_sequence<T>;
    };

    template <typename T>
    struct tail<std::integer_sequence<T>> {};

    template <typename Seq>
    using tail_t = typename tail<Seq>::type;

    template <typename Seq, auto I> 
    struct push_back;

    template <typename T, T... I, T J>
    struct push_back<std::integer_sequence<T, I...>, J> {
        using type = std::integer_sequence<T, I..., J>;
    };

    template <typename Seq, auto I>
    using push_back_t = typename push_back<Seq, I>::type;

    template <typename, auto>
    struct push_front;

    template <typename T, T... I, T J>
    struct push_front<std::integer_sequence<T, I...>, J> {
        using type = std::integer_sequence<T, J, I...>;
    };

    template <typename Seq, auto I>
    using push_front_t = typename push_front<Seq, I>::type;

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

// Because of the bug in gcc 7.2.0, the internal compiler error occurs. So we cannot use this. But we can use these sort meta functions if we use clang
//    namespace detail {
//        template <typename Seq, auto I, template <auto, auto> class Cmp>
//        struct insert;
//
//        template <typename T, T... J, T I, template <T, T> class Cmp>
//        struct insert<std::integer_sequence<T, J...>, I, Cmp> {
//        private:
//            using target = std::integer_sequence<T, J...>;
//            static constexpr T target_head = head_v<target>;
//            // the reason to use identity is in order not to instantiate false branch recursively.
//            using new_tail = 
//                typename std::conditional_t<
//                    Cmp<I, target_head>::value,
//                    type_traits::identity<target>,
//                    insert<tail_t<target>, I, Cmp>
//                >::type;
//            static constexpr T new_head = Cmp<I, target_head>::value ? I : target_head;
//        public:
//            using type = push_front_t<new_tail, new_head>;
//        };
//
//        template <typename T, T I, template <T, T> class Cmp>
//        struct insert<std::integer_sequence<T>, I, Cmp> {
//            using type = std::integer_sequence<T, I>;
//        };
//    }
//
//    template <auto X, auto Y>
//    struct less_meta {
//        static constexpr auto value = X < Y;
//    };
//
//    template <typename, template <auto, auto> class = less_meta>
//    struct sort;
//
//    template <typename T, T I, T... J, template <T, T> class Compare>
//    struct sort<std::integer_sequence<T, I, J...>, Compare> {
//    private:
//        using sorted_tail = typename sort<std::integer_sequence<T, J...>, Compare>::type;
//    public:
//        using type = typename detail::insert<sorted_tail, I, Compare>::type;
//    };
//
//    template <typename T, T I, template <T, T> class Compare>
//    struct sort<std::integer_sequence<T, I>, Compare> {
//        using type = std::integer_sequence<T, I>;
//    };
//
//    template <typename Seq, template <auto, auto> class Compare = less_meta>
//    using sort_t = typename sort<Seq, Compare>::type;
}

#endif

