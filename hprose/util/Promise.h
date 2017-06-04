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
 * hprose/util/Promise.h                                  *
 *                                                        *
 * Promise A+ for cpp.                                    *
 * modified by https://github.com/xhawk18/promise-cpp     *
 *                                                        *
 * LastModified: May 26, 2017                             *
 * Author: xiaoyur347 <xiaoyur347@gmail.com>              *
 *                                                        *
\**********************************************************/

/*
 * Promise API implemented by cpp as Javascript promise style 
 *
 * Copyright (c) 2016, xhawk18
 * at gmail.com
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <hprose/util/Config.h>
#include <hprose/util/TypeTraits.h>
#include <algorithm>
#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <typeinfo>
#include <utility>

#ifdef HPROSE_HAS_TYPEINDEX
#include <typeindex>
#endif
#include <type_traits>

#ifdef PM_DEBUG
#define pm_assert(x)    assert(x)
#else
#define pm_assert(x)    do{ } while(0)
#endif

namespace hprose {
namespace util {

#ifndef PROMISE_HAS_TYPEINDEX
typedef const std::type_info *type_index;
inline type_index get_type_index(const std::type_info &info){
    return type_index(&info);
}
#else // PROMISE_HAS_TYPEINDEX
typedef std::type_index type_index;
inline type_index get_type_index(const std::type_info &info){
    return type_index(info);
}
#endif // PROMISE_HAS_TYPEINDEX

template<class P, class M>
inline size_t pm_offsetof(const M P::*member) {
    return (size_t)&reinterpret_cast<char const&>((reinterpret_cast<P*>(0)->*member));
}

template<class P, class M, class T>
inline P* pm_container_of(T* ptr, const M P::*member) {
    return reinterpret_cast<P*>(reinterpret_cast<char*>(ptr) - pm_offsetof(member));
}

template<typename T>
inline void pm_throw(const T &t){
    throw t;
}



template<bool match_uint8, bool match_uint16, bool match_uint32>
struct pm_offset_impl {
    typedef uint64_t type;
};
template<bool match_uint16, bool match_uint32>
struct pm_offset_impl<true, match_uint16, match_uint32> {
    typedef uint8_t type;
};
template<bool match_uint32>
struct pm_offset_impl<false, true, match_uint32> {
    typedef uint16_t type;
};
template<>
struct pm_offset_impl<false, false, true> {
    typedef uint32_t type;
};



template<size_t SIZE, size_t ADDR_ALIGN>
struct pm_offset {
    typedef typename pm_offset_impl<
        ((SIZE + ADDR_ALIGN - 1) / ADDR_ALIGN <= 0x100),
        ((SIZE + ADDR_ALIGN - 1) / ADDR_ALIGN <= 0x10000UL),
        ((SIZE + ADDR_ALIGN - 1) / ADDR_ALIGN <= 0x100000000ULL) >::type type;
};

//allocator
struct pm_stack {
    typedef void *itr_t;
    static inline itr_t ptr_to_itr(void *ptr) {
        return ptr;
    }

    static inline void *itr_to_ptr(itr_t itr) {
        return itr;
    }
};

template< class T, class... Args >
inline T *pm_stack_new(Args&&... args) {
    return new
        T(args...);
}


//List
struct pm_list {
    typedef pm_stack::itr_t itr_t;

    pm_list()
        : prev_(pm_stack::ptr_to_itr(reinterpret_cast<void *>(this)))
        , next_(pm_stack::ptr_to_itr(reinterpret_cast<void *>(this))) {
    }

    inline pm_list *prev() {
        return reinterpret_cast<pm_list *>(pm_stack::itr_to_ptr(prev_));
    }

    inline pm_list *next() {
        return reinterpret_cast<pm_list *>(pm_stack::itr_to_ptr(next_));
    }

    inline void prev(pm_list *other) {
        prev_ = pm_stack::ptr_to_itr(reinterpret_cast<void *>(other));
    }

    inline void next(pm_list *other) {
        next_ = pm_stack::ptr_to_itr(reinterpret_cast<void *>(other));
    }

    /* Connect or disconnect two lists. */
    static void toggleConnect(pm_list *list1, pm_list *list2) {
        pm_list *prev1 = list1->prev();
        pm_list *prev2 = list2->prev();
        prev1->next(list2);
        prev2->next(list1);
        list1->prev(prev2);
        list2->prev(prev1);
    }

    /* Connect two lists. */
    static void connect(pm_list *list1, pm_list *list2) {
        toggleConnect(list1, list2);
    }

    /* Disconnect tow lists. */
    static void disconnect(pm_list *list1, pm_list *list2) {
        toggleConnect(list1, list2);
    }

    /* Same as listConnect */
    void attach(pm_list *node) {
        connect(this, node);
    }

    /* Make node in detach mode */
    void detach () {
        disconnect(this, this->next());
    }

    /* Move node to list, after moving,
       node->next == this
       this->prev == node
     */
    void move(pm_list *node) {
#if 1
        node->prev()->next(node->next());
        node->next()->prev(node->prev());

        node->next(this);
        node->prev(this->prev());
        this->prev()->next(node);
        this->prev(node);
#else
        node->detach();
        attach(node);
#endif
    }

    /* Check if list is empty */
    int empty() {
        return (this->next() == this);
    }

private:
    itr_t prev_;
    itr_t next_;
};


struct pm_memory_pool {
    pm_list free_;
    size_t size_;
    pm_memory_pool(size_t size)
        : free_()
        , size_(size){
    }
};

//allocator
struct pm_memory_pool_buf_header {
    pm_memory_pool_buf_header(pm_memory_pool *pool)
        : pool_(pm_stack::ptr_to_itr(reinterpret_cast<void *>(pool)))
        , ref_count_(0){
    }

