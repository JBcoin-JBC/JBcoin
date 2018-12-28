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

#ifndef JBCOIN_PROTOCOL_SECRETKEY_H_INCLUDED
#define JBCOIN_PROTOCOL_SECRETKEY_H_INCLUDED

#include <jbcoin/basics/Buffer.h>
#include <jbcoin/basics/Slice.h>
#include <jbcoin/crypto/KeyType.h> // move to protocol/
#include <jbcoin/protocol/PublicKey.h>
#include <jbcoin/protocol/Seed.h>
#include <jbcoin/protocol/tokens.h>
#include <array>
#include <string>

namespace jbcoin {

/** A secret key. */
class SecretKey
{
private:
    std::uint8_t buf_[32];

public:
    SecretKey() = default;
    SecretKey (SecretKey const&) = default;
    SecretKey& operator= (SecretKey const&) = default;

    ~SecretKey();

    SecretKey (std::array<std::uint8_t, 32> const& data);
    SecretKey (Slice const& slice);

    std::uint8_t const*
    data() const
    {
        return buf_;
    }

    std::size_t
    size() const
    {
        return sizeof(buf_);
    }

    /** Convert the secret key to a hexadecimal string.

        @note The operator<< function is deliberately omitted
        to avoid accidental exposure of secret key material.
    */
    std::string
    to_string() const;
};

inline
bool
operator== (SecretKey const& lhs,
    SecretKey const& rhs)
{
    return lhs.size() == rhs.size() &&
        std::memcmp(lhs.data(),
            rhs.data(), rhs.size()) == 0;
}

inline
bool
operator!= (SecretKey const& lhs,
    SecretKey const& rhs)
{
    return ! (lhs == rhs);
}

//------------------------------------------------------------------------------

/** Parse a secret key */
template <>
boost::optional<SecretKey>
parseBase58 (TokenType type, std::string const& s);

inline
std::string
toBase58 (TokenType type, SecretKey const& sk)
{
    return base58EncodeToken(
        type, sk.data(), sk.size());
}

/** Create a secret key using secure random numbers. */
SecretKey
randomSecretKey();

/** Generate a new secret key deterministically. */
SecretKey
generateSecretKey (KeyType type, Seed const& seed);

/** Derive the public key from a secret key. */
PublicKey
derivePublicKey (KeyType type, SecretKey const& sk);

/** Generate a key pair deterministically.

    This algorithm is specific to JBCoin:

    For secp256k1 key pairs, the seed is converted
    to a Generator and used to compute the key pair
    corresponding to ordinal 0 for the generator.
*/
std::pair<PublicKey, SecretKey>
generateKeyPair (KeyType type, Seed const& seed);

/** Create a key pair using secure random numbers. */
std::pair<PublicKey, SecretKey>
randomKeyPair (KeyType type);

/** Generate a signature for a message digest.
    This can only be used with secp256k1 since Ed25519's
    security properties come, in part, from how the message
    is hashed.
*/
/** @{ */
Buffer
signDigest (PublicKey const& pk, SecretKey const& sk,
    uint256 const& digest);

inline
Buffer
signDigest (KeyType type, SecretKey const& sk,
    uint256 const& digest)
{
    return signDigest (derivePublicKey(type, sk), sk, digest);
}
/** @} */

/** Generate a signature for a message.
    With secp256k1 signatures, the data is first hashed with
    SHA512-Half, and the resulting digest is signed.
*/
/** @{ */
Buffer
sign (PublicKey const& pk,
    SecretKey const& sk, Slice const& message);

inline
Buffer
sign (KeyType type, SecretKey const& sk,
    Slice const& message)
{
    return sign (derivePublicKey(type, sk), sk, message);
}
/** @} */

} // jbcoin

#endif
