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

#include <jbcoin/app/ledger/LedgerMaster.h>
#include <jbcoin/app/ledger/TransactionMaster.h>
#include <jbcoin/app/main/Application.h>
#include <jbcoin/app/misc/NetworkOPs.h>
#include <jbcoin/app/misc/Transaction.h>
#include <jbcoin/net/RPCErr.h>
#include <jbcoin/protocol/ErrorCodes.h>
#include <jbcoin/protocol/JsonFields.h>
#include <jbcoin/rpc/Context.h>
#include <jbcoin/rpc/impl/RPCHelpers.h>

namespace jbcoin {

// {
//   transaction: <hex>
// }

static
bool
isHexTxID (std::string const& txid)
{
    if (txid.size () != 64)
        return false;

    auto const ret = std::find_if (txid.begin (), txid.end (),
        [](std::string::value_type c)
        {
            return !std::isxdigit (static_cast<unsigned char>(c));
        });

    return (ret == txid.end ());
}

static
bool
isValidated (RPC::Context& context, std::uint32_t seq, uint256 const& hash)
{
    if (!context.ledgerMaster.haveLedger (seq))
        return false;

    if (seq > context.ledgerMaster.getValidatedLedger ()->info().seq)
        return false;

    return context.ledgerMaster.getHashBySeq (seq) == hash;
}

bool
getMetaHex (Ledger const& ledger,
    uint256 const& transID, std::string& hex)
{
    SHAMapTreeNode::TNType type;
    auto const item =
        ledger.txMap().peekItem (transID, type);

    if (!item)
        return false;

    if (type != SHAMapTreeNode::tnTRANSACTION_MD)
        return false;

    SerialIter it (item->slice());
    it.getVL (); // skip transaction
    hex = strHex (makeSlice(it.getVL ()));
    return true;
}

Json::Value doTx (RPC::Context& context)
{
    if (!context.params.isMember (jss::transaction))
        return rpcError (rpcINVALID_PARAMS);

    bool binary = context.params.isMember (jss::binary)
            && context.params[jss::binary].asBool ();

    auto const txid  = context.params[jss::transaction].asString ();

    if (!isHexTxID (txid))
        return rpcError (rpcNOT_IMPL);

    auto txn = context.app.getMasterTransaction ().fetch (
        from_hex_text<uint256>(txid), true);

    if (!txn)
        return rpcError (rpcTXN_NOT_FOUND);

    Json::Value ret = txn->getJson (1, binary);

    if (txn->getLedger () == 0)
        return ret;

    if (auto lgr = context.ledgerMaster.getLedgerBySeq (txn->getLedger ()))
    {
        bool okay = false;

        if (binary)
        {
            std::string meta;

            if (getMetaHex (*lgr, txn->getID (), meta))
            {
                ret[jss::meta] = meta;
                okay = true;
            }
        }
        else
        {
            auto rawMeta = lgr->txRead (txn->getID()).second;
            if (rawMeta)
            {
                auto txMeta = std::make_shared<TxMeta> (txn->getID (),
                    lgr->seq (), *rawMeta, context.app.journal ("TxMeta"));
                okay = true;
                auto meta = txMeta->getJson (0);
                addPaymentDeliveredAmount (meta, context, txn, txMeta);
                ret[jss::meta] = meta;
            }
        }

        if (okay)
            ret[jss::validated] = isValidated (
                context, lgr->info().seq, lgr->info().hash);
    }

    return ret;
}

} // jbcoin
