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

#ifndef JBCOIN_RPC_JSON_BODY_H
#define JBCOIN_RPC_JSON_BODY_H

#include <jbcoin/json/json_value.h>
#include <jbcoin/json/to_string.h>

#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http/message.hpp>

namespace jbcoin {

/// Body that holds JSON
struct json_body
{
    explicit json_body() = default;

    using value_type = Json::Value;

    class reader
    {
        using dynamic_buffer_type = boost::beast::multi_buffer;
            
        dynamic_buffer_type buffer_;

    public:
        using const_buffers_type =
            typename dynamic_buffer_type::const_buffers_type;

        using is_deferred = std::false_type;

        template<bool isRequest, class Fields>
        explicit
        reader(boost::beast::http::message<
            isRequest, json_body, Fields> const& m)
        {
            stream(m.body,
                [&](void const* data, std::size_t n)
                {
                    buffer_.commit(boost::asio::buffer_copy(
                        buffer_.prepare(n), boost::asio::buffer(data, n)));
                });
        }

        void
        init(boost::beast::error_code&) noexcept
        {
        }

        boost::optional<std::pair<const_buffers_type, bool>>
        get(boost::beast::error_code& ec)
        {
            return {{buffer_.data(), false}};
        }

        void
        finish(boost::beast::error_code&)
        {
        }
    };

    class writer
    {
        std::string body_string_;

    public:
        using const_buffers_type =
            boost::asio::const_buffer;

        template<bool isRequest, class Fields>
        explicit
        writer(boost::beast::http::message<isRequest,
                json_body, Fields> const& msg)
                : body_string_(to_string(msg.body()))
        {
        }

        void
        init(boost::beast::error_code& ec)
        {
            ec.assign(0, ec.category());
        }

        boost::optional<std::pair<const_buffers_type, bool>>
        get(boost::beast::error_code& ec)
        {
            ec.assign(0, ec.category());
            return {{const_buffers_type{
                body_string_.data(), body_string_.size()}, false}};
        }
    };
};

} // jbcoin

#endif