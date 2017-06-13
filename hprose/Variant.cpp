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

#include <hprose/Variant.h>
#include <hprose/io/Reader.h>
#include <hprose/io/Writer.h>

namespace hprose {

#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
#define HP_DATA(x) x
#define HP_MOVE(x) std::move(x)
#define HP_DELETE(x) detail::Destroy::destroy(&x)
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
#define HP_DATA(x) *x
#define HP_MOVE(x) x
#define HP_DELETE(x) delete x
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS

const char *Variant::typeName() const {
    switch (type) {
        case Null:
            return "void *";
        case Bool:
            return "bool";
        case Int64:
            return "int";
        case Double:
            return "double";
        case String:
            return "std::string";
        case Bytes:
            return "Bytes";
        case Time:
            return "std::tm";
        case Array:
            return "Array";
        case Reference:
            return "Ref";
        case Other:
            return "Any";
        default:
            abort();
    }
}

void Variant::decode(io::Reader &reader) {
    switch (type) {
        case Null:
            reader.readNull();
            break;
        case Bool:
            data.vBool = reader.unserialize<bool>();
            break;
        case Int64:
            data.vInt64 = reader.readInteger<int64_t>();
            break;
        case Double:
            data.vDouble = reader.readFloat<double>();
            break;
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
        case String:
            data.vString.reset(new std::string(reader.readString<std::string>()));
            break;
        case Bytes:
            break;
        case Time:
            data.vTime.reset(new std::tm(reader.unserialize<std::tm>()));
            break;
        case Array:
            break;
        case Reference:
            break;
        case Other:
            //data.vOther.reset(new hprose::Any(reader.unserialize<Any>()));
            break;
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
        case String:
            HP_DATA(data.vString) = reader.readString<std::string>();
            break;
        case Bytes:
            break;
        case Time:
            HP_DATA(data.vTime) = reader.unserialize<std::tm>();
            break;
        case Array:
            break;
        case Reference:
            break;
        case Other:
            //HP_DATA(data.vOther) = reader.unserialize<Any>();
            break;
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
        default:
            break;
    }
}

void Variant::encode(io::Writer &writer) const {
    switch (type) {
        case Null:
            writer.writeNull();
            break;
        case Bool:
            writer.writeBool(data.vBool);
            break;
        case Int64:
            writer.writeInteger(data.vInt64);
            break;
        case Double:
            writer.writeFloat(data.vDouble);
            break;
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
        case String:
            writer.writeString(*data.vString.get());
            break;
        case Bytes:
            break;
        case Time:
            writer.writeTime(*data.vTime.get());
            break;
        case Array:
            break;
        case Reference:
            break;
        case Other:
            break;
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
        case String:
            writer.writeString(HP_DATA(data.vString));
            break;
        case Bytes:
            break;
        case Time:
            writer.writeTime(HP_DATA(data.vTime));
            break;
        case Array:
            break;
        case Reference:
            break;
        case Other:
            break;
#endif  // HPROSE_HAS_UNRESTRICTED_UNIONS
        default:
            break;
    }
}

Variant &Variant::operator=(const Variant &o) {
    if (&o != this) {
        if (type == o.type) {
            switch (type) {
                case Null:
                    data.vNull = nullptr;
                    break;
                case Bool:
                    data.vBool = o.data.vBool;
                    break;
                case Int64:
                    data.vInt64 = o.data.vInt64;
                    break;
                case Double:
                    data.vDouble = o.data.vDouble;
                    break;
                case String:
                    HP_DATA(data.vString) = HP_DATA(o.data.vString);
                    break;
                case Bytes:
                    break;
                case Time:
                    HP_DATA(data.vTime) = HP_DATA(o.data.vTime);
                    break;
                case Array:
                    HP_DATA(data.vArray) = HP_DATA(o.data.vArray);
                    break;
                case Reference:
                    HP_DATA(data.vRef) = HP_DATA(o.data.vRef);
                    break;
                case Other:
                    HP_DATA(data.vOther) = HP_DATA(o.data.vOther);
                    break;
                default:
                    break;
            }
        } else {
            destroy();
            switch (type) {
                case Null:
                    data.vNull = nullptr;
                    break;
                case Bool:
                    data.vBool = o.data.vBool;
                    break;
                case Int64:
                    data.vInt64 = o.data.vInt64;
                    break;
                case Double:
                    data.vDouble = o.data.vDouble;
                    break;
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
                case String:
                    new (&data.vString) std::shared_ptr<std::string>(o.data.vString);
                    break;
                case Bytes:
                    break;
                case Time:
                    new (&data.vTime) std::shared_ptr<std::tm>(o.data.vTime);
                    break;
                case Array:
                    new (&data.vArray) std::shared_ptr<std::vector<Variant>>(o.data.vArray);
                    break;
                case Reference:
                    new (&data.vRef) Ref(o.data.vRef);
                    break;
                case Other:
                    new (&data.vOther) std::shared_ptr<Any>(o.data.vOther);
                    break;
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
                case String:
                    data.vString = new std::string(*o.data.vString);
                    break;
                case Bytes:
                    break;
                case Time:
                    data.vTime = new std::tm(*o.data.vTime);
                    break;
                case Array:
                    data.vArray = new std::vector<Variant>(*o.data.vArray);
                    break;
                case Reference:
                    data.vRef = new Ref(*o.data.vRef);
                    break;
                case Other:
                    data.vOther = new Any(*o.data.vOther);
                    break;
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
                default:
                    break;
            }
            type = o.type;
        }
    }
    return *this;
}

Variant &Variant::operator=(Variant &&o) noexcept {
    if (&o != this) {
        if (type == o.type) {
            switch (type) {
                case Null:
                    data.vNull = nullptr;
                    break;
                case Bool:
                    data.vBool = o.data.vBool;
                    break;
                case Int64:
                    data.vInt64 = o.data.vInt64;
                    break;
                case Double:
                    data.vDouble = o.data.vDouble;
                    break;
                case String:
                    data.vString = HP_MOVE(o.data.vString);
                    break;
                case Bytes:
                    break;
                case Time:
                    data.vTime = HP_MOVE(o.data.vTime);
                    break;
                case Array:
                    data.vArray = HP_MOVE(o.data.vArray);
                    break;
                case Reference:
                    data.vRef = HP_MOVE(o.data.vRef);
                    break;
                case Other:
                    data.vOther = HP_MOVE(o.data.vOther);
                    break;
                default:
                    break;
            }
        } else {
            destroy();
            switch (o.type) {
                case Null:
                    data.vNull = nullptr;
                    break;
                case Bool:
                    data.vBool = o.data.vBool;
                    break;
                case Int64:
                    data.vInt64 = o.data.vInt64;
                    break;
                case Double:
                    data.vDouble = o.data.vDouble;
                    break;
#ifdef HPROSE_HAS_UNRESTRICTED_UNIONS
                case String:
                    new (&data.vString) std::shared_ptr<std::string>(std::move(o.data.vString));
                    break;
                case Bytes:
                    break;
                case Time:
                    new (&data.vTime) std::shared_ptr<std::tm>(std::move(o.data.vTime));
                    break;
                case Array:
                    new (&data.vArray) std::shared_ptr<std::vector<Variant>>(std::move(o.data.vArray));
                    break;
                case Reference:
                    new (&data.vRef) Ref(std::move(o.data.vRef));
                    break;
                case Other:
                    new (&data.vOther) std::shared_ptr<Any>(std::move(o.data.vOther));
                    break;
#else // HPROSE_HAS_UNRESTRICTED_UNIONS
                case String:
                    data.vString = HP_MOVE(o.data.vString);
                    break;
                case Bytes:
                    break;
                case Time:
                    data.vTime = HP_MOVE(o.data.vTime);
                    break;
                case Array:
                    data.vArray = HP_MOVE(o.data.vArray);
                    break;
                case Reference:
                    data.vRef = HP_MOVE(o.data.vRef);
                    break;
                case Other:
                    data.vOther = HP_MOVE(o.data.vOther);
                    break;
#endif // HPROSE_HAS_UNRESTRICTED_UNIONS
                default:
                    break;
            }
            type = o.type;
        }
#ifndef HPROSE_HAS_UNRESTRICTED_UNIONS
        o.type = Null;
#endif
    }
    return *this;
}

void Variant::destroy() noexcept {
    switch (type) {
        case Null:
        case Bool:
        case Int64:
        case Double:
            break;
        case String:
            HP_DELETE(data.vString);
            break;
        case Bytes:
            break;
        case Time:
            HP_DELETE(data.vTime);
            break;
        case Array:
            HP_DELETE(data.vArray);
            break;
        case Reference:
            HP_DELETE(data.vRef);
            break;
        case Other:
            HP_DELETE(data.vOther);
            break;
        default:
            break;
    }

    type = Null;
    data.vNull = nullptr;
}

} // hprose
