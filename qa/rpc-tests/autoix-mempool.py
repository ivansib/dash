#!/usr/bin/env python3
# Copyright (c) 2018 The Dash Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.mininode import *
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from time import *

'''
autoix-mempool.py

Checks if automatic InstantSend locks stop working when transaction mempool 
is full (more than 0.1 part from max value). 

'''

MASTERNODE_COLLATERAL = 1000
MAX_MEMPOOL_SIZE = 5 # max node mempool in MBs
MB_SIZE = 1000000 # C++ code use this coefficient to calc MB in mempool
AUTO_IX_MEM_THRESHOLD = 0.1


class MasternodeInfo:
    def __init__(self, key, collateral_id, collateral_out):
        self.key = key
        self.collateral_id = collateral_id
        self.collateral_out = collateral_out


class AutoIXMempoolTest(BitcoinTestFramework):
    def __init__(self):
        super().__init__()
        self.mn_count = 10
        self.num_nodes = self.mn_count + 3
        self.mninfo = []
        self.setup_clean_chain = True
        self.is_network_split = False
        # set sender,  receiver,  isolated nodes
        self.receiver_idx = self.num_nodes - 2
        self.sender_idx = self.num_nodes - 3
        # additional args
        self.extra_args = ["-maxmempool=%d" % MAX_MEMPOOL_SIZE]

    def create_simple_node(self):
        idx = len(self.nodes)
        args = ["-debug"] + self.extra_args
        self.nodes.append(start_node(idx, self.options.tmpdir,
                                     args))
        for i in range(0, idx):
            connect_nodes(self.nodes[i], idx)

    def get_mnconf_file(self):
        return os.path.join(self.options.tmpdir, "node0/regtest/masternode.conf")

    def prepare_masternodes(self):
        for idx in range(0, self.mn_count):
            key = self.nodes[0].masternode("genkey")
            address = self.nodes[0].getnewaddress()
            txid = self.nodes[0].sendtoaddress(address, MASTERNODE_COLLATERAL)
            txrow = self.nodes[0].getrawtransaction(txid, True)
            collateral_vout = 0
            for vout_idx in range(0, len(txrow["vout"])):
                vout = txrow["vout"][vout_idx]
                if vout["value"] == MASTERNODE_COLLATERAL:
                    collateral_vout = vout_idx
            self.nodes[0].lockunspent(False,
                                      [{"txid": txid, "vout": collateral_vout}])
            self.mninfo.append(MasternodeInfo(key, txid, collateral_vout))

    def write_mn_config(self):
        conf = self.get_mnconf_file()
        f = open(conf, 'a')
        for idx in range(0, self.mn_count):
            f.write("mn%d 127.0.0.1:%d %s %s %d\n" % (idx + 1, p2p_port(idx + 1),
                                                      self.mninfo[idx].key,
                                                      self.mninfo[idx].collateral_id,
                                                      self.mninfo[idx].collateral_out))
        f.close()

    def create_masternodes(self):
        for idx in range(0, self.mn_count):
            args = ['-debug=masternode', '-externalip=127.0.0.1', '-masternode=1',
                    '-masternodeprivkey=%s' % self.mninfo[idx].key] + self.extra_args
            self.nodes.append(start_node(idx + 1, self.options.tmpdir, args))
            for i in range(0, idx + 1):
                connect_nodes(self.nodes[i], idx + 1)

    def setup_network(self):
        self.nodes = []
        # create faucet node for collateral and transactions
        args = ["-debug"] + self.extra_args
        self.nodes.append(start_node(0, self.options.tmpdir, args))
        required_balance = MASTERNODE_COLLATERAL * self.mn_count + 1
        while self.nodes[0].getbalance() < required_balance:
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
        # create masternodes
        self.prepare_masternodes()
        self.write_mn_config()
        stop_node(self.nodes[0], 0)
        args = ["-debug",
                "-sporkkey=cP4EKFyJsHT39LDqgdcB43Y3YXjNyjb5Fuas1GQSeAtjnZWmZEQK"] + \
               self.extra_args
        self.nodes[0] = start_node(0, self.options.tmpdir,
                                   args)
        self.create_masternodes()
        # create connected simple nodes
        for i in range(0, self.num_nodes - self.mn_count - 1):
            self.create_simple_node()
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(1)
        # sync nodes
        self.sync_all()
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        sync_masternodes(self.nodes)
        for i in range(1, self.mn_count + 1):
            res = self.nodes[0].masternode("start-alias", "mn%d" % i)
            assert(res["result"] == 'successful')
        sync_masternodes(self.nodes)
        mn_info = self.nodes[0].masternodelist("status")
        assert(len(mn_info) == self.mn_count)
        for status in mn_info.values():
            assert(status == 'ENABLED')

    def get_autoix_bip9_status(self):
        info = self.nodes[0].getblockchaininfo()
        return info['bip9_softforks']['autoix']['status']

    def activate_autoix_bip9(self):
        # sync nodes periodically
        # if we sync them too often, activation takes too many time
        # if we sync them too rarely, nodes failed to update its state and
        # bip9 status is not updated
        # so, in this code nodes are synced once per 20 blocks
        counter = 0
        sync_period = 10

        while self.get_autoix_bip9_status() == 'defined':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()
                sync_masternodes(self.nodes)

        while self.get_autoix_bip9_status() == 'started':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()
                sync_masternodes(self.nodes)

        while self.get_autoix_bip9_status() == 'locked_in':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()
                sync_masternodes(self.nodes)

        # sync nodes
        self.sync_all()
        sync_masternodes(self.nodes)

        assert(self.get_autoix_bip9_status() == 'active')

    def get_autoix_spork_state(self):
        info = self.nodes[0].spork('active')
        return info['SPORK_16_INSTANTSEND_AUTOLOCKS']

    def set_autoix_spork_state(self, state):
        if state:
            value = 0
        else:
            value = 4070908800
        self.nodes[0].spork('SPORK_16_INSTANTSEND_AUTOLOCKS', value)

    def enforce_masternode_payments(self):
        self.nodes[0].spork('SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT', 0)

    def create_raw_trx(self, node_from, node_to, amount, min_inputs, max_inputs):
        assert(min_inputs <= max_inputs)
        # fill inputs
        inputs=[]
        balances = node_from.listunspent()
        in_amount = 0.0
        last_amount = 0.0
        for tx in balances:
            if len(inputs) < min_inputs:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount += float(tx['amount'])
                inputs.append(input)
            elif in_amount > amount:
                break
            elif len(inputs) < max_inputs:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount += float(tx['amount'])
                inputs.append(input)
            else:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount -= last_amount
                in_amount += float(tx['amount'])
                inputs[-1] = input
            last_amount = float(tx['amount'])

        assert(len(inputs) > 0)
        assert(in_amount > amount)
        # fill outputs
        receiver_address = node_to.getnewaddress()
        change_address = node_from.getnewaddress()
        fee = 0.001
        outputs={}
        outputs[receiver_address] = satoshi_round(amount)
        outputs[change_address] = satoshi_round(in_amount - amount - fee)
        rawtx = node_from.createrawtransaction(inputs, outputs)
        return node_from.signrawtransaction(rawtx)

    def check_IX_lock(self, txid, node):
        # wait for instantsend locks
        start = time()
        locked = False
        while True:
            is_trx = node.gettransaction(txid)
            if is_trx['instantlock']:
                locked = True
                break
            if time() > start + 10:
                break
            sleep(0.1)
        return locked

    # sends regular IX with high fee and may inputs (not-simple transaction)
    def send_regular_IX(self, sender, receiver):
        receiver_addr = receiver.getnewaddress()
        txid = sender.instantsendtoaddress(receiver_addr, 1.0)
        return self.check_IX_lock(txid, sender)

    # sends simple trx, it should become IX if autolocks are allowed
    def send_simple_tx(self, sender, receiver):
        raw_tx = self.create_raw_trx(sender, receiver, 1.0, 1, 4)
        txid = self.nodes[0].sendrawtransaction(raw_tx['hex'])
        self.sync_all()
        return self.check_IX_lock(txid, sender)

    def get_mempool_size(self, node):
        info = node.getmempoolinfo()
        return info['usage']

    def fill_mempool(self):
        node = self.nodes[0]
        rec_address = node.getnewaddress()
        while self.get_mempool_size(node) < MAX_MEMPOOL_SIZE * MB_SIZE * AUTO_IX_MEM_THRESHOLD + 10000:
            node.sendtoaddress(rec_address, 1.0)
            sleep(0.1)
        self.sync_all()

    def run_test(self):
        self.enforce_masternode_payments()  # required for bip9 activation
        self.activate_autoix_bip9()
        self.set_autoix_spork_state(True)

        # check pre-conditions for autoIX
        assert(self.get_autoix_bip9_status() == 'active')
        assert(self.get_autoix_spork_state())

        # autoIX is working
        assert(self.send_simple_tx(self.nodes[0], self.nodes[self.receiver_idx]))

        # send funds for InstantSend  after filling mempool and give them 6 confirmations
        rec_address = self.nodes[self.receiver_idx].getnewaddress()
        self.nodes[0].sendtoaddress(rec_address, 500.0)
        self.nodes[0].sendtoaddress(rec_address, 500.0)
        self.sync_all()
        for i in range(0, 2):
            self.nodes[self.receiver_idx].generate(1)
        self.sync_all()

        # fill mempool with transactions
        self.fill_mempool()

        # autoIX is not working now
        assert(not self.send_simple_tx(self.nodes[self.receiver_idx], self.nodes[0]))
        # regular IX is still working
        assert(self.send_regular_IX(self.nodes[self.receiver_idx], self.nodes[0]))


if __name__ == '__main__':
    AutoIXMempoolTest().main()
