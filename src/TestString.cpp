// Copyright 2017 Till Kolditz
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
 * TestString.cpp
 *
 *  Created on: 24.03.2018
 *      Author: Till Kolditz - Till.Kolditz@gmail.com
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <random>
#include <limits>
#include <type_traits>

#include <Util/Stopwatch.hpp>
#include <Util/AlignedBlock.hpp>
#include <Util/Test.hpp>

#include <Strings/Strings.hpp>
#include <Strings/StringsAN.hpp>
#include <Strings/StringsXOR.hpp>

template<typename DATARAW>
void ResetBlock(
        const DataGenerationConfiguration & dataGenConfig,
        AlignedBlock buf) {
    DATARAW mask;
    if (std::is_signed_v<DATARAW>) {
        mask = std::numeric_limits<DATARAW>::min();
    } else {
        mask = std::numeric_limits<DATARAW>::max();
    }
    if (dataGenConfig.numEffectiveBitsData && dataGenConfig.numEffectiveBitsData.value() < (sizeof(DATARAW) * 8)) {
        mask = static_cast<DATARAW>((1ull << dataGenConfig.numEffectiveBitsData.value()) - 1ull);
    }
    if (dataGenConfig.numIneffectiveLSBsData) {
        if (dataGenConfig.numEffectiveBitsData && dataGenConfig.numEffectiveBitsData.value() <= dataGenConfig.numIneffectiveLSBsData) {
            throw std::runtime_error("dataGenConfig.numEffectiveBitsData <= dataGenConfig.numIneffectiveLSBsData makes no sense!");
        }
        mask &= ~static_cast<DATARAW>((1ull << dataGenConfig.numIneffectiveLSBsData.value()) - 1ull);
    }
    auto pInEnd = buf.template end<DATARAW>();
    DATARAW value = static_cast<DATARAW>(12783);
    DATARAW* pIn = buf.template begin<DATARAW>();
    while (pIn < pInEnd) {
        DATARAW x;
        do {
            x = mask & value;
            value = value * static_cast<DATARAW>(7577) + static_cast<DATARAW>(10467);
        } while (x == 0);
        if (dataGenConfig.multiplicator) {
            x *= dataGenConfig.multiplicator.value();
        }
        *pIn++ = x;
    }
}

template<typename T>
void test_buffer(
        const T * beg,
        const char * const name,
        const size_t NUM) {
    auto x = beg;
    while (*x) {
        ++x;
    }
    if (static_cast<size_t>(x - beg) != (NUM - 1)) {
        std::cerr << "#zero value found in " << name << " at position " << (x - beg) << " instead of expected " << (NUM - 1) << std::endl;
    }
}