    pm_list list_;
    pm_stack::itr_t pool_;
    int16_t ref_count_;

    static inline void *to_ptr(pm_memory_pool_buf_header *header) {
        struct dummy_pool_buf {
            pm_memory_pool_buf_header header_;
            struct {
                void *buf_[1];
            } buf_;
        };
        dummy_pool_buf *buf = reinterpret_cast<dummy_pool_buf *>(
            reinterpret_cast<char *>(header) - pm_offsetof(&dummy_pool_buf::header_));
        return (void *)&buf->buf_;
    }

    static inline void *to_ptr(pm_list *list){
        pm_memory_pool_buf_header *header = pm_container_of(list, &pm_memory_pool_buf_header::list_);
        return pm_memory_pool_buf_header::to_ptr(header);
    }

    static inline pm_memory_pool_buf_header *from_ptr(void *ptr) {
        struct dummy_pool_buf {
            pm_memory_pool_buf_header header_;
            struct {
                void *buf_[1];
            } buf_;
        };
        dummy_pool_buf *buf = pm_container_of(ptr, &dummy_pool_buf::buf_);
        return &buf->header_;
    }
};

template <size_t SIZE>
struct pm_memory_pool_buf {
    struct buf_t {
        buf_t() {}
        void *buf[(SIZE + sizeof(void *) - 1) / sizeof(void *)];
    };

    pm_memory_pool_buf(pm_memory_pool *pool)
        : header_(pool) {
    }

    pm_memory_pool_buf_header header_;
    buf_t buf_;
};

template <size_t SIZE>
struct pm_size_allocator {
    static inline pm_memory_pool *get_memory_pool() {
        static pm_memory_pool *pool_ = nullptr;
        if(pool_ == nullptr)
            pool_ = pm_stack_new<pm_memory_pool>(SIZE);
        return pool_;
    }
};

struct pm_allocator {
private:
    static std::mutex &get_mutex() {
        static std::mutex mutex_;
        return mutex_;
    }
    static pm_memory_pool_buf_header *obtain_pool_buf(pm_memory_pool *pool){
        pm_list *node = pool->free_.next();
        node->detach();
        pm_memory_pool_buf_header *header = pm_container_of(node, &pm_memory_pool_buf_header::list_);
        return header;
    }

    template <size_t SIZE>
    static void *obtain_impl() {
        std::lock_guard<std::mutex> guard(get_mutex());
        pm_memory_pool *pool = pm_size_allocator<SIZE>::get_memory_pool();
        if (pool->free_.empty()) {
            pm_memory_pool_buf<SIZE> *pool_buf = 
                pm_stack_new<pm_memory_pool_buf<SIZE>>(pool);
            //printf("++++ obtain = %p %d\n", (void *)&pool_buf->buf_, sizeof(T));
            return (void *)&pool_buf->buf_;
        }
        else {
            pm_memory_pool_buf_header *header = obtain_pool_buf(pool);
            pm_memory_pool_buf<SIZE> *pool_buf = pm_container_of
                (header, &pm_memory_pool_buf<SIZE>::header_);
            //printf("++++ obtain = %p %d\n", (void *)&pool_buf->buf_, sizeof(T));
            return (void *)&pool_buf->buf_;
        }
    }

    static void release(void *ptr) {
        //printf("--- release = %p\n", ptr);
        std::lock_guard<std::mutex> guard(get_mutex());
        pm_memory_pool_buf_header *header = pm_memory_pool_buf_header::from_ptr(ptr);
        pm_memory_pool *pool = reinterpret_cast<pm_memory_pool *>(pm_stack::itr_to_ptr(header->pool_));
        pool->free_.move(&header->list_);
    }

    static void add_ref_impl(void *object) {
        //printf("add_ref %p\n", object);
        if (object != nullptr) {
            std::lock_guard<std::mutex> guard(get_mutex());
            pm_memory_pool_buf_header *header = pm_memory_pool_buf_header::from_ptr(object);
            //printf("++ %p %d -> %d\n", pool_buf, pool_buf->ref_count_, pool_buf->ref_count_ + 1);
            ++header->ref_count_;
        }
    }

    static bool dec_ref_impl(void *object) {
        //printf("dec_ref %p\n", object);
        if (object != nullptr) {
            std::lock_guard<std::mutex> guard(get_mutex());
            pm_memory_pool_buf_header *header = pm_memory_pool_buf_header::from_ptr(object);
            //printf("-- %p %d -> %d\n", pool_buf, pool_buf->ref_count_, pool_buf->ref_count_ - 1);
            pm_assert(header->ref_count_ > 0);
            --header->ref_count_;
            if (header->ref_count_ == 0) {
                //release(object);
                return true;
            }
        }
        return false;
    }

public:
    template <typename T>
    static inline void *obtain() {
        return obtain_impl<sizeof(T)>();
    }

    template<typename T>
    static void add_ref(T *object) {
        add_ref_impl(reinterpret_cast<void *>(const_cast<T *>(object)));
    }

    template<typename T>
    static void dec_ref(T *object) {
        if(dec_ref_impl(reinterpret_cast<void *>(const_cast<T *>(object)))){
            object->~T();
            release(reinterpret_cast<void *>(const_cast<T *>(object)));
        }
    }
};

template< class T, class... Args >
inline T *pm_new(Args&&... args) {
    T *object = new(pm_allocator::template obtain<T>()) T(args...);
    pm_allocator::add_ref(object);
    return object;
}

template< class T >
inline void pm_delete(T *object){
    pm_allocator::dec_ref(object);
}

// Any library
// See http://www.boost.org/libs/any for Documentation.
// what:  variant type any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Ed Brey, Mark Rodgers, Peter Dimov, and James Curran
// when:  July 2001
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95

