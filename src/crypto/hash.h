/* copyright (c) 2019, tini2p
 * all rights reserved.
 * 
 * redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * this software is provided by the copyright holders and contributors "as is"
 * and any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose are
 * disclaimed. in no event shall the copyright holder or contributors be liable
 * for any direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute goods or
 * services; loss of use, data, or profits; or business interruption) however
 * caused and on any theory of liability, whether in contract, strict liability,
 * or tort (including negligence or otherwise) arising in any way out of the use
 * of this software, even if advised of the possibility of such damage.
*/

#ifndef SRC_CRYPTO_HASH_H_
#define SRC_CRYPTO_HASH_H_

#include <memory>

#include <boost/endian/arithmetic.hpp>

#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "src/exception/exception.h"

namespace tini2p
{
namespace crypto
{
namespace hash
{
enum
{
  Poly1305Len = 16,
  Sha256Len = 32,
};

using Sha256 = std::array<std::uint8_t, Sha256Len>;

template <class HmacKey, class Input, class Digest>
inline void HmacSha256(const HmacKey& key, const Input& input, Digest& digest)
{
  if (digest.size() != Sha256Len)
    exception::Exception{"Crypto: Hash", __func__}.throw_ex<std::length_error>(
        "invalid digest size.");

  CryptoPP::HMAC<CryptoPP::SHA256> hmac(key.data(), key.size());
  hmac.Update(input.data(), input.size());
  hmac.Final(digest.data());
}
}  // namespace hash
}  // namespace crypto
}  // namespace tini2p

#endif  // SRC_CRYPTO_HASH_H_