int main(
        int argc,
        char** argv) {
    const constexpr size_t NUM = 64 * 1024 * 1024;
    const constexpr size_t NUM_BYTES_CHAR = NUM * sizeof(unsigned char);
    const constexpr size_t NUM_BYTES_SHORT = NUM * sizeof(unsigned short);
    const constexpr size_t NUM_BYTES_INT = NUM * sizeof(unsigned int);

    Stopwatch sw;
    int res;
    int64_t time_base;
    int64_t time;

    char* ptr_end;
    unsigned short A = 233;
    // force the compiler to not assume A a constant
    if (argc > 1) {
        A = strtol(argv[1], &ptr_end, 10);
    }

    {
        std::cout << "#Buffer with " << NUM << " 8-bit limbs --> " << NUM_BYTES_CHAR << " bytes for unprotected and XOR, " << NUM_BYTES_SHORT << " bytes for AN\n";
        AlignedBlock buf_char_1(NUM_BYTES_CHAR, 16);
        AlignedBlock buf_char_2(NUM_BYTES_CHAR, 16);
        DataGenerationConfiguration config(8);
        ResetBlock<unsigned char>(config, buf_char_1);
        *(buf_char_1.template end<char>() - 1) = 0;
        ResetBlock<unsigned char>(config, buf_char_2);
        *(buf_char_2.template end<char>() - 1) = 0;
        const char * uc1 = buf_char_1.template begin<const char>();
        const char * uc2 = buf_char_2.template begin<const char>();
        test_buffer(uc1, "buf_char_1", NUM);
        test_buffer(uc2, "buf_char_2", NUM);

        sw.Reset();
        res = std::strcmp(uc1, uc2);
        time = time_base = sw.Current();
        std::cout << "std::strcmp," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = strcmp2(uc1, uc2, NUM);
        time = sw.Current();
        std::cout << "naive," << res << ',' << time << ',' << time_base << '\n';

        char old_xor_char1 = strxor(uc1);
        char old_xor_char2 = strxor(uc2);
        sw.Reset();
        res = strcmp2_xor(uc1, uc2, old_xor_char1, old_xor_char2, NUM);
        time = sw.Current();
        std::cout << "naive XOR," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp(uc1, uc2, NUM);
        time = sw.Current();
        std::cout << "Kankovski," << res << "," << time << ',' << time_base << '\n';

        __m128i old_xor_mm1 = _mm_strxor<unsigned char>(reinterpret_cast<const __m128i *>(uc1), NUM);
        __m128i old_xor_mm2 = _mm_strxor<unsigned char>(reinterpret_cast<const __m128i *>(uc2), NUM);
        sw.Reset();
        res = _mm_strcmp_xor(uc1, uc2, old_xor_mm1, old_xor_mm2, NUM);
        time = sw.Current();
        std::cout << "K. XOR," << res << ',' << time << ',' << time_base << '\n';

        AlignedBlock buf_short_1(NUM_BYTES_SHORT, 16);
        AlignedBlock buf_short_2(NUM_BYTES_SHORT, 16);
        DataGenerationConfiguration config2(8, 0, 8, 0, A);
        ResetBlock<unsigned short>(config2, buf_short_1);
        *(buf_short_1.template end<unsigned short>() - 1) = 0;
        ResetBlock<unsigned short>(config2, buf_short_2);
        *(buf_short_2.template end<unsigned short>() - 1) = 0;
        const unsigned short * us1 = buf_short_1.template begin<const unsigned short>();
        const unsigned short * us2 = buf_short_2.template begin<const unsigned short>();
        test_buffer(us1, "buf_short_1", NUM);
        test_buffer(us2, "buf_short_2", NUM);

        sw.Reset();
        res = strcmp2_AN(us1, us2, A, NUM);
        time = sw.Current();
        std::cout << "naive AN," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = strcmp2_AN_accu(us1, us2, A, NUM);
        time = sw.Current();
        std::cout << "naive AN accu," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp_AN(us1, us2, A, NUM);
        time = sw.Current();
        std::cout << "K. AN," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp_AN_accu(us1, us2, A, NUM);
        time = sw.Current();
        std::cout << "K. AN accu," << res << ',' << time << ',' << time_base << '\n';
    }

    {
        std::cout << "\n\n#Buffer with " << NUM << " 16-bit limbs --> " << NUM_BYTES_SHORT << " bytes for unprotected and XOR, " << NUM_BYTES_INT << " bytes for AN\n";
        AlignedBlock buf_short_3(NUM_BYTES_SHORT, 16);
        AlignedBlock buf_short_4(NUM_BYTES_SHORT, 16);
        DataGenerationConfiguration config3(8); // make sure we have no zero-bytes
        ResetBlock<unsigned char>(config3, buf_short_3);
        ResetBlock<unsigned char>(config3, buf_short_4);
        const unsigned short * us1 = buf_short_3.template begin<const unsigned short>();
        const unsigned short * us2 = buf_short_4.template begin<const unsigned short>();
        *(buf_short_3.template end<unsigned char>() - 1) = 0;
        *(buf_short_4.template end<unsigned char>() - 1) = 0;
        test_buffer(reinterpret_cast<const char*>(us1), "buf_char_3", NUM_BYTES_SHORT);
        test_buffer(reinterpret_cast<const char*>(us2), "buf_char_4", NUM_BYTES_SHORT);
        *(buf_short_3.template end<unsigned short>() - 1) = 0;
        *(buf_short_4.template end<unsigned short>() - 1) = 0;
        test_buffer(us1, "buf_short_3", NUM_BYTES_CHAR);
        test_buffer(us2, "buf_short_4", NUM_BYTES_CHAR);

        sw.Reset();
        res = std::strcmp(reinterpret_cast<const char*>(us1), reinterpret_cast<const char*>(us2));
        time = time_base = sw.Current();
        std::cout << "std::strcmp," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = strcmp2(us1, us2, NUM);
        time = sw.Current();
        std::cout << "naive," << res << ',' << time << ',' << time_base << '\n';

        unsigned short old_xor_short1 = strxor(us1);
        unsigned short old_xor_short2 = strxor(us2);
        sw.Reset();
        res = strcmp2_xor(us1, us2, old_xor_short1, old_xor_short2, NUM);
        time = sw.Current();
        std::cout << "naive XOR," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp(us1, us2, NUM);
        time = sw.Current();
        std::cout << "Kankovski," << res << ',' << time << ',' << time_base << '\n';

        auto old_xor_mm1 = _mm_strxor<unsigned short>(reinterpret_cast<const __m128i *>(us1), NUM);
        auto old_xor_mm2 = _mm_strxor<unsigned short>(reinterpret_cast<const __m128i *>(us2), NUM);
        sw.Reset();
        res = _mm_strcmp_xor(us1, us2, old_xor_mm1, old_xor_mm2, NUM);
        time = sw.Current();
        std::cout << "K. XOR," << res << ',' << time << ',' << time_base << '\n';

        if (argc > 2) {
            A = strtol(argv[2], &ptr_end, 10);
        } else {
            A = 63877;
        }

        AlignedBlock buf_int_1(NUM_BYTES_INT, 16);
        AlignedBlock buf_int_2(NUM_BYTES_INT, 16);
        DataGenerationConfiguration config4(16, 1, 16, 1, A); // make at least 2-bit limbs so that the upper short is never null!
        ResetBlock<unsigned int>(config4, buf_int_1);
        *(buf_int_1.template end<unsigned int>() - 1) = 0;
        ResetBlock<unsigned int>(config4, buf_int_2);
        *(buf_int_2.template end<unsigned int>() - 1) = 0;
        const unsigned int * ui1 = buf_int_1.template begin<const unsigned int>();
        const unsigned int * ui2 = buf_int_2.template begin<const unsigned int>();
        test_buffer(ui1, "buf_int_1", NUM);
        test_buffer(ui2, "buf_int_2", NUM);

        sw.Reset();
        res = strcmp2_AN(ui1, ui2, A, NUM);
        time = sw.Current();
        std::cout << "naive AN," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = strcmp2_AN_accu(ui1, ui2, A, NUM);
        time = sw.Current();
        std::cout << "naive AN accu," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp_AN(ui1, ui2, A, NUM);
        time = sw.Current();
        std::cout << "K. AN," << res << ',' << time << ',' << time_base << '\n';

        sw.Reset();
        res = _mm_strcmp_AN_accu(ui1, ui2, A, NUM);
        time = sw.Current();
        std::cout << "K. AN accu," << res << ',' << time << ',' << time_base << '\n';
    }

    return 0;
}
