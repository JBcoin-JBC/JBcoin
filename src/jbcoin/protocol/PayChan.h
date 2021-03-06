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

#ifndef JBCOIN_PROTOCOL_PAYCHAN_H_INCLUDED
#define JBCOIN_PROTOCOL_PAYCHAN_H_INCLUDED

#include <jbcoin/basics/base_uint.h>
#include <jbcoin/protocol/HashPrefix.h>
#include <jbcoin/protocol/Serializer.h>
#include <jbcoin/protocol/JBCAmount.h>

namespace jbcoin {

inline
void
serializePayChanAuthorization (
    Serializer& msg,
    uint256 const& key,
    JBCAmount const& amt)
{
    msg.add32 (HashPrefix::paymentChannelClaim);
    msg.add256 (key);
    msg.add64 (amt.drops ());
}

} // jbcoin

#endif
