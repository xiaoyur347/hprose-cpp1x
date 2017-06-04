/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * hprose/util/Apply.h                                    *
 *                                                        *
 * apply tuple for cpp.                                   *
 *                                                        *
 * LastModified: May 29, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/util/Tuple.h>
#include <hprose/util/TypeTraits.h>
#include <functional>
#include <type_traits>

namespace hprose
{
namespace util
{

//https://github.com/tomaszkam/proposals/blob/master/invoke/type_traits.hpp
template<typename T>
struct target_type
{
    typedef void type;
};

template<typename Class, typename Member>
struct target_type<Member Class::*>
{
    typedef Class type;
};

template<typename T>
class object_type
{
    typedef typename std::decay<T>::type remove_cv_type;
public:
    typedef typename std::remove_pointer<remove_cv_type>::type type;
};

template<typename Object, typename Pointer>
struct is_target_type :
    public std::integral_constant<
        bool,
        std::is_base_of<
            typename target_type<Pointer>::type,
            typename object_type<Object>::type
        >::value
    >
{};

template<typename F, typename Tuple, std::size_t... I>
inline auto apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
    -> typename function_traits<F>::ret_type
{
    auto functor = std::bind(std::forward<F>(f),
        std::get<I>(std::forward<Tuple>(t))...);
    return functor();
}

template<typename F, typename Tuple>
auto apply(F&& f, Tuple&& t)
    ->  typename std::enable_if<
            !is_target_type<
                Tuple,
                F
            >::value,
            decltype(apply_impl(std::forward<F>(f),
                std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>()))
        >::type
{
    return apply_impl(std::forward<F>(f),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>());
}

template<typename F, typename Object, typename Tuple, std::size_t... I>
inline auto apply_impl(F&& f, Object&& object, Tuple&& t, std::index_sequence<I...>)
    -> typename function_traits<F>::ret_type
{
    (void)t;
    auto functor = std::bind(std::forward<F>(f),
        std::forward<Object>(object),
        std::get<I>(std::forward<Tuple>(t))...);
    return functor();
}

template<typename F, typename Object, typename Tuple>
auto apply(F&& f, Object&& object, Tuple&& t)
    -> decltype(apply_impl(std::forward<F>(f),
        std::forward<Object>(object),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>()))
{
    return apply_impl(std::forward<F>(f),
        std::forward<Object>(object),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>());
}

template<typename F, typename Object>
auto apply(F&& f, Object&& object)
    ->  typename std::enable_if<
            is_target_type<
                Object,
                F
            >::value,
            typename function_traits<F>::ret_type
        >::type
{
    auto functor = std::bind(std::forward<F>(f),
        std::forward<Object>(object));
    return functor();
}

}
} // namespace hprose::util