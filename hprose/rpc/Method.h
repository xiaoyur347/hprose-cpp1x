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
 * Method.h                                               *
 *                                                        *
 * hprose Method for cpp.                                 *
 *                                                        *
 * LastModified: May 02, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/util/Apply.h>
#include <hprose/rpc/ResultMode.h>
#include <hprose/rpc/ServiceContext.h>

#include <string>

namespace hprose {
namespace rpc {

class Method {
public:
    typedef std::function<void(ServiceContext &context)> task;

    task method;
    ResultMode mode;
    bool simple;
    bool oneway;

    Method()
        : mode(Normal),
          simple(true),
          oneway(false) {

    }

    Method(task method,
        ResultMode mode = Normal,
        bool simple = true,
        bool oneway = false)
        : method(method),
          mode(mode),
          simple(simple),
          oneway(oneway) {
    }

    void invoke(ServiceContext &context) const {
        method(context);
    }
};

template<typename F>
    typename std::enable_if<
        !std::is_void<typename function_traits<F>::ret_type>::value,
        Method
    >::type
makeMethod(F&&f) {
    typedef typename function_traits<F>::arg_type arg_type;
    auto fun = [f](hprose::rpc::ServiceContext &context) {
        arg_type s;
        context.reader.unserialize(s);
        auto ret = hprose::util::apply(f, s);
        context.writer.serialize(ret);
    };
    return Method(fun);
}

template<typename F, typename Object>
    typename std::enable_if<
        !std::is_void<typename function_traits<F>::ret_type>::value,
        Method
    >::type
makeMethod(F&&f, Object&& object) {
    typedef typename function_traits<F>::arg_type arg_type;
    auto fun = [f, object](hprose::rpc::ServiceContext &context) {
        arg_type s;
        context.reader.unserialize(s);
        auto ret = hprose::util::apply(f, object, s);
        context.writer.serialize(ret);
    };
    return Method(fun);
}

template<typename F>
    typename std::enable_if<
        std::is_void<typename function_traits<F>::ret_type>::value,
        Method
    >::type
makeMethod(F&&f) {
    typedef typename function_traits<F>::arg_type arg_type;
    auto fun = [f](hprose::rpc::ServiceContext &context) {
        arg_type s;
        context.reader.unserialize(s);
        hprose::util::apply(f, s);
        context.writer.serialize(nullptr);
    };
    return Method(fun);
}

template<typename F, typename Object>
    typename std::enable_if<
        std::is_void<typename function_traits<F>::ret_type>::value,
        Method
    >::type
makeMethod(F&&f, Object&& object) {
    typedef typename function_traits<F>::arg_type arg_type;
    auto fun = [f, object](hprose::rpc::ServiceContext &context) {
        arg_type s;
        context.reader.unserialize(s);
        hprose::util::apply(f, object, s);
        context.writer.serialize(nullptr);
    };
    return Method(fun);
}

}
} // namespace hprose.rpc