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
 * Methods.h                                              *
 *                                                        *
 * hprose Methods for cpp.                                *
 *                                                        *
 * LastModified: May 02, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/rpc/Method.h>

#include <mutex>

namespace hprose {
namespace rpc {

class Methods {
public:
    void addMethod(const std::string &aliasName, Method &&method) {
        std::lock_guard<std::mutex> guard(mutex);
        methods[aliasName] = method;
    }

    Method *getMethod(const std::string &aliasName) {
        std::lock_guard<std::mutex> guard(mutex);
        if (methods.find(aliasName) == methods.end()) {
            return nullptr;
        }
        return &methods[aliasName];
    }

    void invoke(const std::string &aliasName,
        ServiceContext &context) {
        std::unique_lock<std::mutex> lock(mutex);
        const Method &method = methods[aliasName];
        lock.unlock();
        method.invoke(context);
    }
private:
    mutable std::mutex mutex;
    std::unordered_map<std::string, Method> methods;
};

}
} // namespace hprose.rpc