// Copyright 2016,2017 Till Kolditz, Stefan de Bruijn
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

#include <string>
#include <cstring>
#include <cstdint>
#include <optional>

#ifdef OMP
#include <omp.h>
#endif

#include "Util/AlignedBlock.hpp"
#include "Util/TestInfo.hpp"

struct TestBase0 {

    virtual ~TestBase0() {
    }

    virtual const std::string & getSIMDtypeName() = 0;

    virtual bool HasCapabilities() = 0;
};

struct TestConfiguration {
    const size_t numIterations;

    TestConfiguration(
            const size_t numIterations)
            : numIterations(numIterations) {
    }
};

struct EncodeConfiguration :
        public TestConfiguration {
    EncodeConfiguration(
            const TestConfiguration & config)
            : TestConfiguration(config) {
    }
};

struct CheckConfiguration :
        public TestConfiguration {
    CheckConfiguration(
            const TestConfiguration & config)
            : TestConfiguration(config) {
    }
};

struct ArithmeticConfiguration :
        public TestConfiguration {
    size_t operand;
    ArithmeticConfiguration(
            const TestConfiguration & config,
            const size_t operand)
            : TestConfiguration(config),
              operand(operand) {
    }
};

struct ReencodeConfiguration :
        public TestConfiguration {
    size_t newA;
    ReencodeConfiguration(
            const TestConfiguration & config,
            const size_t newA)
            : TestConfiguration(config),
              newA(newA) {
    }
};

struct DecodeConfiguration :
        public TestConfiguration {
    DecodeConfiguration(
            const TestConfiguration & config)
            : TestConfiguration(config) {
    }
};

struct CheckAndDecodeConfiguration :
        public TestConfiguration {
    CheckAndDecodeConfiguration(
            const TestConfiguration & config)
            : TestConfiguration(config) {
    }
};

struct DataGenerationConfiguration {
    const std::optional<size_t> numEffectiveLSBs;

    DataGenerationConfiguration()
            : numEffectiveLSBs(std::nullopt) {
    }

    DataGenerationConfiguration(
            const size_t numEffectiveLSBs)
            : numEffectiveLSBs(numEffectiveLSBs) {
    }
};

struct TestBase :
        virtual public TestBase0 {

protected:
    std::string name;
    AlignedBlock in;
    AlignedBlock out;

public:
    TestBase(
            const std::string & name,
            AlignedBlock & in,
            AlignedBlock & out);

    TestBase(
            TestBase & other);

    virtual ~TestBase();

    virtual void ResetBuffers(
            const DataGenerationConfiguration & config) = 0;

    virtual size_t getInputTypeSize() = 0;

    virtual size_t getOutputTypeSize() = 0;

    // Encoding
    virtual void PreEnc(
            const EncodeConfiguration & config);

    virtual void RunEnc(
            const EncodeConfiguration & config) = 0;

    // Check-Only
    virtual bool DoCheck();

    virtual void PreCheck(
            const CheckConfiguration & config);

    virtual void RunCheck(
            const CheckConfiguration & config);

    // Arithmetic
    virtual bool DoArith();

    virtual void PreArith(
            const ArithmeticConfiguration & config);

    virtual void RunArith(
            const ArithmeticConfiguration & config);

    // Reencode
    virtual bool DoReenc();

    virtual void PreReenc(
            const ReencodeConfiguration & config);

    virtual void RunReenc(
            const ReencodeConfiguration & config);

    // Decoding-Only
    virtual bool DoDec();

    virtual void PreDec(
            const DecodeConfiguration & config);

    virtual void RunDec(
            const DecodeConfiguration & config);

    // Check-And-Dec
    virtual bool DoCheckDec();

    virtual void PreCheckDec(
            const CheckAndDecodeConfiguration & config);

    virtual void RunCheckDec(
            const CheckAndDecodeConfiguration & config);

    // Execute test:
    virtual TestInfos Execute(
            const TestConfiguration & config,
            const DataGenerationConfiguration & configDataGen);
};

struct SequentialTest :
        virtual public TestBase0 {

    virtual ~SequentialTest();

    virtual const std::string & getSIMDtypeName() override;

    virtual bool HasCapabilities() override;
};

struct SSE42Test :
        virtual public TestBase0 {

    virtual ~SSE42Test();

    virtual const std::string & getSIMDtypeName() override;

    virtual bool HasCapabilities() override;
};

struct AVX2Test :
        virtual public TestBase0 {

    virtual ~AVX2Test();

    virtual const std::string & getSIMDtypeName() override;

    virtual bool HasCapabilities() override;
};

#ifdef _MSC_VER
#ifdef DATA
#undef DATA
#endif

#ifdef CS
#undef CS
#endif
#endif

template<typename DATAIN, typename DATAOUT>
struct Test :
        public TestBase {

    Test(
            const std::string & name,
            AlignedBlock & in,
            AlignedBlock & out)
            : TestBase(name, in, out) {
    }

    virtual ~Test() {
    }

    virtual size_t getInputTypeSize() override {
        return sizeof(DATAIN);
    }

    virtual size_t getOutputTypeSize() override {
        return sizeof(DATAOUT);
    }

    void ResetBuffers(
            const DataGenerationConfiguration & config) override {
        DATAIN mask = static_cast<DATAIN>(-1);
        if (config.numEffectiveLSBs) {
            mask = static_cast<DATAIN>((1ull << *config.numEffectiveLSBs) - 1ull);
        }
        auto pInEnd = this->in.template end<DATAIN>();
        DATAIN value = static_cast<DATAIN>(12783);
        for (DATAIN* pIn = this->in.template begin<DATAIN>(); pIn < pInEnd; ++pIn) {
            *pIn = mask & value;
            value = value * static_cast<DATAIN>(7577) + static_cast<DATAIN>(10467);
        }

        memset(this->out.begin(), 0, this->out.nBytes);
    }
};