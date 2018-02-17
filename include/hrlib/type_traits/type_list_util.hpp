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

    template <typename, typename>
    struct concat;
    template <template <typename...> class Tpl, typename... Ts1, typename... Ts2>
    struct concat<Tpl<Ts1...>, Tpl<Ts2...>> {
        using type = Tpl<Ts1..., Ts2...>;
    };
    template <typename Tpl1, typename Tpl2>
    using concat_t = typename concat<Tpl1, Tpl2>::type;

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

    template <typename, typename, std::size_t>
    struct insert;
    template <template <typename...> class Tpl, typename... Ts, typename T, std::size_t N>
    struct insert<Tpl<Ts...>, T, N> {
        static_assert(0 <= N);
    private:
        using new_head = std::conditional_t<N == 0, T, head_t<Tpl<Ts...>>>;
        using new_tail = typename std::conditional_t<N == 0, identity<Tpl<Ts...>>, insert<tail_t<Tpl<Ts...>>, T, N - 1>>::type;
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

    template <typename Tpl, std::size_t N>
    struct get: std::conditional_t<N == 0, head<Tpl>, get<tail_t<Tpl>, N - 1>>{};
    template <typename Tpl, std::size_t N>
    using get_t = typename get<Tpl, N>::type;

    template <typename, typename, std::size_t N = 0>
    struct find;
    template <template <typename...> class Tpl, typename... Ts, typename T, std::size_t N>
    struct find<Tpl<Ts...>, T, N>: 
        std::conditional_t<std::is_same_v<head_t<Tpl<Ts...>>, T>, identity_value<N>, find<tail_t<Tpl<Ts...>>, T, N + 1>>{};
    template <template <typename...> class Tpl, typename T, std::size_t N>
    struct find<Tpl<>, T, N>: identity_value<N>{};
    template <typename Tpl, typename T>
    constexpr std::size_t find_v = find<Tpl, T>::value;

}

#endif //HRLIB_TYPE_TRAITS_TYPE_LIST_UTIL

