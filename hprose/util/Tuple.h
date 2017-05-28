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
 * hprose/util/Tuple.h                                    *
 *                                                        *
 * tuple c++14 for cpp.                                   *
 * modified by https://github.com/xhawk18/promise-cpp     *
 *                                                        *
 * LastModified: May 27, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

#pragma once

#if __GNUC__ == 4 && __GNUC_MINOR__ == 5
    #include <hprose/util/TupleCompatible.h>
#else
    #include <tuple>
#endif

#if __cplusplus <= 201103L && !defined(_MSC_VER)
namespace std {

template <size_t... Ints>
struct index_sequence
{
    typedef index_sequence type;
    typedef size_t value_type;
    static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
};

template <class Sequence1, class Sequence2>
struct _merge_and_renumber;

template <size_t... I1, size_t... I2>
struct _merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
    : index_sequence<I1..., (sizeof...(I1)+I2)...>
{ };

template <size_t N>
struct make_index_sequence
    : _merge_and_renumber<typename make_index_sequence<N / 2>::type,
    typename make_index_sequence<N - N / 2>::type>
{ };

template<> struct make_index_sequence<0> : index_sequence<> { };
template<> struct make_index_sequence<1> : index_sequence<0> { };

} // namespace std
#endif // __cplusplus