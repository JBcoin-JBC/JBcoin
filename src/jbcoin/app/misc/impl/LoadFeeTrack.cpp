//------------------------------------------------------------------------------
/*
    This file is part of jbcoind: https://github.com/jbcoin/jbcoind
    Copyright (c) 2012, 2013 JBCoin Labs Inc.

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

#include <jbcoin/app/misc/LoadFeeTrack.h>
#include <jbcoin/basics/contract.h>
#include <jbcoin/basics/Log.h>
#include <jbcoin/core/Config.h>
#include <jbcoin/ledger/ReadView.h>
#include <jbcoin/protocol/STAmount.h>
#include <jbcoin/protocol/JsonFields.h>
#include <cstdint>
#include <type_traits>

namespace jbcoin {

bool
LoadFeeTrack::raiseLocalFee ()
{
    std::lock_guard <std::mutex> sl (lock_);

    if (++raiseCount_ < 2)
        return false;

    std::uint32_t origFee = localTxnLoadFee_;

    // make sure this fee takes effect
    if (localTxnLoadFee_ < remoteTxnLoadFee_)
        localTxnLoadFee_ = remoteTxnLoadFee_;

    // Increase slowly
    localTxnLoadFee_ += (localTxnLoadFee_ / lftFeeIncFraction);

    if (localTxnLoadFee_ > lftFeeMax)
        localTxnLoadFee_ = lftFeeMax;

    if (origFee == localTxnLoadFee_)
        return false;

    JLOG(j_.debug()) << "Local load fee raised from " <<
        origFee << " to " << localTxnLoadFee_;
    return true;
}

bool
LoadFeeTrack::lowerLocalFee ()
{
    std::lock_guard <std::mutex> sl (lock_);
    std::uint32_t origFee = localTxnLoadFee_;
    raiseCount_ = 0;

    // Reduce slowly
    localTxnLoadFee_ -= (localTxnLoadFee_ / lftFeeDecFraction );

    if (localTxnLoadFee_ < lftNormalFee)
        localTxnLoadFee_ = lftNormalFee;

    if (origFee == localTxnLoadFee_)
        return false;

    JLOG(j_.debug()) << "Local load fee lowered from " <<
        origFee << " to " << localTxnLoadFee_;
    return true;
}

//------------------------------------------------------------------------------

// NIKB TODO: Once we get C++17, we can replace lowestTerms
//            with this:
//
// template <class T1, class T2,
//     class = std::enable_if_t<
//         std::is_integral_v<T1> &&
//         std::is_integral_v<T2>>
// >
// void lowestTerms(T1& a,  T2& b)
// {
//     if (auto const gcd = std::gcd(a, b))
//     {
//         a /= gcd;
//         b /= gcd;
//     }
// }

template <class T1, class T2,
    class = std::enable_if_t <
        std::is_integral<T1>::value &&
        std::is_unsigned<T1>::value &&
        sizeof(T1) <= sizeof(std::uint64_t) >,
    class = std::enable_if_t <
        std::is_integral<T2>::value &&
        std::is_unsigned<T2>::value &&
        sizeof(T2) <= sizeof(std::uint64_t) >
>
void lowestTerms(T1& a,  T2& b)
{
    if (a == 0 && b == 0)
        return;

    std::uint64_t x = a, y = b;
    while (y != 0)
    {
        auto t = x % y;
        x = y;
        y = t;
    }
    a /= x;
    b /= x;
}

// Scale using load as well as base rate
std::uint64_t
scaleFeeLoad(std::uint64_t fee, LoadFeeTrack const& feeTrack,
    Fees const& fees, bool bUnlimited)
{
    return 0;
}

} // jbcoin