template<typename T>
struct remove_rcv {
    typedef typename std::remove_reference<T>::type r_type;
    typedef typename std::remove_cv<r_type>::type type;
};

template<typename T>
struct remove_rcv<const T *> {
    typedef typename remove_rcv<T *>::type type;
};

template<typename T>
struct remove_rcv<const T &> {
    typedef typename remove_rcv<T &>::type type;
};

template<typename T>
struct void_ptr_type {
    static void *cast(const T &t) {
        return reinterpret_cast<void *>(const_cast<char *>(&reinterpret_cast<const char &>(t)));
    }
};

template<typename T>
struct void_ptr_type<const T *> {
    typedef const T* PT;
    static void *cast(const PT &t) {
        return (void *)(&t);
    }
};

template<typename T>
void *void_ptr_cast(const T &t) {
    return void_ptr_type<T>::cast(t);
}

template<typename TUPLE, std::size_t SIZE>
struct offset_tuple_impl {
    template<size_t I_SIZE, std::size_t... I>
    struct offset_array {
        void *offsets_[I_SIZE];

        offset_array(const TUPLE *tuple)
            : offsets_{ void_ptr_cast(std::get<I>(*tuple))... } {
        }
    };

    template<std::size_t... I>
    static offset_array<SIZE, I...> get_array(const TUPLE *tuple, const std::index_sequence<I...> &) {
        return offset_array<SIZE, I...>(tuple);
    }

    decltype(get_array(nullptr, std::make_index_sequence<SIZE>())) value_;

    offset_tuple_impl(const TUPLE *tuple)
        : value_(get_array(tuple, std::make_index_sequence<SIZE>())) {
    }

    void *tuple_offset(size_t i) const {
        return value_.offsets_[i];
    }
};

template<typename TUPLE>
struct offset_tuple_impl<TUPLE, 0> {
    offset_tuple_impl(const TUPLE *) {
    }
    void *tuple_offset(size_t) const {
        return nullptr;
    }
};

template<typename NOT_TUPLE>
struct offset_tuple
    : public offset_tuple_impl<NOT_TUPLE, 0> {
    offset_tuple(const NOT_TUPLE *tuple)
        : offset_tuple_impl<NOT_TUPLE, 0>(tuple) {
    }
};

template<typename ...T>
struct offset_tuple<std::tuple<T...>>
    : public offset_tuple_impl<std::tuple<T...>, std::tuple_size<std::tuple<T...>>::value> {
    offset_tuple(const std::tuple<T...> *tuple)
        : offset_tuple_impl<std::tuple<T...>, std::tuple_size<std::tuple<T...>>::value>(tuple) {
    }
};

template<typename TUPLE, std::size_t SIZE>
struct type_tuple_impl {
    template<size_t I_SIZE, std::size_t... I>
    struct type_index_array {
        type_index types_[I_SIZE];
        type_index types_rcv_[I_SIZE];

        type_index_array()
            : types_{ get_type_index(typeid(typename std::tuple_element<I, TUPLE>::type))... }
            , types_rcv_{ get_type_index(typeid(typename remove_rcv<typename std::tuple_element<I, TUPLE>::type>::type))... } {
        }
    };

    template<std::size_t... I>
    static type_index_array<SIZE, I...> get_array(const std::index_sequence<I...> &) {
        return type_index_array<SIZE, I...>();
    }

    decltype(get_array(std::make_index_sequence<SIZE>())) value_;

    type_tuple_impl()
        : value_(get_array(std::make_index_sequence<SIZE>())) {
    }

    type_index tuple_type(size_t i) const {
        return value_.types_[i];
    }

    type_index tuple_rcv_type(size_t i) const {
        return value_.types_rcv_[i];
    }
};

template<typename TUPLE>
struct type_tuple_impl<TUPLE, 0> {
    static const std::size_t size_ = 0;
    type_index tuple_type(size_t) const {
        return get_type_index(typeid(void));
    }
    type_index tuple_rcv_type(size_t) const {
        return get_type_index(typeid(void));
    }
};

template<typename NOT_TUPLE>
struct type_tuple :
    public type_tuple_impl<void, 0> {
    static const std::size_t size_ = 0;
};

template<typename ...T>
struct type_tuple<std::tuple<T...>> :
    public type_tuple_impl<std::tuple<T...>, std::tuple_size<std::tuple<T...>>::value> {
    static const std::size_t size_ = std::tuple_size<std::tuple<T...>>::value;
};

template<typename FUNC>
struct func_traits_impl {
    typedef decltype(&FUNC::operator()) func_type;
    typedef typename func_traits_impl<func_type>::ret_type ret_type;
    typedef typename func_traits_impl<func_type>::arg_type arg_type;
};

template<typename RET, class T, typename ...ARG>
struct func_traits_impl< RET(T::*)(ARG...) const > {
    typedef RET ret_type;
    typedef std::tuple<ARG...> arg_type;
};

template<typename RET, typename ...ARG>
struct func_traits_impl< RET(*)(ARG...) > {
    typedef RET ret_type;
    typedef std::tuple<ARG...> arg_type;
};

template<typename RET, typename ...ARG>
struct func_traits_impl< RET(ARG...) > {
    typedef RET ret_type;
    typedef std::tuple<ARG...> arg_type;
};

template<typename FUNC>
struct func_traits {
    typedef typename func_traits_impl<FUNC>::ret_type ret_type;
    typedef typename remove_reference_tuple<typename func_traits_impl<FUNC>::arg_type>::type arg_type;
};


class pm_any {
public: // structors
    pm_any()
        : content(0) {
    }

    template<typename ValueType>
    pm_any(const ValueType & value)
        : content(pm_new<holder<ValueType>>(value)) {
    }

