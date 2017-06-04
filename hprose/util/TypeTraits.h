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
 * hprose/util/TypeTraits.h                               *
 *                                                        *
 * type_traits for cpp.                                   *
 *                                                        *
 * LastModified: Jun 4, 2017                              *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/util/Tuple.h>
#include <cstddef>
#include <functional>

template<typename T>
struct tuple_remove_reference {
    typedef typename std::remove_reference<T>::type r_type;
    typedef typename std::remove_cv<r_type>::type type;
};

template<typename T, std::size_t SIZE>
struct tuple_remove_reference<T[SIZE]> {
    typedef typename tuple_remove_reference<const T *>::type type;
};

template<typename TUPLE>
struct remove_reference_tuple {
    static const std::size_t size_ = std::tuple_size<TUPLE>::value;

    template<size_t SIZE, std::size_t... I>
    struct converted {
        typedef std::tuple<typename tuple_remove_reference<typename std::tuple_element<I, TUPLE>::type>::type...> type;
    };

    template<std::size_t... I>
    static converted<size_, I...> get_type(const std::index_sequence<I...> &) {
        return converted<size_, I...>();
    }

    typedef decltype(get_type(std::make_index_sequence<size_>())) converted_type;
    typedef typename converted_type::type type;
};

template<typename T>
struct function_traits_impl;

template<typename R, typename T, typename... Args>
struct function_traits_impl<R(T::*)(Args...)> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename R, typename T, typename... Args>
struct function_traits_impl<R(T::*)(Args...) const> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename R, typename... Args>
struct function_traits_impl<R(*)(Args...)> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename R, typename... Args>
struct function_traits_impl<std::function<R(Args...)>> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename FUNC>
class function_traits {
    typedef typename std::decay<FUNC>::type decay_type;
public:
    typedef typename function_traits_impl<decay_type>::ret_type ret_type;
    typedef typename remove_reference_tuple<typename function_traits_impl<decay_type>::arg_type>::type arg_type;
};