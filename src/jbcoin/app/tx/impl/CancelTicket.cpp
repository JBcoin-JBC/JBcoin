//------------------------------------------------------------------------------
/*
    This file is part of jbcoind: https://github.com/jbcoin/jbcoind
    Copyright (c) 2014 JBCoin Labs Inc.

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

#include <jbcoin/app/tx/impl/CancelTicket.h>
#include <jbcoin/basics/Log.h>
#include <jbcoin/protocol/Feature.h>
#include <jbcoin/protocol/Indexes.h>
#include <jbcoin/protocol/TxFlags.h>
#include <jbcoin/ledger/View.h>

namespace jbcoin {

NotTEC
CancelTicket::preflight (PreflightContext const& ctx)
{
    if (! ctx.rules.enabled(featureTickets))
        return temDISABLED;

    if (ctx.tx.getFlags() & tfUniversalMask)
        return temINVALID_FLAG;

    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    return preflight2 (ctx);
}

TER
CancelTicket::doApply ()
{
    uint256 const ticketId = ctx_.tx.getFieldH256 (sfTicketID);

    // VFALCO This is highly suspicious, we're requiring that the
    //        transaction provide the return value of getTicketIndex?
    SLE::pointer sleTicket = view().peek (keylet::ticket(ticketId));

    if (!sleTicket)
        return tecNO_ENTRY;

    auto const ticket_owner =
        sleTicket->getAccountID (sfAccount);

    bool authorized =
        account_ == ticket_owner;

    // The target can also always remove a ticket
    if (!authorized && sleTicket->isFieldPresent (sfTarget))
        authorized = (account_ == sleTicket->getAccountID (sfTarget));

    // And finally, anyone can remove an expired ticket
    if (!authorized && sleTicket->isFieldPresent (sfExpiration))
    {
        using tp = NetClock::time_point;
        using d = tp::duration;
        auto const expiration = tp{d{sleTicket->getFieldU32(sfExpiration)}};

        if (view().parentCloseTime() >= expiration)
            authorized = true;
    }

    if (!authorized)
        return tecNO_PERMISSION;

    std::uint64_t const hint (sleTicket->getFieldU64 (sfOwnerNode));

    if (! ctx_.view().dirRemove(
            keylet::ownerDir(ticket_owner), hint, ticketId, false))
    {
        return tefBAD_LEDGER;
    }

    auto viewJ = ctx_.app.journal ("View");
    adjustOwnerCount(view(), view().peek(
        keylet::account(ticket_owner)), -1, viewJ);
    ctx_.view ().erase (sleTicket);

    return tesSUCCESS;
}

}
