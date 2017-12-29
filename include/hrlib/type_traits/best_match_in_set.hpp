#ifndef HRLIB_TYPE_TRAITS_BEST_MATCH_IN_SET
#define HRLIB_TYPE_TRAITS_BEST_MATCH_IN_SET

#include "identity.hpp"

namespace hrlib::type_traits {
    namespace detail { 
        template <typename ...Types>
        struct match_overloads: match_overloads<Types>... {
            using match_overloads<Types>::match...;
        };
        
        template <>
        struct match_overloads<>{};
        
        template <typename T>
        struct match_overloads<T> {
            static identity<T> match(identity<T>);
        };
    }
    
    template <typename T, typename... Types>
    struct best_match_in_set {
        using type = typename decltype(detail::match_overloads<Types...>::match(identity<T>{}))::type;
    };
    
    template <typename T, typename... Types>
    using best_match_in_set_t = typename best_match_in_set<T, Types...>::type;
}

#endif

