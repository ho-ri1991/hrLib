#ifndef HRLIB_TYPE_TRAITS_TYPE_LIST_UTIL
#define HRLIB_TYPE_TRAITS_TYPE_LIST_UTIL

#include <type_traits>
#include "identity.hpp"

namespace hrlib::type_traits {
    template <typename>
    struct head;
    template <template <typename...> class Tpl, typename T, typename... Ts>
    struct head<Tpl<T, Ts...>> {
        using type = T;
    };
    template <template <typename...> class Tpl>
    struct head<Tpl<>> {};
    template <typename Tpl>
    using head_t = typename head<Tpl>::type;
    
    template <typename>
    struct tail;
    template <template <typename...> class Tpl, typename T, typename... Ts>
    struct tail<Tpl<T, Ts...>> {
        using type = Tpl<Ts...>;
    };
    template <template <typename...> class Tpl>
    struct tail<Tpl<>>{};
    template <typename Tpl>
    using tail_t = typename tail<Tpl>::type;

    template <typename>
    struct size;
    template <template <typename...> class Tpl, typename... Ts>
    struct size<Tpl<Ts...>> {
        static constexpr auto value = sizeof...(Ts);
    };
    template <typename Tpl>
    constexpr auto size_v = size<Tpl>::value;

    template <typename Tpl>
    struct empty: std::conditional_t<size_v<Tpl> == 0, std::true_type, std::false_type>{};
    template <typename Tpl>
    constexpr bool empty_v = empty<Tpl>::value;

    template <typename Tpl, typename... Tpls>
    struct concat
    {
      using type = typename concat<Tpl, typename concat<Tpls...>::type>::type;
    };
    template <template <typename...> class Tpl, typename... Ts1, typename... Ts2>
    struct concat<Tpl<Ts1...>, Tpl<Ts2...>>
    {
      using type = Tpl<Ts1..., Ts2...>;
    };
    template <typename... Tpls>
    using concat_t = typename concat<Tpls...>::type;

    template <typename, typename>
    struct push_front;
    template <template <typename...> class Tpl, typename... Ts, typename T>
    struct push_front<Tpl<Ts...>, T> {
        using type = Tpl<T, Ts...>;
    };
    template<typename Tpl, typename T>
    using push_front_t = typename push_front<Tpl, T>::type;

    template <typename, typename>
    struct push_back;
    template <template <typename...> class Tpl, typename... Ts, typename T>
    struct push_back<Tpl<Ts...>, T> {
        using type = Tpl<Ts..., T>;
    };
    template <typename Tpl, typename T>
    using push_back_t = typename push_back<Tpl, T>::type;

    template <typename Tpl, typename T, std::size_t N>
    struct insert {
        static_assert(0 <= N);
    private:
        using new_head = std::conditional_t<N == 0, T, head_t<Tpl>>;
        using new_tail = typename std::conditional_t<N == 0, identity<Tpl>, insert<tail_t<Tpl>, T, N - 1>>::type;
    public:
        using type = push_front_t<new_tail, new_head>;
    };
    template <template <typename...> class Tpl, typename T>
    struct insert<Tpl<>, T, 0> {
        using type = Tpl<T>;
    };
    template <typename Tpl, typename T, std::size_t N>
    using insert_t = typename insert<Tpl, T, N>::type;

    template <typename, std::size_t>
    struct delete_nth;
    template <template <typename...> class Tpl, typename... Ts, std::size_t N>
    struct delete_nth<Tpl<Ts...>, N> {
        static_assert(0 <= N);
    private:
        using current_head = Tpl<head_t<Tpl<Ts...>>>;
        using current_tail = tail_t<Tpl<Ts...>>;
        using new_head = std::conditional_t<N == 0, Tpl<>, current_head>;
        using new_tail = typename std::conditional_t<N == 0, identity<current_tail>, delete_nth<current_tail, N - 1>>::type;
    public:
        using type = concat_t<new_head, new_tail>;
    };
    template <template <typename...> class Tpl, std::size_t N>
    struct delete_nth<Tpl<>, N> {
        static_assert(0 <= N);
        using type = Tpl<>;
    };
    template <typename Tpl, std::size_t N>
    using delete_nth_t = typename delete_nth<Tpl, N>::type;

    template <typename Tpl, bool isEmpty = empty_v<Tpl>>
    struct reverse {
        using type = push_back_t<typename reverse<tail_t<Tpl>>::type, head_t<Tpl>>;
    };
    template <typename Tpl>
    struct reverse<Tpl, true> {
        using type = Tpl;
    };
    template <typename Tpl>
    using reverse_t = typename reverse<Tpl>::type;

    template <typename Tpl, bool isEmpty = empty_v<Tpl>>
    struct last {
        using type = head_t<reverse_t<Tpl>>;
    };
    template <typename Tpl>
    struct last<Tpl, true> {};
    template <typename Tpl>
    using last_t = typename last<Tpl>::type;

