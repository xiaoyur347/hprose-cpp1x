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
 * MethodTest.cpp                                         *
 *                                                        *
 * method test for cpp.                                   *
 *                                                        *
 * LastModified: May 29, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#include <hprose/rpc/Method.h>

#include <gtest/gtest.h>

using namespace hprose::rpc;

static void GlobalNoParamNoReturn() {
}

static void GlobalParamNoReturn(int i) {
    (void)i;
}

static int GlobalParamReturn(int i) {
    return i + 1;
}

class MethodClass {
public:
    MethodClass() : value(0) {
    }
    void NoParamNoReturn() {
    }
    void ParamNoReturn(int i) {
        (void)i;
    }
    int ParamReturn(int i) {
        return i + 1;
    }
    int ConstReturn() const {
        return value;
    }
private:
    int value;
};

TEST(Method, GlobalNoParamNoReturn) {
    makeMethod(&GlobalNoParamNoReturn);
}

TEST(Method, GlobalParamNoReturn) {
    makeMethod(&GlobalParamNoReturn);
}

TEST(Method, GlobalParamReturn) {
    makeMethod(&GlobalParamReturn);
}

TEST(Method, ClassNoParamNoReturn) {
    MethodClass c;
    makeMethod(&MethodClass::NoParamNoReturn, &c);
    makeMethod(&MethodClass::NoParamNoReturn, c);
}

TEST(Method, ClassParamNoReturn) {
    MethodClass c;
    makeMethod(&MethodClass::ParamNoReturn, &c);
    makeMethod(&MethodClass::ParamNoReturn, c);
}

TEST(Method, ClassParamReturn) {
    MethodClass c;
    makeMethod(&MethodClass::ParamReturn, &c);
    makeMethod(&MethodClass::ParamReturn, c);
}

TEST(Method, ClassConstReturn) {
    MethodClass c;
    makeMethod(&MethodClass::ConstReturn, &c);
    makeMethod(&MethodClass::ConstReturn, c);
}