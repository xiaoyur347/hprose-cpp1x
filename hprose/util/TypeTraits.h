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
#include <functional>

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
struct function_traits_impl<R(&)(Args...)> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename R, typename... Args>
struct function_traits_impl<std::function<R(Args...)>> {
    typedef R ret_type;
    typedef std::tuple<Args...> arg_type;
};

template<typename FUNC>
struct function_traits {
    typedef typename function_traits_impl<FUNC>::ret_type ret_type;
    typedef typename function_traits_impl<FUNC>::arg_type arg_type;
};