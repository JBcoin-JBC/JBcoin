//------------------------------------------------------------------------------
/*
    This file is part of jbcoind: https://github.com/jbcoin/jbcoind
    Copyright (c) 2012-2014 JBCoin Labs Inc.

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

#include <jbcoin/app/main/Application.h>
#include <jbcoin/app/misc/LoadFeeTrack.h>
#include <jbcoin/app/paths/JBCoinState.h>
#include <jbcoin/ledger/ReadView.h>
#include <jbcoin/net/RPCErr.h>
#include <jbcoin/protocol/ErrorCodes.h>
#include <jbcoin/protocol/JsonFields.h>
#include <jbcoin/protocol/TxFlags.h>
#include <jbcoin/rpc/Context.h>
#include <jbcoin/rpc/impl/RPCHelpers.h>
#include <jbcoin/rpc/impl/Tuning.h>

namespace jbcoin {

static void fillTransaction (
    RPC::Context& context,
    Json::Value& txArray,
    AccountID const& accountID,
    std::uint32_t& sequence,
    ReadView const& ledger)
{
    txArray["Sequence"] = Json::UInt (sequence++);
    txArray["Account"] = context.app.accountIDCache().toBase58 (accountID);
    auto& fees = ledger.fees();
    // Convert the reference transaction cost in fee units to drops
    // scaled to represent the current fee load.
    txArray["Fee"] = Json::UInt (scaleFeeLoad(fees.units,
        context.app.getFeeTrack(), fees, false));
}

// {
//   account: <account>|<account_public_key>
//   ledger_hash : <ledger>
//   ledger_index : <ledger_index>
//   limit: integer                 // optional, number of problems
//   role: gateway|user             // account role to assume
//   transactions: true             // optional, reccommend transactions
// }
Json::Value doNoJBCoinCheck (RPC::Context& context)
{
    auto const& params (context.params);
    if (! params.isMember (jss::account))
        return RPC::missing_field_error ("account");

    if (! params.isMember ("role"))
        return RPC::missing_field_error ("role");
    bool roleGateway = false;
    {
        std::string const role = params["role"].asString();
        if (role == "gateway")
            roleGateway = true;
        else if (role != "user")
        return RPC::invalid_field_error ("role");
    }

    unsigned int limit;
    if (auto err = readLimitField(limit, RPC::Tuning::noJBCoinCheck, context))
        return *err;

    bool transactions = false;
    if (params.isMember (jss::transactions))
        transactions = params["transactions"].asBool();

    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);
    if (! ledger)
        return result;

    Json::Value dummy;
    Json::Value& jvTransactions =
        transactions ? (result[jss::transactions] = Json::arrayValue) : dummy;

    std::string strIdent (params[jss::account].asString ());
    AccountID accountID;

    if (auto jv = RPC::accountFromString (accountID, strIdent))
    {
        for (auto it (jv.begin ()); it != jv.end (); ++it)
            result[it.memberName ()] = *it;

        return result;
    }

    auto const sle = ledger->read(keylet::account(accountID));
    if (! sle)
        return rpcError (rpcACT_NOT_FOUND);

    std::uint32_t seq = sle->getFieldU32 (sfSequence);

    Json::Value& problems = (result["problems"] = Json::arrayValue);

    bool bDefaultJBCoin = sle->getFieldU32 (sfFlags) & lsfDefaultJBCoin;

    if (bDefaultJBCoin & ! roleGateway)
    {
        problems.append ("You appear to have set your default jbcoin flag even though you "
            "are not a gateway. This is not recommended unless you are experimenting");
    }
    else if (roleGateway & ! bDefaultJBCoin)
    {
        problems.append ("You should immediately set your default jbcoin flag");
        if (transactions)
        {
            Json::Value& tx = jvTransactions.append (Json::objectValue);
            tx["TransactionType"] = "AccountSet";
            tx["SetFlag"] = 8;
            fillTransaction (context, tx, accountID, seq, *ledger);
        }
    }

    forEachItemAfter (*ledger, accountID,
            uint256(), 0, limit,
        [&](std::shared_ptr<SLE const> const& ownedItem)
        {
            if (ownedItem->getType() == ltJBCOIN_STATE)
            {
                bool const bLow = accountID == ownedItem->getFieldAmount(sfLowLimit).getIssuer();

                bool const bNoJBCoin = ownedItem->getFieldU32(sfFlags) &
                    (bLow ? lsfLowNoJBCoin : lsfHighNoJBCoin);

                std::string problem;
                bool needFix = false;
                if (bNoJBCoin & roleGateway)
                {
                    problem = "You should clear the no jbcoin flag on your ";
                    needFix = true;
                }
                else if (! roleGateway & ! bNoJBCoin)
                {
                    problem = "You should probably set the no jbcoin flag on your ";
                    needFix = true;
                }
                if (needFix)
                {
                    AccountID peer =
                        ownedItem->getFieldAmount (bLow ? sfHighLimit : sfLowLimit).getIssuer();
                    STAmount peerLimit = ownedItem->getFieldAmount (bLow ? sfHighLimit : sfLowLimit);
                    problem += to_string (peerLimit.getCurrency());
                    problem += " line to ";
                    problem += to_string (peerLimit.getIssuer());
                    problems.append (problem);

                    STAmount limitAmount (ownedItem->getFieldAmount (bLow ? sfLowLimit : sfHighLimit));
                    limitAmount.setIssuer (peer);

                    Json::Value& tx = jvTransactions.append (Json::objectValue);
                    tx["TransactionType"] = "TrustSet";
                    tx["LimitAmount"] = limitAmount.getJson (0);
                    tx["Flags"] = bNoJBCoin ? tfClearNoJBCoin : tfSetNoJBCoin;
                    fillTransaction(context, tx, accountID, seq, *ledger);

                    return true;
                }
            }
            return false;
        });

    return result;
}

} // jbcoin
