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

#include <AN/AN_sse42_8x16_8x32.hpp>

namespace coding_benchmark {

    template<size_t UNROLL>
    struct AN_sse42_8x16_8x32_u_divmod :
            public AN_sse42_8x16_8x32<uint16_t, uint32_t, UNROLL> {

        using AN_sse42_8x16_8x32<uint16_t, uint32_t, UNROLL>::AN_sse42_8x16_8x32;

        virtual ~AN_sse42_8x16_8x32_u_divmod() {
        }

        virtual bool DoCheck() override {
            return true;
        }

        virtual void RunCheck(
                const CheckConfiguration & config) override {
            for (size_t iteration = 0; iteration < config.numIterations; ++iteration) {
                auto data = this->bufEncoded.template begin<__m128i >();
                auto dataEnd = this->bufEncoded.template end<__m128i >();
                while (data <= (dataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t k = 0; k < UNROLL; ++k) {
                        auto mmIn = _mm_lddqu_si128(data);
                        // auto mm_pd1 = _mm_cvtepi32_pd(mmIn); // lower 2 converted
                        // auto mm_pd2 = _mm_cvtepi32_pd(_mm_shuffle_epi32(mmIn, 0xEE)); // higher 2 converted
                        // auto res1 = _mm_div_pd(mm_pd1, mm_A);
                        // auto res2 = _mm_div_pd(mm_pd2, mm_A);
                        // auto mm_unenc =
                        if ((_mm_extract_epi32(mmIn, 0) % this->A != 0) || (_mm_extract_epi32(mmIn, 1) % this->A != 0) || (_mm_extract_epi32(mmIn, 2) % this->A != 0)
                                || (_mm_extract_epi32(mmIn, 3) % this->A != 0)) { // we need to do this "hack" because comparison is only on signed integers!
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(data) - this->bufEncoded.template begin<uint32_t>(), iteration);
                        }
                        ++data;
                    }
                }
                // here follows the non-unrolled remainder
                while (data <= (dataEnd - 1)) {
                    auto mmIn = _mm_lddqu_si128(data);
                    if ((_mm_extract_epi32(mmIn, 0) % this->A != 0) || (_mm_extract_epi32(mmIn, 1) % this->A != 0) || (_mm_extract_epi32(mmIn, 2) % this->A != 0)
                            || (_mm_extract_epi32(mmIn, 3) % this->A != 0)) { // we need to do this "hack" because comparison is only on signed integers!
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(data) - this->bufEncoded.template begin<uint32_t>(), iteration);
                    }
                    ++data;
                }
                if (data < dataEnd) {
                    auto dataEnd2 = reinterpret_cast<uint32_t*>(dataEnd);
                    for (auto data2 = reinterpret_cast<uint32_t*>(data); data2 < dataEnd2; ++data2) {
                        if ((*data2 % this->A) != 0) {
                            throw ErrorInfo(__FILE__, __LINE__, data2 - this->bufEncoded.template begin<uint32_t>(), iteration);
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
            AN_sse42_8x16_8x32_u_divmod & test;
            const ArithmeticConfiguration & config;
            const size_t iteration;
            ArithmetorChecked(
                    AN_sse42_8x16_8x32_u_divmod & test,
                    const ArithmeticConfiguration & config,
                    const size_t iteration)
                    : test(test),
                      config(config),
                      iteration(iteration) {
            }
            void operator()(
                    ArithmeticConfiguration::Add) {
                auto mmData = test.bufEncoded.template begin<__m128i >();
                const auto mmDataEnd = test.bufEncoded.template end<__m128i >();
                auto mmOut = test.bufResult.template begin<__m128i >();
                uint32_t operandEnc = config.operand * test.A;
                auto mmOperandEnc = _mm_set1_epi32(operandEnc);
                while (mmData <= (mmDataEnd - UNROLL)) {
                    // let the compiler unroll the loop
                    for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                        auto mmIn = _mm_lddqu_si128(mmData++);
                        if ((_mm_extract_epi32(mmIn, 0) % test.A != 0) || (_mm_extract_epi32(mmIn, 1) % test.A != 0) || (_mm_extract_epi32(mmIn, 2) % test.A != 0)
                                || (_mm_extract_epi32(mmIn, 3) % test.A != 0)) {
                            throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - test.bufEncoded.template begin<uint32_t>(), iteration);
                        }
                        _mm_storeu_si128(mmOut++, _mm_add_epi32(mmIn, mmOperandEnc));
                    }
                }
                // remaining numbers
                while (mmData <= (mmDataEnd - 1)) {
                    auto mmIn = _mm_lddqu_si128(mmData++);
                    if ((_mm_extract_epi32(mmIn, 0) % test.A != 0) || (_mm_extract_epi32(mmIn, 1) % test.A != 0) || (_mm_extract_epi32(mmIn, 2) % test.A != 0)
                            || (_mm_extract_epi32(mmIn, 3) % test.A != 0)) {
                        throw ErrorInfo(__FILE__, __LINE__, reinterpret_cast<uint32_t*>(mmData) - test.bufEncoded.template begin<uint32_t>(), iteration);
                    }
                    _mm_storeu_si128(mmOut++, _mm_add_epi32(mmIn, mmOperandEnc));
                }
                if (mmData < mmDataEnd) {
                    auto data32End = reinterpret_cast<uint32_t*>(mmDataEnd);
                    auto out32 = reinterpret_cast<uint32_t*>(mmOut);
                    for (auto data32 = reinterpret_cast<uint32_t*>(mmData); data32 < data32End; ++data32, ++out32) {
                        if (*data32 % test.A != 0) {
                            throw ErrorInfo(__FILE__, __LINE__, data32 - test.bufEncoded.template begin<uint32_t>(), iteration);
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
                const constexpr size_t VALUES_PER_SIMDREG = sizeof(__m128i) /sizeof (uint32_t);
            const constexpr size_t VALUES_PER_UNROLL = UNROLL * VALUES_PER_SIMDREG;
            size_t numValues = this->bufRaw.template end<int16_t>() - this->bufRaw.template begin<int16_t>();
            size_t i = 0;
            auto dataIn = this->bufEncoded.template begin<__m128i>();
            auto dataOut = this->bufResult.template begin<uint64_t>();
            auto mm_A = _mm_set1_pd(static_cast<double>(this->A_INV));
            auto mmShuffle = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x0B0A0908, 0x03020100);
            for (; i <= (numValues - VALUES_PER_UNROLL); i += VALUES_PER_UNROLL) {
                // let the compiler unroll the loop
                for (size_t unroll = 0; unroll < UNROLL; ++unroll) {
                    auto tmp = _mm_lddqu_si128(dataIn++);
                    auto tmp1 = _mm_cvtepi32_pd(tmp);
                    auto tmp2 = _mm_cvtepi32_pd(_mm_srli_si128(tmp, 8));
                    tmp1 = _mm_div_pd(tmp1, mm_A);
                    tmp2 = _mm_div_pd(tmp2, mm_A);
                    auto tmp3 = _mm_cvtpd_epi32(tmp1);
                    auto tmp4 = _mm_cvtpd_epi32(tmp2);
                    tmp = _mm_unpacklo_epi16(tmp3, tmp4);
                    tmp = _mm_shuffle_epi8(tmp, mmShuffle);
                    *dataOut++ = _mm_extract_epi64(tmp, 0);
                }
            }
            // remaining numbers
            for (; i <= (numValues - VALUES_PER_SIMDREG); i += VALUES_PER_SIMDREG) {
                auto tmp = _mm_lddqu_si128(dataIn++);
                auto tmp1 = _mm_cvtepi32_pd(tmp);
                auto tmp2 = _mm_cvtepi32_pd(_mm_srli_si128(tmp, 8));
                tmp1 = _mm_div_pd(tmp1, mm_A);
                tmp2 = _mm_div_pd(tmp2, mm_A);
                auto tmp3 = _mm_cvtpd_epi32(tmp1);
                auto tmp4 = _mm_cvtpd_epi32(tmp2);
                tmp = _mm_unpacklo_epi16(tmp3, tmp4);
                tmp = _mm_shuffle_epi8(tmp, mmShuffle);
                *dataOut++ = _mm_extract_epi64(tmp, 0);
            }
            if (i < numValues) {
                auto out16 = reinterpret_cast<uint16_t*>(dataOut);
                auto in32 = reinterpret_cast<uint32_t*>(dataIn);
                for (; i < numValues; ++i, ++in32, ++out16) {
                    *out16 = *in32 * this->A_INV;
                }
            }
        }
    }
};

}