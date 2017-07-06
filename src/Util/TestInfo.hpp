// Copyright 2016 Till Kolditz, Stefan de Bruijn
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
#include "Stopwatch.hpp"

struct TestInfo {

    bool isExecuted;
    int64_t nanos;
    std::string error;

    TestInfo();

    TestInfo(
            int64_t nanos);

    TestInfo(
            const std::string & error);

    TestInfo(
            bool isExecuted,
            int64_t nanos,
            const std::string & error);

    TestInfo(
            const TestInfo & other);

    virtual ~TestInfo();

    void set(
            int64_t nanos);

    void set(
            const std::string & error);

    TestInfo & operator=(
            const TestInfo &);
};

struct TestInfos {

    std::string name;
    std::string simd;
    TestInfo encode;
    TestInfo check;
    TestInfo arithmetic;
    TestInfo decode;
    TestInfo checkAndDecode;

    TestInfos(
            const std::string & name,
            const std::string & simd);

    TestInfos(
            const std::string & name,
            const std::string & simd,
            TestInfo & encode,
            TestInfo & check,
            TestInfo & arithmetic,
            TestInfo & decode,
            TestInfo & checkAndDecode);

    TestInfos(
            TestInfos && other);

    virtual ~TestInfos();
};