#include <hrlib/integer_sequence_util/integer_sequence_util.hpp>
#include <iostream>
#include <boost/type_index.hpp>

using namespace hrlib::integer_sequence_util;

template <int I>
struct meta_add_1 {
    static constexpr int value = I + 1;
};

template <auto X, auto Y>
struct Less {
    static constexpr auto value = X < Y;
};

int main(){
    static_assert(head_v<std::integer_sequence<int, 1, 2, 3>> == 1);
    static_assert(head_v<std::integer_sequence<int, 1>> == 1);
    head<std::integer_sequence<int>> h; //just instantiatable

    static_assert(last_v<std::integer_sequence<int, 1, 2, 3>> == 3);
    static_assert(last_v<std::integer_sequence<int, 1>> == 1);
    last<std::integer_sequence<int>> t; //just instantiatable

    static_assert(
            std::is_same_v<
                tail_t<std::integer_sequence<int, 1, 2, 3, 4>>,
                std::integer_sequence<int, 2, 3, 4>
            >
    );
    static_assert(
            std::is_same_v<
                tail_t<std::integer_sequence<int, 1>>,
                std::integer_sequence<int>
            >
    );

    static_assert(
            std::is_same_v<
                push_back_t<std::integer_sequence<int, 1, 2, 3>, 4>,
                std::integer_sequence<int, 1, 2, 3, 4>
            >
    );

    static_assert(
            std::is_same_v<
                push_front_t<std::integer_sequence<int, 1, 2, 3>, 4>,
                std::integer_sequence<int, 4, 1, 2, 3>
            >
    );
 
    static_assert(
        std::is_same_v<
            concat_t<std::integer_sequence<int, -1, 5, 3>, std::integer_sequence<int, 1, 6, 2>>, 
            std::integer_sequence<int, -1, 5, 3,  1, 6, 2>
        >
    );

   constexpr auto seq = concat_fn(std::integer_sequence<int, -1, 5, 3>{}, std::integer_sequence<int, 1, 6, 2>{});
    static_assert(
        std::is_same_v<
            std::remove_const_t<decltype(seq)>, 
            std::integer_sequence<int, -1, 5, 3, 1, 6, 2>
        >
    );

    static_assert(
            std::is_same_v<
                transform<std::integer_sequence<int, 1, -1>, meta_add_1>::type, 
                std::integer_sequence<int, 2, 0>
            >
    );

    constexpr auto fn = [](int x){ return 2 * x; };
    static_assert(
            std::is_same_v<
                std::remove_const_t<decltype(transform_fn(std::integer_sequence<int, 0, 1, -1>{}, fn))>,
                std::integer_sequence<int, 0, 2, -2>
            >
    );

    static_assert(
            std::is_same_v<
                reverse<std::integer_sequence<int, 1, 0, -2, 0, 3, 1>>::type,
                std::integer_sequence<int, 1, 3, 0, -2, 0, 1>
            >
    );

    static_assert(
            std::is_same_v<
                std::remove_const_t<decltype(reverse_fn(std::integer_sequence<int, 1, 0, -2, 0, 3, 1>{}))>,
                std::integer_sequence<int, 1, 3, 0, -2, 0, 1>
            >
    );

// sort meta function is currently not available due to the bug in gcc 7.2.0. We can use this if we use clang
//    static_assert(
//            std::is_same_v<
//                sort_t<std::integer_sequence<int, 2, 1, 3, 2>>, std::integer_sequence<int, 1, 2, 2, 3>
//            >
//    );

    return 0;
}

