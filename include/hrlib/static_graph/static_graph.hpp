#ifndef HRLIB_STATIC_GRAPH_STATIC_GRAPH
#define HRLIB_STATIC_GRAPH_STATIC_GRAPH

#include <utility>
#include <type_traits>
#include <tuple>
#include <cassert>
#include <hrlib/type_traits/type_traits.hpp>

namespace hrlib::static_graph
{
  template <typename... Ts>
  struct type_list {};
  
  namespace type_list_meta
  {
    template <typename>
    struct to_tuple;
    template <typename... Ts>
    struct to_tuple<type_list<Ts...>>
    {
      using type = std::tuple<Ts...>;
    };
    template <typename TypeList>
    using to_tuple_t = typename to_tuple<TypeList>::type;
  }

  struct single_node_tag{};
  struct chained_node_tag{};
  struct or_node_tag{};

  struct terminal_node { static constexpr bool is_visited = true; };
  static constexpr auto terminal = terminal_node{};

  namespace detail
  {
    template <typename Node, typename Enabler = void>
    struct single_node_constraints_impl: std::false_type {};
    template <typename Node>
    struct single_node_constraints_impl<
      Node,
      std::void_t<
        std::enable_if_t<std::is_same_v<std::decay_t<Node>::node_type_tag, single_node_tag>>,
        decltype(std::decay_t<Node>::make_chained(std::declval<std::tuple<std::decay_t<Node>>>())),
        decltype(std::decay_t<Node>::make_or(std::declval<std::tuple<std::decay_t<Node>>>())),
        decltype(std::declval<Node>().template copy<terminal_node>()),
        decltype(std::declval<Node>().next),
        decltype(std::declval<Node>().construct_connection())
      >
    >: std::true_type {};

    template <typename Node, typename Enabler = void>
    struct chained_node_constraints_impl: std::false_type {};
    template <typename Node>
    struct chained_node_constraints_impl<
      Node,
      std::void_t<
        std::enable_if_t<std::is_same_v<typename std::decay_t<Node>::node_type_tag, chained_node_tag>>,
        typename std::decay_t<Node>::head_type,
        typename std::decay_t<Node>::last_type,
        decltype(std::declval<Node>().template copy<terminal_node>()),
        decltype(std::declval<Node>().get_nodes()),
        decltype(std::declval<Node>().construct_connection())
      >
    >: std::true_type {};

    template <typename Node, typename Enabler = void>
    struct or_node_constraints_impl: std::false_type {};
    template <typename Node>
    struct or_node_constraints_impl<
      Node,
      std::void_t<
        std::enable_if_t<std::is_same_v<typename std::decay_t<Node>::node_type_tag, or_node_tag>>,
        typename std::decay_t<Node>::head_type,
        typename std::decay_t<Node>::last_type,
        decltype(std::declval<Node>().template copy<terminal_node>()),
        decltype(std::declval<Node>().get_nodes()),
        decltype(std::declval<Node>().construct_connection())
      >
    >: std::true_type {};

    template <typename Node, typename NodeTypeTag = typename Node::node_type_tag>
    struct node_heads_impl;
    template <typename Tpl>
    struct or_node_heads_helper;

