#include <type_traits>
#include <tuple>
#include <iostream>
#include <cassert>
#include <hrlib/static_graph/static_graph.hpp>
#include <boost/type_index.hpp>

template <typename Node, std::size_t N>
constexpr std::pair<bool, std::size_t> constexpr_dfs_impl(Node* node, const int (&ans)[N], std::size_t index);
template <std::size_t I = 0, std::size_t N, typename... Ns>
constexpr std::pair<bool, std::size_t> constexpr_dfs_impl(std::tuple<Ns...> nodes, const int (&ans)[N], std::size_t index);
template <std::size_t N>
constexpr std::pair<bool, std::size_t> constexpr_dfs_impl(const hrlib::static_graph::terminal_node*, const int (&ans)[N], std::size_t index) { return std::make_pair(true, index) ;}

template <typename Node, std::size_t N>
constexpr std::pair<bool, std::size_t> constexpr_dfs_impl(Node* node, const int (&ans)[N], std::size_t index)
{
  if (node->is_visited)
    return std::make_pair(true, index);
  node->is_visited = true;
  if (node->value != ans[index])
    return std::make_pair(false, 0);
  else
    return constexpr_dfs_impl(node->next, ans, index + 1);
}
template <std::size_t I = 0, std::size_t N, typename... Ns>
constexpr std::pair<bool, std::size_t> constexpr_dfs_impl(std::tuple<Ns...> nodes, const int (&ans)[N], std::size_t index)
{
  if (std::get<I>(nodes)->value != ans[index])
    return std::make_pair(false, 0);
  auto res = constexpr_dfs_impl(std::get<I>(nodes)->next, ans, index + 1);
  if (!res.first)
    return std::make_pair(false, 0);
  
  if constexpr (I < sizeof...(Ns) - 1)
  {
    return constexpr_dfs_impl<I + 1>(nodes, ans, res.second);
  }
  else
    return res;
}

template <typename Root>
constexpr bool constexpr_dfs(Root root, const int (&ans)[Root::node_num])
{
  root.construct_connection();
  return constexpr_dfs_impl(hrlib::static_graph::node_heads<Root>::get_head_pointers(root), ans, 0).first;
}

template <typename Next = hrlib::static_graph::terminal_node>
struct inner_node: hrlib::static_graph::single_node_base<Next>
{
  using base_type = hrlib::static_graph::single_node_base<Next>;
  bool is_visited = false;
  template <typename T>
  using this_template = inner_node<T>;
  template <typename NextNodeType>
  constexpr this_template<NextNodeType> copy() const { return this_template<NextNodeType>{phrase, value}; }
  const char* phrase = "<null>";
  int value = 0;
  inner_node() = default;
  constexpr inner_node(const char* phrase): base_type(), phrase(phrase) {}
  constexpr inner_node(int val): base_type(), value(val) {}
  constexpr inner_node(const char* phrase, int val): base_type(), phrase(phrase), value(val) {}
  static constexpr std::size_t node_num = 1;
  inner_node(const inner_node&) = default;
  inner_node(inner_node&&) = default;
  ~inner_node() = default;
};

int main ()
{
  using namespace hrlib::static_graph;
  constexpr auto n1 = inner_node<>{1} + (inner_node<>{2} + inner_node<>{3});
  constexpr auto n2 = (inner_node<>{6} + inner_node<>{5}) + inner_node<>{4};
  constexpr auto n3 = (inner_node<>{7} + inner_node<>{8}) + (inner_node<>{9} | inner_node<>{10});
  constexpr auto n4 = n1 + (inner_node<>{11} | n3 | inner_node{12}) + n2;
  static constexpr int ans[std::remove_const_t<decltype(n4)>::node_num] = {1, 2, 3, 11, 6, 5, 4, 7, 8, 9, 10, 12};
  static constexpr int ans1[std::remove_const_t<decltype(n4)>::node_num] = {1, 2, 3, 11, 6, 5, 4, 7, 8, 9, 10, 11};
  static_assert(constexpr_dfs(n4, ans));
  static_assert(!constexpr_dfs(n4, ans1));
  static_assert(is_combinable_node_v<std::remove_cv_t<decltype(n1)>>);
  static_assert(is_combinable_node_v<std::remove_cv_t<decltype(n4)>>);
  static_assert(is_combinable_node_v<inner_node<>>);
  static_assert(!is_combinable_node_v<const char*>);
  static_assert(!is_combinable_node_v<decltype("abcd")>);
  static_assert(is_combinable_single_node_v<inner_node<>>);
  static_assert(!is_combinable_single_node_v<decltype("aaa")>);
  constexpr auto n5 = (inner_node<>{11} | n3 | inner_node{12}) + n2;
  static constexpr int ans2[std::remove_const_t<decltype(n5)>::node_num] = {11, 6, 5, 4, 7, 8, 9, 10, 12};
  static constexpr int ans3[std::remove_const_t<decltype(n5)>::node_num] = {11, 66, 5, 4, 7, 8, 9, 10, 12};
  static_assert(constexpr_dfs(n5, ans2));
  static_assert(!constexpr_dfs(n5, ans3));
  return 0;
}

