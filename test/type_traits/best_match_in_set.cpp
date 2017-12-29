#include <iterator>
#include <type_traits>
#include <list>
#include <vector>
#include <hrlib/type_traits/best_match_in_set.hpp>

using namespace hrlib;

int main(){
    static_assert(std::is_same_v<
                                 std::random_access_iterator_tag,
                                 type_traits::best_match_in_set_t<
				                     typename std::iterator_traits<std::vector<int>::iterator>::iterator_category,
				                     std::input_iterator_tag,
				                     std::bidirectional_iterator_tag,
				                     std::random_access_iterator_tag
				 >>);
    static_assert(std::is_same_v<
                                 std::bidirectional_iterator_tag,
                                 type_traits::best_match_in_set_t<
        				             typename std::iterator_traits<std::list<int>::iterator>::iterator_category,
                				     std::input_iterator_tag,
                				     std::bidirectional_iterator_tag,
                				     std::random_access_iterator_tag
				 >>);

   return 0;
}

