// Copyright (c) 2016 Till Kolditz, Stefan de Bruijn
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
 * File:   AN_avx2_16x16_16x32.hpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 13. Dezember 2016, 00:11
 */

#pragma once

#include <cstdint>

#include "ANTest.hpp"

template<size_t UNROLL>
struct AN_avx2_16x16_16x32 : public ANTest<uint16_t, uint32_t, UNROLL>, public AVX2Test {

    AN_avx2_16x16_16x32 (const char* const name, AlignedBlock & in, AlignedBlock & out, uint32_t A, uint32_t Ainv) :
            ANTest<uint16_t, uint32_t, UNROLL>(name, in, out, A, Ainv) {
    }

    virtual
    ~AN_avx2_16x16_16x32 () {
    }

    void
    RunEnc (const size_t numIterations) override {
        for (size_t iter = 0; iter < numIterations; ++iter) {
            __m256i *dataIn = this->in.template begin<__m256i>();
            __m256i *dataInEnd = this->in.template end<__m256i>();
            __m128i *dataOut = this->out.template begin<__m128i>(); // since AVX2 does not shuffle as desired, we need to store 128-bit vectors only
            __m256i mmA = _mm256_set1_epi32(this->A);

            // _mm256_shuffle_epi8 works only on 128-bit lanes, so we have to work on the first 4 16-bit values and the third ones, then on the second and fourth ones
            __m256i mmShuffle1 = _mm256_set_epi32(0xFFFF0706, 0xFFFF0504, 0xFFFF0302, 0xFFFF0100, 0xFFFF0706, 0xFFFF0504, 0xFFFF0302, 0xFFFF0100);
            __m256i mmShuffle2 = _mm256_set_epi32(0xFFFF0F0E, 0xFFFF0D0C, 0xFFFF0B0A, 0xFFFF0908, 0xFFFF0F0E, 0xFFFF0D0C, 0xFFFF0B0A, 0xFFFF0908);
            while (dataIn <= (dataInEnd - UNROLL)) {
                // let the compiler unroll the loop
                for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                    __m256i m256 = _mm256_lddqu_si256(dataIn++);
                    __m256i tmp1 = _mm256_mullo_epi32(_mm256_shuffle_epi8(m256, mmShuffle1), mmA);
                    __m256i tmp2 = _mm256_mullo_epi32(_mm256_shuffle_epi8(m256, mmShuffle2), mmA);
                    _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp1, 0));
                    _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp2, 0));
                    _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp1, 1));
                    _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp2, 1));
                }
            }

            while (dataIn <= (dataInEnd - 1)) {
                __m256i m256 = _mm256_lddqu_si256(dataIn++);
                __m256i tmp1 = _mm256_mullo_epi32(_mm256_shuffle_epi8(m256, mmShuffle1), mmA);
                __m256i tmp2 = _mm256_mullo_epi32(_mm256_shuffle_epi8(m256, mmShuffle2), mmA);
                _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp1, 0));
                _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp2, 0));
                _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp1, 1));
                _mm_storeu_si128(dataOut++, _mm256_extractf128_si256(tmp2, 1));
            }

            // multiply remaining numbers sequentially
            if (dataIn < dataInEnd) {
                auto data16 = reinterpret_cast<uint16_t*>(dataIn);
                auto data16End = reinterpret_cast<uint16_t*>(dataInEnd);
                auto out32 = reinterpret_cast<uint32_t*>(dataOut);
                do {
                    *out32++ = static_cast<uint32_t>(*data16++) * this->A;
                } while (data16 < data16End);
            }
        }
    }
};