Sibcoin Core version 0.17.0.0
==========================

Release is now available from:

  <https://www.sibcoin.org/downloads/#wallets>

This is a new major version release, bringing new features, various bugfixes and other improvements.

Please report bugs using the issue tracker at github:

  <https://github.com/ivansib/sibcoin/issues>


Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Sibcoin-Qt (on Mac) or
sibcoind/sibcoin-qt (on Linux). If you upgrade after DIP0003 activation you will
have to reindex (start with -reindex-chainstate or -reindex) to make sure
your wallet has all the new data synced.

### Providing "masternodeblsprivkey" is mandatory when the node is launched as a masternode

Even though DIP3 is not activated yet, we decided to make "masternodeblsprivkey" 
mandatory when configuring and running a masternode ("masternode=1"). Please note that your masternode
will fail to start when "masternodeblsprivkey" is not specified. 

This enforcement was added to catch misconfigurations of masternodes which would otherwise 
stay unnoticed until spork 16 activation and thus surprise and hurt masternode owners.


Downgrade warning
-----------------

### Downgrade to a version < 0.17.0.0

Downgrading to a version smaller than 0.17 is only supported as long as DIP2/DIP3
has not been activated. Activation will happen when enough miners signal compatibility
through a BIP9 (bit 3) deployment.

Notable changes
===============

DIP0002 - Special Transactions
------------------------------
Currently, new features and consensus mechanisms have to be implemented on top of the restrictions
imposed by the simple nature of transactions. Since classical transactions can only carry inputs
and outputs, they are most useful for financial transactions (i.e. transfers of quantities of Sibcoin
between addresses on the distributed ledger). These inputs and outputs carry scripts and signatures
which are used to authorize and validate the transaction.

To implement new on-chain features and consensus mechanisms which do not fit into this concept of
financial transactions, it is often necessary to misuse the inputs/outputs and their corresponding
scripts to add additional data and meaning to a transaction. For example, new opcodes would have
to be introduced to mark a transaction as something special and add a payload. In other cases,
OP_RETURN has been misused to store data on-chain.

The introduction of special transactions will require the whole Sibcoin ecosystem to perform a one-time
mandatory update of all the software and libraries involved. Software and libraries will have to be
changed so that they can differentiate between classical transactions and special transactions.
Deserialization of a classical transaction remains unchanged. Deserialization of a special transaction
requires the software/library to at least implement skipping and ignoring the extra_payload field.
Validation and processing of the inputs and outputs of a special transaction must remain identical to
classical transactions.

Read more: https://github.com/dashpay/dips/blob/master/dip-0002.md

DIP0003 - Deterministic Masternode Lists
----------------------------------------
This DIP provides on-chain consensus for masternode lists that in turn allow for deterministic quorum
derivation and service scoring of masternode rewards.

In the previous system, each node maintained its own individual masternode list. Masternodes gained
entry to that masternode list after the owner created a 1000 Sibcoin UTXO and the masternode broadcast
a "masternode broadcast/announcement" P2P message. This in turn set the masternode to a PRE_ENABLED
state in the list maintained by each node. Masternodes then regularly broadcasted ping messages to
keep the masternode in ENABLED state.

The previous system was maintained with consensus mechanisms that predated Satoshi Nakamoto’s solution
to the Byzantine Generals Problem. This meant that each node needed to maintain their own individual
masternode list with P2P messages and not a blockchain based solution. Due to the nature of the P2P
system, there was no guarantee that nodes would come to the same conclusion on what the masternode
list ought to look like. Discrepancies might, for example, occur due to a different order of message
reception or if messages had not been received at all. This posed some risks in regard to consensus
and limited the possible uses of quorums by the system.

Additionally, the previous system required a complicated and failure prone "masternode sync" after
the initial startup of the node. After the blockchain was synced, the node would request the current
masternode list, the reward payment votes, and then verify the received list. This process tended to
take an unnecessarily long amount of time and sometimes resulted in failure.

In the new system, the masternode list is derived entirely from information found on-chain. New
masternodes are added by new special transactions called Provider Registration Transactions
(abbreviated as ProRegTx). They are only removed by spending the collateral. A ProRegTx is a special
transaction which includes either a 4000-SIB collateral payment or a reference to it, along with
other payload information (DIP0002).

The new system is going to be activated via combination of a BIP9-like deployment (bit 3) and new spork
(`SPORK_16_DETERMINISTIC_MNS_ENABLED`).

Read more: https://github.com/dashpay/dips/blob/master/dip-0003.md
Upgrade instructions: https://docs.dash.org/DIP3-masternode-upgrade

