// ----------------------------------------------------------------------
// Copyright (c) 2016, Steven Gregory Popovitch - greg7mdp@gmail.com
// All rights reserved.
// 
// Code derived derived from Boost libraries.
// Boost software licence reproduced below.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * The name of Steven Gregory Popovitch may not be used to 
// endorse or promote products derived from this software without 
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Boost Software License - Version 1.0 - August 17th, 2003
// 
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// ---------------------------------------------------------------------------

//  ----------------------------------------------------------------------
//                  H A S H    F U N C T I O N S
//                  ----------------------------
//
//    Implements spp::spp_hash() and spp::hash_combine()
//
//    The exact same code is duplicated in sparsepp.h.
//
//    WARNING: Any change here has to be duplicated in sparsepp.h.
//  ----------------------------------------------------------------------

#if !defined(spp_utils_h_guard_)
#define spp_utils_h_guard_

#if defined(_MSC_VER) 
    #if (_MSC_VER >= 1600 )                      // vs2010 (1900 is vs2015)
        #include <functional>
        #define SPP_HASH_CLASS std::hash
    #else
        #include  <hash_map>
        #define SPP_HASH_CLASS stdext::hash_compare
    #endif
    #if (_MSC_FULL_VER < 190021730)
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#elif defined(__GNUC__)
    #if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
        #include <functional>
        #define SPP_HASH_CLASS std::hash

        #if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) < 40600
            #define SPP_NO_CXX11_NOEXCEPT
        #endif
    #else
        #include <tr1/unordered_map>
        #define SPP_HASH_CLASS std::tr1::hash
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#elif defined __clang__
    #include <functional>
    #define SPP_HASH_CLASS  std::hash

    #if !__has_feature(cxx_noexcept)
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#else
    #include <functional>
    #define SPP_HASH_CLASS  std::hash
#endif

#ifdef SPP_NO_CXX11_NOEXCEPT
    #define SPP_NOEXCEPT
#else
    #define SPP_NOEXCEPT noexcept
#endif

#ifdef SPP_NO_CXX11_CONSTEXPR
    #define SPP_CONSTEXPR
#else
    #define SPP_CONSTEXPR constexpr
#endif

#define SPP_INLINE

#ifndef SPP_NAMESPACE
    #define SPP_NAMESPACE spp
#endif

namespace SPP_NAMESPACE
{

template <class T>
struct spp_hash
{
    SPP_INLINE size_t operator()(const T &__v) const SPP_NOEXCEPT 
    {
        SPP_HASH_CLASS<T> hasher;
        return hasher(__v);
    }
};

template <class T>
struct spp_hash<T *>
{
    static size_t spp_log2 (size_t val) SPP_NOEXCEPT 
    {
        size_t res = 0;
        while (val > 1) 
        {
            val >>= 1;
            res++;
        }
        return res;
    }

    SPP_INLINE size_t operator()(const T *__v) const SPP_NOEXCEPT 
    {
        static const size_t shift = 3; // spp_log2(1 + sizeof(T)); // T might be incomplete!
        return static_cast<size_t>((*(reinterpret_cast<const uintptr_t *>(&__v))) >> shift);
    }
};

// from http://burtleburtle.net/bob/hash/integer.html
// fast and efficient for power of two table sizes where we always 
// consider the last bits.
// ---------------------------------------------------------------
inline size_t spp_mix_32(uint32_t a)
{
    a = a ^ (a >> 4);
    a = (a ^ 0xdeadbeef) + (a << 5);
    a = a ^ (a >> 11);
    return static_cast<size_t>(a);
}

// Maybe we should do a more thorough scrambling as described in 
// https://gist.github.com/badboy/6267743
// -------------------------------------------------------------
inline size_t spp_mix_64(uint64_t a)
{
    a = a ^ (a >> 4);
    a = (a ^ 0xdeadbeef) + (a << 5);
    a = a ^ (a >> 11);
    return a;
}

template <>
struct spp_hash<bool> : public std::unary_function<bool, size_t>
{
    SPP_INLINE size_t operator()(bool __v) const SPP_NOEXCEPT 
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<char> : public std::unary_function<char, size_t>
{
    SPP_INLINE size_t operator()(char __v) const SPP_NOEXCEPT 
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<signed char> : public std::unary_function<signed char, size_t>
{
    SPP_INLINE size_t operator()(signed char __v) const SPP_NOEXCEPT 
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<unsigned char> : public std::unary_function<unsigned char, size_t>
{
    SPP_INLINE size_t operator()(unsigned char __v) const SPP_NOEXCEPT 
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<wchar_t> : public std::unary_function<wchar_t, size_t>
{
    SPP_INLINE size_t operator()(wchar_t __v) const SPP_NOEXCEPT 
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<int16_t> : public std::unary_function<int16_t, size_t>
{
    SPP_INLINE size_t operator()(int16_t __v) const SPP_NOEXCEPT 
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <> 
struct spp_hash<uint16_t> : public std::unary_function<uint16_t, size_t>
{
    SPP_INLINE size_t operator()(uint16_t __v) const SPP_NOEXCEPT 
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<int32_t> : public std::unary_function<int32_t, size_t>
{
    SPP_INLINE size_t operator()(int32_t __v) const SPP_NOEXCEPT 
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<uint32_t> : public std::unary_function<uint32_t, size_t>
{
    SPP_INLINE size_t operator()(uint32_t __v) const SPP_NOEXCEPT 
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<int64_t> : public std::unary_function<int64_t, size_t>
{
    SPP_INLINE size_t operator()(int64_t __v) const SPP_NOEXCEPT 
    { return spp_mix_64(static_cast<uint64_t>(__v)); }
};

template <>
struct spp_hash<uint64_t> : public std::unary_function<uint64_t, size_t>
{
    SPP_INLINE size_t operator()(uint64_t __v) const SPP_NOEXCEPT 
    { return spp_mix_64(static_cast<uint64_t>(__v)); }
};

template <>
struct spp_hash<float> : public std::unary_function<float, size_t>
{
    SPP_INLINE size_t operator()(float __v) const SPP_NOEXCEPT
    {
        // -0.0 and 0.0 should return same hash
        uint32_t *as_int = reinterpret_cast<uint32_t *>(&__v);
        return (__v == 0) ? static_cast<size_t>(0) : spp_mix_32(*as_int);
    }
};

template <>
struct spp_hash<double> : public std::unary_function<double, size_t>
{
    SPP_INLINE size_t operator()(double __v) const SPP_NOEXCEPT
    {
        // -0.0 and 0.0 should return same hash
        uint64_t *as_int = reinterpret_cast<uint64_t *>(&__v);
        return (__v == 0) ? static_cast<size_t>(0) : spp_mix_64(*as_int);
    }
};

template <class T, int sz> struct Combiner
{
    inline void operator()(T& seed, T value);
};

template <class T> struct Combiner<T, 4>
{
    inline void  operator()(T& seed, T value)
    {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};

template <class T> struct Combiner<T, 8>
{
    inline void  operator()(T& seed, T value)
    {
        seed ^= value + T(0xc6a4a7935bd1e995) + (seed << 6) + (seed >> 2);
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    spp::spp_hash<T> hasher;
    Combiner<std::size_t, sizeof(std::size_t)> combiner;

    combiner(seed, hasher(v));
}
    
}

#endif // spp_utils_h_guard_

