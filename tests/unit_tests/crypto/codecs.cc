/* Copyright (c) 2019, tini2p                                                                 //
 *                                                                                            //
 * All rights reserved.                                                                       //
 *                                                                                            //
 * Redistribution and use in source and binary forms, with or without modification, are       //
 * permitted provided that the following conditions are met:                                  //
 *                                                                                            //
 * 1. Redistributions of source code must retain the above copyright notice, this list of     //
 *    conditions and the following disclaimer.                                                //
 *                                                                                            //
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list     //
 *    of conditions and the following disclaimer in the documentation and/or other            //
 *    materials provided with the distribution.                                               //
 *                                                                                            //
 * 3. Neither the name of the copyright holder nor the names of its contributors may be       //
 *    used to endorse or promote products derived from this software without specific         //
 *    prior written permission.                                                               //
 *                                                                                            //
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY        //
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF    //
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL     //
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       //
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,               //
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    //
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,          //
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF    //
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               //
 */

#include <catch2/catch.hpp>

#include "src/crypto/codecs.h"

struct CodecsFixture
{
  /// @brief Destination hash
  const std::vector<std::uint8_t> dest_hash{
      0x81, 0x70, 0x0c, 0x3a, 0xbc, 0x0f, 0x7c, 0x74, 0xaf, 0xf3, 0x2c,
      0xf6, 0x67, 0xba, 0xd7, 0x51, 0x39, 0x1a, 0x13, 0xce, 0x40, 0x30,
      0xba, 0x4d, 0x76, 0xb6, 0xd1, 0xcb, 0x0d, 0x16, 0x8d, 0x92};

