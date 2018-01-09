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

#pragma once

#ifndef AN_AVX2
#error "Clients must not include this file directly, but file <AN/AN_avx2.hpp>!"
#endif

#include <AN/AN_avx2_16x16_16x32.hpp>

namespace coding_benchmark {

    template<size_t UNROLL>
    struct AN_avx2_16x16_16x32_u_inv :
            public AN_avx2_16x16_16x32<uint16_t, uint32_t, UNROLL> {

        typedef AN_avx2_16x16_16x32<uint16_t, uint32_t, UNROLL> BASE;
        typedef simd::mm<__m256i, uint32_t> mmEnc;
        typedef simd::mm_op<__m256i, uint32_t, std::less_equal> mmEncLE;

        using BASE::NUM_VALUES_PER_SIMDREG;
        using BASE::NUM_VALUES_PER_UNROLL;

        using BASE::AN_avx2_16x16_16x32;

        virtual ~AN_avx2_16x16_16x32_u_inv() {
        }

        virtual bool DoCheck() override {
            return true;
        }

        virtual void RunCheck(
                const CheckConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                auto mmData = this->bufEncoded.template begin<__m256i >();
                auto mmDataEnd = this->bufEncoded.template end<__m256i >();
                uint32_t dMax = std::numeric_limits<uint16_t>::max();
                __m256i mmDMax = _mm256_set1_epi32(dMax); // we assume 16-bit input data
                __m256i mmAInv = _mm256_set1_epi32(this->A_INV);
                while (mmData <= (mmDataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t k = 0; k < UNROLL; ++k) {
                        auto mmInDec = _mm256_mullo_epi32(_mm256_lddqu_si256(mmData), mmAInv);
                        if (mmEncLE::cmp_mask(mmInDec, mmDMax) == mmEnc::FULL_MASK) {
                            ++mmData;
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - this->bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
                // here follows the non-unrolled remainder
                while (mmData <= (mmDataEnd - 1)) {
                    auto mmInDec = _mm256_mullo_epi32(_mm256_lddqu_si256(mmData), mmAInv);
                    if (mmEncLE::cmp_mask(mmInDec, mmDMax) == mmEnc::FULL_MASK) {
                        ++mmData;
                    } else {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - this->bufEncoded.template begin<uint32_t>(), iteration);
                    }
                }
                if (mmData < mmDataEnd) {
                    auto dataEnd2 = reinterpret_cast<uint32_t*>(mmDataEnd);
                    auto data2 = reinterpret_cast<uint32_t*>(mmData);
                    while (data2 < dataEnd2) {
                        if ((*data2 * this->A_INV) > dMax) {
                            ++data2;
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(data2) - this->bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
            }
        }

        bool DoArithmeticChecked(
                const ArithmeticConfiguration & config) override {
            return std::visit(ArithmeticSelector(), config.mode);
        }

        struct ArithmetorChecked {
            AN_avx2_16x16_16x32_u_inv & test;
            const ArithmeticConfiguration & config;
            const size_t iteration;
            ArithmetorChecked(
                    AN_avx2_16x16_16x32_u_inv & test,
                    const ArithmeticConfiguration & config,
                    const size_t iteration)
                    : test(test),
                      config(config),
                      iteration(iteration) {
            }
            template<template<typename = void> class func>
            void impl() {
                uint32_t dMax = std::numeric_limits<uint16_t>::max();
                __m256i mmDMax = _mm256_set1_epi32(dMax); // we assume 16-bit input data
                __m256i mmAinv = _mm256_set1_epi32(test.A_INV);
                auto mmData = test.bufEncoded.template begin<__m256i >();
                const auto mmDataEnd = test.bufEncoded.template end<__m256i >();
                auto mmOut = test.bufResult.template begin<__m256i >();
                uint32_t operandEnc = config.operand * test.A;
                auto mmOperandEnc = mm<__m256i, uint32_t>::set1(operandEnc);
                while (mmData <= (mmDataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                        auto mmIn = _mm256_lddqu_si256(mmData++);
                        auto mmIn2 = _mm256_mullo_epi32(mmIn, mmAinv);
                        if (simd::mm_op<__m256i, uint32_t, std::greater_equal>::cmp_mask(mmIn2, mmDMax) == simd::mm<__m256i, uint32_t>::FULL_MASK) {
                            _mm256_storeu_si256(mmOut++, mm_op<__m256i, uint32_t, func>::compute(mmIn, mmOperandEnc));
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - test.bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
                // remaining numbers
                while (mmData <= (mmDataEnd - 1)) {
                    auto mmIn = _mm256_lddqu_si256(mmData++);
                    auto mmIn2 = _mm256_mullo_epi32(mmIn, mmAinv);
                    if (simd::mm_op<__m256i, uint32_t, std::greater_equal>::cmp_mask(mmIn2, mmDMax) == simd::mm<__m256i, uint32_t>::FULL_MASK) {
                        _mm256_storeu_si256(mmOut++, mm_op<__m256i, uint32_t, func>::compute(mmIn, mmOperandEnc));
                    } else {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - test.bufEncoded.template begin<uint32_t>(), iteration);
                    }
                }
                if (mmData < mmDataEnd) {
                    func<> functor;
                    auto data32End = reinterpret_cast<uint32_t*>(mmDataEnd);
                    auto out32 = reinterpret_cast<uint32_t*>(mmOut);
                    for (auto data32 = reinterpret_cast<uint32_t*>(mmData); data32 < data32End; ++data32, ++out32) {
                        auto tmp = *data32 * test.A_INV;
                        if (tmp <= dMax) {
                            *out32 = functor(*data32, operandEnc);
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, data32 - test.bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
            }
            void operator()(
                    ArithmeticConfiguration::Add) {
                impl<add>();
            }
            void operator()(
                    ArithmeticConfiguration::Sub) {
                impl<sub>();
            }
            void operator()(
                    ArithmeticConfiguration::Mul) {
                impl<mul>();
            }
            void operator()(
                    ArithmeticConfiguration::Div) {
                impl<div>();
            }
        };

        void RunArithmeticChecked(
                const ArithmeticConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                std::visit(ArithmetorChecked(*this, config, iteration), config.mode);
            }
        }

        bool DoDecodeChecked() override {
            return true;
        }

        void RunDecodeChecked(
                const DecodeConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                size_t numValues = this->getNumValues();
                size_t i = 0;
                uint32_t dMax = std::numeric_limits<uint16_t>::max();
                __m256i mmDMax = _mm256_set1_epi32(dMax); // we assume 16-bit input data
                auto mmData = this->bufEncoded.template begin<__m256i >();
                auto mmOut = this->bufResult.template begin<__m128i >();
                auto mmAInv = _mm256_set1_epi32(this->A_INV);
                auto mmShuffle = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll, 0x1D1C191815141110ll, 0x0D0C090805040100ll);
                for (; i <= (numValues - NUM_VALUES_PER_UNROLL); i += NUM_VALUES_PER_UNROLL) {
                    // let the compiler unroll the loop
                    for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                        auto mmIn = _mm256_lddqu_si256(mmData++);
                        auto mmInDec = _mm256_mullo_epi32(mmIn, mmAInv);
                        if (simd::mm_op<__m256i, uint32_t, std::greater_equal>::cmp_mask(mmInDec, mmDMax) == simd::mm<__m256i, uint32_t>::FULL_MASK) {
                            _mm_storeu_si128(mmOut++, _mm256_extracti128_si256(_mm256_shuffle_epi8(mmInDec, mmShuffle), 0));
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - this->bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
                // remaining numbers
                for (; i <= (numValues - NUM_VALUES_PER_SIMDREG); i += NUM_VALUES_PER_SIMDREG) {
                    auto mmIn = _mm256_lddqu_si256(mmData++);
                    auto mmInDec = _mm256_mullo_epi32(mmIn, mmAInv);
                    if (simd::mm_op<__m256i, uint32_t, std::greater_equal>::cmp_mask(mmInDec, mmDMax) == simd::mm<__m256i, uint32_t>::FULL_MASK) {
                        _mm_storeu_si128(mmOut++, _mm256_extracti128_si256(_mm256_shuffle_epi8(mmInDec, mmShuffle), 0));
                    } else {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - this->bufEncoded.template begin<uint32_t>(), iteration);
                    }
                }
                if (i < numValues) {
                    auto out16 = reinterpret_cast<uint16_t*>(mmOut);
                    auto data32 = reinterpret_cast<uint32_t*>(mmData);
                    for (; i < numValues; ++i, ++data32, ++out16) {
                        auto tmp = *data32 * this->A_INV;
                        if (tmp <= dMax) {
                            *out16 = tmp;
                        } else {
                            throw ErrorInfo(__FILE__, __LINE__, data32 - this->bufEncoded.template begin<uint32_t>(), iteration);
                        }
                    }
                }
            }
        }
    };

}