    template <typename... Ns>
    struct or_node_heads_helper<std::tuple<Ns...>>
    {
      using type = type_traits::concat_t<typename node_heads_impl<Ns>::type...>;
      template <std::size_t... Is>
      static constexpr auto get_head_pointers_impl(std::tuple<Ns...>& nodes, std::index_sequence<Is...>)
      {
        return std::tuple_cat(node_heads_impl<Ns>::get_head_pointers(std::get<Is>(nodes))...);
      }
    };
    template <typename SingleNode>
    struct node_heads_impl<SingleNode, single_node_tag>
    {
      using type = type_list<SingleNode>;
      static constexpr auto get_head_pointers(SingleNode& node) { return std::make_tuple(&node); }
    };
    template <typename ChainedNode>
    struct node_heads_impl<ChainedNode, chained_node_tag>
    {
      using type = typename node_heads_impl<typename ChainedNode::head_type>::type;
      static constexpr auto get_head_pointers(ChainedNode& chained_node)
      {
        return node_heads_impl<typename ChainedNode::head_type>::get_head_pointers(
          std::get<0>(chained_node.get_nodes())
        );
      }
    };
    template <typename OrNode>
    struct node_heads_impl<OrNode, or_node_tag>
      : or_node_heads_helper<std::decay_t<decltype(std::declval<OrNode>().get_nodes())>>
    {
      using tuple_type = std::decay_t<decltype(std::declval<OrNode>().get_nodes())>;
      using base_type = or_node_heads_helper<tuple_type>;
      static constexpr auto get_head_pointers(OrNode& or_node)
      {
        return base_type::get_head_pointers_impl(
          or_node.get_nodes(),
          std::make_index_sequence<
            std::tuple_size_v<tuple_type>
          >{}
        );
      }
    };

    template <typename Node, typename NodeTypeTag = typename Node::node_type_tag>
    struct node_lasts_impl;
    template <typename Tpl>
    struct or_node_lasts_helper;
    template <typename... Ns>
    struct or_node_lasts_helper<std::tuple<Ns...>>
    {
      using type = type_traits::concat_t<typename node_lasts_impl<Ns>::type...>;
      template <std::size_t... Is>
      static constexpr auto get_last_pointers_impl(std::tuple<Ns...>& nodes, std::index_sequence<Is...>)
      {
        return std::tuple_cat(node_lasts_impl<Ns>::get_last_pointers(std::get<Is>(nodes))...);
      }
    };
    
    template <typename SingleNode>
    struct node_lasts_impl<SingleNode, single_node_tag>
    {
      using type = type_list<SingleNode>;
      static constexpr auto get_last_pointers(SingleNode& node) { return std::make_tuple(&node); }
    };
    template <typename ChainedNode>
    struct node_lasts_impl<ChainedNode, chained_node_tag>
    {
      using type = typename node_lasts_impl<typename ChainedNode::last_type>::type;
      static constexpr auto get_last_pointers(ChainedNode& chained_node)
      {
        using tuple_type = std::decay_t<decltype(std::declval<ChainedNode>().get_nodes())>;
        return node_lasts_impl<typename ChainedNode::last_type>::get_last_pointers(
          std::get<std::tuple_size_v<tuple_type> - 1>(chained_node.get_nodes())
        );
      }
    };
    template <typename OrNode>
    struct node_lasts_impl<OrNode, or_node_tag>
      : or_node_lasts_helper<std::decay_t<decltype(std::declval<OrNode>().get_nodes())>>
    {
      using tuple_type = std::decay_t<decltype(std::declval<OrNode>().get_nodes())>;
      using base_type = or_node_lasts_helper<tuple_type>;
      static constexpr auto get_last_pointers(OrNode& or_node)
      {
        return base_type::get_last_pointers_impl(
          or_node.get_nodes(),
          std::make_index_sequence<
            std::tuple_size_v<tuple_type>
          >{}
        );
      }
    };
  }

  template <typename Node>
  struct single_node_constraints: detail::single_node_constraints_impl<Node> {};
  template <typename Node>
  struct chained_node_constraints: detail::chained_node_constraints_impl<Node> {};
  template <typename Node>
  struct or_node_constraints: detail::or_node_constraints_impl<Node> {};

  template <typename, typename, typename...>
  struct chained_node;
  template <typename, typename, typename...>
  struct or_node;

  template <typename Node>
  struct node_heads: detail::node_heads_impl<Node> {};
  template <typename Node>
  using node_heads_t = typename node_heads<Node>::type;

  template <typename Node>
  struct node_lasts: detail::node_lasts_impl<Node> {};
  template <typename Node>
  using node_lasts_t = typename node_lasts<Node>::type;