  /// @brief Destination
  const std::vector<std::uint8_t> dest{
      0xd8, 0xc3, 0x01, 0xd8, 0xc9, 0x7d, 0x7b, 0x36, 0x4f, 0x51, 0x74, 0xf2,
      0x7a, 0x57, 0x50, 0x4f, 0x50, 0xf7, 0xff, 0x79, 0xd6, 0x11, 0x63, 0x5e,
      0x4d, 0xae, 0x4e, 0x21, 0x4f, 0x28, 0x15, 0x08, 0xae, 0x43, 0x64, 0x49,
      0xb2, 0x08, 0x07, 0xa2, 0xcc, 0x89, 0x02, 0x8d, 0x53, 0xf6, 0xc3, 0x03,
      0x01, 0xc1, 0xf5, 0x40, 0x62, 0x3a, 0x75, 0x6e, 0xca, 0x6e, 0x34, 0x91,
      0x83, 0x21, 0xad, 0x95, 0xef, 0xd5, 0xcd, 0x1a, 0x65, 0x46, 0xda, 0x56,
      0xbe, 0x83, 0xd8, 0xa1, 0x13, 0xe0, 0x59, 0x17, 0xfd, 0x21, 0x1d, 0x31,
      0xcf, 0x90, 0xa9, 0xbe, 0xbd, 0xff, 0x22, 0x86, 0xcb, 0x3c, 0xc7, 0x51,
      0x51, 0x21, 0x85, 0xc1, 0x5f, 0x3b, 0x19, 0xee, 0x28, 0xa3, 0x4e, 0x37,
      0xe2, 0xfd, 0x72, 0x97, 0x29, 0x11, 0x13, 0xd7, 0x9e, 0x76, 0xbb, 0x11,
      0xc6, 0x75, 0xbc, 0x3f, 0x84, 0x16, 0x94, 0x84, 0x25, 0x6f, 0xe5, 0xb5,
      0x44, 0x1c, 0x91, 0x6a, 0x58, 0x29, 0x9e, 0xb7, 0xe9, 0x43, 0xc0, 0x93,
      0xe8, 0xe9, 0x93, 0x5c, 0x2f, 0x84, 0x05, 0x5f, 0x91, 0xf5, 0xb4, 0xe5,
      0x9c, 0xab, 0x1d, 0xaa, 0x5a, 0x6a, 0xd0, 0xf8, 0x24, 0x96, 0x9a, 0x42,
      0x4c, 0x88, 0x5a, 0x45, 0x78, 0xc1, 0x00, 0x00, 0x82, 0xb9, 0x33, 0x69,
      0xf4, 0x69, 0xb5, 0x4b, 0xd1, 0xc5, 0xc4, 0xd1, 0x67, 0x8e, 0xdc, 0x1e,
      0x1e, 0x04, 0x1b, 0xd2, 0x12, 0x8c, 0xc1, 0x9b, 0x70, 0x06, 0x18, 0x58,
      0x49, 0xaa, 0xe9, 0x1a, 0xb5, 0x56, 0xd5, 0x8d, 0xd6, 0xb2, 0x06, 0x68,
      0x4b, 0x7f, 0x14, 0xbf, 0xd3, 0xcf, 0x9e, 0x19, 0xbd, 0xd8, 0x42, 0x78,
      0x70, 0xa8, 0x13, 0x40, 0x79, 0x0a, 0x65, 0xc3, 0xf6, 0xd6, 0x13, 0xf4,
      0x95, 0x70, 0x65, 0xe5, 0xf7, 0xa3, 0x3c, 0x22, 0x94, 0x01, 0x7d, 0x79,
      0x39, 0xaa, 0x37, 0xe9, 0x1a, 0x8a, 0x21, 0x00, 0x82, 0xb1, 0x37, 0x66,
      0x1f, 0x1f, 0x4f, 0xb5, 0xfd, 0xa3, 0xe8, 0xfd, 0xe1, 0x00, 0xf3, 0xfa,
      0xd8, 0x70, 0x0c, 0xe4, 0xb6, 0x22, 0xa8, 0xc6, 0xae, 0x58, 0xb0, 0xf0,
      0x55, 0x6f, 0x91, 0x38, 0x85, 0xd2, 0x7e, 0xb1, 0x01, 0x63, 0xe8, 0x22,
      0xe9, 0x55, 0x76, 0x68, 0x52, 0xc1, 0x44, 0xdb, 0x81, 0x1b, 0xba, 0x90,
      0x6f, 0x02, 0xb7, 0x42, 0x92, 0x1e, 0x9b, 0xba, 0x57, 0x13, 0xf2, 0xdc,
      0x59, 0x6f, 0xcf, 0x1e, 0xca, 0x26, 0x34, 0x8a, 0xc4, 0x02, 0x22, 0x31,
      0x50, 0x4d, 0x56, 0xe5, 0x1a, 0xad, 0x05, 0x3e, 0x5d, 0x43, 0x6e, 0x5b,
      0xef, 0xee, 0xad, 0x98, 0x1e, 0x1e, 0xc3, 0x7c, 0x3b, 0xb2, 0xa3, 0x20,
      0x80, 0x25, 0x3b, 0xc1, 0x96, 0x9f, 0x13, 0x23, 0x0c, 0x58, 0x6c, 0x59,
      0xd3, 0xad, 0xf7, 0x47, 0xe9, 0x91, 0xde, 0xe7, 0xa4, 0xc6, 0x20, 0x45,
      0x8d, 0xd8, 0x6e, 0x87, 0x34, 0x43, 0x8d, 0x6f, 0xbc, 0xc5, 0xe3, 0xd6,
      0x47, 0xd3, 0xce, 0x10, 0x30, 0x92, 0xdb, 0x09, 0x63, 0xc9, 0x21, 0x68,
      0xb8, 0x39, 0x4e, 0x9d, 0x48, 0xf4, 0xa7, 0xbd, 0x7c, 0xea, 0x4f, 0x67,
      0x84, 0x91, 0xec, 0xfc, 0x7f, 0x11, 0x57, 0x9b, 0x67, 0xe0, 0xb2, 0x32,
      0x87, 0x86, 0x4d, 0xfc, 0x9c, 0xfd, 0x1a, 0x35};
};

