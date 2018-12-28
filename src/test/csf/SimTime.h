//------------------------------------------------------------------------------
/*
    This file is part of jbcoind: https://github.com/jbcoin/jbcoind
    Copyright (c) 2012-2017 JBCoin Labs Inc

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef JBCOIN_TEST_CSF_SIMTIME_H_INCLUDED
#define JBCOIN_TEST_CSF_SIMTIME_H_INCLUDED

#include <jbcoin/beast/clock/manual_clock.h>
#include <chrono>

namespace jbcoin {
namespace test {
namespace csf {

using RealClock = std::chrono::system_clock;
using RealDuration = RealClock::duration;
using RealTime = RealClock::time_point;

using SimClock = beast::manual_clock<std::chrono::steady_clock>;
using SimDuration = typename SimClock::duration;
using SimTime = typename SimClock::time_point;

}  // namespace csf
}  // namespace test
}  // namespace jbcoin

#endif