    pm_any(const pm_any & other)
        : content(other.content ? other.content->clone() : 0) {
    }

    ~pm_any() {
        if (content != nullptr) {
            pm_delete(content);
        }
    }

public: // modifiers

    pm_any & swap(pm_any & rhs) {
        std::swap(content, rhs.content);
        return *this;
    }

    template<typename ValueType>
    pm_any & operator=(const ValueType & rhs) {
        pm_any(rhs).swap(*this);
        return *this;
    }

    pm_any & operator=(const pm_any & rhs) {
        pm_any(rhs).swap(*this);
        return *this;
    }

public: // queries
    bool empty() const {
        return !content;
    }
    
    void clear() {
        pm_any().swap(*this);
    }

    const std::type_info & type() const {
        return content ? content->type() : typeid(void);
    }

    std::size_t tuple_size() const {
        return content ? content->tuple_size() : 0;
    }

    type_index tuple_type(size_t i) const {
        return content ? content->tuple_type(i) : get_type_index(typeid(void));
    }

    void *tuple_element(size_t i) const {
        return content ? content->tuple_element(i) : nullptr;
    }

public: // types (public so any_cast can be non-friend)
    class placeholder {
    public: // structors
        virtual ~placeholder() {
        }

    public: // queries
        virtual const std::type_info & type() const = 0;
        virtual std::size_t tuple_size() const = 0;
        virtual type_index tuple_type(size_t i) const = 0;
        virtual void *tuple_element(size_t i) const = 0;

        virtual placeholder * clone() const = 0;
    };

    template<typename ValueType>
    class holder : public placeholder {
    public: // structors
        holder(const ValueType & value)
            : held(value)
            , type_tuple_()
            , offset_tuple_(&held) {
        }

    public: // queries
        virtual const std::type_info & type() const {
            return typeid(ValueType);
        }

        virtual std::size_t tuple_size() const {
            return type_tuple_.size_;
        }

        virtual type_index tuple_type(size_t i) const {
            return type_tuple_.tuple_type(i);
        }

        virtual void *tuple_element(size_t i) const {
            return offset_tuple_.tuple_offset(i);
        }

        virtual placeholder * clone() const {
            return pm_new<holder>(held);
        }
    public: // representation
        ValueType held;
        type_tuple<ValueType> type_tuple_;
        offset_tuple<ValueType> offset_tuple_;
    private: // intentionally left unimplemented
        holder & operator=(const holder &);
    };

public: // representation (public so any_cast can be non-friend)
    placeholder * content;
};

class bad_any_cast : public std::bad_cast {
public:
    type_index from_;
    type_index to_;
    bad_any_cast(const type_index &from, const type_index &to)
        : from_(from)
        , to_(to) {
    }
    virtual const char * what() const throw() {
        return "bad_any_cast";
    }
};

template<typename ValueType>
ValueType * any_cast(pm_any *operand) {
    typedef typename pm_any::template holder<ValueType> holder_t;
    return operand &&
        operand->type() == typeid(ValueType)
        ? &static_cast<holder_t *>(operand->content)->held
        : 0;
}

template<typename ValueType>
inline const ValueType * any_cast(const pm_any *operand) {
    return any_cast<ValueType>(const_cast<pm_any *>(operand));
}

template<typename ValueType>
ValueType any_cast(pm_any & operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;

    nonref * result = any_cast<nonref>(&operand);
    if (!result)
        pm_throw(bad_any_cast(get_type_index(operand.type()), get_type_index(typeid(ValueType))));
    return *result;
}

