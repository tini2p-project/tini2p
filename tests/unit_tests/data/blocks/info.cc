/* Copyright (c) 2019, tini2p
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <catch2/catch.hpp>

#include "src/data/blocks/info.h"

using tini2p::data::InfoBlock;

struct InfoBlockFixture
{
  InfoBlockFixture() : info(new tini2p::data::Info()), block(info)
  {
  }

  tini2p::data::Info::shared_ptr info;
  InfoBlock block;
};

TEST_CASE_METHOD(
    InfoBlockFixture,
    "InfoBlock has a block ID",
    "[block]")
{
  REQUIRE(block.type() == InfoBlock::Type::Info);
}

TEST_CASE_METHOD(
    InfoBlockFixture,
    "InfoBlock has a block size",
    "[block]")
{
  REQUIRE(block.data_size() >= InfoBlock::MinInfoLen);
  REQUIRE(block.data_size() <= InfoBlock::MaxInfoLen);
}

TEST_CASE_METHOD(
    InfoBlockFixture,
    "InfoBlock serializes and deserializes a valid block",
    "[block]")
{
  // serialize a valid block
  REQUIRE_NOTHROW(block.serialize());

  REQUIRE_NOTHROW(block.deserialize());
}

TEST_CASE_METHOD(
    InfoBlockFixture,
    "InfoBlock fails to deserialize invalid ID",
    "[block]")
{
  // serialize a valid block
  REQUIRE_NOTHROW(block.serialize());

  // invalidate the block ID
  ++block.buffer()[InfoBlock::TypeOffset];
  REQUIRE_THROWS(block.deserialize());

  block.buffer()[InfoBlock::TypeOffset] -= 2;
  REQUIRE_THROWS(block.deserialize());
}

TEST_CASE_METHOD(
    InfoBlockFixture,
    "InfoBlock fails to deserialize invalid size",
    "[block]")
{
  // serialize a valid block
  REQUIRE_NOTHROW(block.serialize());

  // invalidate the size
  tini2p::write_bytes(&block.buffer()[InfoBlock::SizeOffset], InfoBlock::MaxInfoLen + 1);
  REQUIRE_THROWS(block.deserialize());

  tini2p::write_bytes(&block.buffer()[InfoBlock::SizeOffset], InfoBlock::MinInfoLen - 1);
  REQUIRE_THROWS(block.deserialize());
}
