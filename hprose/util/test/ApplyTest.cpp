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
 * ApplyTest.cpp                                          *
 *                                                        *
 * apply tuple test for cpp.                              *
 *                                                        *
 * LastModified: May 29, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#include <hprose/util/Apply.h>

#include <gtest/gtest.h>

using namespace hprose::util;

static int globalIncrement(int i) {
    return i + 1;
}

TEST(Apply, GlobalFunctionNotGetAddress) {
    auto ret = apply(globalIncrement, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, GlobalFunctionGetAddress) {
    auto ret = apply(&globalIncrement, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

class ApplyClass {
public:
    static int staticIncrement(int i) {
        return i + 1;
    }
    ApplyClass() : value(0) {
    }
    explicit ApplyClass(int v) : value(v) {
    }
    int increment(int i) {
        return i + 1;
    }
    int autoIncrement() {
        ++value;
        return value;
    }
    int getValue() const {
        return value;
    }
private:
    int value;
};

TEST(Apply, ClassStaticFunctionNotGetAddress) {
    auto ret = apply(ApplyClass::staticIncrement, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, ClassStaticFunctionGetAddress) {
    auto ret = apply(&ApplyClass::staticIncrement, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, InTupleConstructReadOnly) {
    auto ret = apply(&ApplyClass::increment, std::make_tuple(ApplyClass(), 1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, InTupleObjectReadOnly) {
    ApplyClass c;
    auto ret = apply(&ApplyClass::increment, std::make_tuple(c, 1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, InTuplePointerReadOnly) {
    ApplyClass c;
    auto ret = apply(&ApplyClass::increment, std::make_tuple(&c, 1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, OutTupleConstructReadOnly) {
    auto ret = apply(&ApplyClass::increment, ApplyClass(), std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, OutTupleObjectReadOnly) {
    ApplyClass c;
    auto ret = apply(&ApplyClass::increment, c, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, OutTuplePointerReadOnly) {
    ApplyClass c;
    auto ret = apply(&ApplyClass::increment, &c, std::make_tuple(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, InTupleConstructReadWrite) {
    auto ret = apply(&ApplyClass::autoIncrement, std::make_tuple(ApplyClass(1)));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, InTupleObjectReadWrite) {
    ApplyClass c(1);
    auto ret = apply(&ApplyClass::autoIncrement, std::make_tuple(c));
    ASSERT_EQ(ret , 2);
    ASSERT_EQ(c.getValue(), 1);
}

TEST(Apply, InTuplePointerReadWrite) {
    ApplyClass c(1);
    auto ret = apply(&ApplyClass::autoIncrement, std::make_tuple(&c));
    ASSERT_EQ(ret , 2);
    ASSERT_EQ(c.getValue(), 2);
}

TEST(Apply, OutTupleConstructReadWrite) {
    auto ret = apply(&ApplyClass::autoIncrement, ApplyClass(1), std::make_tuple());
    ASSERT_EQ(ret , 2);
}

TEST(Apply, NoTupleConstructReadWrite) {
    auto ret = apply(&ApplyClass::autoIncrement, ApplyClass(1));
    ASSERT_EQ(ret , 2);
}

TEST(Apply, NoTupleObjectReadWrite) {
    ApplyClass c(1);
    auto ret = apply(&ApplyClass::autoIncrement, c);
    ASSERT_EQ(ret , 2);
    ASSERT_EQ(c.getValue(), 1);
}

TEST(Apply, NoTuplePointerReadWrite) {
    ApplyClass c(1);
    auto ret = apply(&ApplyClass::autoIncrement, &c);
    ASSERT_EQ(ret , 2);
    ASSERT_EQ(c.getValue(), 2);
}