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
 * hprose/util/concurrent/test/PromiseTest.cpp            *
 *                                                        *
 * Promise A+ for cpp.                                    *
 * modified by https://github.com/xhawk18/promise-cpp     *
 *                                                        *
 * LastModified: May 23, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#include <hprose/util/Promise.h>

#include <gtest/gtest.h>

using namespace hprose::util;

TEST(Promise, ResolveAtOnce)
{
    Promise promise = newPromise([](Promise d) {
        d.resolve(1, 2, 3);
    }).then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    });
}

TEST(Promise, ResolveLater)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    });
    promise.resolve(1, 2, 3);
}

TEST(Promise, RejectConstChar)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    });
    promise.reject("reject");
}

TEST(Promise, RejectString)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    });
    promise.reject(std::string("reject"));
}

TEST(Promise, RejectThen)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    }).then([](){
        printf("fail then\n");
    });
    promise.reject(std::string("reject"));
}

TEST(Promise, ResolveFinally)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    }).finally([](){
        printf("finally\n");
    });
    promise.resolve(1, 2, 3);
}

TEST(Promise, RejectFinally)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    }).finally([](){
        printf("finally\n");
    });
    promise.reject(std::string("reject"));
}

TEST(Promise, ResolveFinallyThen)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
    }).then([]() {
        printf("then\n");
    }).fail([](const char *str) {
        printf("fail const char=%s\n", str);
    }).fail([](const std::string &str) {
        printf("fail string=%s\n", str.c_str());
    }).fail([](int i) {
        printf("fail int=%d\n", i);
    }).finally([](){
        printf("finally\n");
    }).then([](){
        printf("finally then\n");
    });
    promise.resolve(1, 2, 3);
}

TEST(Promise, ResolveChain)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
        return "123";
    }).then([](const char* str) {
        printf("resolve %s\n", str);
        return 1;
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return i + 1;
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return i + 1;
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return i + 1;
    });
    promise.resolve(1, 2, 3);
}

TEST(Promise, ResolveChainBroken)
{
    Promise promise = newPromise([](Promise) {
    });
    promise.then([](int a, int b, int c) {
        printf("a=%d,b=%d,c=%d\n", a, b, c);
        return "123";
    }).then([](const char* str) {
        printf("resolve %s\n", str);
        return 1;
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return i + 1;
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return "123";
    }).then([](int i) {
        printf("resolve i=%d\n", i);
        return i + 1;
    });
    promise.resolve(1, 2, 3);
}
