#ifndef HRLIB_TYPE_TRAITS_IDENTITY
#define HRLIB_TYPE_TRAITS_IDENTITY

namespace hrlib::type_traits {
    template <typename T>
    struct identity {
        using type = T;
    };

    template <auto X>
    struct identity_value {
        static constexpr auto value = X;
    };
}

#endif

