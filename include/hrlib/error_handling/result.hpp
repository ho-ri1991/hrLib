#ifndef HRLIB_ERROR_HANDLING_RESULT
#define HRLIB_ERROR_HANDLING_RESULT

#include <string>
#include <tuple>
#include <functional>
#include <variant>
#include <hrlib/type_traits/type_traits.hpp>

namespace hrlib::error_handling {
    //forward decralation
    template <typename WrapType, typename ErrType>
    class Result;

    namespace result {
        //meta function for checking if the type is a Result<T, E> class
        template <typename T>
        using is_result_type = type_traits::is_match_template<Result, T>;
        template <typename T>
        constexpr bool is_result_type_v = is_result_type<T>::value;

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

    } 

    template <typename WrapType, typename ErrType = std::string>
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
        template <typename Fn, typename WrapType_ = std::decay_t<std::invoke_result_t<Fn, const ok_wrap_type&>>>
        Result<WrapType_, ErrType> map(Fn fn) const& noexcept(std::is_nothrow_invocable_r_v<WrapType_, Fn, const ok_wrap_type&> 
                                                              && std::is_nothrow_constructible_v<Result<WrapType_, ErrType>, result::Ok<WrapType_>&&>
                                                              && std::is_nothrow_constructible_v<Result<WrapType_, ErrType>, const result::Err<ErrType>&>){
            using ok_type = result::Ok<WrapType_>;
            using result_type = Result<WrapType_, ErrType>;
            return (*this) ? result_type(ok_type(fn(std::get<Ok>(var).data))) : result_type(std::get<Err>(var));
        }
        template <typename Fn, typename WrapType_ = std::decay_t<std::invoke_result_t<Fn, ok_wrap_type&&>>>
        Result<WrapType_, ErrType> map(Fn fn) && noexcept(std::is_nothrow_invocable_r_v<WrapType_, Fn, ok_wrap_type&&> 
                                                          && std::is_nothrow_constructible_v<Result<WrapType_, ErrType>, result::Ok<WrapType_>&&>
                                                          && std::is_nothrow_constructible_v<Result<WrapType_, ErrType>, result::Err<ErrType>&&>){
            using ok_type = result::Ok<WrapType_>;
            using result_type = Result<WrapType_, ErrType>;
            return (*this) ? result_type(ok_type(fn(std::move(std::get<Ok>(var).data)))) : result_type(std::get<Err>(std::move(var)));
        }                     
        template <
                  typename Fn, 
                  typename Result_ = std::enable_if_t<
                          result::is_result_type_v<std::decay_t<std::invoke_result_t<Fn, const ok_wrap_type&>>>,
                          std::decay_t<std::invoke_result_t<Fn, const ok_wrap_type&>>
                      >,
                  typename = std::enable_if_t<std::is_same_v<typename Result_::error_wrap_type, error_wrap_type>>
                 >
        Result_ flat_map(Fn fn) const& noexcept(std::is_nothrow_invocable_r_v<Result_, Fn, const ok_wrap_type&>
                                                && std::is_nothrow_constructible_v<Result_, const Err&>) {
            return (*this) ? fn(std::get<Ok>(var).data) : std::get<Err>(var);
        }
        template <
                  typename Fn, 
                  typename Result_ = std::enable_if_t<
                          result::is_result_type_v<std::decay_t<std::invoke_result_t<Fn, ok_wrap_type&&>>>,
                          std::decay_t<std::invoke_result_t<Fn, ok_wrap_type&&>>
                      >,
                  typename = std::enable_if_t<std::is_same_v<typename Result_::error_wrap_type, error_wrap_type>>
                 >
        Result_ flat_map(Fn fn) && noexcept(std::is_nothrow_invocable_r_v<Result_, Fn, ok_wrap_type&&>
                                            && std::is_nothrow_constructible_v<Result_, Err&&>) {
            return (*this) ? fn(std::move(std::get<Ok>(var).data)) : std::get<Err>(std::move(var));
        }
    public:
        template <typename Matcher>
        decltype(auto) match(Matcher&& matcher) const& { return std::visit(std::forward<Matcher>(matcher), var); }
        template <typename Matcher>
        decltype(auto) match(Matcher&& matcher) && { return std::visit(std::forward<Matcher>(matcher), std::move(var)); }
    };

    namespace result {
        //a type for merging errors in Result, this type is used when we use DefaultMergePolicy
        template <typename... Ts>
        struct ErrorTuple {
            std::tuple<Ts...> errors;
            constexpr ErrorTuple(const std::tuple<Ts...>& tuple) noexcept(std::is_nothrow_copy_constructible_v<std::tuple<Ts...>>): errors(tuple){}
            constexpr ErrorTuple(std::tuple<Ts...>&& tuple) noexcept(std::is_nothrow_move_constructible_v<std::tuple<Ts...>>): errors(std::move(tuple)){}
            ErrorTuple() = default;
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
            return std::tuple_cat(std::forward<ErrorTuples>(errorTuples).errors...);
        }

        //a type for merging ok in Result, this type is used when we use DefaultMergePolicy
        template <typename... Ts>
        struct OkTuple {
            std::tuple<Ts...> oks;
            constexpr OkTuple(const std::tuple<Ts...>& tuple) noexcept(std::is_nothrow_copy_constructible_v<std::tuple<Ts...>>): oks(tuple){}
            constexpr OkTuple(std::tuple<Ts...>&& tuple) noexcept(std::is_nothrow_move_constructible_v<std::tuple<Ts...>>): oks(std::move(tuple)){}
            OkTuple() = default;
        };
        //deducation guide
        template <typename... Ts>
        OkTuple(const std::tuple<Ts...>&)->OkTuple<Ts...>;
        template <typename... Ts>
        OkTuple(std::tuple<Ts...>&&)->OkTuple<Ts...>;

        template <typename T>
        using is_ok_tuple = type_traits::is_match_template<OkTuple, T>;
        template <typename T>
        constexpr bool is_ok_tuple_v = is_ok_tuple<T>::value;
        
        template <typename... OkTuples>
        constexpr auto okTupleCat(OkTuples&&... okTuples)
            -> std::enable_if_t<(is_ok_tuple_v<OkTuples> && ...), decltype(OkTuple(std::tuple_cat(std::declval<OkTuples>().oks...)))> {
            return std::tuple_cat(std::forward<OkTuples>(okTuples).oks...);
        }

        struct DefaultMergePolicy {
        private:
            template <typename Ok1, typename Ok2>
            static constexpr auto merge_ok(Ok1&& ok1, Ok2&& ok2) {
                using wrap_type1 = typename std::decay_t<Ok1>::wrap_type;
                using wrap_type2 = typename std::decay_t<Ok2>::wrap_type;
                if constexpr (is_ok_tuple_v<wrap_type1> && is_ok_tuple_v<wrap_type2>) {
                    return Ok(okTupleCat(std::forward<Ok1>(ok1).data, std::forward<Ok2>(ok2).data));
                } else if constexpr (is_ok_tuple_v<wrap_type1> && !is_ok_tuple_v<wrap_type2>) {
                    return Ok(okTupleCat(std::forward<Ok1>(ok1).data, OkTuple(std::tuple(std::forward<Ok2>(ok2).data))));
                } else if constexpr (!is_ok_tuple_v<wrap_type1> && is_ok_tuple_v<wrap_type2>) {
                    return Ok(okTupleCat(OkTuple(std::tuple(std::forward<Ok1>(ok1).data)), std::forward<Ok2>(ok2).data));
                } else {
                    return Ok(OkTuple(std::tuple(std::forward<Ok1>(ok1).data, std::forward<Ok2>(ok2).data)));
                }
            }
            template <typename Err1, typename Err2>
            static constexpr auto merge_err(Err1&& err1, Err2&& err2) {
                using wrap_type1 = typename std::decay_t<Err1>::wrap_type;
                using wrap_type2 = typename std::decay_t<Err2>::wrap_type;
                if constexpr (is_error_tuple_v<wrap_type1> && is_error_tuple_v<wrap_type2>) {
                    return Err(errorTupleCat(std::forward<Err1>(err1).err, std::forward<Err2>(err2).err));
                } else if constexpr (is_error_tuple_v<wrap_type1> && !is_error_tuple_v<wrap_type2>) {
                    return Err(errorTupleCat(std::forward<Err1>(err1).err, ErrorTuple(std::tuple(std::forward<Err2>(err2).err))));
                } else if constexpr (!is_error_tuple_v<wrap_type1> && is_error_tuple_v<wrap_type2>) {
                    return Err(errorTupleCat(ErrorTuple(std::tuple(std::forward<Err1>(err1).err)), std::forward<Err2>(err2).err));
                } else {
                    return Err(ErrorTuple(std::tuple(std::forward<Err1>(err1).err, std::forward<Err2>(err2).err)));
                }
            }
        public:
            //default error value, specialize this class if we want to change default value
            template <typename T>
            struct DefaultErrorValue {
                inline static const T value = T{};
            };

            template <typename Result1, typename Result2, typename = std::enable_if_t<is_result_type_v<std::decay_t<Result1>> && is_result_type_v<std::decay_t<Result2>>>>
            static constexpr auto merge(Result1&& result1, Result2&& result2) {
                using ok1 = typename std::decay_t<Result1>::Ok;
                using err1 = typename std::decay_t<Result1>::Err;
                using ok2 = typename std::decay_t<Result2>::Ok;
                using err2 = typename std::decay_t<Result2>::Err;
                using result_type = Result<typename decltype(merge_ok(std::declval<ok1>(), std::declval<ok2>()))::wrap_type, typename decltype(merge_err(std::declval<err1>(), std::declval<err2>()))::wrap_type>;
                if(result1 && result2) {
                    return result_type(merge_ok(Ok(std::forward<Result1>(result1).get_ok()), Ok(std::forward<Result2>(result2).get_ok())));
                } else if(result1 && !result2) {
                    return result_type(merge_err(Err(DefaultErrorValue<typename err1::wrap_type>::value), Err(std::forward<Result2>(result2).get_err())));
                } else if(!result1 && result2) {
                    return result_type(merge_err(Err(std::forward<Result1>(result1).get_err()), Err(DefaultErrorValue<typename err2::wrap_type>::value)));
                } else {
                    return result_type(merge_err(Err(std::forward<Result1>(result1).get_err()), Err(std::forward<Result2>(result2).get_err())));
                }
            }
        };

        namespace detail {
            template <typename MergePolicy, typename ReultTuple, typename WrapTypeList, typename ErrorTypeList>
            struct SequenceHelper;

            template <typename MergePolicy, typename ResultTuple, typename WrapType1, typename WrapType2, typename... WrapTypes, typename ErrorType1, typename ErrorType2, typename... ErrorTypes>
            struct SequenceHelper<MergePolicy, ResultTuple, std::tuple<WrapType1, WrapType2, WrapTypes...>, std::tuple<ErrorType1, ErrorType2, ErrorTypes...>> {
                static_assert(sizeof...(WrapTypes) == sizeof...(ErrorTypes));
                static_assert(type_traits::is_match_template_v<std::tuple, std::decay_t<ResultTuple>>);
                static_assert(is_result_type_v<decltype(MergePolicy::merge(std::declval<Result<WrapType1, ErrorType1>>(), std::declval<Result<WrapType2, ErrorType2>>()))>);
                static auto sequence(ResultTuple results) {
                    constexpr auto current_index = std::tuple_size_v<std::decay_t<ResultTuple>> - sizeof...(WrapTypes) - 2;
                    if constexpr (sizeof...(WrapTypes) == 0) {
                        return MergePolicy::merge(std::get<current_index>(std::forward<ResultTuple>(results)), std::get<current_index + 1>(std::forward<ResultTuple>(results))); 
                    } else {
                        auto prev = SequenceHelper<MergePolicy, ResultTuple, std::tuple<WrapType2, WrapTypes...>, std::tuple<ErrorType2, ErrorTypes...>>::sequence(std::forward<ResultTuple>(results));
                        return MergePolicy::merge(std::get<current_index>(std::forward<ResultTuple>(results)), std::move(prev));
                    }
                }
            };
        }

        template <typename MergePolicy = DefaultMergePolicy, typename... Ts, typename... Errs>
        auto sequence(const std::tuple<Result<Ts, Errs>...>& results) { return detail::SequenceHelper<MergePolicy, const std::tuple<Result<Ts, Errs>...>, std::tuple<Ts...>, std::tuple<Errs...>>::sequence(results); }

        template <typename MergePolicy = DefaultMergePolicy, typename... Ts, typename... Errs>
        auto sequence(std::tuple<Result<Ts, Errs>...>&& results) { return detail::SequenceHelper<MergePolicy, std::tuple<Result<Ts, Errs>...>&&, std::tuple<Ts...>, std::tuple<Errs...>>::sequence(std::move(results)); }

        template <typename ErrorMergePolicy = DefaultMergePolicy, typename... Results>
        auto sequence(Results... results) { return sequence(std::tuple(std::forward<Results>(results)...)); }
    }
}

#endif

