#ifndef HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL
#define HRLIB_INTEGER_SEQUENCE_UTIL_INTEGER_SEQUENCE_UTIL

#include <utility>
#include <type_traits>
#include <hrlib/type_traits/type_traits.hpp>

namespace hrlib::integer_sequence_util {
    // head meta-function
    template <typename>
    struct head;

    template <typename T, T I, T... J>
    struct head<std::integer_sequence<T, I, J...>> {
        static constexpr T value = I;
    };

    template <typename T>
    struct head<std::integer_sequence<T>>{}; // for SFINAE friendly

    template <typename Seq>
    static constexpr auto head_v = head<Seq>::value;

    // tail meta-function
    template <typename>
    struct tail;

    template <typename T, T I, T... J>
    struct tail<std::integer_sequence<T, I, J...>> {
        using type = std::integer_sequence<T, J...>;
    };

    template <typename T>
    struct tail<std::integer_sequence<T>> {}; // for SFINAE friendly

    template <typename Seq>
    using tail_t = typename tail<Seq>::type;

    // size meta-function
    template <typename Seq>
    struct size;

    template <typename T, T... I>
    struct size<std::integer_sequence<T, I...>> {
        static constexpr auto value = sizeof...(I);
    };

    template <typename Seq>
    static constexpr auto size_v = size<Seq>::value;

    // push_bach meta-function
    template <typename Seq, auto I>
    struct push_back;

    template <typename T, T... I, T J>
    struct push_back<std::integer_sequence<T, I...>, J> {
        using type = std::integer_sequence<T, I..., J>;
    };

    template <typename Seq, auto I>
    using push_back_t = typename push_back<Seq, I>::type;

    // push_front meta-function
    template <typename, auto>
    struct push_front;

    template <typename T, T... I, T J>
    struct push_front<std::integer_sequence<T, I...>, J> {
        using type = std::integer_sequence<T, J, I...>;
    };

    template <typename Seq, auto I>
    using push_front_t = typename push_front<Seq, I>::type;

    // concat meta-function
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

    // reverse meta-function
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

    template <typename T>
    struct reverse<std::integer_sequence<T>> {
        using type = std::integer_sequence<T>;
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

    template <typename T>
    constexpr auto reverse_fn(std::integer_sequence<T>) noexcept {
        return std::integer_sequence<T>{};
    }

    // last meta-function
    template <typename>
    struct last;

    template <typename T, T... I>
    struct last<std::integer_sequence<T, I...>>: head<reverse_t<std::integer_sequence<T, I...>>> {};

    template <typename Seq>
    static constexpr auto last_v = last<Seq>::value;

    // get meta-function
    template <typename, std::size_t>
    struct get;

    template <typename T, T I, T...  J, std::size_t N>
    struct get<std::integer_sequence<T, I, J...>, N>: get<std::integer_sequence<T, J...>, N - 1>{};

    template <typename T, T I, T... J>
    struct get<std::integer_sequence<T, I, J...>, 0> {
        static constexpr T value = I;
    };

    template <typename T, std::size_t N>
    struct get<std::integer_sequence<T>, N>{};
    
    template <typename Seq, std::size_t I>
    static constexpr auto get_v = get<Seq, I>::value;

    // transform meta-function
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

    // range meta-function
    template <typename T, T Begin, T End>
    struct range {
    private:
        static_assert(Begin <= End);
        static constexpr auto fn = [](T x){return x + Begin;};
    public:
        using type = std::remove_const_t<decltype(transform_fn(std::make_integer_sequence<T, End - Begin>{}, fn))>;
    };

    template <typename T, T Begin, T End>
    using range_t = typename range<T, Begin, End>::type;

    // slice meta-function
    namespace detail {
        template <typename, typename>
        struct slice_impl;

        template <typename T, T... I, std::size_t... J>
        struct slice_impl<std::integer_sequence<T, I...>, std::integer_sequence<std::size_t, J...>> {
            using type = std::integer_sequence<T, get_v<std::integer_sequence<T, I...>, J>...>;
        };

        template <typename Seq, typename HelperSeq>
        using slice_impl_t = typename slice_impl<Seq, HelperSeq>::type;
    }

    template <typename, std::size_t, std::size_t>
    struct slice;

    template <typename T, T... I, std::size_t Begin, std::size_t End>
    struct slice<std::integer_sequence<T, I...>, Begin, End> {
        static_assert(Begin <= End);
        static_assert(Begin < sizeof...(I) && End <= sizeof...(I));
        using type = detail::slice_impl_t<std::integer_sequence<T, I...>, range_t<std::size_t, Begin, End>>;
    };

