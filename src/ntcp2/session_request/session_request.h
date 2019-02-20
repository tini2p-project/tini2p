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

#ifndef SRC_SESSION_REQUEST_SESSION_REQUEST_H_
#define SRC_SESSION_REQUEST_SESSION_REQUEST_H_

#include <chrono>

#include <noise/protocol/handshakestate.h>

#include "src/crypto/aes.h"
#include "src/crypto/rand.h"

#include "src/data/router/info.h"

#include "src/ntcp2/session_request/kdf.h"
#include "src/ntcp2/session_request/meta.h"
#include "src/ntcp2/session_request/options.h"

namespace tini2p
{
namespace ntcp2
{
struct SessionRequestMessage
{
  std::vector<std::uint8_t> data, padding;
  std::array<std::uint8_t, tini2p::meta::ntcp2::session_request::CiphertextSize>
      ciphertext;
  SessionRequestOptions options;

  SessionRequestMessage()
      : data(meta::ntcp2::session_request::NoisePayloadSize), options()
  {
    if (options.pad_len)
    {
      padding.resize(options.pad_len);
      crypto::RandBytes(padding.data(), padding.size());
    }
  }

  SessionRequestMessage(
      const std::uint16_t m3p2_len,
      const std::uint16_t pad_len)
      : data(meta::ntcp2::session_request::NoisePayloadSize + pad_len),
        options(m3p2_len, pad_len)
  {
    if (pad_len)
    {
      padding.resize(pad_len);
      crypto::RandBytes(padding.data(), padding.size());
    }
  }
};

template <class Role_t>
class SessionRequest
{
  Role_t role_;
  NoiseHandshakeState* state_;
  ntcp2::SessionRequestKDF kdf_;
  tini2p::crypto::aes::CBCEncryption encryption_;
  tini2p::crypto::aes::CBCDecryption decryption_;

 public:
  /// @brief Create a SessionRequest processor for a given destination
  /// @param state Pointer to initialized Noise handshake state
  /// @param router_hash Hash of destination RouterIdentity
  /// @param iv Remote AES IV for key obfuscation
  /// @throw Invalid argument on null handshake state
  SessionRequest(
      NoiseHandshakeState* state,
      const tini2p::data::IdentHash& router_hash,
      const crypto::aes::IV& iv)
      : state_(state),
        kdf_(state_),
        encryption_(router_hash, iv),
        decryption_(router_hash, iv)
  {
    if (!state)
      exception::Exception{"SessionRequest", __func__}
          .throw_ex<std::invalid_argument>("null handshake state.");
  }

  /// @brief Get a mutable reference to the KDF object
  decltype(kdf_)& kdf() noexcept
  {
    return kdf_;
  }

  /// @brief Get a const reference to the KDF object
  const decltype(kdf_)& kdf() const noexcept
  {
    return kdf_;
  }

  /// @brief Get a const reference to the AES encryptor
  const decltype(encryption_)& encryption() const noexcept
  {
    return encryption_;
  }

  /// @brief Get a const reference to the AES decryptor
  const decltype(decryption_)& decryption() const noexcept
  {
    return decryption_;
  }

  /// @brief Process session request message based on role
  void ProcessMessage(SessionRequestMessage& message)
  {
    if (role_.id() == noise::InitiatorRole)
      Write(message);  // write and encrypt message
    else
      Read(message);  // decrypt and read message
  }

 private:
  void Write(SessionRequestMessage& message)
  {
    namespace meta = tini2p::meta::ntcp2::session_request;
    namespace x25519 = tini2p::crypto::x25519;

    const exception::Exception ex{"SessionRequest", __func__};

    NoiseBuffer data /*output*/, payload /*input*/;

    // ensure enough room to hold Noise payload + padding
    message.data.resize(
        meta::NoisePayloadSize + (std::uint16_t)message.options.pad_len);

    auto& in = message.options.buffer;
    auto& out = message.data;

    noise::RawBuffers bufs{in.data(), in.size(), out.data(), out.size()};
    noise::setup_buffers(data, payload, bufs);
    noise::write_message(state_, &data, &payload, ex);

    // encrypt ephemeral key in place
    encryption_.Process(
        out.data(), x25519::PubKeyLen, out.data(), x25519::PubKeyLen);

    // save ciphertext for session created KDF
    save_ciphertext(message);

    if (message.options.pad_len < meta::MinPaddingSize
        || message.options.pad_len > meta::MaxPaddingSize)
      ex.throw_ex<std::length_error>("invalid padding length.");

    std::copy(
        message.padding.begin(),
        message.padding.end(),
        &message.data[meta::PaddingOffset]);
  }

  void Read(SessionRequestMessage& message)
  {
    namespace meta = tini2p::meta::ntcp2::session_request;
    namespace x25519 = tini2p::crypto::x25519;

    const exception::Exception ex{"SessionRequest", __func__};

    NoiseBuffer data /*input*/, payload /*output*/;

    auto& in = message.data;
    auto& out = message.options.buffer;
    const auto& in_size = meta::NoisePayloadSize;

    if (in.size() < meta::MinSize || in.size() > meta::MaxSize)
      ex.throw_ex<std::length_error>("invalid message size.");

    // save ciphertext for session created KDF
    save_ciphertext(message);

    // decrypt ephemeral key in place
    decryption_.Process(
        in.data(), x25519::PubKeyLen, in.data(), x25519::PubKeyLen);

    noise::RawBuffers bufs{in.data(), in_size, out.data(), out.size()};
    noise::setup_buffers(payload, data, bufs);
    noise::read_message(state_, &data, &payload, ex);

    // deserialize options from buffer
    message.options.deserialize();

    if (message.options.pad_len < meta::MinPaddingSize
        || message.options.pad_len > meta::MaxPaddingSize)
      ex.throw_ex<std::length_error>("invalid padding length.");

    if (message.data.size() - meta::NoisePayloadSize == message.options.pad_len)
      {
        message.padding.resize(message.options.pad_len);
        const auto& pad_begin = &message.data[meta::PaddingOffset];
        auto& pad = message.padding;
        std::copy(pad_begin, pad_begin + pad.size(), pad.begin());
      }
  }

  void save_ciphertext(SessionRequestMessage& message)
  {
    namespace meta = tini2p::meta::ntcp2::session_request;

    const auto c = &message.data[meta::CiphertextOffset];
    std::copy(c, c + meta::CiphertextSize, message.ciphertext.data());
  }
};
}  // namespace ntcp2
}  // namespace tini2p

#endif  // SRC_SESSION_REQUEST_SESSION_REQUEST_H_
