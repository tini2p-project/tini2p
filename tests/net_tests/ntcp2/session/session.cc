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

#include <iostream>

#include <catch2/catch.hpp>

#include "src/ntcp2/session/session.h"
#include "src/ntcp2/session/listener.h"
#include "src/ntcp2/session/manager.h"

namespace crypto = tini2p::crypto;
namespace meta = tini2p::meta::ntcp2::session;

using namespace tini2p::ntcp2;

struct SessionFixture
{
  SessionFixture()
      : host(
            boost::asio::ip::tcp::v4(),
            crypto::RandInRange<std::uint16_t>(9111, 10135)),
        host_v6(
            boost::asio::ip::tcp::v6(),
            crypto::RandInRange<std::uint16_t>(9111, 10135)),
        dest(new tini2p::data::Info(
            std::make_unique<tini2p::data::Identity>(),
            std::vector<tini2p::data::Address>{
                tini2p::data::Address(host.address().to_string(), host.port()),
                tini2p::data::Address(
                    host_v6.address().to_string(),
                    host_v6.port())})),
        info(new tini2p::data::Info()),
        manager(dest.get(), host, host_v6),
        init(dest.get(), info.get())
  {
    using BlockPtr = std::unique_ptr<tini2p::data::Block>;

    msg.blocks.emplace_back(BlockPtr(new tini2p::data::PaddingBlock(3)));

    // give session listeners time to start before sending requests
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  ~SessionFixture()
  {
    REQUIRE_NOTHROW(init.Stop());
    REQUIRE_NOTHROW(manager.Stop());

    // wait while all sessions + listeners shut down
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  /// @brief Initialize a mock IPv4 NTCP2 session
  decltype(auto) InitializeSession()
  {
    REQUIRE_NOTHROW(init.Start(false /*prefer_v6*/));
    REQUIRE_NOTHROW(init.Wait());
    REQUIRE(init.ready());

    Session<SessionResponder>* remote;
    REQUIRE_NOTHROW(
        remote =
            manager.listener(meta::IP_t::v4)->session(info->noise_keys().pk));

    REQUIRE(remote);
    REQUIRE_NOTHROW(remote->Wait());
    REQUIRE(remote->ready());

    return remote;
  }

  /// @brief Initialize a mock IPv6 NTCP2 session
  decltype(auto) InitializeSessionV6()
  {
    REQUIRE_NOTHROW(init.Start());
    REQUIRE_NOTHROW(init.Wait());
    REQUIRE(init.ready());

    Session<SessionResponder>* remote;
    REQUIRE_NOTHROW(
        remote =
            manager.listener(meta::IP_t::v6)->session(info->noise_keys().pk));

    REQUIRE(remote);
    REQUIRE_NOTHROW(remote->Wait());
    REQUIRE(remote->ready());

    return remote;
  }

  boost::asio::ip::tcp::endpoint host, host_v6;
  std::unique_ptr<tini2p::data::Info> dest, info;
  Session<SessionInitiator> init;
  SessionManager manager;
  DataPhaseMessage msg;
};

TEST_CASE_METHOD(
    SessionFixture,
    "IPv4 Session creates a connection to a destination",
    "[session]")
{
  REQUIRE(InitializeSession());
}

TEST_CASE_METHOD(
    SessionFixture,
    "IPv6 Session creates a connection to a destination",
    "[session]")
{
  REQUIRE(InitializeSessionV6());
}

TEST_CASE_METHOD(
    SessionFixture,
    "IPv4 Session writes and reads after successful connection",
    "[session]")
{
  auto remote = InitializeSession();

  REQUIRE(remote);

  REQUIRE_NOTHROW(init.Write(msg));
  REQUIRE_NOTHROW(remote->Read(msg));

  REQUIRE_NOTHROW(remote->Write(msg));
  REQUIRE_NOTHROW(init.Read(msg));
}

TEST_CASE_METHOD(
    SessionFixture,
    "IPv6 Session writes and reads after successful connection",
    "[session]")
{
  auto remote_v6 = InitializeSessionV6();

  REQUIRE(remote_v6);

  REQUIRE_NOTHROW(init.Write(msg));
  REQUIRE_NOTHROW(remote_v6->Read(msg));

  REQUIRE_NOTHROW(remote_v6->Write(msg));
  REQUIRE_NOTHROW(init.Read(msg));
}

TEST_CASE_METHOD(
    SessionFixture,
    "Manager Session writes and reads after successful connection",
    "[session]")
{
  auto* mgr_init = manager.session(dest.get());

  REQUIRE(mgr_init);
  REQUIRE_NOTHROW(mgr_init->Start());
  REQUIRE_NOTHROW(mgr_init->Wait());
  REQUIRE(mgr_init->ready());

  auto* remote_v6 =
      manager.listener(meta::IP_t::v6)->session(dest->noise_keys().pk);

  REQUIRE(remote_v6);
  REQUIRE_NOTHROW(remote_v6->Wait());
  REQUIRE(remote_v6->ready());

  REQUIRE_NOTHROW(mgr_init->Write(msg));
  REQUIRE_NOTHROW(remote_v6->Read(msg));

  REQUIRE_NOTHROW(remote_v6->Write(msg));
  REQUIRE_NOTHROW(mgr_init->Read(msg));

  REQUIRE_NOTHROW(mgr_init->Stop());
}

TEST_CASE_METHOD(
    SessionFixture,
    "SessionManager rejects multiple sessions to same destination",
    "[session]")
{
  REQUIRE_NOTHROW(manager.session(dest.get()));

  REQUIRE_THROWS(manager.session(dest.get()));
}

TEST_CASE_METHOD(
    SessionFixture,
    "Session rejects reading/writing data phase messages w/o valid handshake",
    "[session]")
{
  REQUIRE(!init.ready());
  REQUIRE_THROWS(init.Write(msg));
  REQUIRE_THROWS(init.Read(msg));

  boost::asio::io_context ctx;
  Session<SessionResponder> resp(
      dest.get(),
      boost::asio::ip::tcp::socket(ctx, boost::asio::ip::tcp::v6()));

  REQUIRE(!resp.ready());
  REQUIRE_THROWS(resp.Write(msg));
  REQUIRE_THROWS(resp.Read(msg));
}

TEST_CASE_METHOD(
    SessionFixture,
    "SessionManager rejects null RouterInfo for outbound sessions",
    "[session]")
{
  REQUIRE_THROWS(manager.session(nullptr));
}

// TODO(tini2p): fix manager + session sockets to be able to restart.
//   Use executor_work_guard to keep io_context::run from returning
//TEST_CASE_METHOD(
//    SessionFixture,
//    "SessionManager rejects new connection for already existing session",
//    "[session]")
//{
//  decltype(init) s0(dest.get(), info.get());
//  REQUIRE_NOTHROW(s0.Start());
//
//  decltype(init) s1(dest.get(), info.get());
//  REQUIRE_NOTHROW(s1.Start());
//  REQUIRE_NOTHROW(s1.Stop());
//
//  std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//  REQUIRE(manager.blacklisted(s0.connect_key()));
//}