  template <typename NextNodeType>
  struct next_node
  {
    using next_hold_type = std::add_pointer_t<NextNodeType>;
    next_hold_type next{};
  };
  template <typename... Ts>
  struct next_node<std::tuple<Ts...>>
  {
    using next_hold_type = std::tuple<std::add_pointer_t<Ts>...>;
    next_hold_type next{};
  };
  template <>
  struct next_node<terminal_node>
  {
    using next_hold_type = const terminal_node*;
    next_hold_type next = &terminal;
  };

  template <typename N1, typename N2, typename... Ns>
  struct chained_node
  {
    using node_type_tag = chained_node_tag;
    using head_type = N1;
    using last_type = typename std::conditional_t<sizeof...(Ns) == 0, type_traits::identity<N2>, type_traits::last<type_list<Ns...>>>::type;
    using content_type = std::tuple<N1, N2, Ns...>;
    static constexpr auto chain_size = sizeof...(Ns) + 2;
    content_type ns;
    constexpr chained_node() = default;
    constexpr chained_node(const content_type& ns): ns(ns) {}
    constexpr chained_node(content_type&& ns): ns(std::move(ns)) {}
    constexpr chained_node(const chained_node&) = default;
    constexpr chained_node(chained_node&&) = default;
    ~chained_node() = default;
    static constexpr std::size_t node_num = N1::node_num + N2::node_num + (Ns::node_num + ... + 0);
  private:
    template <typename NextNodeType, typename Node1, typename Node2, typename... Nodes>
    static constexpr auto merge(Node1&& node1, Node2&& node2, Nodes&&... nodes)
    {
      if constexpr (sizeof...(Nodes) == 0)
        return merge<NextNodeType>(std::forward<Node1>(node1), std::forward<Node2>(node2));
      else
        return merge<NextNodeType>(std::forward<Node1>(node1), merge<NextNodeType>(std::forward<Node2>(node2), std::forward<Nodes>(nodes)...));
    }
    template <typename NextNodeType, typename Node1, typename Node2>
    static constexpr auto merge(Node1&& node1, Node2&& node2)
    {
      using Node2Type = std::decay_t<Node2>;
      if constexpr (type_traits::is_match_template_v<std::tuple, Node2Type>)
      {
        using Node1NextNodeTypeTuple = type_list_meta::to_tuple_t<node_heads_t<std::tuple_element_t<0, Node2Type>>>;
        using Node1NextNodeType = std::conditional_t<std::tuple_size_v<Node1NextNodeTypeTuple> == 1, std::tuple_element_t<0, Node1NextNodeTypeTuple>, Node1NextNodeTypeTuple>;
        return std::tuple_cat(std::make_tuple(node1.template copy<Node1NextNodeType>()), std::forward<Node2>(node2));
      }
      else
      {
        using Node1NextNodeTypeTuple = type_list_meta::to_tuple_t<node_heads_t<decltype(node2.template copy<NextNodeType>())>>;
        using Node1NextNodeType = std::conditional_t<std::tuple_size_v<Node1NextNodeTypeTuple> == 1, std::tuple_element_t<0, Node1NextNodeTypeTuple>, Node1NextNodeTypeTuple>;
        return std::make_tuple(node1.template copy<Node1NextNodeType>(), node2.template copy<NextNodeType>());
      }
    }
    template <typename... Nodes>
    static constexpr auto make(const std::tuple<Nodes...>& nodes) { return chained_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static constexpr auto make(std::tuple<Nodes...>&& nodes) { return chained_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    constexpr auto copy_impl(std::index_sequence<Is...>) const { return merge<NextNodeType>(std::get<Is>(ns)...); }
    template <std::size_t I, typename Tuple1, typename Tuple2>
    static constexpr void set_tuple_helper(Tuple1&& t1, Tuple2&& t2)
    {
      std::get<I>(t1) = std::get<I>(t2);
      if constexpr (I < std::tuple_size_v<std::decay_t<Tuple1>> - 1)
        set_tuple_helper<I + 1>(t1, t2);
    }
    template <std::size_t I, typename Tuple1, typename Tuple2>
    static constexpr void set_next_node_helper(Tuple1&& t1, Tuple2&& t2)
    {
      if constexpr (I < std::tuple_size_v<Tuple1>)
      {
        if constexpr (std::tuple_size_v<std::decay_t<Tuple2>> == 1)
          std::get<I>(t1)->next = std::get<0>(t2);
        else
          set_tuple_helper<0>(std::get<I>(t1)->next, t2);
//          std::get<I>(t1)->next = t2;
        set_next_node_helper<I + 1>(std::forward<Tuple1>(t1), std::forward<Tuple2>(t2));
      }
    }
    template <std::size_t I>
    constexpr void construct_connection_impl()
    {
      std::get<I>(ns).construct_connection();
      if constexpr (I < sizeof...(Ns) + 1)
      {
        set_next_node_helper<0>(node_lasts<std::tuple_element_t<I, decltype(ns)>>::get_last_pointers(std::get<I>(ns)), node_heads<std::tuple_element_t<I + 1, decltype(ns)>>::get_head_pointers(std::get<I + 1>(ns)));
        construct_connection_impl<I + 1>();
      }
    }
  public:
    constexpr content_type& get_nodes() & { return ns; }
    constexpr const content_type& get_nodes() const & { return ns; }
    constexpr content_type&& get_nodes() && { return std::move(ns); }
    template <typename NextNodeType>
    constexpr auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    constexpr void construct_connection() { construct_connection_impl<0>(); }
  };

  template <typename N1, typename N2, typename... Ns>
  struct or_node
  {
    using node_type_tag = or_node_tag;
    using content_type = std::tuple<N1, N2, Ns...>;
    content_type ns;
    constexpr or_node() = default;
    constexpr or_node(const std::tuple<N1, N2, Ns...>& ns): ns(ns) {}
    constexpr or_node(std::tuple<N1, N2, Ns...>&& ns): ns(std::move(ns)) {}
    constexpr or_node(const or_node&) = default;
    constexpr or_node(or_node&&) = default;
    ~or_node() = default;
    static constexpr std::size_t node_num = N1::node_num + N2::node_num + (Ns::node_num + ... + 0);
  private:
    template <typename... Nodes>
    static constexpr auto make(const std::tuple<Nodes...>& nodes) { return or_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static constexpr auto make(std::tuple<Nodes...>&& nodes) { return or_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    constexpr auto copy_impl(std::index_sequence<Is...>) const { return std::make_tuple(std::get<Is>(ns).template copy<NextNodeType>()...); }
    template <std::size_t I>
    constexpr void construct_connection_impl()
    {
      if constexpr (I == sizeof...(Ns) + 2)
        return;
      else
      {
        std::get<I>(ns).construct_connection();
        construct_connection_impl<I + 1>();
        return;
      }
    }
  public:
    constexpr content_type& get_nodes() & { return ns; }
    constexpr const content_type& get_nodes() const & { return ns; }
    constexpr content_type&& get_nodes() && { return std::move(ns); }
    template <typename NextNodeType>
    constexpr auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    constexpr void construct_connection() { construct_connection_impl<0>(); }
  };

  template <typename NextNode = terminal_node>
  struct single_node_base: next_node<NextNode>
  {
    using node_type_tag = single_node_tag;
    template <typename... Ns>
    static constexpr auto make_chained(const std::tuple<Ns...>& ns) { return chained_node(ns); }
    template <typename... Ns>
    static constexpr auto make_chained(std::tuple<Ns...>&& ns) { return chained_node(std::move(ns)); }
    template <typename... Ns>
    static constexpr auto make_or(const std::tuple<Ns...>& ns) { return or_node(ns); }
    template <typename... Ns>
    static constexpr auto make_or(std::tuple<Ns...>&& ns) { return or_node(std::move(ns)); }
    constexpr void construct_connection() const {}
  };

  template <typename Node, typename Enabler = void>
  struct is_combinable_single_node_impl: std::false_type {};
  template <typename Node>
  struct is_combinable_single_node_impl<
    Node,
    std::void_t<
      decltype(std::declval<Node>().template copy<terminal_node>()),
      decltype(std::declval<Node>().next),
      decltype(std::declval<Node>().construct_connection())
    >
  >: std::true_type{};
  template <typename Node>
  struct is_combinable_single_node: is_combinable_single_node_impl<Node> {};
  template <typename Node>
  constexpr bool is_combinable_single_node_v = is_combinable_single_node<Node>::value;

  template <typename Node>
  struct is_combinable_node:
    std::bool_constant<
      std::disjunction_v<
        is_combinable_single_node<Node>,
        type_traits::is_match_template<chained_node, Node>,
        type_traits::is_match_template<or_node, Node>
      >
    > {};
  template <typename Node>
  constexpr bool is_combinable_node_v = is_combinable_node<Node>::value;

  template <
    typename N1,
    typename N2,
    typename = std::enable_if_t<
      std::conjunction_v<is_combinable_node<std::decay_t<N1>>, is_combinable_node<std::decay_t<N2>>>
    >
  >
  constexpr auto operator+(N1&& n1, N2&& n2)
  {
    using Node1 = std::decay_t<N1>;
    using Node2 = std::decay_t<N2>;
    using Node2Heads = type_list_meta::to_tuple_t<node_heads_t<Node2>>;
    using Node1NewNextNodes = std::conditional_t<std::tuple_size_v<Node2Heads> == 1, std::tuple_element_t<0, Node2Heads>, Node2Heads>;
    auto head_nodes = n1.template copy<Node1NewNextNodes>();
    if constexpr (
      type_traits::is_match_template_v<chained_node, Node1> &&
      type_traits::is_match_template_v<chained_node, Node2>
    )
    { // both nodes are chained node
      return chained_node(std::tuple_cat(std::move(head_nodes).get_nodes(), std::forward<N2>(n2).get_nodes()));
    }
    else if constexpr (
      type_traits::is_match_template_v<chained_node, Node1> &&
      !type_traits::is_match_template_v<chained_node, Node2>
    )
    {
      return chained_node(std::tuple_cat(std::move(head_nodes).get_nodes(), std::make_tuple(std::forward<N2>(n2))));
    }
    else if constexpr (
      !type_traits::is_match_template_v<chained_node, Node1> &&
      type_traits::is_match_template_v<chained_node, Node2>
    )
    {
      return chained_node(std::tuple_cat(std::make_tuple(std::move(head_nodes)), std::forward<N2>(n2).get_nodes()));
    }
    else
    {
      return chained_node(std::make_tuple(std::move(head_nodes), std::forward<N2>(n2)));
    }
  }
  template <
    typename N1,
    typename N2,
    typename = std::enable_if_t<
      std::conjunction_v<is_combinable_node<std::decay_t<N1>>, is_combinable_node<std::decay_t<N2>>>
    >
  >
  constexpr auto operator|(N1&& n1, N2&& n2)
  {
    using Node1 = std::decay_t<N1>;
    using Node2 = std::decay_t<N2>;
    if constexpr (
      type_traits::is_match_template_v<or_node, Node1> &&
      type_traits::is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::forward<N1>(n1).get_nodes(), std::forward<N2>(n2).get_nodes()));
    }
    else if constexpr (
      type_traits::is_match_template_v<or_node, Node1> &&
      !type_traits::is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::forward<N1>(n1).get_nodes(), std::tuple(std::forward<N2>(n2))));
    }
    else if constexpr (
      !type_traits::is_match_template_v<or_node, Node1> &&
      type_traits::is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::tuple(std::forward<N1>(n1)), std::forward<N2>(n2).get_nodes()));
    }
    else
    {
      return or_node(std::tuple(std::forward<N1>(n1), std::forward<N2>(n2)));
    }
  }
}

#endif

