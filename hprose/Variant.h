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
 * hprose/Variant.h                                       *
 *                                                        *
 * variant type for cpp.                                  *
 *                                                        *
 * LastModified: Dec 26, 2016                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#pragma once

#include <hprose/Any.h>
#include <hprose/Ref.h>
#include <hprose/util/TypeIndex.h>
#include <hprose/util/Util.h>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

namespace hprose {

namespace io {
class Reader;
class Writer;
}

class Variant {
public:
    enum Type {
        Null,
        Bool,
        Int64,
        Double,
        String,
        Bytes,
        Time,
        Array,
        Reference,
        Other
    };

    Variant();
#ifdef HPROSE_HAS_NULLPTR
    Variant(std::nullptr_t);
#endif

    Variant(bool v);

    Variant(uint8_t v);
    Variant(int8_t v);
    Variant(uint16_t v);
    Variant(int16_t v);
    Variant(uint32_t v);
    Variant(int32_t v);
    Variant(uint64_t v);
    Variant(int64_t v);

    Variant(float v);
    Variant(double v);

    Variant(const char *v);
    Variant(std::string v);

    Variant(std::tm v);

    Variant(Ref ref);

    template<class T>
    Variant(const T &v);

    Variant(const Variant &o);
    Variant(Variant &&o) noexcept;
    ~Variant() noexcept;

    Variant &operator=(const Variant &o);
    Variant &operator=(Variant &&o) noexcept;

    bool isNull() const;
    bool isBool() const;
    bool isInt64() const;
    bool isDouble() const;
    bool isString() const;
    bool isTime() const;
    bool isRef() const;
    bool isOther() const;

    const char *typeName() const;

    void decode(io::Reader &reader);
    void encode(io::Writer &writer) const;

    bool getBool() const;

    template<class T>
    typename std::enable_if<std::is_integral<T>::value, T>::type
    getInteger() const;

    template<class T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type
    getFloat() const;

#ifdef HPROSE_HAS_REF_QUALIFIER
    const std::string &getString() const &;
    const std::tm &getTime() const &;
    const Ref &getRef() const &;
    const Any &getOther() const &;
#else // HPROSE_HAS_REF_QUALIFIER
    const std::string &getString() const;
    const std::tm &getTime() const;
    const Ref &getRef() const;
    const Any &getOther() const;
#endif // HPROSE_HAS_REF_QUALIFIER

private:
    template<class T> struct GetAddrImpl;

    template<class T> T       *getAddress() noexcept;
    template<class T> T const *getAddress() const noexcept;

    void destroy() noexcept;

    Type type;

    union Data {
        explicit Data() : vNull(nullptr) {}
        ~Data() {}

        void *vNull;
        bool vBool;
        int64_t vInt64;
        double vDouble;
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
        std::shared_ptr<std::string> vString;
        std::shared_ptr<std::tm> vTime;
        std::shared_ptr<std::vector<Variant> > vArray;
        Ref vRef;
        std::shared_ptr<Any> vOther;
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
        std::string* vString;
        std::tm* vTime;
        std::vector<Variant>* vArray;
        Ref* vRef;
        Any* vOther;
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
    } data;
};

} // hprose

#include <hprose/Variant-inl.h>