    template <typename Seq, std::size_t Begin, std::size_t End>
    using slice_t = typename slice<Seq, Begin, End>::type;

    // find meta-function
    namespace detail {
        template <typename Seq, auto I, std::size_t N>
        struct find_impl;

        template <typename T, T I, T... J, T K, std::size_t N>
        struct find_impl<std::integer_sequence<T, I, J...>, K, N>:
            std::conditional_t<
                I == K, 
                type_traits::identity_value<N>,
                find_impl<std::integer_sequence<T, J...>, K, N + 1>
            > {};
        
        template <typename T, T K, std::size_t N>
        struct find_impl<std::integer_sequence<T>, K, N> {
            static constexpr std::size_t value = N;
        };

        template <typename Seq, template <auto> class Fn, std::size_t N>
        struct find_if_impl;

        template <typename T, T I, T... J, template <T> class Fn, std::size_t N>
        struct find_if_impl<std::integer_sequence<T, I, J...>, Fn, N>:
            std::conditional_t<
                Fn<I>::value,
                type_traits::identity_value<N>,
                find_if_impl<std::integer_sequence<T, J...>, Fn, N + 1>
            >{};
        
        template <typename T, template <T> class Fn, std::size_t N>
        struct find_if_impl<std::integer_sequence<T>, Fn, N>: type_traits::identity_value<N>{};

        template <typename T, T... I, typename Fn>
        constexpr auto find_if_fn_impl(std::integer_sequence<T, I...>, Fn fn , std::size_t n) noexcept(std::is_nothrow_invocable_r_v<bool, Fn, T>){
            if constexpr (sizeof...(I) == 0) {
                return n;
            } else {
                using target = std::integer_sequence<T, I...>;
                if(fn(head_v<target>)) return n;
                else return find_if_fn_impl(tail_t<target>{}, fn, n + 1);
            }
        }
    }

    template <typename Seq, auto I>
    struct find;

    template <typename T, T... I, T J>
    struct find<std::integer_sequence<T, I...>, J>: detail::find_impl<std::integer_sequence<T, I...>, J, 0>{};

    template <typename Seq, auto I>
    static constexpr std::size_t find_v = find<Seq, I>::value;

    template <typename Seq, template <auto> class Fn>
    struct find_if;

    template <typename T, T... I, template <auto> class Fn>
    struct find_if<std::integer_sequence<T, I...>, Fn>: detail::find_if_impl<std::integer_sequence<T, I...>, Fn, 0>{};

    template <typename Seq, template <auto> class  Fn>
    static constexpr std::size_t find_if_v = find_if<Seq, Fn>::value;

    template <typename T, T... I, typename Fn>
    constexpr std::size_t find_if_fn(std::integer_sequence<T, I...> seq, Fn fn) noexcept(std::is_nothrow_invocable_r_v<bool, Fn, T>) {
       return detail::find_if_fn_impl(seq, fn, 0);
    }

    namespace detail {
        template <typename T, T I, T... J, typename Fn>
        constexpr auto insert(std::integer_sequence<T, J...>, Fn fn) noexcept{
            if constexpr (sizeof...(J) == 0) {
                return std::integer_sequence<T, I>{};
            } else {
                using target = std::integer_sequence<T, J...>;
                if constexpr (fn(I, head_v<target>)) {
                    return push_front_t<target, I>{};
                } else {
                    auto new_tail = insert<T, I>(tail_t<target>{}, fn);
                    return push_front_t<std::remove_const_t<decltype(new_tail)>, head_v<target>>{};
                }
            }
        }

        template <typename T, T I, T... J, typename Fn>
        constexpr auto sort_fn_impl(std::integer_sequence<T, I, J...> seq, Fn fn) noexcept{
            if constexpr (sizeof...(J) == 0){
                return std::integer_sequence<T, I>{};
            } else {
                auto new_tail = sort_fn_impl(std::integer_sequence<T, J...>{}, fn);
                return insert<T, I>(new_tail, fn);
            }
        }

        template <typename T, typename Fn>
        constexpr auto sort_fn_impl(std::integer_sequence<T>, Fn fn) noexcept{
            return std::integer_sequence<T>{};
        }
    }

    template <typename T, T I, T... J, typename Fn>
    constexpr auto sort_fn(std::integer_sequence<T, I, J...> seq, Fn fn) noexcept{
        return detail::sort_fn_impl(seq, fn);
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