template<typename ValueType>
inline ValueType any_cast(const pm_any &operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;
    return any_cast<const nonref &>(const_cast<pm_any &>(operand));
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



template<typename RET, typename FUNC, std::size_t ...I>
struct call_tuple_t {
    typedef typename func_traits<FUNC>::arg_type func_arg_type;
    typedef typename remove_reference_tuple<std::tuple<RET>>::type ret_type;
    
    static ret_type call(const FUNC &func, pm_any &arg) {
        func_arg_type new_arg(*reinterpret_cast<typename std::tuple_element<I, func_arg_type>::type *>(arg.tuple_element(I))...);
        arg.clear();
        return ret_type(func(std::get<I>(new_arg)...));
    }
};

template<typename FUNC, std::size_t ...I>
struct call_tuple_t<void, FUNC, I...> {
    typedef typename func_traits<FUNC>::arg_type func_arg_type;
    typedef std::tuple<> ret_type;

    static std::tuple<> call(const FUNC &func, pm_any &arg) {
        func_arg_type new_arg(*reinterpret_cast<typename std::tuple_element<I, func_arg_type>::type *>(arg.tuple_element(I))...);
        (void)new_arg;
        arg.clear();
        func(std::get<I>(new_arg)...);
        return std::tuple<>();
    }
};

template<typename RET>
struct call_tuple_ret_t {
    typedef typename remove_reference_tuple<std::tuple<RET>>::type ret_type;
};

template<>
struct call_tuple_ret_t<void> {
    typedef std::tuple<> ret_type;
};

template<typename FUNC, std::size_t ...I>
inline auto call_tuple_as_argument(const FUNC &func, pm_any &arg, const std::index_sequence<I...> &) 
    -> typename call_tuple_ret_t<typename func_traits<FUNC>::ret_type>::ret_type
{
    typedef typename func_traits<FUNC>::ret_type ret_type;

    return call_tuple_t<ret_type, FUNC, I...>::call(func, arg);
}

template<typename FUNC>
inline typename std::enable_if<
            0 == std::tuple_size<typename func_traits<FUNC>::arg_type>::value,
            bool>::type
verify_func_arg(const FUNC &, pm_any &arg) {
    if (arg.tuple_size() != 0) {
        return false;
        //pm_throw(bad_any_cast(type_index(arg.type()), get_type_index(typeid(func_arg_type))));
    }

    return true;
}

template<typename FUNC>
inline typename std::enable_if<
            0 != std::tuple_size<typename func_traits<FUNC>::arg_type>::value,
            bool>::type
verify_func_arg(const FUNC &, pm_any &arg) {
    typedef typename func_traits<FUNC>::arg_type func_arg_type;
    type_tuple<func_arg_type> tuple_func;

    if (arg.tuple_size() < tuple_func.size_) {
        return false;
        //pm_throw(bad_any_cast(type_index(arg.type()), get_type_index(typeid(func_arg_type))));
    }

    for (size_t i = tuple_func.size_; i-- != 0; ) {
        if (arg.tuple_type(i) != tuple_func.tuple_type(i)
            && arg.tuple_type(i) != tuple_func.tuple_rcv_type(i)) {
            //printf("== %s ==> %s\n", arg.tuple_type(i).name(), tuple_func.tuple_type(i).name());
            return false;
            //pm_throw(bad_any_cast(arg.tuple_type(i), tuple_func.tuple_type(i)));
        }
    }

    return true;
}

template<typename FUNC>
inline auto call_func(const FUNC &func, pm_any &arg)
    -> typename call_tuple_ret_t<typename func_traits<FUNC>::ret_type>::ret_type
{
    typedef typename func_traits<FUNC>::arg_type func_arg_type;
    //type_tuple<func_arg_type> tuple_func;

    return call_tuple_as_argument(func, arg, std::make_index_sequence<type_tuple<func_arg_type>::size_>());
}

struct Bypass {};
struct State;

template< class T >
class pm_shared_ptr {
public:
    ~pm_shared_ptr() {
        pm_allocator::dec_ref(object_);
    }

    explicit pm_shared_ptr(T *object)
        : object_(object) {
    }

    explicit pm_shared_ptr()
        : object_(nullptr) {
    }

    pm_shared_ptr(pm_shared_ptr const &ptr)
        : object_(ptr.object_) {
        pm_allocator::add_ref(object_);
    }

    pm_shared_ptr &operator=(pm_shared_ptr const &ptr) {
        pm_shared_ptr(ptr).swap(*this);
        return *this;
    }

    bool operator==(pm_shared_ptr const &ptr) const {
        return object_ == ptr.object_;
    }

    bool operator!=(pm_shared_ptr const &ptr) const {
        return !(*this == ptr);
    }

    bool operator==(T const *ptr) const {
        return object_ == ptr;
    }

    bool operator!=(T const *ptr) const {
        return !(*this == ptr);
    }

    inline T *operator->() const {
        return object_;
    }

    inline T *obtain_rawptr() {
        pm_allocator::add_ref(object_);
        return object_;
    }

    inline void release_rawptr() {
        pm_allocator::dec_ref(object_);
    }

    void clear() {
        pm_shared_ptr().swap(*this);
    }

//private:

    inline void swap(pm_shared_ptr &ptr) {
        std::swap(object_, ptr.object_);
    }

    T *object_;
};

template< class T, class... Args >
inline pm_shared_ptr<T> pm_make_shared(Args&&... args) {
    return pm_shared_ptr<T>(pm_new<T>(args...));
}

template< class T, class B, class... Args >
inline pm_shared_ptr<B> pm_make_shared2(Args&&... args) {
    return pm_shared_ptr<B>(pm_new<T>(args...));
}


template<typename T>
class pm_shared_ptr_promise {
public:
    ~pm_shared_ptr_promise() {
        pm_allocator::dec_ref(object_);
    }

    explicit pm_shared_ptr_promise(T *object)
        : object_(object) {
    }

    explicit pm_shared_ptr_promise()
        : object_(nullptr) {
    }

    pm_shared_ptr_promise(pm_shared_ptr_promise const &ptr)
        : object_(ptr.object_) {
        pm_allocator::add_ref(object_);
    }

    pm_shared_ptr_promise(pm_shared_ptr<T> const &ptr)
        : object_(ptr.object_) {
        pm_allocator::add_ref(object_);
    }

    pm_shared_ptr_promise &operator=(pm_shared_ptr_promise const &ptr) {
        pm_shared_ptr_promise(ptr).swap(*this);
        return *this;
    }

    bool operator==(pm_shared_ptr_promise const &ptr) const {
        return object_ == ptr.object_;
    }

    bool operator!=(pm_shared_ptr_promise const &ptr) const {
        return !(*this == ptr);
    }

    bool operator==(T const *ptr) const {
        return object_ == ptr;
    }

    bool operator!=(T const *ptr) const {
        return !(*this == ptr);
    }

    inline T *operator->() const {
        return object_;
    }

    inline T *obtain_rawptr() {
        pm_allocator::add_ref(object_);
        return object_;
    }

    inline void release_rawptr() {
        pm_allocator::dec_ref(object_);
    }

    pm_shared_ptr_promise find_pending() const {
        return object_->find_pending();
    }
    
    void reject_pending() {
        if(object_ != nullptr)
            object_->reject_pending();
    }

    void clear() {
        pm_shared_ptr_promise().swap(*this);
    }

    template <typename ...RET_ARG>
    void resolve(const RET_ARG &... ret_arg) const {
        object_->template resolve<RET_ARG...>(ret_arg...);
    }

    template <typename ...RET_ARG>
    void reject(const RET_ARG &... ret_arg) const {
        object_->template reject<RET_ARG...>(ret_arg...);
    }

    pm_shared_ptr_promise then(pm_shared_ptr_promise &promise) {
        return object_->then(promise);
    }

    template <typename FUNC_ON_RESOLVED, typename FUNC_ON_REJECTED>
    pm_shared_ptr_promise then(FUNC_ON_RESOLVED on_resolved, FUNC_ON_REJECTED on_rejected) const {
        return object_->template then<FUNC_ON_RESOLVED, FUNC_ON_REJECTED>(on_resolved, on_rejected);
    }

    template <typename FUNC_ON_RESOLVED>
    pm_shared_ptr_promise then(FUNC_ON_RESOLVED on_resolved) const {
        return object_->template then<FUNC_ON_RESOLVED>(on_resolved);
    }

    template <typename FUNC_ON_REJECTED>
    pm_shared_ptr_promise fail(FUNC_ON_REJECTED on_rejected) const {
        return object_->template fail<FUNC_ON_REJECTED>(on_rejected);
    }

    template <typename FUNC_ON_ALWAYS>
    pm_shared_ptr_promise always(FUNC_ON_ALWAYS on_always) const {
        return object_->template always<FUNC_ON_ALWAYS>(on_always);
    }

    template <typename FUNC_ON_FINALLY>
    pm_shared_ptr_promise finally(FUNC_ON_FINALLY on_finally) const {
        return object_->template finally<FUNC_ON_FINALLY>(on_finally);
    }

private:
    inline void swap(pm_shared_ptr_promise &ptr) {
        std::swap(object_, ptr.object_);
    }

    T *object_;
};

typedef pm_shared_ptr_promise<State> Promise;

typedef void(*FnSimple)();

template <typename RET, typename FUNC>
struct ResolveChecker;
template <typename RET, typename FUNC>
struct RejectChecker;
template<std::size_t ARG_SIZE, typename FUNC>
struct ExCheck;

inline Promise newHeadPromise(void);

struct PromiseCaller{
    virtual ~PromiseCaller(){};
    virtual Promise call(Promise &self, State *caller) = 0;
};

template <typename FUNC_ON_RESOLVED>
struct ResolvedCaller
    : public PromiseCaller{
    typedef typename func_traits<FUNC_ON_RESOLVED>::ret_type resolve_ret_type;
    FUNC_ON_RESOLVED on_resolved_;

    ResolvedCaller(const FUNC_ON_RESOLVED &on_resolved)
        : on_resolved_(on_resolved){}

    virtual Promise call(Promise &self, State *caller) {
        return ResolveChecker<resolve_ret_type, FUNC_ON_RESOLVED>::call(on_resolved_, self, caller);
    }
};

template <typename FUNC_ON_REJECTED>
struct RejectedCaller
    : public PromiseCaller{
    typedef typename func_traits<FUNC_ON_REJECTED>::ret_type reject_ret_type;
    FUNC_ON_REJECTED on_rejected_;

    RejectedCaller(const FUNC_ON_REJECTED &on_rejected)
        : on_rejected_(on_rejected){}

    virtual Promise call(Promise &self, State *caller) {
        return RejectChecker<reject_ret_type, FUNC_ON_REJECTED>::call(on_rejected_, self, caller);
    }
};

struct State {
    Promise next_;
    pm_stack::itr_t prev_;
    pm_any any_;
    PromiseCaller *resolved_;
    PromiseCaller *rejected_;

    enum status_t {
        kInit       = 0,
        kResolved   = 1,
        kRejected   = 2,
        kFinished   = 3
    };
    std::atomic<uint8_t> status_;//: 2;

    State(const State &) = delete;
    explicit State()
        : next_(nullptr)
        , prev_(pm_stack::ptr_to_itr(nullptr))
        , resolved_(nullptr)
        , rejected_(nullptr)
        , status_(kInit)
        {
        //printf("size promise = %d %d %d\n", (int)sizeof(*this), (int)sizeof(prev_), (int)sizeof(next_));
    }

    virtual ~State() {
        clear_func();
        if (next_.operator->()) {
            next_->prev_ = pm_stack::ptr_to_itr(nullptr);
        }
    }

    template <typename RET_ARG>
    void prepare_resolve(const RET_ARG &ret_arg) {
        if (status_ != kInit) return;
        status_ = kResolved;
        any_ = ret_arg;
    }

    template <typename ...RET_ARG>
    void resolve(const RET_ARG &... ret_arg) {
        typedef typename remove_reference_tuple<std::tuple<RET_ARG...>>::type arg_type;
        prepare_resolve(arg_type(ret_arg...));
        if(status_ == kResolved)
            call_next();
    }

    template <typename RET_ARG>
    void prepare_reject(const RET_ARG &ret_arg) {
        if (status_ != kInit) return;
        status_ = kRejected;
        any_ = ret_arg;
    }

    template <typename ...RET_ARG>
    void reject(const RET_ARG &...ret_arg) {
        typedef typename remove_reference_tuple<std::tuple<RET_ARG...>>::type arg_type;
        prepare_reject(arg_type(ret_arg...));
        if(status_ == kRejected)
            call_next();
    }

    Promise call_resolve(Promise &self, State *caller){
        if(resolved_ == nullptr){
            self->prepare_resolve(caller->any_);
            return self;
        }
        Promise ret = resolved_->call(self, caller);
        if(ret != self)
            joinDeferObject(self, ret);
        return ret;
    }

    Promise call_reject(Promise &self, State *caller){
        if(rejected_ == nullptr){
            self->prepare_reject(caller->any_);
            return self;
        }
        Promise ret = rejected_->call(self, caller);
        if(ret != self)
            joinDeferObject(self, ret);
        return ret;
    }

    void clear_func() {
        pm_delete(resolved_);
        resolved_ = nullptr;
        pm_delete(rejected_);
        rejected_ = nullptr;
    }

    template <typename FUNC>
    void run(FUNC func, Promise d) {
        try {
            func(d);
        } catch(const bad_any_cast &ex) {
            d->reject(ex);
        } catch(...) {
            d->reject(std::current_exception());
        }
    }
    
    Promise call_next() {
        if(status_ == kResolved) {
            if(next_.operator->()){
                pm_allocator::add_ref(this);
                status_ = kFinished;
                Promise d = next_->call_resolve(next_, this);
                this->any_.clear();
                next_->clear_func();
                if(d.operator->())
                    d->call_next();
                //next_.clear();
                pm_allocator::dec_ref(this);
                return d;
            }
        }
        else if(status_ == kRejected) {
            if(next_.operator->()){
                pm_allocator::add_ref(this);
                status_ = kFinished;
                Promise d = next_->call_reject(next_, this);
                this->any_.clear();
                next_->clear_func();
                if (d.operator->())
                    d->call_next();
                //next_.clear();
                pm_allocator::dec_ref(this);
                return d;
            }
        }

        return next_;
    }


    Promise then_impl(PromiseCaller *resolved, PromiseCaller *rejected){
        Promise promise = newHeadPromise();
        promise->resolved_ = resolved;
        promise->rejected_ = rejected;
        return then(promise);
    }

    Promise then(Promise &promise) {
        joinDeferObject(this, promise);
        //printf("2prev_ = %d %x %x\n", (int)promise->prev_, pm_stack::itr_to_ptr(promise->prev_), this);
        return call_next();
    }

    template <typename FUNC_ON_RESOLVED, typename FUNC_ON_REJECTED>
    Promise then(const FUNC_ON_RESOLVED &on_resolved, const FUNC_ON_REJECTED &on_rejected) {
        return then_impl(static_cast<PromiseCaller *>(pm_new<ResolvedCaller<FUNC_ON_RESOLVED>>(on_resolved)),
                         static_cast<PromiseCaller *>(pm_new<RejectedCaller<FUNC_ON_REJECTED>>(on_rejected)));
    }

    template <typename FUNC_ON_RESOLVED>
    Promise then(const FUNC_ON_RESOLVED &on_resolved) {
        return then_impl(static_cast<PromiseCaller *>(pm_new<ResolvedCaller<FUNC_ON_RESOLVED>>(on_resolved)),
                         static_cast<PromiseCaller *>(nullptr));
    }

    template <typename FUNC_ON_REJECTED>
    Promise fail(const FUNC_ON_REJECTED &on_rejected) {
        return then_impl(static_cast<PromiseCaller *>(nullptr),
                         static_cast<PromiseCaller *>(pm_new<RejectedCaller<FUNC_ON_REJECTED>>(on_rejected)));
    }

    template <typename FUNC_ON_ALWAYS>
    Promise always(const FUNC_ON_ALWAYS &on_always) {
        return then<FUNC_ON_ALWAYS, FUNC_ON_ALWAYS>(on_always, on_always);
    }

    template <typename FUNC_ON_FINALLY>
    Promise finally(const FUNC_ON_FINALLY &on_finally) {
        return then([on_finally](State *caller) -> Bypass {
            if(verify_func_arg(on_finally, caller->any_))
                call_func(on_finally, caller->any_);
            return Bypass();
        }, [on_finally](Promise &self, State *caller) -> Bypass {
            typedef typename func_traits<FUNC_ON_FINALLY>::arg_type arg_type;
            if (caller->any_.type() == typeid(std::exception_ptr)) {
                ExCheck<std::tuple_size<arg_type>::value, FUNC_ON_FINALLY>::call(on_finally, self, caller);
            }
            else {
                if (verify_func_arg(on_finally, caller->any_))
                    call_func(on_finally, caller->any_);
            }
            return Bypass();
        });
    }


    Promise find_pending() {
        if (status_ == kInit) {
            State *p = this;
            State *prev = static_cast<State *>(pm_stack::itr_to_ptr(p->prev_));
            while (prev != nullptr) {
                if (prev->status_ != kInit)
                    return prev->next_;
                p = prev;
                prev = static_cast<State *>(pm_stack::itr_to_ptr(p->prev_));
            }

            pm_allocator::add_ref(p);
            return Promise(p);
        }
        else {
            State *p = this;
            State *next = p->next_.operator->();
            while (next != nullptr) {
                if (next->status_ == kInit)
                    return p->next_;
                p = next;
                next = p->next_.operator->();
            }
            return Promise();
        }
    }

    void reject_pending(){
        Promise pending = find_pending();
        if(pending.operator->() != nullptr)
            pending.reject();
    }

    static State *get_head(State *p){
        while(p){
            State *prev = static_cast<State *>(pm_stack::itr_to_ptr(p->prev_));
            if(prev == nullptr) break;
            p = prev;
        }
        return p;
    }
    static State *get_tail(State *p){
        while(p){
            Promise &next = p->next_;
            if(next.operator->() == nullptr) break;
            p = next.operator->();
        }
        return p;
    }
    
    
    static inline void joinDeferObject(State *self, Promise &next){
        /* Check if there's any functions return null Promise object */
        pm_assert(next.operator->() != nullptr);

        State *head = get_head(next.operator->());
        State *tail = get_tail(next.operator->());

        if(self->next_.operator->()){
            self->next_->prev_ = pm_stack::ptr_to_itr(reinterpret_cast<void *>(tail));
            //printf("5prev_ = %d %x\n", (int)self->next_->prev_, pm_stack::itr_to_ptr(self->next_->prev_));
        }
        tail->next_ = self->next_;
        pm_allocator::add_ref(head);
        self->next_ = Promise(head);
        head->prev_ = pm_stack::ptr_to_itr(reinterpret_cast<void *>(self));
        //printf("6prev_ = %d %x\n", (int)next->prev_, pm_stack::itr_to_ptr(next->prev_));
    }

    static inline void joinDeferObject(Promise &self, Promise &next){
        joinDeferObject(self.operator->(), next);
    }
};


template <typename RET, typename FUNC>
struct ResolveChecker {
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            if (verify_func_arg(func, caller->any_))
                self->prepare_resolve(call_func(func, caller->any_));
            else
                self->prepare_reject(caller->any_);
        } catch(...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template <typename FUNC>
struct ResolveChecker<Promise, FUNC> {
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            if (verify_func_arg(func, caller->any_)) {
                Promise ret = std::get<0>(call_func(func, caller->any_));
                return ret;
            }
            else {
                self->prepare_reject(caller->any_);
                return self;
            }
        } catch(...) {
            self->prepare_reject(std::current_exception());
            return self;
        }
    }
};