TEST_CASE_METHOD(CodecsFixture, "Codec Base32 encodes a dest hash", "[codec]")
{
  using Catch::Matchers::Equals;
  using vec = std::vector<std::uint8_t>;

  const std::string base32("qfyayov4b56hjl7tft3gpowxke4rue6oiaylutlww3i4wdiwrwja");

  const std::string encoded(tini2p::crypto::Base32::Encode(dest_hash));

  REQUIRE_THAT(base32, Equals(encoded));

  REQUIRE(encoded.size() == 52);
  REQUIRE(tini2p::crypto::Base32::decoded_len(encoded.size()) == dest_hash.size());
  const std::vector<std::uint8_t> decoded(
      tini2p::crypto::Base32::Decode(encoded.c_str(), encoded.size()));

  REQUIRE_THAT(vec(decoded.begin(), decoded.end()), Equals(vec(dest_hash.begin(), dest_hash.end())));
}

TEST_CASE_METHOD(CodecsFixture, "Codec Base64 encodes a dest hash", "[codec]")
{
  using Catch::Matchers::Equals;

  const std::string base64("gXAMOrwPfHSv8yz2Z7rXUTkaE85AMLpNdrbRyw0WjZI=");

  const tini2p::exception::Exception ex{"Base64", "test"};

  const std::string encoded(tini2p::crypto::Base64::Encode(dest_hash));

  REQUIRE_THAT(base64, Equals(encoded));

  const tini2p::crypto::SecBytes decoded(
      tini2p::crypto::Base64::Decode(encoded));

  REQUIRE_THAT(static_cast<std::vector<std::uint8_t>>(decoded), Equals(dest_hash));
}

TEST_CASE_METHOD(CodecsFixture, "Codec Base64 encodes full destination", "[codec]")
{
  using Catch::Matchers::Equals;

  const std::string base64("2MMB2Ml9ezZPUXTyeldQT1D3~3nWEWNeTa5OIU8oFQiuQ2RJsggHosyJAo1T9sMDAcH1QGI6dW7KbjSRgyGtle~VzRplRtpWvoPYoRPgWRf9IR0xz5Cpvr3~IobLPMdRUSGFwV87Ge4oo0434v1ylykRE9eedrsRxnW8P4QWlIQlb-W1RByRalgpnrfpQ8CT6OmTXC-EBV-R9bTlnKsdqlpq0PgklppCTIhaRXjBAACCuTNp9Gm1S9HFxNFnjtweHgQb0hKMwZtwBhhYSarpGrVW1Y3WsgZoS38Uv9PPnhm92EJ4cKgTQHkKZcP21hP0lXBl5fejPCKUAX15Oao36RqKIQCCsTdmHx9Ptf2j6P3hAPP62HAM5LYiqMauWLDwVW-ROIXSfrEBY-gi6VV2aFLBRNuBG7qQbwK3QpIem7pXE~LcWW~PHsomNIrEAiIxUE1W5RqtBT5dQ25b7-6tmB4ew3w7sqMggCU7wZafEyMMWGxZ0633R-mR3uekxiBFjdhuhzRDjW-8xePWR9POEDCS2wljySFouDlOnUj0p7186k9nhJHs~H8RV5tn4LIyh4ZN~Jz9GjU=");

  const std::string encoded(
      tini2p::crypto::Base64::Encode(dest.data(), dest.size()));

  REQUIRE_THAT(base64, Equals(encoded));

  const std::vector<std::uint8_t> decoded(
      tini2p::crypto::Base64::Decode(encoded.c_str(), encoded.size()));

  REQUIRE_THAT(decoded, Equals(dest));
}

TEST_CASE_METHOD(CodecsFixture, "Codec rejects invalid encoding", "[codec]")
{
  const std::string base32("1!@#$%^&*()_-+=");
  REQUIRE_THROWS(tini2p::crypto::Base32::Decode(base32.c_str(), base32.size()));

  const std::string base64("!@#$%^&*()_-+");
  REQUIRE_THROWS(tini2p::crypto::Base64::Decode(base64.c_str(), base64.size()));
}

TEST_CASE_METHOD(CodecsFixture, "Codec rejects null arguments", "[codec]")
{
  REQUIRE_THROWS(tini2p::crypto::Base32::Encode(nullptr, 0));
  REQUIRE_THROWS(tini2p::crypto::Base32::Decode(nullptr, 0));
  REQUIRE_THROWS(tini2p::crypto::Base64::Encode(nullptr, 0));
  REQUIRE_THROWS(tini2p::crypto::Base64::Decode(nullptr, 0));
}
