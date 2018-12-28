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

#include <jbcoin/ledger/ApplyViewImpl.h>
#include <jbcoin/basics/contract.h>
#include <cassert>

namespace jbcoin {

ApplyViewImpl::ApplyViewImpl(
    ReadView const* base, ApplyFlags flags)
    : ApplyViewBase (base, flags)
{
}

void
ApplyViewImpl::apply (OpenView& to,
    STTx const& tx, TER ter,
        beast::Journal j)
{
    items_.apply(to, tx, ter, deliver_, j);
}

std::size_t
ApplyViewImpl::size ()
{
    return items_.size ();
}

void
ApplyViewImpl::visit (
    OpenView& to,
    std::function <void (
        uint256 const& key,
        bool isDelete,
        std::shared_ptr <SLE const> const& before,
        std::shared_ptr <SLE const> const& after)> const& func)
{
    items_.visit (to, func);
}

} // jbcoin
