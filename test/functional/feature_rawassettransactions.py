#!/usr/bin/env python3
# Copyright (c) 2018 The Bitcoin Core developers
# Copyright (c) 2017 The Raven Core developers
# Copyright (c) 2018 The Rito Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the rawtransaction RPCs for asset transactions.
"""
from io import BytesIO
from pprint import *
from test_framework.test_framework import RitoTestFramework
from test_framework.util import *
from test_framework.mininode import *


def get_tx_issue_hex(node, asset_name, asset_quantity, asset_units=0):
    to_address = node.getnewaddress()
    change_address = node.getnewaddress()
    unspent = node.listunspent()[0]
    inputs = [{k: unspent[k] for k in ['txid', 'vout']}]
    outputs = {
        'n1issueAssetXXXXXXXXXXXXXXXXWdnemQ': 500,
        change_address: float(unspent['amount']) - 500.0001,
        to_address: {
            'issue': {
                'asset_name':       asset_name,
                'asset_quantity':   asset_quantity,
                'units':            asset_units,
                'reissuable':       1,
                'has_ipfs':         0,
            }
        }
    }
    tx_issue = node.createrawtransaction(inputs, outputs)
    tx_issue_signed = node.signrawtransaction(tx_issue)
    tx_issue_hex = tx_issue_signed['hex']
    return tx_issue_hex


class RawAssetTransactionsTest(RitoTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 3

    def activate_assets(self):
        self.log.info("Generating RITO for node[0] and activating assets...")
        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        n0.generate(1)
        self.sync_all()
        n0.generate(431)
        self.sync_all()
        assert_equal("active", n0.getblockchaininfo()['bip9_softforks']['assets']['status'])

    def reissue_tampering_test(self):
        self.log.info("Tampering with raw reissues...")

        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        ########################################
        # issue a couple of assets
        asset_name = 'REISSUE_TAMPERING'
        owner_name = f"{asset_name}!"
        alternate_asset_name = 'ANOTHER_ASSET'
        alternate_owner_name = f"{alternate_asset_name}!"
        n0.sendrawtransaction(get_tx_issue_hex(n0, asset_name, 1000))
        n0.sendrawtransaction(get_tx_issue_hex(n0, alternate_asset_name, 1000))
        n0.generate(1)

        ########################################
        # try a reissue with no owner input
        to_address = n0.getnewaddress()
        unspent = n0.listunspent()[0]
        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            'n1ReissueAssetXXXXXXXXXXXXXXWG9NLd': 100,
            n0.getnewaddress(): float(unspent['amount']) - 100.0001,
            to_address: {
                'reissue': {
                    'asset_name':       asset_name,
                    'asset_quantity':   1000,
                }
            }
        }
        tx_hex = n0.createrawtransaction(inputs, outputs)
        tx_signed = n0.signrawtransaction(tx_hex)['hex']
        assert_raises_rpc_error(-26, "bad-tx-inputs-outputs-mismatch Bad Transaction - " +
                                f"Trying to create outpoint for asset that you don't have: {owner_name}",
                                n0.sendrawtransaction, tx_signed)

        ########################################
        # try a reissue where the owner input doesn't match the asset name
        unspent_asset_owner = n0.listmyassets(alternate_owner_name, True)[alternate_owner_name]['outpoints'][0]
        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]
        tx_hex = n0.createrawtransaction(inputs, outputs)
        tx_signed = n0.signrawtransaction(tx_hex)['hex']
        assert_raises_rpc_error(-26, "bad-tx-inputs-outputs-mismatch Bad Transaction - " +
                                f"Trying to create outpoint for asset that you don't have: {owner_name}",
                                n0.sendrawtransaction, tx_signed)

        ########################################
        # fix it to use the right input
        unspent_asset_owner = n0.listmyassets(owner_name, True)[owner_name]['outpoints'][0]
        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]
        tx_hex = n0.createrawtransaction(inputs, outputs)

        ########################################
        # try tampering to change the name of the asset being issued
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnr = '72766e72' #rvnr
        op_drop = '75'
        for n in range(0, len(tx.vout)):
            out = tx.vout[n]
            if rvnr in bytes_to_hex_str(out.scriptPubKey):
                script_hex = bytes_to_hex_str(out.scriptPubKey)
                reissue_script_hex = script_hex[script_hex.index(rvnr) + len(rvnr):-len(op_drop)]
                f = BytesIO(hex_str_to_bytes(reissue_script_hex))
                reissue = CScriptReissue()
                reissue.deserialize(f)
                reissue.name = alternate_asset_name.encode()
                tampered_reissue = bytes_to_hex_str(reissue.serialize())
                tampered_script = script_hex[:script_hex.index(rvnr)] + rvnr + tampered_reissue + op_drop
                tx.vout[n].scriptPubKey = hex_str_to_bytes(tampered_script)
        tx_hex_bad = bytes_to_hex_str(tx.serialize())
        tx_signed = n0.signrawtransaction(tx_hex_bad)['hex']
        assert_raises_rpc_error(-26, "bad-txns-reissue-owner-outpoint-not-found", n0.sendrawtransaction, tx_signed)

        ########################################
        # try tampering to remove owner output
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnt = '72766e74' #rvnt
        # remove the owner output from vout
        bad_vout = list(filter(lambda out : rvnt not in bytes_to_hex_str(out.scriptPubKey), tx.vout))
        tx.vout = bad_vout
        tx_hex_bad = bytes_to_hex_str(tx.serialize())
        tx_signed = n0.signrawtransaction(tx_hex_bad)['hex']
        assert_raises_rpc_error(-26, "bad-txns-reissue-owner-outpoint-not-found",
                                n0.sendrawtransaction, tx_signed)

        ########################################
        # try tampering to remove asset output...
        # ...this is actually ok, just an awkward donation to reissue burn address!

        ########################################
        # reissue!
        tx_signed = n0.signrawtransaction(tx_hex)['hex']
        tx_hash = n0.sendrawtransaction(tx_signed)
        assert_is_hash_string(tx_hash)
        n0.generate(1)
        assert_equal(2000, n0.listmyassets(asset_name)[asset_name])

    def issue_tampering_test(self):
        self.log.info("Tampering with raw issues...")

        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        ########################################
        # get issue tx
        asset_name = 'TAMPER_TEST_ASSET'
        tx_issue_hex = get_tx_issue_hex(n0, asset_name, 1000)

        ########################################
        # try tampering to issue an asset with no owner output
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_issue_hex))
        tx.deserialize(f)
        rvno = '72766e6f' #rvno
        # remove the owner output from vout
        bad_vout = list(filter(lambda out : rvno not in bytes_to_hex_str(out.scriptPubKey), tx.vout))
        tx.vout = bad_vout
        tx_bad_issue = bytes_to_hex_str(tx.serialize())
        tx_bad_issue_signed = n0.signrawtransaction(tx_bad_issue)['hex']
        assert_raises_rpc_error(-26, "bad-txns-bad-asset-transaction",
                                n0.sendrawtransaction, tx_bad_issue_signed)

        ########################################
        # try tampering to issue an asset with duplicate owner outputs
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_issue_hex))
        tx.deserialize(f)
        rvno = '72766e6f' #rvno
        # find the owner output from vout and insert a duplicate back in
        owner_vout = list(filter(lambda out : rvno in bytes_to_hex_str(out.scriptPubKey), tx.vout))[0]
        tx.vout.insert(-1, owner_vout)
        tx_bad_issue = bytes_to_hex_str(tx.serialize())
        tx_bad_issue_signed = n0.signrawtransaction(tx_bad_issue)['hex']
        assert_raises_rpc_error(-26, "bad-txns-failed-issue-asset-formatting-check",
                                n0.sendrawtransaction, tx_bad_issue_signed)

        ########################################
        # try tampering to issue an owner token with no asset
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_issue_hex))
        tx.deserialize(f)
        rvnq = '72766e71' #rvnq
        # remove the owner output from vout
        bad_vout = list(filter(lambda out : rvnq not in bytes_to_hex_str(out.scriptPubKey), tx.vout))
        tx.vout = bad_vout
        tx_bad_issue = bytes_to_hex_str(tx.serialize())
        tx_bad_issue_signed = n0.signrawtransaction(tx_bad_issue)['hex']
        assert_raises_rpc_error(-26, "bad-txns-bad-asset-transaction",
                                n0.sendrawtransaction, tx_bad_issue_signed)

        ########################################
        # try tampering to issue a mismatched owner/asset
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_issue_hex))
        tx.deserialize(f)
        rvno = '72766e6f' #rvno
        op_drop = '75'
        # change the owner name
        for n in range(0, len(tx.vout)):
            out = tx.vout[n]
            if rvno in bytes_to_hex_str(out.scriptPubKey):
                owner_out = out
                owner_script_hex = bytes_to_hex_str(owner_out.scriptPubKey)
                asset_script_hex = owner_script_hex[owner_script_hex.index(rvno) + len(rvno):-len(op_drop)]
                f = BytesIO(hex_str_to_bytes(asset_script_hex))
                owner = CScriptOwner()
                owner.deserialize(f)
                owner.name = b"NOT_MY_ASSET!"
                tampered_owner = bytes_to_hex_str(owner.serialize())
                tampered_script = owner_script_hex[:owner_script_hex.index(rvno)] + rvno + tampered_owner + op_drop
                tx.vout[n].scriptPubKey = hex_str_to_bytes(tampered_script)
        tx_bad_issue = bytes_to_hex_str(tx.serialize())
        tx_bad_issue_signed = n0.signrawtransaction(tx_bad_issue)['hex']
        assert_raises_rpc_error(-26, "bad-txns-issue-owner-name-doesn't-match",
                                n0.sendrawtransaction, tx_bad_issue_signed)

        ########################################
        # try tampering to make owner output script invalid
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_issue_hex))
        tx.deserialize(f)
        rvno = '72766e6f' #rvno
        RITOO = '52564e4f' #RITOO
        # change the owner output script type to be invalid
        for n in range(0, len(tx.vout)):
            out = tx.vout[n]
            if rvno in bytes_to_hex_str(out.scriptPubKey):
                owner_script_hex = bytes_to_hex_str(out.scriptPubKey)
                tampered_script = owner_script_hex.replace(rvno, RITOO)
                tx.vout[n].scriptPubKey = hex_str_to_bytes(tampered_script)
        tx_bad_issue = bytes_to_hex_str(tx.serialize())
        tx_bad_issue_signed = n0.signrawtransaction(tx_bad_issue)['hex']
        assert_raises_rpc_error(-26, "bad-txns-bad-asset-script",
                                n0.sendrawtransaction, tx_bad_issue_signed)

        ########################################
        # try to generate and issue an asset that already exists
        tx_duplicate_issue_hex = get_tx_issue_hex(n0, asset_name, 42)
        n0.sendrawtransaction(tx_issue_hex)
        n0.generate(1)
        assert_raises_rpc_error(-8, f"Invalid parameter: asset_name '{asset_name}' has already been used",
                                get_tx_issue_hex, n0, asset_name, 55)
        assert_raises_rpc_error(-26, f"bad-txns-issue-Invalid parameter: asset_name '{asset_name}' has already been used",
                                n0.sendrawtransaction, tx_duplicate_issue_hex)


    def issue_reissue_transfer_test(self):
        self.log.info("Doing a big issue-reissue-transfer test...")
        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        ########################################
        # issue
        to_address = n0.getnewaddress()
        change_address = n0.getnewaddress()
        unspent = n0.listunspent()[0]
        inputs = [{k: unspent[k] for k in ['txid', 'vout']}]
        outputs = {
            'n1issueAssetXXXXXXXXXXXXXXXXWdnemQ': 500,
            change_address: float(unspent['amount']) - 500.0001,
            to_address: {
                'issue': {
                    'asset_name':       'TEST_ASSET',
                    'asset_quantity':   1000,
                    'units':            0,
                    'reissuable':       1,
                    'has_ipfs':         0,
                }
            }
        } 
        tx_issue = n0.createrawtransaction(inputs, outputs)
        tx_issue_signed = n0.signrawtransaction(tx_issue)
        tx_issue_hash = n0.sendrawtransaction(tx_issue_signed['hex'])
        assert_is_hash_string(tx_issue_hash)
        self.log.info("issue tx: " + tx_issue_hash)

        n0.generate(1)
        self.sync_all()
        assert_equal(1000, n0.listmyassets('TEST_ASSET')['TEST_ASSET'])
        assert_equal(1, n0.listmyassets('TEST_ASSET!')['TEST_ASSET!'])

        ########################################
        # reissue
        unspent = n0.listunspent()[0]
        unspent_asset_owner = n0.listmyassets('TEST_ASSET!', True)['TEST_ASSET!']['outpoints'][0]

        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]

        outputs = {
            'n1ReissueAssetXXXXXXXXXXXXXXWG9NLd': 100,
            change_address: float(unspent['amount']) - 100.0001,
            to_address: {
                'reissue': {
                    'asset_name':       'TEST_ASSET',
                    'asset_quantity':   1000,
                }
            }
        }

        tx_reissue = n0.createrawtransaction(inputs, outputs)
        tx_reissue_signed = n0.signrawtransaction(tx_reissue)
        tx_reissue_hash = n0.sendrawtransaction(tx_reissue_signed['hex'])
        assert_is_hash_string(tx_reissue_hash)
        self.log.info("reissue tx: " + tx_reissue_hash)

        n0.generate(1)
        self.sync_all()
        assert_equal(2000, n0.listmyassets('TEST_ASSET')['TEST_ASSET'])
        assert_equal(1, n0.listmyassets('TEST_ASSET!')['TEST_ASSET!'])

        self.sync_all()

        ########################################
        # transfer
        remote_to_address = n1.getnewaddress()
        unspent = n0.listunspent()[0]
        unspent_asset = n0.listmyassets('TEST_ASSET', True)['TEST_ASSET']['outpoints'][0]
        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            change_address: float(unspent['amount']) - 0.0001,
            remote_to_address: {
                'transfer': {
                    'TEST_ASSET': 400
                }
            },
            to_address: {
                'transfer': {
                    'TEST_ASSET': float(unspent_asset['amount']) - 400
                }
            }
        }
        tx_transfer = n0.createrawtransaction(inputs, outputs)
        tx_transfer_signed = n0.signrawtransaction(tx_transfer)
        tx_hex = tx_transfer_signed['hex']

        ########################################
        # try tampering with the sig
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        script_sig = bytes_to_hex_str(tx.vin[1].scriptSig)
        tampered_sig = script_sig[:-8] + "deadbeef"
        tx.vin[1].scriptSig = hex_str_to_bytes(tampered_sig)
        tampered_hex = bytes_to_hex_str(tx.serialize())
        assert_raises_rpc_error(-26, "mandatory-script-verify-flag-failed (Script failed an OP_EQUALVERIFY operation)",
                                n0.sendrawtransaction, tampered_hex)

        ########################################
        # try tampering with the asset script
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnt = '72766e74' #rvnt
        op_drop = '75'
        # change asset outputs from 400,600 to 500,500
        for i in range(1, 3):
            script_hex = bytes_to_hex_str(tx.vout[i].scriptPubKey)
            f = BytesIO(hex_str_to_bytes(script_hex[script_hex.index(rvnt) + len(rvnt):-len(op_drop)]))
            transfer = CScriptTransfer()
            transfer.deserialize(f)
            transfer.amount = 50000000000
            tampered_transfer = bytes_to_hex_str(transfer.serialize())
            tampered_script = script_hex[:script_hex.index(rvnt)] + rvnt + tampered_transfer + op_drop
            tx.vout[i].scriptPubKey = hex_str_to_bytes(tampered_script)
        tampered_hex = bytes_to_hex_str(tx.serialize())
        assert_raises_rpc_error(-26, "mandatory-script-verify-flag-failed (Signature must be zero for failed CHECK(MULTI)SIG operation)",
                                n0.sendrawtransaction, tampered_hex)

        ########################################
        # try tampering with asset outs so ins and outs don't add up
        for n in (-20, -2, -1, 1, 2, 20):
            bad_outputs = {
                change_address: float(unspent['amount']) - 0.0001,
                remote_to_address: {
                    'transfer': {
                        'TEST_ASSET': 400
                    }
                },
                to_address: {
                    'transfer': {
                        'TEST_ASSET': float(unspent_asset['amount']) - (400 + n)
                    }
                }
            }
            tx_bad_transfer = n0.createrawtransaction(inputs, bad_outputs)
            tx_bad_transfer_signed = n0.signrawtransaction(tx_bad_transfer)
            tx_bad_hex = tx_bad_transfer_signed['hex']
            assert_raises_rpc_error(-26, "bad-tx-inputs-outputs-mismatch Bad Transaction - Assets would be burnt TEST_ASSET",
                                    n0.sendrawtransaction, tx_bad_hex)

        ########################################
        # try tampering with asset outs so they don't use proper units
        for n in (-0.1, -0.00000001, 0.1, 0.00000001):
            bad_outputs = {
                change_address: float(unspent['amount']) - 0.0001,
                remote_to_address: {
                    'transfer': {
                        'TEST_ASSET': (400 + n)
                    }
                },
                to_address: {
                    'transfer': {
                        'TEST_ASSET': float(unspent_asset['amount']) - (400 + n)
                    }
                }
            }
            tx_bad_transfer = n0.createrawtransaction(inputs, bad_outputs)
            tx_bad_transfer_signed = n0.signrawtransaction(tx_bad_transfer)
            tx_bad_hex = tx_bad_transfer_signed['hex']
            assert_raises_rpc_error(-26, "bad-txns-transfer-asset-amount-not-match-units",
                                    n0.sendrawtransaction, tx_bad_hex)

        ########################################
        # try tampering to change the output asset name to one that doesn't exist
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnt = '72766e74' #rvnt
        op_drop = '75'
        # change asset name
        for n in range(0, len(tx.vout)):
            out = tx.vout[n]
            if rvnt in bytes_to_hex_str(out.scriptPubKey):
                script_hex = bytes_to_hex_str(out.scriptPubKey)
                f = BytesIO(hex_str_to_bytes(script_hex[script_hex.index(rvnt) + len(rvnt):-len(op_drop)]))
                transfer = CScriptTransfer()
                transfer.deserialize(f)
                transfer.name = b"ASSET_DOES_NOT_EXIST"
                tampered_transfer = bytes_to_hex_str(transfer.serialize())
                tampered_script = script_hex[:script_hex.index(rvnt)] + rvnt + tampered_transfer + op_drop
                tx.vout[n].scriptPubKey = hex_str_to_bytes(tampered_script)
        tampered_hex = bytes_to_hex_str(tx.serialize())
        assert_raises_rpc_error(-26, "bad-txns-transfer-asset-not-exist",
                                n0.sendrawtransaction, tampered_hex)

        ########################################
        # try tampering to change the output asset name to one that exists
        alternate_asset_name = "ALTERNATE"
        n0.issue(alternate_asset_name)
        n0.generate(1)
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnt = '72766e74' #rvnt
        op_drop = '75'
        # change asset name
        for n in range(0, len(tx.vout)):
            out = tx.vout[n]
            if rvnt in bytes_to_hex_str(out.scriptPubKey):
                script_hex = bytes_to_hex_str(out.scriptPubKey)
                f = BytesIO(hex_str_to_bytes(script_hex[script_hex.index(rvnt) + len(rvnt):-len(op_drop)]))
                transfer = CScriptTransfer()
                transfer.deserialize(f)
                transfer.name = alternate_asset_name.encode()
                tampered_transfer = bytes_to_hex_str(transfer.serialize())
                tampered_script = script_hex[:script_hex.index(rvnt)] + rvnt + tampered_transfer + op_drop
                tx.vout[n].scriptPubKey = hex_str_to_bytes(tampered_script)
        tampered_hex = bytes_to_hex_str(tx.serialize())
        assert_raises_rpc_error(-26, "bad-tx-inputs-outputs-mismatch Bad Transaction - " +
                                f"Trying to create outpoint for asset that you don't have: {alternate_asset_name}",
                                n0.sendrawtransaction, tampered_hex)

        ########################################
        # try tampering to remove the asset output
        tx = CTransaction()
        f = BytesIO(hex_str_to_bytes(tx_hex))
        tx.deserialize(f)
        rvnt = '72766e74' #rvnt
        # remove the transfer output from vout
        bad_vout = list(filter(lambda out : rvnt not in bytes_to_hex_str(out.scriptPubKey), tx.vout))
        tx.vout = bad_vout
        tampered_hex = bytes_to_hex_str(tx.serialize())
        assert_raises_rpc_error(-26, "bad-tx-asset-inputs-size-does-not-match-outputs-size",
                                n0.sendrawtransaction, tampered_hex)

        ########################################
        # send the good transfer
        tx_transfer_hash = n0.sendrawtransaction(tx_hex)
        assert_is_hash_string(tx_transfer_hash)
        self.log.info("transfer tx: " + tx_transfer_hash)

        n0.generate(1)
        self.sync_all()
        assert_equal(1600, n0.listmyassets('TEST_ASSET')['TEST_ASSET'])
        assert_equal(1, n0.listmyassets('TEST_ASSET!')['TEST_ASSET!'])
        assert_equal(400, n1.listmyassets('TEST_ASSET')['TEST_ASSET'])


    def unique_assets_test(self):
        self.log.info("Testing unique assets...")
        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        bad_burn = "n1BurnXXXXXXXXXXXXXXXXXXXXXXU1qejP"
        unique_burn = "n1issueUniqueAssetXXXXXXXXXXS4695i"

        root = "RINGU"
        owner = f"{root}!"
        n0.issue(root)
        n0.generate(10)

        asset_tags = ["myprecious1", "bind3", "gold7", "men9"]
        ipfs_hashes = ["QmWWQSuPMS6aXCbZKpEjPHPUZN2NjB3YrhJTHsV4X3vb2t"] * len(asset_tags)

        to_address = n0.getnewaddress()
        change_address = n0.getnewaddress()
        unspent = n0.listunspent()[0]
        unspent_asset_owner = n0.listmyassets(owner, True)[owner]['outpoints'][0]

        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]

        burn = 5 * len(asset_tags)

        # try first with bad burn address
        outputs = {
            bad_burn: burn,
            change_address: float(unspent['amount']) - (burn + 0.0001),
            to_address: {
                'issue_unique': {
                    'root_name':    root,
                    'asset_tags':   asset_tags,
                    'ipfs_hashes':  ipfs_hashes,
                }
            }
        }
        hex = n0.createrawtransaction(inputs, outputs)
        signed_hex = n0.signrawtransaction(hex)['hex']
        assert_raises_rpc_error(-26, "bad-txns-issue-unique-asset-burn-outpoints-not-found", \
                                n0.sendrawtransaction, signed_hex)

        # switch to proper burn address
        outputs = {
            unique_burn: burn,
            change_address: float(unspent['amount']) - (burn + 0.0001),
            to_address: {
                'issue_unique': {
                    'root_name':    root,
                    'asset_tags':   asset_tags,
                    'ipfs_hashes':  ipfs_hashes,
                }
            }
        }
        hex = n0.createrawtransaction(inputs, outputs)
        signed_hex = n0.signrawtransaction(hex)['hex']
        tx_hash = n0.sendrawtransaction(signed_hex)
        n0.generate(1)
        self.sync_all()

        for tag in asset_tags:
            asset_name = f"{root}#{tag}"
            assert_equal(1, n0.listmyassets()[asset_name])
            assert_equal(1, n0.listassets(asset_name, True)[asset_name]['has_ipfs'])
            assert_equal(ipfs_hashes[0], n0.listassets(asset_name, True)[asset_name]['ipfs_hash'])


    def unique_assets_via_issue_test(self):
        self.log.info("Testing unique assets via issue...")
        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        root = "RINGU2"
        owner = f"{root}!"
        n0.issue(root)
        n0.generate(1)

        asset_name = f"{root}#unique"

        to_address = n0.getnewaddress()
        change_address = n0.getnewaddress()
        owner_change_address = n0.getnewaddress()
        unspent = n0.listunspent()[0]
        unspent_asset_owner = n0.listmyassets(owner, True)[owner]['outpoints'][0]

        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]

        burn = 5
        outputs = {
            'n1issueUniqueAssetXXXXXXXXXXS4695i': burn,
            change_address: float(unspent['amount']) - (burn + 0.0001),
            owner_change_address: {
                'transfer': {
                    owner: 1,
                }
            },
            to_address: {
                'issue': {
                    'asset_name':       asset_name,
                    'asset_quantity':   100000000,
                    'units':            0,
                    'reissuable':       0,
                    'has_ipfs':         0,
                }
            },
        }

        ########################################
        # bad qty
        for n in (2, 20, 20000):
            outputs[to_address]['issue']['asset_quantity'] = n
            assert_raises_rpc_error(-8, "Invalid parameter: amount must be 100000000", n0.createrawtransaction, inputs, outputs)
        outputs[to_address]['issue']['asset_quantity'] = 1

        ########################################
        # bad units
        for n in (1, 2, 3, 4, 5, 6, 7, 8):
            outputs[to_address]['issue']['units'] = n
            assert_raises_rpc_error(-8, "Invalid parameter: units must be 0", n0.createrawtransaction, inputs, outputs)
        outputs[to_address]['issue']['units'] = 0

        ########################################
        # reissuable
        outputs[to_address]['issue']['reissuable'] = 1
        assert_raises_rpc_error(-8, "Invalid parameter: reissuable must be 0", n0.createrawtransaction, inputs, outputs)
        outputs[to_address]['issue']['reissuable'] = 0

        ########################################
        # ok
        hex = n0.signrawtransaction(n0.createrawtransaction(inputs, outputs))['hex']
        n0.sendrawtransaction(hex)
        n0.generate(1)
        assert_equal(1, n0.listmyassets()[root])
        assert_equal(1, n0.listmyassets()[asset_name])
        assert_equal(1, n0.listmyassets()[owner])


    def bad_ipfs_hash(self):
        self.log.info("Testing bad ipfs_hash...")
        n0 = self.nodes[0]

        asset_name = 'SOME_OTHER_ASSET_3'
        owner = f"{asset_name}!"
        to_address = n0.getnewaddress()
        change_address = n0.getnewaddress()
        unspent = n0.listunspent()[0]
        bad_hash = "RncvyefkqQX3PpjpY5L8B2yMd47XrVwAipr6cxUt2zvYU8"

        ########################################
        # issue
        inputs = [{k: unspent[k] for k in ['txid', 'vout']}]
        outputs = {
            'n1issueAssetXXXXXXXXXXXXXXXXWdnemQ': 500,
            change_address: float(unspent['amount']) - 500.0001,
            to_address: {
                'issue': {
                    'asset_name':       asset_name,
                    'asset_quantity':   1,
                    'units':            0,
                    'reissuable':       1,
                    'has_ipfs':         1,
                    'ipfs_hash':        bad_hash,
                }
            }
        }
        assert_raises_rpc_error(-8, "Invalid parameter: ipfs_hash must start with 'Qm'.", n0.createrawtransaction, inputs, outputs)

        ########################################
        # reissue
        n0.issue(asset_name=asset_name, qty=1000, to_address=to_address, change_address=change_address, \
                 units=0, reissuable=True, has_ipfs=False)
        n0.generate(1)
        unspent_asset_owner = n0.listmyassets(owner, True)[owner]['outpoints'][0]
        inputs = [
            {k: unspent[k] for k in ['txid', 'vout']},
            {k: unspent_asset_owner[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            'n1ReissueAssetXXXXXXXXXXXXXXWG9NLd': 100,
            change_address: float(unspent['amount']) - 100.0001,
            to_address: {
                'reissue': {
                    'asset_name':       asset_name,
                    'asset_quantity':   1000,
                    'ipfs_hash':        bad_hash,
                }
            }
        }
        assert_raises_rpc_error(-8, "Invalid parameter: ipfs_hash must start with 'Qm'.", n0.createrawtransaction, inputs, outputs)


    def atomic_swaps(self):
        self.log.info("Testing atomic swaps...")
        n0, n1, n2 = self.nodes[0], self.nodes[1], self.nodes[2]

        jaina = "JAINA"
        anduin = "ANDUIN"
        jaina_owner = f"{jaina}!"
        anduin_owner = f"{anduin}!"

        starting_amount = 1000

        receive1 = n1.getnewaddress()
        change1 = n1.getnewaddress()
        n0.sendtoaddress(receive1, 50000)
        n0.generate(1); self.sync_all()
        n1.issue(jaina, starting_amount)
        n1.generate(1); self.sync_all()
        balance1 = float(n1.getwalletinfo()['balance'])

        receive2 = n2.getnewaddress()
        change2 = n2.getnewaddress()
        n0.sendtoaddress(receive2, 50000)
        n0.generate(1); self.sync_all()
        n2.issue(anduin, starting_amount)
        n2.generate(1); self.sync_all()
        balance2 = float(n2.getwalletinfo()['balance'])

        ########################################
        # rvn for assets

        # n1 buys 400 ANDUIN from n2 for 4000 RITO
        price = 4000
        amount = 400
        fee = 0.0001

        unspent1 = n1.listunspent()[0]
        unspent_amount1 = float(unspent1['amount'])
        assert(unspent_amount1 > price + fee)

        unspent_asset2 = n2.listmyassets(anduin, True)[anduin]['outpoints'][0]
        unspent_asset_amount2 = unspent_asset2['amount']
        assert(unspent_asset_amount2 > amount)

        inputs = [
            {k: unspent1[k] for k in ['txid', 'vout']},
            {k: unspent_asset2[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            receive1: {
                'transfer': {
                    anduin: amount
                }
            },
            change1: unspent_amount1 - price - fee,
            receive2: price,
            change2: {
                'transfer': {
                    anduin: unspent_asset_amount2 - amount
                }
            },
        }

        unsigned = n1.createrawtransaction(inputs, outputs)
        signed1 = n1.signrawtransaction(unsigned)['hex']
        signed2 = n2.signrawtransaction(signed1)['hex']
        _tx_id = n0.sendrawtransaction(signed2)
        n0.generate(10)
        self.sync_all()

        newbalance1 = float(n1.getwalletinfo()['balance'])
        newbalance2 = float(n2.getwalletinfo()['balance'])
        assert_equal(balance1 - price - fee, newbalance1)
        assert_equal(balance2 + price, newbalance2)

        assert_equal(amount, int(n1.listmyassets()[anduin]))
        assert_equal(starting_amount - amount, int(n2.listmyassets()[anduin]))


        ########################################
        # rvn for owner

        # n2 buys JAINA! from n1 for 20000 RITO
        price = 20000
        amount = 1
        balance1 = newbalance1
        balance2 = newbalance2

        unspent2 = next(u for u in n2.listunspent() if u['amount'] > price + fee)
        unspent_amount2 = float(unspent2['amount'])

        unspent_owner1 = n1.listmyassets(jaina_owner, True)[jaina_owner]['outpoints'][0]
        unspent_owner_amount1 = unspent_owner1['amount']
        assert_equal(amount, unspent_owner_amount1)

        inputs = [
            {k: unspent2[k] for k in ['txid', 'vout']},
            {k: unspent_owner1[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            receive1: price,
            receive2: {
                'transfer': {
                    jaina_owner: amount
                }
            },
            change2: unspent_amount2 - price - fee,
        }

        unsigned = n2.createrawtransaction(inputs, outputs)
        signed2 = n2.signrawtransaction(unsigned)['hex']
        signed1 = n1.signrawtransaction(signed2)['hex']
        _tx_id = n0.sendrawtransaction(signed1)
        n0.generate(10)
        self.sync_all()

        newbalance1 = float(n1.getwalletinfo()['balance'])
        newbalance2 = float(n2.getwalletinfo()['balance'])
        assert_equal(balance1 + price, newbalance1)
        assert_equal(balance2 - price - fee, newbalance2)

        assert_does_not_contain_key(jaina_owner, n1.listmyassets())
        assert_equal(amount, int(n2.listmyassets()[jaina_owner]))

        ########################################
        # assets for assets and owner

        # n1 buys ANDUIN! and 300 ANDUIN from n2 for 900 JAINA
        price = 900
        amount = 300
        amount_owner = 1
        balance1 = newbalance1
        balance2 = newbalance2

        unspent1 = n1.listunspent()[0]
        unspent_amount1 = float(unspent1['amount'])
        assert(unspent_amount1 > fee)

        unspent_asset1 = n1.listmyassets(jaina, True)[jaina]['outpoints'][0]
        unspent_asset_amount1 = unspent_asset1['amount']

        unspent_asset2 = n2.listmyassets(anduin, True)[anduin]['outpoints'][0]
        unspent_asset_amount2 = unspent_asset2['amount']

        unspent_owner2 = n2.listmyassets(anduin_owner, True)[anduin_owner]['outpoints'][0]
        unspent_owner_amount2 = unspent_owner2['amount']

        assert(unspent_asset_amount1 > price)
        assert(unspent_asset_amount2 > amount)
        assert_equal(amount_owner, unspent_owner_amount2)

        inputs = [
            {k: unspent1[k] for k in ['txid', 'vout']},
            {k: unspent_asset1[k] for k in ['txid', 'vout']},
            {k: unspent_asset2[k] for k in ['txid', 'vout']},
            {k: unspent_owner2[k] for k in ['txid', 'vout']},
        ]
        outputs = {
            receive1: {
                'transfer': {
                    anduin: amount,
                    anduin_owner: amount_owner,
                }
            },
            # output map can't use change1 twice...
            n1.getnewaddress(): unspent_amount1 - fee,
            change1: {
                'transfer': {
                    jaina: unspent_asset_amount1 - price
                }
            },
            receive2: {
                'transfer': {
                    jaina: price
                }
            },
            change2: {
                'transfer': {
                    anduin: unspent_asset_amount2 - amount
                }
            },
        }

        unsigned = n1.createrawtransaction(inputs, outputs)
        signed1 = n1.signrawtransaction(unsigned)['hex']
        signed2 = n2.signrawtransaction(signed1)['hex']
        _tx_id = n0.sendrawtransaction(signed2)
        n0.generate(10)
        self.sync_all()

        newbalance1 = float(n1.getwalletinfo()['balance'])
        assert_equal(balance1 - fee, newbalance1)

        assert_does_not_contain_key(anduin_owner, n2.listmyassets())
        assert_equal(amount_owner, int(n1.listmyassets()[anduin_owner]))

        assert_equal(unspent_asset_amount1 - price, n1.listmyassets()[jaina])
        assert_equal(unspent_asset_amount2 - amount, n2.listmyassets()[anduin])


    def run_test(self):
        self.activate_assets()
        self.issue_reissue_transfer_test()
        self.unique_assets_test()
        self.issue_tampering_test()
        self.reissue_tampering_test()
        self.unique_assets_via_issue_test()
        self.bad_ipfs_hash()
        self.atomic_swaps()

if __name__ == '__main__':
    RawAssetTransactionsTest().main()
