// Copyright (c) 2017 Till Kolditz
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * File:   Hamming_avx2.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 15-08-2017 16:16
 */

#ifdef __AVX2__

#include <Hamming/Hamming_simd.hpp>

/*
 * For the following algorithms, see
 * https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 * Credit also goes to the book "Hacker's Delight" 2nd Edition, by Henry S. Warren, Jr., published at Addison-Wesley
 */

namespace coding_benchmark {

    template struct hamming_t<uint16_t, __m256i >;
    template struct hamming_t<uint32_t, __m256i >;

    template<>
    __m128i hamming_t<uint16_t, __m256i >::computeHamming(
            __m256i data) {
        auto pattern1 = _mm256_set1_epi16(static_cast<int16_t>(0xAD5B));
        auto pattern2 = _mm256_set1_epi16(static_cast<int16_t>(0x366D));
        auto pattern3 = _mm256_set1_epi16(static_cast<int16_t>(0xC78E));
        auto pattern4 = _mm256_set1_epi16(static_cast<int16_t>(0x07F0));
        auto pattern5 = _mm256_set1_epi16(static_cast<int16_t>(0xF800));
        auto mask = _mm_set1_epi8(0x01);
        __m128i tmp2 = mm<__m256i, uint16_t>::popcount(_mm256_and_si256(data, pattern1));
        __m128i hamming = _mm_slli_epi16(_mm_and_si128(tmp2, mask), 1);
        __m128i tmp1 = mm<__m256i, uint16_t>::popcount(_mm256_and_si256(data, pattern2));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 2));
        tmp1 = mm<__m256i, uint16_t>::popcount(_mm256_and_si256(data, pattern3));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 3));
        tmp1 = mm<__m256i, uint16_t>::popcount(_mm256_and_si256(data, pattern4));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 4));
        tmp1 = mm<__m256i, uint16_t>::popcount(_mm256_and_si256(data, pattern5));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 5));
        tmp1 = _mm_add_epi8(mm<__m256i, uint16_t>::popcount(std::forward<__m256i >(data)), tmp2);
        tmp1 = _mm_and_si128(tmp1, mask);
        hamming = _mm_or_si128(hamming, tmp1);
        return hamming;
    }

    template<>
    __m128i hamming_t<uint16_t, __m256i >::computeHamming2(
            __m256i data) {
        static auto pattern1 = _mm256_set1_epi16(static_cast<int16_t>(0xAD5B));
        static auto pattern2 = _mm256_set1_epi16(static_cast<int16_t>(0x366D));
        static auto pattern3 = _mm256_set1_epi16(static_cast<int16_t>(0xC78E));
        static auto pattern4 = _mm256_set1_epi16(static_cast<int16_t>(0x07F0));
        static auto pattern5 = _mm256_set1_epi16(static_cast<int16_t>(0xF800));
        static auto mask = _mm_set1_epi8(0x01);
        __m128i tmp2 = mm<__m256i, uint16_t>::popcount2(_mm256_and_si256(data, pattern1));
        __m128i hamming = _mm_slli_epi16(_mm_and_si128(tmp2, mask), 1);
        __m128i tmp1 = mm<__m256i, uint16_t>::popcount2(_mm256_and_si256(data, pattern2));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 2));
        tmp1 = mm<__m256i, uint16_t>::popcount2(_mm256_and_si256(data, pattern3));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 3));
        tmp1 = mm<__m256i, uint16_t>::popcount2(_mm256_and_si256(data, pattern4));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 4));
        tmp1 = mm<__m256i, uint16_t>::popcount2(_mm256_and_si256(data, pattern5));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 5));
        tmp1 = _mm_add_epi8(mm<__m256i, uint16_t>::popcount2(std::forward<__m256i >(data)), tmp2);
        tmp1 = _mm_and_si128(tmp1, mask);
        hamming = _mm_or_si128(hamming, tmp1);
        return hamming;
    }

    template<>
    __m128i hamming_t<uint16_t, __m256i >::computeHamming3(
            __m256i data) {
        static auto pattern1 = _mm256_set1_epi16(static_cast<int16_t>(0xAD5B));
        static auto pattern2 = _mm256_set1_epi16(static_cast<int16_t>(0x366D));
        static auto pattern3 = _mm256_set1_epi16(static_cast<int16_t>(0xC78E));
        static auto pattern4 = _mm256_set1_epi16(static_cast<int16_t>(0x07F0));
        static auto pattern5 = _mm256_set1_epi16(static_cast<int16_t>(0xF800));
        static auto mask = _mm_set1_epi8(0x01);
        __m128i tmp2 = mm<__m256i, uint16_t>::popcount3(_mm256_and_si256(data, pattern1));
        __m128i hamming = _mm_slli_epi16(_mm_and_si128(tmp2, mask), 1);
        __m128i tmp1 = mm<__m256i, uint16_t>::popcount3(_mm256_and_si256(data, pattern2));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 2));
        tmp1 = mm<__m256i, uint16_t>::popcount3(_mm256_and_si256(data, pattern3));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 3));
        tmp1 = mm<__m256i, uint16_t>::popcount3(_mm256_and_si256(data, pattern4));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 4));
        tmp1 = mm<__m256i, uint16_t>::popcount3(_mm256_and_si256(data, pattern5));
        tmp1 = _mm_and_si128(tmp1, mask);
        tmp2 = _mm_xor_si128(tmp2, tmp1);
        hamming = _mm_or_si128(hamming, _mm_slli_epi16(tmp1, 5));
        tmp1 = _mm_add_epi8(mm<__m256i, uint16_t>::popcount3(std::forward<__m256i >(data)), tmp2);
        tmp1 = _mm_and_si128(tmp1, mask);
        hamming = _mm_or_si128(hamming, tmp1);
        return hamming;
    }

    template<>
    bool hamming_t<uint16_t, __m256i >::code_cmp_eq(
            __m128i hamming1,
            __m128i hamming2) {
        return _mm_movemask_epi8(_mm_cmpeq_epi8(hamming1, hamming2)) == 0xFFFF; // all equal?
    }

    template<>
    bool hamming_t<uint16_t, __m256i >::isValid() {
        return hamming_t<uint16_t, __m256i >::code_cmp_eq(this->code, hamming_t<uint16_t, __m256i >::computeHamming(this->data));
    }

    template<>
    void hamming_t<uint16_t, __m256i >::store(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        _mm_storeu_si128(&this->code, computeHamming(data));
    }

    template<>
    void hamming_t<uint16_t, __m256i >::store2(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        _mm_storeu_si128(&this->code, computeHamming2(data));
    }

    template<>
    void hamming_t<uint16_t, __m256i >::store3(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        _mm_storeu_si128(&this->code, computeHamming3(data));
    }

    template<>
    uint64_t hamming_t<uint32_t, __m256i >::computeHamming(
            __m256i data) {
        static auto pattern1 = _mm256_set1_epi32(0x56AAAD5B);
        static auto pattern2 = _mm256_set1_epi32(0x9B33366D);
        static auto pattern3 = _mm256_set1_epi32(0xE3C3C78E);
        static auto pattern4 = _mm256_set1_epi32(0x03FC07F0);
        static auto pattern5 = _mm256_set1_epi32(0x03FFF800);
        static auto pattern6 = _mm256_set1_epi32(0xFC000000);
        uint64_t hamming = 0;
        uint64_t tmp1(0), tmp2(0);
        tmp2 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern1)) & 0x0101010101010101;
        hamming |= tmp2 << 1;
        tmp1 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern2)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 2;
        tmp1 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern3)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 3;
        tmp1 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern4)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 4;
        tmp1 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern5)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 5;
        tmp1 = mm<__m256i, uint32_t>::popcount(_mm256_and_si256(data, pattern6)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 6;
        hamming |= (mm<__m256i, uint32_t>::popcount(std::forward<__m256i >(data)) + tmp2) & 0x0101010101010101;
        return hamming;
    }

    template<>
    uint64_t hamming_t<uint32_t, __m256i >::computeHamming2(
            __m256i data) {
        static auto pattern1 = _mm256_set1_epi32(0x56AAAD5B);
        static auto pattern2 = _mm256_set1_epi32(0x9B33366D);
        static auto pattern3 = _mm256_set1_epi32(0xE3C3C78E);
        static auto pattern4 = _mm256_set1_epi32(0x03FC07F0);
        static auto pattern5 = _mm256_set1_epi32(0x03FFF800);
        static auto pattern6 = _mm256_set1_epi32(0xFC000000);
        uint64_t hamming = 0;
        uint64_t tmp1(0), tmp2(0);
        tmp2 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern1)) & 0x0101010101010101;
        hamming |= tmp2 << 1;
        tmp1 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern2)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 2;
        tmp1 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern3)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 3;
        tmp1 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern4)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 4;
        tmp1 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern5)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 5;
        tmp1 = mm<__m256i, uint32_t>::popcount2(_mm256_and_si256(data, pattern6)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 6;
        hamming |= (mm<__m256i, uint32_t>::popcount2(std::forward<__m256i >(data)) + tmp2) & 0x0101010101010101;
        return hamming;
    }

    template<>
    uint64_t hamming_t<uint32_t, __m256i >::computeHamming3(
            __m256i data) {
        static auto pattern1 = _mm256_set1_epi32(0x56AAAD5B);
        static auto pattern2 = _mm256_set1_epi32(0x9B33366D);
        static auto pattern3 = _mm256_set1_epi32(0xE3C3C78E);
        static auto pattern4 = _mm256_set1_epi32(0x03FC07F0);
        static auto pattern5 = _mm256_set1_epi32(0x03FFF800);
        static auto pattern6 = _mm256_set1_epi32(0xFC000000);
        uint64_t hamming = 0;
        uint64_t tmp1(0), tmp2(0);
        tmp2 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern1)) & 0x0101010101010101;
        hamming |= tmp2 << 1;
        tmp1 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern2)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 2;
        tmp1 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern3)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 3;
        tmp1 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern4)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 4;
        tmp1 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern5)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 5;
        tmp1 = mm<__m256i, uint32_t>::popcount3(_mm256_and_si256(data, pattern6)) & 0x0101010101010101;
        tmp2 ^= tmp1;
        hamming |= tmp1 << 6;
        hamming |= (mm<__m256i, uint32_t>::popcount3(std::forward<__m256i >(data)) + tmp2) & 0x0101010101010101;
        return hamming;
    }

    template<>
    bool hamming_t<uint32_t, __m256i >::code_cmp_eq(
            uint64_t hamming1,
            uint64_t hamming2) {
        return hamming1 == hamming2; // all equal?
    }

    template<>
    bool hamming_t<uint32_t, __m256i >::isValid() {
        return hamming_t<uint32_t, __m256i >::code_cmp_eq(this->code, hamming_t<uint32_t, __m256i >::computeHamming(this->data));
    }

    template<>
    void hamming_t<uint32_t, __m256i >::store(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        this->code = computeHamming(data);
    }

    template<>
    void hamming_t<uint32_t, __m256i >::store2(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        this->code = computeHamming2(data);
    }

    template<>
    void hamming_t<uint32_t, __m256i >::store3(
            __m256i data) {
        _mm256_storeu_si256(&this->data, data);
        this->code = computeHamming3(data);
    }

}

#endif
