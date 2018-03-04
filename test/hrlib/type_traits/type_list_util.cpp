#include <hrlib/type_traits/type_list_util.hpp>
#include <boost/type_index.hpp>
#include <iostream>

template <typename... Args>
struct TypeList {};

template <typename T>
struct Predicate1{
    static constexpr bool value = std::is_same_v<T, int>;
};

using namespace hrlib::type_traits;

int main(){
    static_assert(std::is_same_v<head_t<TypeList<int, bool, double>>, int>);
    static_assert(std::is_same_v<head_t<TypeList<int>>, int>);
    head<TypeList<>> h;
    
    static_assert(std::is_same_v<tail_t<TypeList<int, bool, double>>, TypeList<bool, double>>);
    static_assert(std::is_same_v<tail_t<TypeList<int>>, TypeList<>>);
    tail<TypeList<>> t;

    static_assert(size_v<TypeList<int, int, int>> == 3);
    static_assert(size_v<TypeList<>> == 0);

    static_assert(empty_v<TypeList<>>);
    static_assert(!empty_v<TypeList<int>>);
    static_assert(!empty_v<TypeList<int, bool, double>>);

    static_assert(std::is_same_v<concat_t<TypeList<int, bool, double>, TypeList<int, float>>, TypeList<int, bool, double, int, float>>);
    static_assert(std::is_same_v<concat_t<TypeList<>, TypeList<int, bool>>, TypeList<int, bool>>);

    static_assert(std::is_same_v<push_back_t<TypeList<>, int>, TypeList<int>>);
    static_assert(std::is_same_v<push_back_t<TypeList<int, bool>, bool>, TypeList<int, bool, bool>>);

    static_assert(std::is_same_v<push_front_t<TypeList<>, int>, TypeList<int>>);
    static_assert(std::is_same_v<push_front_t<TypeList<int, bool>, int>, TypeList<int, int, bool>>);

    static_assert(std::is_same_v<insert_t<TypeList<int, bool, double>, float, 0>, TypeList<float, int, bool, double>>);
    static_assert(std::is_same_v<insert_t<TypeList<int, bool, double>, float, 1>, TypeList<int, float, bool, double>>);
    static_assert(std::is_same_v<insert_t<TypeList<int, bool, double>, float, 2>, TypeList<int, bool, float, double>>);
    static_assert(std::is_same_v<insert_t<TypeList<int, bool, double>, float, 3>, TypeList<int, bool, double, float>>);

    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 0>, TypeList<bool, double>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 1>, TypeList<int, double>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 2>, TypeList<int, bool>>);
    static_assert(std::is_same_v<delete_nth_t<TypeList<int, bool, double>, 3>, TypeList<int, bool, double>>);

    static_assert(std::is_same_v<reverse_t<TypeList<int, bool, double, float>>, TypeList<float, double, bool, int>>);
    static_assert(std::is_same_v<reverse_t<TypeList<int>>, TypeList<int>>);
    static_assert(std::is_same_v<reverse_t<TypeList<>>, TypeList<>>);

    static_assert(std::is_same_v<last_t<TypeList<int, bool, double>>, double>);
    static_assert(std::is_same_v<last_t<TypeList<int>>, int>);
    last<TypeList<>> l;

    static_assert(std::is_same_v<get_t<TypeList<int, bool, double>, 0>, int>);
    static_assert(std::is_same_v<get_t<TypeList<int, bool, double>, 1>, bool>);
    static_assert(std::is_same_v<get_t<TypeList<int, bool, double>, 2>, double>);

    static_assert(find_v<TypeList<int, bool, int, double>, int> == 0);
    static_assert(find_v<TypeList<int, bool, int, double>, bool> == 1);
    static_assert(find_v<TypeList<int, bool, int, double>, double> == 3);
    static_assert(find_v<TypeList<int, bool, int, double>, float> == 4);

    static_assert(find_if_v<TypeList<int, bool, int, double>, Predicate1> == 0);
    static_assert(find_if_v<TypeList<bool, int, double>, Predicate1> == 1);
    static_assert(find_if_v<TypeList<bool, bool, int, double>, Predicate1> == 2);
    static_assert(find_if_v<TypeList<bool, double>, Predicate1> == 2);

    static_assert(is_permutation_v<TypeList<int, bool, double>, TypeList<bool, int, double>>);
    static_assert(is_permutation_v<TypeList<int, bool, double, int>, TypeList<int, bool, int, double>>);
    static_assert(!is_permutation_v<TypeList<int, int, double>, TypeList<bool, int, double>>);

    static_assert(std::is_same_v<unique_t<TypeList<int, int, int>>, TypeList<int>>);
    static_assert(std::is_same_v<unique_t<TypeList<int, bool, double, int>>, TypeList<int, bool, double>>);
    static_assert(std::is_same_v<unique_t<TypeList<int, bool, double>>, TypeList<int, bool, double>>);
    static_assert(std::is_same_v<unique_t<TypeList<int, bool, double, bool, int, float>>, TypeList<int, bool, double, float>>);
    static_assert(std::is_same_v<unique_t<TypeList<>>, TypeList<>>);

    static_assert(std::is_same_v<split_t<TypeList<int, bool, double, int>, 0>, TypeList<TypeList<>,TypeList<int, bool, double, int>>>);
    static_assert(std::is_same_v<split_t<TypeList<int, bool, double, int>, 1>, TypeList<TypeList<int>,TypeList<bool, double, int>>>);
    static_assert(std::is_same_v<split_t<TypeList<int, bool, double, int>, 2>, TypeList<TypeList<int, bool>,TypeList<double, int>>>);
    static_assert(std::is_same_v<split_t<TypeList<int, bool, double, int>, 3>, TypeList<TypeList<int, bool, double>,TypeList<int>>>);
    static_assert(std::is_same_v<split_t<TypeList<int, bool, double, int>, 4>, TypeList<TypeList<int, bool, double, int>,TypeList<>>>);

    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 0, 4>, TypeList<int, bool, double, float>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 0, 3>, TypeList<int, bool, double>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 0, 2>, TypeList<int, bool>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 0, 1>, TypeList<int>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 0, 0>, TypeList<>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 1, 4>, TypeList<bool, double, float>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 1, 3>, TypeList<bool, double>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 1, 2>, TypeList<bool>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 1, 1>, TypeList<>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 2, 4>, TypeList<double, float>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 2, 3>, TypeList<double>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 2, 2>, TypeList<>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 3, 4>, TypeList<float>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 3, 3>, TypeList<>>);
    static_assert(std::is_same_v<slice_t<TypeList<int, bool, double, float>, 4, 4>, TypeList<>>);

    return 0;
}