DIP0004 - Simplified Verification of Deterministic Masternode Lists
-------------------------------------------------------------------
A verifiable and correct masternode list is foundational to many Sibcoin features, including verification
of an InstantSend transaction, mixing in PrivateSend and many features of Evolution. The deterministic
masternode lists introduced by DIP0003 enable full derivation and verification of a masternode list via
on-chain data. This, however, requires the full chain to be available to construct or verify this list.
A SPV client does not have the full chain and thus would have to rely on a list provided by one or more
nodes in the network. This provided list must be verifiable by the SPV client without needing the full
chain. This DIP proposes additions to the block’s coinbase transaction and new P2P messages to get and
update a masternode list with additional proof data.

Read more: https://github.com/dashpay/dips/blob/master/dip-0004.md

Mining
------
Please note that masternode payments in `getblocktemplate` rpc are now returned as an array and not as
a single object anymore. Make sure to apply corresponding changes to your pool software.

Also, deterministic masternodes can now set their payout address to a P2SH address. The most common use
case for P2SH is multisig but script can be pretty much anything. If your pool software doesn't recognize
P2SH addresses, the simplest way to fix it is to use `script` field which shows scriptPubKey for each
entry of masternode payments array in `getblocktemplate`.

And finally, after DIP0003 activation your pool software must be able to produce Coinbase Special
Transaction https://github.com/dashpay/dips/blob/master/dip-0004.md#coinbase-special-transaction.
Use `coinbase_payload` from `getblocktemplate` to get extra payload needed to construct this transaction.

PrivateSend
-----------
With further refactoring of PrivateSend code it became possible to implement mixing in few parallel
mixing sessions at once from one single wallet. You can set number of mixing sessions via
`privatesendsessions` cmd-line option or sibcoin.conf. You can pick any number of sessions between 1 and 10,
default is 4 which should be good enough for most users. For this feature to work you should also make
sure that `privatesendmultisession` is set to `1` via cmd-line or `Enable PrivateSend multi-session` is
enabled in GUI.

You can also adjust rounds and amount via `setprivatesendrounds` and `setprivatesendamount` RPC commands
which override corresponding cmd-line params (`privatesendrounds` and `privatesendamount` respectively).

NOTE: Introducing new changes made it incompatible with mixing on
masternodes running on pre-0.17 software. Please keep using 0.16.4 local wallet to mix your coins until
there is some significant number of masternodes running on version 0.17 to make sure you have enough
masternodes to choose from when the wallet picks one to mix funds on.

InstantSend
-----------
With further improvements of networking code it's now possible to handle more load, so we are changing
InstantSend to be always-on for so called "simple txes" - transactions with 4 or less inputs. Such
transactions will be automatically locked even if they only pay minimal fee. According to stats, this
means that up to 90% of currently observed transactions will became automatically locked via InstantSend
with no additional cost to end users or any additional effort from wallet developers or other service
providers.

This feature is going to be activated via combination of a BIP9-like deployment (we are reusing bit 3)
and new spork (`SPORK_17_INSTANTSEND_AUTOLOCKS`).

Historically, InstantSend transactions were shown in GUI and RPC with more confirmations than regular ones,
which caused quite a bit of confusion. This will no longer be the case, instead we are going to show real
blockchain confirmations only and a separate indicator to show if transaction was locked via InstantSend
or not. For GUI it's color highlight and a new column, for RPC commands - `instantlock` field and `addlocked`
param.

One of the issues with InstantSend adoption by SPV wallets (besides lack of Deterministic Masternode List)
was inability to filter all InstantSend messages the same way transactions are filtered. This should be
fixed now and SPV wallets should only get lock votes for transactions they are interested in.

Another popular request was to preserve information about InstantSend locks between wallet restarts, which
is now implemented. This data is stored in a new cache file `instantsend.dat`. You can safely remove it,
if you don't need information about recent transaction locks for some reason (NOTE: make sure it's not one
of your wallets!).

We also added new ZMQ notifications for double-spend attempts which try to override transactions locked
via InstantSend - `zmqpubrawinstantsenddoublespend` and `zmqpubhashinstantsenddoublespend`.

Sporks
------
There are a couple of new sporks introduced in this version `SPORK_16_DETERMINISTIC_MNS_ENABLED` (block
based) and `SPORK_17_INSTANTSEND_AUTOLOCKS` (timestamp based). There is aslo `SPORK_18_QUORUM_DKG_ENABLED`
(timestamp based) which is going to be used on testnet only for now.

Spork data is stored in a new cache file (`sporks.dat`) now.

