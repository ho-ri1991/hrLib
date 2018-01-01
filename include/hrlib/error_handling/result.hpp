#ifndef HRLIB_ERROR_HANDLING_RESULT
#define HRLIB_ERROR_HANDLING_RESULT

#include <string>
#include <tuple>
#include <functional>
#include <hrlib/type_traits/type_traits.hpp>

namespace hrlib::error_handling {
    namespace result {
        template <typename T>
        struct Ok {
            using wrap_type = T;
            T data;
        };
    
        template <typename T>
        struct Err {
            using wrap_type = T;
            T err;
        };

        template <typename... Ts>
        struct ErrorTuple {
            std::tuple<Ts...> errors;
            constexpr ErrorTuple(const std::tuple<Ts...>& tuple): errors(tuple){};
            constexpr ErrorTuple(std::tuple<Ts...>&& tuple): errors(std::move(tuple)){};
        };

        template <typename... Ts>
        ErrorTuple(const std::tuple<Ts...>&)->ErrorTuple<Ts...>;

        template <typename... Ts>
        ErrorTuple(std::tuple<Ts...>&&)->ErrorTuple<Ts...>;

        template <typename T>
        using is_error_tuple = type_traits::is_match_template<ErrorTuple, T>;
        template <typename T>
        constexpr bool is_error_tuple_v = is_error_tuple<T>::value;

        template <typename... ErrorTuples>
        constexpr auto errorTupleCat(ErrorTuples&&... errorTuples) 
            -> std::enable_if_t<(is_error_tuple_v<std::decay_t<ErrorTuples>> && ...), decltype(ErrorTuple(std::tuple_cat(std::declval<ErrorTuples>().errors...)))> {
            return std::tuple_cat(std::forward<ErrorTuples>(errorTuples)...);
        }

        struct DefaultMergePolicy {
            template <typename Err1, typename Err2>
            static constexpr auto mergeError(Err1&& err1, Err2&& err2) {
                using ErrType1 = std::decay_t<Err1>;
                using ErrType2 = std::decay_t<Err2>;
                if constexpr (is_error_tuple_v<ErrType1> && is_error_tuple_v<ErrType2>) {
                    return errorTupleCat(std::forward<Err1>(err1), std::forward<Err2>(err2));
                } else if constexpr (is_error_tuple_v<ErrType1> && !is_error_tuple_v<ErrType2>) {
                    return errorTupleCat(std::forward<Err1>(err1), ErrorTuple(std::tuple(std::forward<Err2>(err2))));
                } else if constexpr (!is_error_tuple_v<ErrType1> && is_error_tuple_v<ErrType2>) {
                    return errorTupleCat(std::tuple(std::forward<Err1>(err1)), std::forward<Err2>(err2));
                } else {
                    return ErrorTuple(std::forward<Err1>(err1), std::forward<Err2>(err2));
                }
            };
        };
    } 

    template <typename WrapType, typename ErrType = std::string, typename MergePolicy = result::DefaultMergePolicy>
    class Result {
    };
}

#endif

