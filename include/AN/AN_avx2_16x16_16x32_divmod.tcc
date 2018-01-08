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

    template<size_t UNROLL, typename DATARAW, typename DATAENC>
    struct AN_avx2_16x16_16x32_divmod :
            public AN_avx2_16x16_16x32<DATARAW, DATAENC, UNROLL> {

        using AN_avx2_16x16_16x32<DATARAW, DATAENC, UNROLL>::AN_avx2_16x16_16x32;

        virtual ~AN_avx2_16x16_16x32_divmod() {
        }

        virtual bool DoCheck() override {
            return true;
        }

        virtual void RunCheck(
                const CheckConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                auto mm_Data = this->bufEncoded.template begin<__m256i >();
                auto mm_DataEnd = this->bufEncoded.template end<__m256i >();
                while (mm_Data <= (mm_DataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t k = 0; k < UNROLL; ++k) {
                        auto mmIn = _mm256_lddqu_si256(mm_Data);
                        if ((_mm256_extract_epi32(mmIn, 0) % this->A != 0) || (_mm256_extract_epi32(mmIn, 1) % this->A != 0) || (_mm256_extract_epi32(mmIn, 2) % this->A != 0)
                                || (_mm256_extract_epi32(mmIn, 3) % this->A != 0) || (_mm256_extract_epi32(mmIn, 4) % this->A != 0) || (_mm256_extract_epi32(mmIn, 5) % this->A != 0)
                                || (_mm256_extract_epi32(mmIn, 6) % this->A != 0) || (_mm256_extract_epi32(mmIn, 7) % this->A != 0)) {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<DATAENC*>(mm_Data) - this->bufEncoded.template begin<DATAENC>(), iteration);
                        }
                        ++mm_Data;
                    }
                }
                // here follows the non-unrolled remainder
                while (mm_Data <= (mm_DataEnd - 1)) {
                    auto mmIn = _mm256_lddqu_si256(mm_Data);
                    if ((_mm256_extract_epi32(mmIn, 0) % this->A != 0) || (_mm256_extract_epi32(mmIn, 1) % this->A != 0) || (_mm256_extract_epi32(mmIn, 2) % this->A != 0)
                            || (_mm256_extract_epi32(mmIn, 3) % this->A != 0) || (_mm256_extract_epi32(mmIn, 4) % this->A != 0) || (_mm256_extract_epi32(mmIn, 5) % this->A != 0)
                            || (_mm256_extract_epi32(mmIn, 6) % this->A != 0) || (_mm256_extract_epi32(mmIn, 7) % this->A != 0)) {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<DATAENC*>(mm_Data) - this->bufEncoded.template begin<DATAENC>(), iteration);
                    }
                    ++mm_Data;
                }
                if (mm_Data < mm_DataEnd) {
                    auto dataEnd2 = reinterpret_cast<DATAENC*>(mm_DataEnd);
                    for (auto data2 = reinterpret_cast<DATAENC*>(mm_Data); data2 < dataEnd2; ++data2) {
                        if ((*data2 % this->A) != 0) {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<DATAENC*>(data2) - this->bufEncoded.template begin<DATAENC>(), iteration);
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
            AN_avx2_16x16_16x32_s_divmod & test;
            const ArithmeticConfiguration & config;
            const size_t iteration;
            ArithmetorChecked(
                    AN_avx2_16x16_16x32_s_divmod & test,
                    const ArithmeticConfiguration & config,
                    const size_t iteration)
                    : test(test),
                      config(config),
                      iteration(iteration) {
            }
            void operator()(
                    ArithmeticConfiguration::Add) {
                auto mmData = test.bufEncoded.template begin<__m256i >();
                const auto mmDataEnd = test.bufEncoded.template end<__m256i >();
                auto mmOut = test.bufResult.template begin<__m256i >();
                DATAENC operandEnc = config.operand * test.A;
                auto mmOperandEnc = _mm256_set1_epi32(operandEnc);
                while (mmData <= (mmDataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                        auto mmIn = _mm256_lddqu_si256(mmData++);
                        if ((_mm256_extract_epi32(mmIn, 0) % test.A != 0) || (_mm256_extract_epi32(mmIn, 1) % test.A != 0) || (_mm256_extract_epi32(mmIn, 2) % test.A != 0)
                                || (_mm256_extract_epi32(mmIn, 3) % test.A != 0) || (_mm256_extract_epi32(mmIn, 4) % test.A != 0) || (_mm256_extract_epi32(mmIn, 5) % test.A != 0)
                                || (_mm256_extract_epi32(mmIn, 6) % test.A != 0) || (_mm256_extract_epi32(mmIn, 7) % test.A != 0)) {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<DATAENC*>(mmData) - test.bufEncoded.template begin<DATAENC>(), iteration);
                        }
                        _mm256_storeu_si256(mmOut++, _mm256_add_epi32(mmIn, mmOperandEnc));
                    }
                }
                // remaining numbers
                while (mmData <= (mmDataEnd - 1)) {
                    auto mmIn = _mm256_lddqu_si256(mmData++);
                    if ((_mm256_extract_epi32(mmIn, 0) % test.A != 0) || (_mm256_extract_epi32(mmIn, 1) % test.A != 0) || (_mm256_extract_epi32(mmIn, 2) % test.A != 0)
                            || (_mm256_extract_epi32(mmIn, 3) % test.A != 0) || (_mm256_extract_epi32(mmIn, 4) % test.A != 0) || (_mm256_extract_epi32(mmIn, 5) % test.A != 0)
                            || (_mm256_extract_epi32(mmIn, 6) % test.A != 0) || (_mm256_extract_epi32(mmIn, 7) % test.A != 0)) {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<DATAENC*>(mmData) - test.bufEncoded.template begin<DATAENC>(), iteration);
                    }
                    _mm256_storeu_si256(mmOut++, _mm256_add_epi32(mmIn, mmOperandEnc));
                }
                if (mmData < mmDataEnd) {
                    auto data32End = reinterpret_cast<DATAENC*>(mmDataEnd);
                    auto out32 = reinterpret_cast<DATAENC*>(mmOut);
                    for (auto data32 = reinterpret_cast<DATAENC*>(mmData); data32 < data32End; ++data32, ++out32) {
                        if (*data32 % test.A != 0) {
                            throw ErrorInfo(__FILE__, __LINE__, data32 - test.bufEncoded.template begin<DATAENC>(), iteration);
                        }
                        *out32 = *data32 + operandEnc;
                    }
                }
            }
            void operator()(
                    ArithmeticConfiguration::Sub) {
            }
            void operator()(
                    ArithmeticConfiguration::Mul) {
            }
            void operator()(
                    ArithmeticConfiguration::Div) {
            }
        };

        void RunArithmeticChecked(
                const ArithmeticConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                std::visit(ArithmetorChecked(*this, config, iteration), config.mode);
            }
        }

        bool DoDecode() override {
            return true;
        }

        void RunDecode(
                const DecodeConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                _ReadWriteBarrier();
                const ssize_t VALUES_PER_SIMDREG = sizeof(__m256i) / sizeof (DATAENC);
                const ssize_t VALUES_PER_UNROLL = UNROLL * VALUES_PER_SIMDREG;
                ssize_t numValues = this->bufRaw.template end<DATARAW>() - this->bufRaw.template begin<DATARAW>();
                ssize_t i = 0;
                auto mm_In = this->bufEncoded.template begin<__m128i >();
                auto mm_Out = this->bufResult.template begin<int64_t>();
                auto mmAinv = _mm256_set1_pd(static_cast<double>(this->A_INV));
                auto mmShuffle = _mm_set_epi32(static_cast<DATAENC>(0xFFFFFFFF), static_cast<DATAENC>(0xFFFFFFFF), static_cast<DATAENC>(0x0D0C0908), static_cast<DATAENC>(0x05040100));
                for (; i <= (numValues - VALUES_PER_UNROLL); i += VALUES_PER_UNROLL) {
                    // let the compiler unroll the loop
                    for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                        auto tmp1 = _mm256_cvtepi32_pd(_mm_lddqu_si128(mm_In++));
                        auto tmp2 = _mm256_cvtepi32_pd(_mm_lddqu_si128(mm_In++));
                        tmp1 = _mm256_div_pd(tmp1, mmAinv);
                        tmp2 = _mm256_div_pd(tmp2, mmAinv);
                        auto tmp3 = _mm_shuffle_epi8(_mm256_cvtpd_epi32(tmp1), mmShuffle);
                        auto tmp4 = _mm_shuffle_epi8(_mm256_cvtpd_epi32(tmp2), mmShuffle);
                        *mm_Out++ = _mm_extract_epi64(tmp3, 0);
                        *mm_Out++ = _mm_extract_epi64(tmp4, 0);
                    }
                }
                // remaining numbers
                for (; i <= (numValues - VALUES_PER_SIMDREG); i += VALUES_PER_SIMDREG) {
                    auto tmp1 = _mm256_cvtepi32_pd(_mm_lddqu_si128(mm_In++));
                    auto tmp2 = _mm256_cvtepi32_pd(_mm_lddqu_si128(mm_In++));
                    tmp1 = _mm256_div_pd(tmp1, mmAinv);
                    tmp2 = _mm256_div_pd(tmp2, mmAinv);
                    auto tmp3 = _mm_shuffle_epi8(_mm256_cvtpd_epi32(tmp1), mmShuffle);
                    auto tmp4 = _mm_shuffle_epi8(_mm256_cvtpd_epi32(tmp2), mmShuffle);
                    *mm_Out++ = _mm_extract_epi64(tmp3, 0);
                    *mm_Out++ = _mm_extract_epi64(tmp4, 0);
                }
                if (i < numValues) {
                    auto out16 = reinterpret_cast<DATARAW*>(mm_Out);
                    auto in32 = reinterpret_cast<DATAENC*>(mm_In);
                    for (; i < numValues; ++i, ++in32, ++out16) {
                        *out16 = *in32 * this->A_INV;
                    }
                }
            }
        }
    };

    template<size_t UNROLL>
    struct AN_avx2_16x16_16x32_s_divmod :
            public AN_avx2_16x16_16x32_divmod<UNROLL, int16_t, int32_t> {
        using AN_avx2_16x16_16x32_divmod<UNROLL, int16_t, int32_t>::AN_avx2_16x16_16x32_divmod;

        virtual ~AN_avx2_16x16_16x32_s_divmod() {
        }
    };

    template<size_t UNROLL>
    struct AN_avx2_16x16_16x32_u_divmod :
            public AN_avx2_16x16_16x32_divmod<UNROLL, uint16_t, uint32_t> {
        using AN_avx2_16x16_16x32_divmod<UNROLL, uint16_t, uint32_t>::AN_avx2_16x16_16x32_divmod;

        virtual ~AN_avx2_16x16_16x32_u_divmod() {
        }
    };

}
