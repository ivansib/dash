Sibcoin Core version 0.16.3.0
==========================

Release is now available from:

  <https://sibcoin.money/download>

This is a new major version release, bringing new features and other improvements.

Please report bugs using the issue tracker at github:

  <https://github.com/ivansib/sibcoin/issues>


Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Dash-Qt (on Mac) or
dashd/dash-qt (on Linux).

Downgrade warning
-----------------

### Downgrade to a version < 0.16.3.0

Because release 0.16.3.0 included the [per-UTXO fix](release-notes/dash/release-notes-0.12.2.2.md#per-utxo-fix)
which changed the structure of the internal database, you will have to reindex
the database if you decide to use any pre-0.16.3.0 version.

Wallet forward or backward compatibility was not affected.


Notable changes
===============

Fix crash bug with duplicate inputs within a transaction
--------------------------------------------------------

There was a critical bug discovered in Bitcoin Core's codebase recently which
can cause node receiving a block to crash https://github.com/bitcoin/bitcoin/pull/14247

Per-UTXO fix
------------

This fixes a potential vulnerability, so called 'Corebleed', which was
demonstrated this summer at the Вrеаkіng Віtсоіn Соnfеrеnсе іn Раrіs. The DoS
can cause nodes to allocate excessive amounts of memory, which leads them to a
halt. You can read more about the fix in the original Bitcoin Core pull request
https://github.com/bitcoin/bitcoin/pull/10195

To fix this issue in Dash Core however, we had to backport a lot of other
improvements from Bitcoin Core, see full list of backports in the detailed
change log below.

Additional indexes fix
----------------------

If you were using additional indexes like `addressindex`, `spentindex` or
`timestampindex` it's possible that they are not accurate. Please consider
reindexing the database by starting your node with `-reindex` command line
option. This is a one-time operation, the issue should be fixed now.

InstantSend fix
---------------

InstantSend should work with multisig addresses now.

PrivateSend fix
---------------

Some internal data structures were not cleared properly, which could lead
to a slightly higher memory consumption over a long period of time. This was
a minor issue which was not affecting mixing speed or user privacy in any way.

Removal of support for local masternodes
----------------------------------------

Keeping a wallet with 1000 DASH unlocked for 24/7 is definitely not a good idea
anymore. Because of this fact, it's also no longer reasonable to update and test
this feature, so it's completely removed now. If for some reason you were still
using it, please follow one of the guides and setup a remote masternode instead.

Python Support
--------------

Support for Python 2 has been discontinued for all test files and tools.

Credits
=======

Thanks to everyone who directly contributed to this release,
as well as everyone who submitted issues and reviewed pull requests.


Older releases
==============

Dash was previously known as Darkcoin.

Darkcoin tree 0.8.x was a fork of Litecoin tree 0.8, original name was XCoin
which was first released on Jan/18/2014.

Darkcoin tree 0.9.x was the open source implementation of masternodes based on
the 0.8.x tree and was first released on Mar/13/2014.

Darkcoin tree 0.10.x used to be the closed source implementation of Darksend
which was released open source on Sep/25/2014.

Dash Core tree 0.11.x was a fork of Bitcoin Core tree 0.9,
Darkcoin was rebranded to Dash.

Dash Core tree 0.12.0.x was a fork of Bitcoin Core tree 0.10.

Sibcoin Core tree 0.15.x was a fork of Dash Core tree 0.11.

Sibcoin Core tree 0.16.0.x was a fork of Dash Core tree 0.12.0.x.

Sibcoin Core tree 0.16.1.x was a fork of Dash Core tree 0.12.1.x.

Sibcoin Core tree 0.16.2.x was a fork of Dash Core tree 0.12.1.x with some critical updates from Dash Core 0.12.2.x.


- [v0.12.3.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.2.md) released Jul/09/2018
- [v0.12.3.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.1.md) released Jul/03/2018
- [v0.12.2.3](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.3.md) released Jan/12/2018
- [v0.12.2.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.2.md) released Dec/17/2017
- [v0.12.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.md) released Nov/08/2017
- [v0.12.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.1.md) released Feb/06/2017
- [v0.12.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.0.md) released Aug/15/2015
- [v0.11.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.2.md) released Mar/04/2015
- [v0.11.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.1.md) released Feb/10/2015
- [v0.11.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.0.md) released Jan/15/2015
- [v0.10.x](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.10.0.md) released Sep/25/2014
- [v0.9.x](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.9.0.md) released Mar/13/2014