template <typename FUNC>
struct ResolveChecker<Bypass, FUNC> {
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            pm_any any = caller->any_;
            func(caller);
            self->prepare_resolve(any);
        }
        catch (...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template <typename RET>
struct ResolveChecker<RET, FnSimple> {
    static Promise call(const FnSimple &func, Promise &self, State *caller) {
        try {
            if (func == nullptr)
                self->prepare_resolve(caller->any_);
            else if (verify_func_arg(func, caller->any_))
                self->prepare_resolve(call_func(func, caller->any_));
            else
                self->prepare_reject(caller->any_);
        } catch(...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template<std::size_t ARG_SIZE, typename FUNC>
struct ExCheck {
    static void call(const FUNC &, Promise &, State *caller) {
        std::exception_ptr eptr = any_cast<std::exception_ptr>(caller->any_);
        throw eptr;
    }
};

template <typename FUNC>
struct ExCheck<0, FUNC> {
    static auto call(const FUNC &func, Promise &, State *caller)
        -> typename call_tuple_ret_t<typename func_traits<FUNC>::ret_type>::ret_type {
        pm_any arg = std::tuple<>();
        caller->any_.clear();
        return call_func(func, arg);
    }
};

template <typename FUNC>
struct ExCheck<1, FUNC> {
    typedef typename func_traits<FUNC>::arg_type arg_type;
    static auto call(const FUNC &func, Promise &, State *caller)
        -> typename call_tuple_ret_t<typename func_traits<FUNC>::ret_type>::ret_type {
        std::exception_ptr eptr = any_cast<std::exception_ptr>(caller->any_);
        try {
            std::rethrow_exception(eptr);
        }
        catch (const typename std::tuple_element<0, arg_type>::type &ret_arg) {
            pm_any arg = arg_type(ret_arg);
            caller->any_.clear();
            return call_func(func, arg);
        }

        /* Will never run to here, just make the compile satisfied! */
        pm_any arg;
        return call_func(func, arg);
    }
};

template <typename RET, typename FUNC>
struct RejectChecker {
    typedef typename func_traits<FUNC>::arg_type arg_type;
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            if(caller->any_.type() == typeid(std::exception_ptr)){
                self->prepare_resolve(ExCheck<std::tuple_size<arg_type>::value, FUNC>::call(func, self, caller));
            }
            else if (verify_func_arg(func, caller->any_))
                self->prepare_resolve(call_func(func, caller->any_));
            else
                self->prepare_reject(caller->any_);
        } catch(...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template <typename FUNC>
struct RejectChecker<Promise, FUNC> {
    typedef typename func_traits<FUNC>::arg_type arg_type;
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            if(caller->any_.type() == typeid(std::exception_ptr)){
                Promise ret = std::get<0>(ExCheck<std::tuple_size<arg_type>::value, FUNC>::call(func, self, caller));
                return ret;
            }
            else if (verify_func_arg(func, caller->any_)) {
                Promise ret = std::get<0>(call_func(func, caller->any_));
                return ret;
            }
            else {
                self->prepare_reject(caller->any_);
                return self;
            }
        }
        catch(...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template <typename FUNC>
struct RejectChecker<Bypass, FUNC> {
    typedef typename func_traits<FUNC>::arg_type arg_type;
    static Promise call(const FUNC &func, Promise &self, State *caller) {
        try {
            pm_any any = caller->any_;
            func(self, caller);
            self->prepare_reject(any);
        }
        catch (...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

template <typename RET>
struct RejectChecker<RET, FnSimple> {
    static Promise call(const FnSimple &func, Promise &self, State *caller) {
        try {
            if (func == nullptr)
                self->prepare_reject(caller->any_);
            else if (verify_func_arg(func, caller->any_))
                self->prepare_resolve(call_func(func, caller->any_));
            else
                self->prepare_reject(caller->any_);
        } catch(...) {
            self->prepare_reject(std::current_exception());
        }
        return self;
    }
};

inline Promise newHeadPromise(){
    return Promise(pm_new<State>());
}

/* Create new promise object */
template <typename FUNC>
inline Promise newPromise(FUNC func) {
    Promise promise = newHeadPromise();
    promise->run(func, promise);
    return promise;
}

/* Loop while func call resolved */
template <typename FUNC>
inline Promise While(FUNC func) {
    return newPromise(func).then([func]() {
        return While(func);
    });
}

#ifdef PROMISE_HAS_VARIADIC_CAPTURE
/* Return a rejected promise directly */
template <typename ...RET_ARG>
inline Promise reject(const RET_ARG &... ret_arg){
    return newPromise([=](Promise &d){ d.reject(ret_arg...); });
}
/* Return a resolved promise directly */
template <typename ...RET_ARG>
inline Promise resolve(const RET_ARG &... ret_arg){
    return newPromise([=](Promise &d){ d.resolve(ret_arg...); });
}
#endif // PROMISE_HAS_VARIADIC_CAPTURE

}
} // namespace hprose/util