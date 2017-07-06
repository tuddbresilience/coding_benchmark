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

#include <cstdint>

#include "XOR_seq.hpp"
#include "../Util/Intrinsics.hpp"

template<size_t BLOCKSIZE>
struct XOR_seq_32_32 :
        public XOR_seq<uint32_t, uint32_t, BLOCKSIZE>,
        public SequentialTest {

    using XOR_seq<uint32_t, uint32_t, BLOCKSIZE>::XOR_seq;

    virtual ~XOR_seq_32_32() {
    }
};