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

namespace hprose {

namespace detail {

struct Destroy {
    template<class T>
    static void destroy(T *t) { t->~T(); }
};

} // hprose::detail

inline Variant::Variant(bool v) : type(Bool) {
    data.vBool = v;
}

inline Variant::Variant(uint8_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(int8_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(uint16_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(int16_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(uint32_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(int32_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(uint64_t v) : type(Int64) {
    data.vInt64 = v;
}
inline Variant::Variant(int64_t v) : type(Int64) {
    data.vInt64 = v;
}

inline Variant::Variant(float v) : type(Double) {
    data.vDouble = v;
}
inline Variant::Variant(double v) : type(Double) {
    data.vDouble = v;
}

#ifdef HPROSE_HAS_DELEGATING_CONSTRUCTORS
inline Variant::Variant() : Variant(nullptr) {}
#else // HPROSE_HAS_DELEGATING_CONSTRUCTORS
inline Variant::Variant() : type(Null) {}
#endif // HPROSE_HAS_DELEGATING_CONSTRUCTORS

#ifdef HPROSE_HAS_NULLPTR
inline Variant::Variant(std::nullptr_t) : type(Null) {}
#endif // HPROSE_HAS_NULLPTR

#ifdef HPROSE_HAS_DELEGATING_CONSTRUCTORS
inline Variant::Variant(const char *v) : Variant(std::string(v)) {}
#else // HPROSE_HAS_DELEGATING_CONSTRUCTORS
inline Variant::Variant(const char *v) : type(String) {
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
    new (&data.vString) std::shared_ptr<std::string>(new std::string(v));
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
    data.vString = new std::string(v);
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
}
#endif // HPROSE_HAS_DELEGATING_CONSTRUCTORS

#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
inline Variant::Variant(std::string v) : type(String) {
    new (&data.vString) std::shared_ptr<std::string>(new std::string(std::move(v)));
}

inline Variant::Variant(std::tm v) : type(Time) {
    new (&data.vTime) std::shared_ptr<std::tm>(new std::tm(std::move(v)));
}

inline Variant::Variant(Ref ref) : type(Reference) {
    new (&data.vRef) Ref(ref);
}

template<class T>
Variant::Variant(const T &v) : type(Other) {
    new (&data.vOther) std::shared_ptr<Any>(new Any(v));
}

#else // HPROSE_HAS_UNRESTRICTED_UNIONS

inline Variant::Variant(std::string v) : type(String) {
    data.vString = new std::string(v);
}

inline Variant::Variant(std::tm v) : type(Time) {
    data.vTime = new std::tm(v);
}

inline Variant::Variant(Ref ref) : type(Reference) {
    data.vRef = new Ref(ref);
}

template<class T>
Variant::Variant(const T &v) : type(Other) {
    data.vOther = new Any(v);
}
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS

inline Variant::Variant(const Variant &o) : type(Null) {
    *this = o;
}

inline Variant::Variant(Variant &&o) noexcept : type(Null) {
    *this = std::move(o);
}

inline Variant::~Variant() noexcept {
    destroy();
}

inline bool Variant::isNull() const { return type == Null; }
inline bool Variant::isBool() const { return type == Bool; }
inline bool Variant::isInt64() const { return type == Int64; }
inline bool Variant::isDouble() const { return type == Double; }
inline bool Variant::isString() const { return type == String; }
inline bool Variant::isTime() const { return type == Time; }
inline bool Variant::isRef() const { return type == Reference; }
inline bool Variant::isOther() const { return type == Other; }

inline bool Variant::getBool() const {
    return data.vBool;
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
Variant::getInteger() const {
    return data.vInt64;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
Variant::getFloat() const {
    return data.vDouble;
}

#ifdef HPROSE_HAS_REF_QUALIFIER
inline const std::string &Variant::getString() const & {
    return *data.vString;
}

inline const std::tm &Variant::getTime() const & {
    return *data.vTime;
}

inline const Ref &Variant::getRef() const & {
    return data.vRef;
}

inline const Any &Variant::getOther() const & {
    return *data.vOther;
}
#else // HPROSE_HAS_REF_QUALIFIER
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
inline const std::string &Variant::getString() const {
    return *data.vString;
}

inline const std::tm &Variant::getTime() const {
    return *data.vTime;
}

inline const Ref &Variant::getRef() const {
    return data.vRef;
}

inline const Any &Variant::getOther() const {
    return *data.vOther;
}
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
inline const std::string &Variant::getString() const {
    return *data.vString;
}

inline const std::tm &Variant::getTime() const {
    return *data.vTime;
}

inline const Ref &Variant::getRef() const {
    return *data.vRef;
}

inline const Any &Variant::getOther() const {
    return *data.vOther;
}
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
#endif // HPROSE_HAS_REF_QUALIFIER

template<class T>
T *Variant::getAddress() noexcept {
    return GetAddrImpl<T>::get(data);
}

template<class T>
T const *Variant::getAddress() const noexcept {
    return const_cast<Variant *>(this)->getAddress<T>();
}

template<class T>
struct Variant::GetAddrImpl {
};

template<>
struct Variant::GetAddrImpl<void *> {
    static void **get(Data &d) noexcept { return &d.vNull; }
};

template<>
struct Variant::GetAddrImpl<bool> {
    static bool *get(Data &d) noexcept { return &d.vBool; }
};

template<>
struct Variant::GetAddrImpl<int64_t> {
    static int64_t *get(Data &d) noexcept { return &d.vInt64; }
};

template<>
struct Variant::GetAddrImpl<double> {
    static double *get(Data &d) noexcept { return &d.vDouble; }
};

#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
template<>
struct Variant::GetAddrImpl<std::shared_ptr<std::string> > {
    static std::shared_ptr<std::string> *get(Data &d) noexcept { return &d.vString; }
};

template<>
struct Variant::GetAddrImpl<std::shared_ptr<std::tm>> {
    static std::shared_ptr<std::tm> *get(Data &d) noexcept { return &d.vTime; }
};

template<>
struct Variant::GetAddrImpl<Ref> {
    static Ref *get(Data &d) noexcept { return &d.vRef; }
};

template<>
struct Variant::GetAddrImpl<std::shared_ptr<Any>> {
    static std::shared_ptr<Any> *get(Data &d) noexcept { return &d.vOther; }
};

#else // HPROSE_HAS_UNRESTRICTED_UNIONS

template<>
struct Variant::GetAddrImpl<std::string* > {
    static std::string* *get(Data &d) noexcept { return &d.vString; }
};

template<>
struct Variant::GetAddrImpl<std::tm* > {
    static std::tm* *get(Data &d) noexcept { return &d.vTime; }
};

template<>
struct Variant::GetAddrImpl<Ref* > {
    static Ref* *get(Data &d) noexcept { return &d.vRef; }
};

template<>
struct Variant::GetAddrImpl<Any* > {
    static Any* *get(Data &d) noexcept { return &d.vOther; }
};

#endif // HPROSE_HAS_UNRESTRICTED_UNIONS

namespace io {
inline void decode(Variant &v, Reader &reader) {
    v.decode(reader);
}
inline void encode(const Variant &v, Writer &writer) {
    v.encode(writer);
}
} // hprose.io

} // hprose