    template <typename Tpl, std::size_t N>
    struct get: std::conditional_t<N == 0, head<Tpl>, get<tail_t<Tpl>, N - 1>>{};
    template <typename Tpl, std::size_t N>
    using get_t = typename get<Tpl, N>::type;

    template <typename Tpl, typename T, std::size_t N = 0>
    struct find: std::conditional_t<std::is_same_v<head_t<Tpl>, T>, identity_value<N>, find<tail_t<Tpl>, T, N + 1>>{};
    template <template <typename...> class Tpl, typename T, std::size_t N>
    struct find<Tpl<>, T, N>: identity_value<N>{};
    template <typename Tpl, typename T>
    constexpr std::size_t find_v = find<Tpl, T>::value;

    template <typename Tpl, template <typename> class Predicate, std::size_t N = 0>
    struct find_if: std::conditional_t<Predicate<head_t<Tpl>>::value, identity_value<N>, find_if<tail_t<Tpl>, Predicate, N + 1>>{};
    template <template <typename...> class Tpl, template <typename> class Predicate, std::size_t N>
    struct find_if<Tpl<>, Predicate, N>: identity_value<N>{};
    template <typename Tpl, template <typename> class Predicate>
    constexpr std::size_t find_if_v = find_if<Tpl, Predicate>::value;

    namespace detail {
        template <typename Tpl1, typename Tpl2, bool is_empty = size_v<Tpl1> == 0>
        struct is_permutation_impl {
        private:
            static constexpr auto find_ans = find_v<Tpl2, head_t<Tpl1>>;
        public:
            using type = 
                typename std::conditional_t<
                    find_ans < size_v<Tpl2>, 
                    is_permutation_impl<tail_t<Tpl1>, delete_nth_t<Tpl2, find_ans>>, 
                    identity<std::false_type>
                >::type;
        };
        template <typename Tpl1, typename Tpl2>
        struct is_permutation_impl<Tpl1, Tpl2, true> {
            using type = std::true_type;
        };
    }

    template <typename Tpl1, typename Tpl2>
    struct is_permutation: std::conditional_t<size_v<Tpl1> != size_v<Tpl2>, identity<std::false_type>, detail::is_permutation_impl<Tpl1, Tpl2>>::type{};
    template <typename Tpl1, typename Tpl2>
    constexpr bool is_permutation_v = is_permutation<Tpl1, Tpl2>::value;

    namespace detail {
        template <typename Tpl, typename T>
        struct unique_impl_helper {
        private:
            static constexpr auto n = find_v<Tpl, T>;
        public:
            using type = 
                typename std::conditional_t<
                    n == size_v<Tpl>,
                    identity<Tpl>,
                    unique_impl_helper<delete_nth_t<Tpl, n>, T>
                >::type;
        };
        template <typename Tpl, bool isEmpty = empty_v<Tpl>>
        struct unique_impl {
        private:
            using deduplicated = 
                push_front_t<
                    typename unique_impl_helper<tail_t<Tpl>, head_t<Tpl>>::type,
                    head_t<Tpl>
                >;
        public:
            using type = 
                push_front_t<
                    typename unique_impl<tail_t<deduplicated>>::type,
                    head_t<deduplicated>
                >;
        };
        template <typename Tpl>
        struct unique_impl<Tpl, true> {
            using type = Tpl;
        };
    }

    template <typename Tpl>
    struct unique: detail::unique_impl<Tpl>{};
    template <typename Tpl>
    using unique_t = typename unique<Tpl>::type;

    template <typename Tpl, std::size_t N>
    struct split;
    template <template <typename...> class Tpl, typename... Ts, std::size_t N>
    struct split<Tpl<Ts...>, N> {
        static_assert(N <= size_v<Tpl<Ts...>>);
    private:
        using target = Tpl<Ts...>;
        using T = typename split<tail_t<target>, N - 1>::type;
    public:
        using type = Tpl<push_front_t<head_t<T>, head_t<target>>, last_t<T>>;
    };
    template <template <typename...> class Tpl, typename... Ts>
    struct split<Tpl<Ts...>, 0> {
        using type = Tpl<Tpl<>, Tpl<Ts...>>;
    };
    template <typename Tpl, std::size_t N>
    using split_t = typename split<Tpl, N>::type;

    template <typename Tpl, std::size_t Begin, std::size_t End>
    struct slice {
        static_assert(Begin <= End && End <= size_v<Tpl>);
        using type = head_t<split_t<last_t<split_t<Tpl, Begin>>, End - Begin>>;
    };
    template <typename Tpl, std::size_t Begin, std::size_t End>
    using slice_t = typename slice<Tpl, Begin, End>::type;
}

#endif //HRLIB_TYPE_TRAITS_TYPE_LIST_UTIL

