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

#include "src/ntcp2/session_request/session_request.h"
#include "src/ntcp2/session_created/session_created.h"
#include "src/ntcp2/session_confirmed/session_confirmed.h"
#include "src/ntcp2/data_phase/data_phase.h"

namespace crypto = tini2p::crypto;
namespace exception = tini2p::exception;
namespace meta = tini2p::meta::ntcp2;

using namespace tini2p::ntcp2;

/// @brief Container for performing a valid mock handshake
struct MockHandshake
{
  MockHandshake()
      : remote_info(new tini2p::data::Info()),
        local_info(new tini2p::data::Info()),
        sco_message(
            local_info.get(),
            crypto::RandInRange(
                meta::session_confirmed::MinPaddingSize,
                meta::session_confirmed::MaxPaddingSize)),
        srq_message(
            sco_message.payload_size(),
            crypto::RandInRange(
                meta::session_request::MinPaddingSize,
                meta::session_request::MaxPaddingSize)),
        scr_message()
  {
    const exception::Exception ex{"MockHandshake", __func__};

    noise::init_handshake<Initiator>(&initiator_state, ex);
    noise::init_handshake<Responder>(&responder_state, ex);

    InitializeSessionRequest();
  }

  /// @brief Initialize SessionRequest initiator + responder
  void InitializeSessionRequest()
  {
    const auto& ident_hash = remote_info->identity().hash();

    srq_initiator = std::make_unique<SessionRequest<Initiator>>(
        initiator_state, ident_hash, remote_info->iv());

    srq_responder = std::make_unique<SessionRequest<Responder>>(
        responder_state, ident_hash, remote_info->iv());
  }

  /// @brief Perform a valid SessionRequest between initiator + responder
  void ValidSessionRequest()
  {
    srq_responder->kdf().generate_keys();
    srq_responder->kdf().get_local_public_key(remote_key);
    srq_responder->kdf().derive_keys();

    srq_initiator->kdf().generate_keys();
    srq_initiator->kdf().derive_keys(remote_key);

    srq_initiator->ProcessMessage(srq_message);
    srq_responder->ProcessMessage(srq_message);
  }

  /// @brief After valid SessionRequest, initialize SessionCreated initiator + responder
  /// @detail Roles are switched according to Noise spec
  void InitializeSessionCreated()
  {
    scr_initiator = std::make_unique<SessionCreated<Initiator>>(
        responder_state, srq_message, router_hash, iv);

    scr_responder = std::make_unique<SessionCreated<Responder>>(
        initiator_state, srq_message, router_hash, iv);
  }

  /// @brief Perform a valid SessionCreated message exchange
  void ValidSessionCreated()
  {
    InitializeSessionCreated();

    scr_initiator->ProcessMessage(scr_message);
    scr_responder->ProcessMessage(scr_message);
  }

  /// @brief After valid SessionCreated, initialize SessionConfirmed initiator + responder
  /// @detail Roles are switched according to Noise spec
  void InitializeSessionConfirmed()
  {
    sco_initiator = std::make_unique<SessionConfirmed<Initiator>>(
        initiator_state, scr_message);

    sco_responder = std::make_unique<SessionConfirmed<Responder>>(
        responder_state, scr_message);
  }

  /// @brief Perform a valid SessionConfirmed message exchange
  void ValidSessionConfirmed()
  {
    InitializeSessionConfirmed();

    sco_initiator->ProcessMessage(sco_message, srq_message.options);
    sco_responder->ProcessMessage(sco_message, srq_message.options);
  }

  /// @brief Initialize a DataPhase exchange after successful SessionConfirmed exchange
  void InitializeDataPhase()
  {
    dp_initiator = std::make_unique<DataPhase<Initiator>>(responder_state);
    dp_responder = std::make_unique<DataPhase<Responder>>(initiator_state);
  }

  NoiseHandshakeState *initiator_state, *responder_state;

  crypto::x25519::PubKey remote_key;
  tini2p::data::IdentHash router_hash;
  crypto::aes::IV iv;
  std::unique_ptr<tini2p::data::Info> remote_info, local_info;

  // handshake messages, session confirmed must be initialized first to initialize the session request message
  SessionConfirmedMessage sco_message;
  SessionRequestMessage srq_message;
  SessionCreatedMessage scr_message;
  DataPhaseMessage dp_message;

  // handshake message handlers
  std::unique_ptr<SessionRequest<Initiator>> srq_initiator; 
  std::unique_ptr<SessionRequest<Responder>> srq_responder; 

  std::unique_ptr<SessionCreated<Initiator>> scr_initiator; 
  std::unique_ptr<SessionCreated<Responder>> scr_responder; 

  std::unique_ptr<SessionConfirmed<Initiator>> sco_initiator; 
  std::unique_ptr<SessionConfirmed<Responder>> sco_responder; 

  std::unique_ptr<DataPhase<Initiator>> dp_initiator; 
  std::unique_ptr<DataPhase<Responder>> dp_responder; 
};
