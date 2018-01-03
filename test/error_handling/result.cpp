#define BOOST_TEST_NO_LIB
#define BOOST_TEST_MAIN

#include <hrlib/error_handling/result.hpp>
#include <string>
#include <boost/test/unit_test.hpp>

using namespace hrlib;
using namespace hrlib::error_handling;

BOOST_AUTO_TEST_SUITE(result_test)
    BOOST_AUTO_TEST_CASE(result_methods) {
        auto result1 = Result<int>(result::Ok(1));
        BOOST_CHECK(result1);
        BOOST_CHECK_EQUAL(result1.get_ok(), 1);
        result1.get_ok() = 2;
        BOOST_CHECK_EQUAL(result1.get_ok(), 2);
        BOOST_CHECK_EQUAL(result1.ok_or([](){ return 3; }), 2);
        static_assert(std::is_same_v<decltype(result1.get_ok()), int&>);
        static_assert(std::is_same_v<decltype(std::move(result1).get_ok()), int&&>);
        BOOST_CHECK_EQUAL(result1.map([](int x){ return x + 1; }).get_ok(), 3);
        BOOST_CHECK_EQUAL(result1.map([](int x){ return std::to_string(x + 1); })
                                 .map([](std::string&& x){ return std::move(x) + "3"; })
                                 .get_ok(), 
                          std::string("33"));
        auto result2 = result1.flat_map([](int x){ return x > 0 ? Result<bool>(result::Ok(true)) : Result<bool>(result::Err(std::string("err"))); });
        BOOST_CHECK(result2);
        BOOST_CHECK_EQUAL(result2.get_ok(), true);
        auto result3 = result1.flat_map([](int x){ return x < 0 ? Result<bool>(result::Ok(true)) : Result<bool>(result::Err(std::string("err"))); });
        BOOST_CHECK(!result3);
        BOOST_CHECK_EQUAL(result3.get_err(), std::string("err"));
        BOOST_CHECK_EXCEPTION(result3.get_ok(),
                              std::bad_variant_access,
                              [](const std::bad_variant_access&){return true;});

        BOOST_CHECK_EQUAL(result1.map([](int x){ return x + 1; }).flat_map([](int x){ return Result<int>(result::Ok(10)); }).get_ok(), 10);

        BOOST_CHECK_EQUAL(result3.ok_or([](){ return false; }), false);
        auto x = result1.match([](auto&& val){
                using T = std::decay_t<decltype(val)>;
                if constexpr (type_traits::is_match_template_v<result::Ok, T>) {
                    return std::to_string(val.data);
                } else {
                    return val.err;
                }
        });
        BOOST_CHECK_EQUAL(x, std::string("2"));

        auto y = result3.match([](auto&& val){
                using T = std::decay_t<decltype(val)>;
                if constexpr (type_traits::is_match_template_v<result::Ok, T>) {
                    return std::to_string(val.data);
                } else {
                    return val.err;
                }
        });
        BOOST_CHECK_EQUAL(y, std::string("err"));
    }
BOOST_AUTO_TEST_SUITE_END()

