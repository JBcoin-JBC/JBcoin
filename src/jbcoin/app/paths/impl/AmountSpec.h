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

#ifndef JBCOIN_PATH_IMPL_AMOUNTSPEC_H_INCLUDED
#define JBCOIN_PATH_IMPL_AMOUNTSPEC_H_INCLUDED

#include <jbcoin/protocol/IOUAmount.h>
#include <jbcoin/protocol/JBCAmount.h>
#include <jbcoin/protocol/STAmount.h>

namespace jbcoin {

struct AmountSpec
{
    explicit AmountSpec() = default;

    bool native;
    union
    {
        JBCAmount jbc;
        IOUAmount iou;
    };
    boost::optional<AccountID> issuer;
    boost::optional<Currency> currency;

    friend
    std::ostream&
    operator << (
        std::ostream& stream,
        AmountSpec const& amt)
    {
        if (amt.native)
            stream << to_string (amt.jbc);
        else
            stream << to_string (amt.iou);
        if (amt.currency)
            stream << "/(" << *amt.currency << ")";
        if (amt.issuer)
            stream << "/" << *amt.issuer << "";
        return stream;
    }
};

struct EitherAmount
{
#ifndef NDEBUG
    bool native = false;
#endif

    union
    {
        IOUAmount iou;
        JBCAmount jbc;
    };

    EitherAmount () = default;

    explicit
    EitherAmount (IOUAmount const& a)
            :iou(a)
    {
    }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
    // ignore warning about half of iou amount being uninitialized
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    explicit
    EitherAmount (JBCAmount const& a)
            :jbc(a)
    {
#ifndef NDEBUG
        native = true;
#endif
    }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    explicit
    EitherAmount (AmountSpec const& a)
    {
#ifndef NDEBUG
        native = a.native;
#endif
        if (a.native)
            jbc = a.jbc;
        else
            iou = a.iou;
    }
};

template <class T>
T&
get (EitherAmount& amt)
{
    static_assert(sizeof(T) == -1, "Must used specialized function");
    return T(0);
}

template <>
inline
IOUAmount&
get<IOUAmount> (EitherAmount& amt)
{
    assert (!amt.native);
    return amt.iou;
}

template <>
inline
JBCAmount&
get<JBCAmount> (EitherAmount& amt)
{
    assert (amt.native);
    return amt.jbc;
}

template <class T>
T const&
get (EitherAmount const& amt)
{
    static_assert(sizeof(T) == -1, "Must used specialized function");
    return T(0);
}

template <>
inline
IOUAmount const&
get<IOUAmount> (EitherAmount const& amt)
{
    assert (!amt.native);
    return amt.iou;
}

template <>
inline
JBCAmount const&
get<JBCAmount> (EitherAmount const& amt)
{
    assert (amt.native);
    return amt.jbc;
}

inline
AmountSpec
toAmountSpec (STAmount const& amt)
{
    assert (amt.mantissa () < std::numeric_limits<std::int64_t>::max ());
    bool const isNeg = amt.negative ();
    std::int64_t const sMant =
        isNeg ? - std::int64_t (amt.mantissa ()) : amt.mantissa ();
    AmountSpec result;

    result.native = isJBC (amt);
    if (result.native)
    {
        result.jbc = JBCAmount (sMant);
    }
    else
    {
        result.iou = IOUAmount (sMant, amt.exponent ());
        result.issuer = amt.issue().account;
        result.currency = amt.issue().currency;
    }

    return result;
}

inline
EitherAmount
toEitherAmount (STAmount const& amt)
{
    if (isJBC (amt))
        return EitherAmount{amt.jbc()};
    return EitherAmount{amt.iou()};
}

inline
AmountSpec
toAmountSpec (
    EitherAmount const& ea,
    boost::optional<Currency> const& c)
{
    AmountSpec r;
    r.native = (!c || isJBC (*c));
    r.currency = c;
    assert (ea.native == r.native);
    if (r.native)
    {
        r.jbc = ea.jbc;
    }
    else
    {
        r.iou = ea.iou;
    }
    return r;
}

}

#endif
