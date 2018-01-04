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
        //a type for merging Result class, this type is used when we use DefaultMergePolicy
        template <typename... Ts>
        struct MergeResult {
            std::tuple<Ts...> tuple;
            constexpr MergeResult(const std::tuple<Ts...>& tuple) noexcept(std::is_nothrow_copy_constructible_v<std::tuple<Ts...>>): tuple(tuple){}
            constexpr MergeResult(std::tuple<Ts...>&& tuple) noexcept(std::is_nothrow_move_constructible_v<std::tuple<Ts...>>): tuple(std::move(tuple)){}
            MergeResult() = default;
        };
        //deducation guide
        template <typename... Ts>
        MergeResult(const std::tuple<Ts...>&)->MergeResult<Ts...>;
        template <typename... Ts>
        MergeResult(std::tuple<Ts...>&&)->MergeResult<Ts...>;
        //meta function which checks if the type is MergeResult<Ts...> class
        template <typename T>
        using is_merge_result = type_traits::is_match_template<MergeResult, T>;
        template <typename T>
        constexpr bool is_merge_result_v = is_merge_result<T>::value;

        template <typename... MergeResults>
        constexpr auto mergeResultCat(MergeResults&&... mergeResults)
            -> std::enable_if_t<(is_merge_result_v<std::decay_t<MergeResults>> && ...), decltype(MergeResult(std::tuple_cat(std::declval<MergeResults>().tuple...)))> {
            return std::tuple_cat(std::forward<MergeResults>(mergeResults).tuple...);
        }

        struct DefaultMergePolicy {
        private:
            template <typename WrapType1, typename WrapType2>
            static constexpr auto merge_helper(WrapType1&& wrap1, WrapType2&& wrap2) {
                using wrap_type1 = std::decay_t<WrapType1>;
                using wrap_type2 = std::decay_t<WrapType2>;
                if constexpr (is_merge_result_v<wrap_type1> && is_merge_result_v<wrap_type2>) {
                    return mergeResultCat(std::forward<WrapType1>(wrap1), std::forward<WrapType2>(wrap2));
                } else if constexpr (is_merge_result_v<wrap_type1> && !is_merge_result_v<wrap_type2>) {
                    return mergeResultCat(std::forward<WrapType1>(wrap1), MergeResult(std::tuple(std::forward<WrapType2>(wrap2))));
                } else if constexpr (!is_merge_result_v<wrap_type1> && is_merge_result_v<wrap_type2>) {
                    return mergeResultCat(MergeResult(std::tuple(std::forward<WrapType1>(wrap1))), std::forward<WrapType2>(wrap2));
                } else {
                    return MergeResult(std::tuple(std::forward<WrapType1>(wrap1), std::forward<WrapType2>(wrap2)));
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
                using ok_wrap_type1 = typename std::decay_t<Result1>::ok_wrap_type;
                using err_wrap_type1 = typename std::decay_t<Result1>::error_wrap_type;
                using ok_wrap_type2 = typename std::decay_t<Result2>::ok_wrap_type;
                using err_wrap_type2 = typename std::decay_t<Result2>::error_wrap_type;
                using result_type = Result<decltype(merge_helper(std::declval<ok_wrap_type1>(), std::declval<ok_wrap_type2>())), decltype(merge_helper(std::declval<err_wrap_type1>(), std::declval<err_wrap_type2>()))>;
                if(result1 && result2) {
                    return result_type(Ok(merge_helper(std::forward<Result1>(result1).get_ok(), std::forward<Result2>(result2).get_ok())));
                } else if(result1 && !result2) {
                    return result_type(Err(merge_helper(DefaultErrorValue<err_wrap_type1>::value, std::forward<Result2>(result2).get_err())));
                } else if(!result1 && result2) {
                    return result_type(Err(merge_helper(std::forward<Result1>(result1).get_err(), DefaultErrorValue<err_wrap_type2>::value)));
                } else {
                    return result_type(Err(merge_helper(std::forward<Result1>(result1).get_err(), std::forward<Result2>(result2).get_err())));
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

