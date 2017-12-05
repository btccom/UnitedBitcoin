UnionBitcoin integration/staging tree
=====================================

[![Build Status](https://travis-ci.org/bitcoin/bitcoin.svg?branch=master)](https://travis-ci.org/bitcoin/bitcoin)

http://u.ub.com

What is UnionBitcoin?
----------------

The purpose of creation of UnionBitcoin is to establish a smart credit currency system with smart contracts that can widely serve the society, based on the credit of zombie Bitcoins. UnionBitcoin’s credit is 100% based on Bitcoin with addition of credit foundation from several other high quality blockchains, and its gene is 100% inherited from Bitcoin. With the extensibility of smart contracts, the practical application value of UnionBitcoin will be greatly enhanced. UnionBitcoin is a super joint credit carrier with the target of establishing a powerful global network of smart value collaboration under the joint credit of Bitcoin and multiple other digital currencies. It is still Bitcoin, but exceeds Bitcoin.


For more information, as well as an immediately useable, binary version of
the Bitcoin Core software, see http://u.ub.com/en/download, or read the
[original whitepaper](https://bitcoincore.org/bitcoin.pdf).

License
-------

UnionBitcoin is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/bitcoin/bitcoin/tags) are created
regularly to indicate new official, stable release versions of Bitcoin Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://lists.linuxfoundation.org/mailman/listinfo/bitcoin-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer IRC can be found on Freenode at #bitcoin-core-dev.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Bitcoin Core's Transifex page](https://www.transifex.com/projects/p/bitcoin/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.

Translators should also subscribe to the [mailing list](https://groups.google.com/forum/#!forum/bitcoin-translators).
