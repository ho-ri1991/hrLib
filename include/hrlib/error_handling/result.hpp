#ifndef HRLIB_ERROR_HANDLING_RESULT
#define HRLIB_ERROR_HANDLING_RESULT

#include <string>
#include <tuple>
#include <functional>
#include <variant>
#include <hrlib/type_traits/type_traits.hpp>

namespace hrlib::error_handling {
    namespace result {
        //a type which represents normal state in the Result class
        template <typename T>
        struct Ok {
            using wrap_type = T;
            wrap_type data;
            constexpr Ok(const wrap_type& data) noexcept(std::is_nothrow_copy_constructible_v<wrap_type>): data(data){}
            constexpr Ok(wrap_type&& data) noexcept(std::is_nothrow_move_constructible_v<wrap_type>): data(std::move(data)){}
            Ok(const Ok&) = default;
            Ok(Ok&&) = default;
            Ok& operator=(const Ok&) = default;
            Ok& operator=(Ok&&) = default;
            ~Ok() = default;
        };

        //a type which represents error state in the Result class
        template <typename T>
        struct Err {
            using wrap_type = T;
            T err;
            constexpr Err(const wrap_type& err) noexcept(std::is_nothrow_copy_constructible_v<wrap_type>): err(err){}
            constexpr Err(wrap_type&& err) noexcept(std::is_nothrow_move_constructible_v<wrap_type>): err(std::move(err)){}
            Err(const Err&) = default;
            Err(Err&&) = default;
            Err& operator=(const Err&) = default;
            Err& operator=(Err&&) = default;
            ~Err() = default;
        };

        //a type for merging errors in Result, this type is used when we use DefaultMergePolicy
        template <typename... Ts>
        struct ErrorTuple {
            std::tuple<Ts...> errors;
            constexpr ErrorTuple(const std::tuple<Ts...>& tuple) noexcept(std::is_nothrow_copy_constructible_v<std::tuple<Ts...>>): errors(tuple){};
            constexpr ErrorTuple(std::tuple<Ts...>&& tuple) noexcept(std::is_nothrow_move_constructible_v<std::tuple<Ts...>>): errors(std::move(tuple)){};
        };

        //deducation guides for ErrorTuple
        template <typename... Ts>
        ErrorTuple(const std::tuple<Ts...>&)->ErrorTuple<Ts...>;
        template <typename... Ts>
        ErrorTuple(std::tuple<Ts...>&&)->ErrorTuple<Ts...>;

        //meta function which returns is the type is an ErrorTuple
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
    public:
        using ok_wrap_type = WrapType;
        using error_wrap_type = ErrType;
        using Ok = result::Ok<ok_wrap_type>;
        using Err = result::Err<error_wrap_type>;
    private:
        std::variant<Ok, Err> var;
    public:
        Result(const Ok& ok) noexcept(std::is_nothrow_copy_constructible_v<Ok>): var(ok){}
        Result(Ok&& ok) noexcept(std::is_nothrow_move_constructible_v<Ok>): var(std::move(ok)){}
        Result(const Err& err) noexcept(std::is_nothrow_copy_constructible_v<Err>): var(err){}
        Result(Err&& err) noexcept(std::is_nothrow_move_constructible_v<Err>): var(std::move(err)){}
        Result(const Result&) = default;
        Result(Result&&) = default;
        Result& operator=(const Result&) = default;
        Result& operator=(Result&&) = default;
        ~Result() = default;
    public:
        explicit operator bool()const noexcept { return std::get_if<Ok>(&var) != nullptr; }
        ok_wrap_type& get_ok() &{ return std::get<Ok>(var).data; }
        const ok_wrap_type& get_ok() const &{ return std::get<Ok>(var).data; }
        ok_wrap_type&& get_ok() &&{ return std::move(std::get<Ok>(var).data); }
        error_wrap_type& get_err() &{ return std::get<Err>(var).err; }
        const error_wrap_type& get_err() const &{ return std::get<Err>(var).err; }
        error_wrap_type&& get_err() &&{ return std::move(std::get<Err>(var)).err; }
    public:
        template <typename Fn>
        auto ok_or(Fn fn) const& noexcept(std::is_nothrow_copy_constructible_v<ok_wrap_type> && std::is_nothrow_invocable_r_v<ok_wrap_type, Fn>)
            -> std::enable_if_t<std::is_invocable_r_v<ok_wrap_type, Fn>, ok_wrap_type> {
            return (*this) ? get_ok() : fn();
        }
        template <typename Fn>
        auto ok_or(Fn fn) && noexcept(std::is_nothrow_move_constructible_v<ok_wrap_type> && std::is_nothrow_invocable_r_v<ok_wrap_type, Fn>)
            -> std::enable_if<std::is_invocable_r_v<ok_wrap_type, Fn>, ok_wrap_type> {
            return (*this) ? std::move(get_ok()) : fn();
        }
    };
}

#endif

