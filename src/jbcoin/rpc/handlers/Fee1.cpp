//------------------------------------------------------------------------------
/*
    This file is part of jbcoind: https://github.com/jbcoin/jbcoind
    Copyright (c) 2012-2015 JBCoin Labs Inc.

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

#include <jbcoin/app/ledger/OpenLedger.h>
#include <jbcoin/app/main/Application.h>
#include <jbcoin/app/misc/TxQ.h>
#include <jbcoin/rpc/Context.h>
#include <jbcoin/protocol/ErrorCodes.h>
#include <jbcoin/protocol/Feature.h>

namespace jbcoin
{
    Json::Value doFee(RPC::Context& context)
    {
        // Bail if fee escalation is not enabled.
        auto const view = context.app.openLedger().current();
        if (!view || !view->rules().enabled(featureFeeEscalation))
        {
            RPC::inject_error(rpcNOT_ENABLED, context.params);
            return context.params;
        }

        auto result = context.app.getTxQ().doRPC(context.app);
        if (result.type() == Json::objectValue)
            return result;
        assert(false);
        RPC::inject_error(rpcINTERNAL, context.params);
        return context.params;
    }
} // jbcoin
