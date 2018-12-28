# The JBC Ledger

The JBC Ledger is a decentralized cryptographic ledger powered by a network of peer-to-peer servers. The JBC Ledger uses a novel Byzantine Fault Tolerant consensus algorithm to settle and record transactions in a secure distributed database without a central operator.



## `jbcoind`
The server software that powers the JBC Ledger is called `jbcoind` and is available in this repository under the permissive [ISC open-source license](LICENSE). The `jbcoind` server is written primarily in C++ and runs on a variety of platforms.




## Source Code
[![travis-ci.org: Build Status](https://travis-ci.org/jbcoin/jbcoind.png?branch=develop)](https://travis-ci.org/jbcoin/jbcoind)
[![codecov.io: Code Coverage](https://codecov.io/gh/jbcoin/jbcoind/branch/develop/graph/badge.svg)](https://codecov.io/gh/jbcoin/jbcoind)

### Repository Contents

| Folder     | Contents                                         |
|:-----------|:-------------------------------------------------|
| `./bin`    | Scripts and data files for JBCoin integrators.   |
| `./Builds` | Platform-specific guides for building `jbcoind`. |
| `./docs`   | Source documentation files and doxygen config.   |
| `./cfg`    | Example configuration files.                     |
| `./src`    | Source code.                                     |

Some of the directories under `src` are external repositories included using
git-subtree. See those directories' README files for more details.


## See Also


To learn about how JBCoin is transforming global payments, visit
<https://jbcoin.io>