Governance
----------
Introduction of Deterministic Masternodes requires replacing of the old masternode private key which was used
both for operating a MN and for voting on proposals with a set of separate keys, preferably fresh new ones.
This means that votes casted for proposals by Masternode Owners via the old system will no longer be valid
after DIP0003 activation and must be re-casted using the new voting key.

Also, you can now get notifications about governance objects or votes via new ZMQ notifications:
`zmqpubhashgovernancevote`, `zmqpubhashgovernanceobject`, `zmqpubrawgovernancevote` and
`zmqpubhashgovernanceobject`.

GUI changes
-----------
Masternodes tab has a new section dedicated to DIP0003 registered masternodes now. After DIP0003 activation
this will be the only section shown here, the two old sections for non-deterministic masternodes will no
longer be available.

There are changes in the way InstantSend transactions are displayed, see `InstantSend` section above.

Some other (mostly minor) issues were also fixed, see `GUI` part of `0.17.0.0 Change log` section below for
detailed list of fixes.

RPC changes
-----------
There are a few changes in existing RPC interfaces in this release:
- `gobject prepare` allows to send proposal transaction as an InstantSend one and also accepts an UTXO reference to spend;
- `masternode status` and `masternode list` show some DIP0003 related info now;
- `previousbits` and `coinbase_payload` fields were added in `getblocktemplate`;
- `getblocktemplate` now returns an array for masternode payments instead of a single object (miners and mining pools have to upgrade their software to support multiple masternode payees);
- masternode and superblock payments in `getblocktemplate` show payee scriptPubKey in `script` field in addition to payee address in `payee`;
- `getblockchaininfo` shows BIP9 deployment progress;
- `help command subCommand` should give detailed help for subcommands e.g. `help protx list`;
- `compressed` option in `masternode genkey`;
- `dumpwallet` shows info about dumped wallet and warns user about security issues;
- `instantlock` field added in output of `getrawmempool`, `getmempoolancestors`, `getmempooldescendants`, `getmempoolentry`,
`getrawtransaction`, `decoderawtransaction`, `gettransaction`, `listtransactions`, `listsinceblock`;
- `addlocked` param added to `getreceivedbyaddress`, `getreceivedbyaccount`, `getbalance`, `sendfrom`, `sendmany`,
`listreceivedbyaddress`, `listreceivedbyaccount`, `listaccounts`.

There are also new RPC commands:
- `protx` (`list`, `info`, `diff`, `register`, `register_fund`, `register_prepare`,
`register_submit`, `update_service`, `update_registrar`, `revoke`);
- `bls generate`;
- `setprivatesendrounds`;
- `setprivatesendamount`;
- `getspecialtxes`.

See `help command` in rpc for more info.

Command-line options
--------------------

New cmd-line options:
- `masternodeblsprivkey`;
- `minsporkkeys`;
- `privatesendsessions`;
- `zmqpubrawinstantsenddoublespend`;
- `zmqpubhashinstantsenddoublespend`;
- `zmqpubhashgovernancevote`;
- `zmqpubhashgovernanceobject`;
- `zmqpubrawgovernancevote`;
- `zmqpubhashgovernanceobject`.

Some of them are Devnet only:
- `budgetparams`;
- `minimumdifficultyblocks`;
- `highsubsidyblocks`;
- `highsubsidyfactor`.

Few cmd-line options are no longer supported:
- `instantsenddepth`;
- `mempoolreplacement`.

See `Help -> Command-line options` in Qt wallet or `sibcoind --help` for more info.

Lots of refactoring and bug fixes
---------------------------------

A lot of refactoring, code cleanups and other small fixes were done in this release.

0.17.0.0 Change log
===================

See detailed [set of changes](https://github.com/ivansib/sibcoin/compare/v0.16.4.0...ivansib:v0.17.0.0).

Credits
=======

Thanks to everyone who directly contributed to this release:

- ivansib
- TriKriSta
- Alexander Block
- UdjinM6
- PastaPastaPasta
- gladcow
- Nathan Marley
- thephez
- strophy
- PaulieD
- InhumanPerfection
- Spencer Lievens
- -k
- Salisbury
- Solar Designer
- Oleg Girko
- Anton Suprunchuk

As well as everyone that submitted issues, reviewed pull requests or helped translating on
[Transifex](https://www.transifex.com/projects/p/sibcoin/).

Older releases
==============

Sibcoin Core tree 0.15.x was a fork of Dash Core tree 0.11,

Sibcoin Core tree 0.16.0.x was a fork of Dash Core tree 0.12.0.x.

Sibcoin Core tree 0.16.1.x was a fork of Dash Core tree 0.12.1.x.

Sibcoin Core tree 0.16.2.x was a fork of Dash Core tree 0.12.2.x.

These releases are considered obsolete. 