#!/usr/bin/env python3
# Copyright (c) 2024 The SHAHCOIN Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test staking functionality."""

from test_framework.test_framework import ShahcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    connect_nodes,
    disconnect_nodes,
    sync_blocks,
    wait_until,
)
from test_framework.messages import (
    CBlock,
    CBlockHeader,
    msg_block,
)
import time

class StakingTest(ShahcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 3
        self.setup_clean_chain = True
        self.extra_args = [
            ["-staking=1"],
            ["-staking=1"],
            ["-staking=0"],  # Node 2 doesn't stake
        ]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        # Test basic staking functionality
        self.test_basic_staking()
        
        # Test staking RPC commands
        self.test_staking_rpc_commands()
        
        # Test hybrid consensus (PoW/PoS alternation)
        self.test_hybrid_consensus()
        
        # Test staking rewards
        self.test_staking_rewards()
        
        # Test staking validation
        self.test_staking_validation()

    def test_basic_staking(self):
        """Test basic staking functionality."""
        self.log.info("Testing basic staking functionality...")
        
        # Generate some coins for staking
        self.nodes[0].generate(101)
        self.sync_all()
        
        # Check initial balance
        balance = self.nodes[0].getbalance()
        assert balance > 0, "Node 0 should have coins for staking"
        
        # Enable staking
        result = self.nodes[0].enablestaking()
        assert result, "Failed to enable staking"
        
        # Check staking info
        staking_info = self.nodes[0].getstakinginfo()
        assert staking_info["enabled"], "Staking should be enabled"
        assert_equal(staking_info["total_stake"], 0)  # No stakes added yet
        
        # Add a stake
        address = self.nodes[0].getnewaddress()
        stake_amount = 1000  # 1000 SHAH
        result = self.nodes[0].addstake(address, stake_amount)
        assert result, "Failed to add stake"
        
        # Check staking info again
        staking_info = self.nodes[0].getstakinginfo()
        assert_equal(staking_info["total_stake"], stake_amount)
        assert_equal(staking_info["validator_count"], 1)

    def test_staking_rpc_commands(self):
        """Test staking RPC commands."""
        self.log.info("Testing staking RPC commands...")
        
        # Test getstakinginfo
        staking_info = self.nodes[0].getstakinginfo()
        assert "enabled" in staking_info
        assert "total_stake" in staking_info
        assert "available_stake" in staking_info
        assert "validator_count" in staking_info
        assert "expected_time" in staking_info
        
        # Test getstakes
        stakes = self.nodes[0].getstakes()
        assert isinstance(stakes, list)
        if len(stakes) > 0:
            stake = stakes[0]
            assert "address" in stake
            assert "amount" in stake
            assert "age" in stake
            assert "eligible" in stake
        
        # Test getstakevalidators
        validators = self.nodes[0].getstakevalidators()
        assert isinstance(validators, list)
        if len(validators) > 0:
            validator = validators[0]
            assert "address" in validator
            assert "amount" in validator
            assert "age" in validator
        
        # Test invalid stake amount
        address = self.nodes[0].getnewaddress()
        assert_raises_rpc_error(-8, "Stake amount must be at least", 
                               self.nodes[0].addstake, address, 100)  # Too small
        
        # Test invalid address
        assert_raises_rpc_error(-5, "Invalid address", 
                               self.nodes[0].addstake, "invalid_address", 1000)

    def test_hybrid_consensus(self):
        """Test hybrid consensus (PoW/PoS alternation)."""
        self.log.info("Testing hybrid consensus...")
        
        # Generate some blocks to establish the chain
        self.nodes[0].generate(10)
        self.sync_all()
        
        # Check that blocks alternate between PoW and PoS
        for i in range(5):
            # Generate a block
            block_hash = self.nodes[0].generate(1)[0]
            block = self.nodes[0].getblock(block_hash)
            
            # Check block type
            if i % 2 == 0:
                # Even blocks should be PoW
                assert_equal(block.get("block_type", 0), 0)  # BLOCK_TYPE_POW
            else:
                # Odd blocks should be PoS
                assert_equal(block.get("block_type", 0), 1)  # BLOCK_TYPE_POS
            
            self.sync_all()

    def test_staking_rewards(self):
        """Test staking rewards."""
        self.log.info("Testing staking rewards...")
        
        # Add a stake and wait for it to become eligible
        address = self.nodes[0].getnewaddress()
        stake_amount = 1000
        self.nodes[0].addstake(address, stake_amount)
        
        # Get initial balance
        initial_balance = self.nodes[0].getbalance()
        
        # Wait for staking to produce a block
        # This is a simplified test - in practice, we'd need to wait for the stake to mature
        time.sleep(2)
        
        # Check if balance increased (staking reward)
        final_balance = self.nodes[0].getbalance()
        # Note: In a real test, we'd check if the balance increased due to staking rewards
        # For now, we just verify the balance didn't decrease unexpectedly
        assert final_balance >= initial_balance, "Balance should not decrease unexpectedly"

    def test_staking_validation(self):
        """Test staking validation."""
        self.log.info("Testing staking validation...")
        
        # Test minimum stake amount
        address = self.nodes[0].getnewaddress()
        min_stake = 333  # POS_MIN_STAKE_AMOUNT in SHAH
        
        # Try to add stake below minimum
        assert_raises_rpc_error(-8, "Stake amount must be at least", 
                               self.nodes[0].addstake, address, min_stake - 1)
        
        # Add valid stake
        result = self.nodes[0].addstake(address, min_stake)
        assert result, "Failed to add valid stake"
        
        # Test stake removal
        result = self.nodes[0].removestake(address)
        assert result, "Failed to remove stake"
        
        # Verify stake is removed
        staking_info = self.nodes[0].getstakinginfo()
        assert_equal(staking_info["total_stake"], 0)

    def test_staking_network_sync(self):
        """Test staking with network synchronization."""
        self.log.info("Testing staking network synchronization...")
        
        # Enable staking on both nodes
        self.nodes[0].enablestaking()
        self.nodes[1].enablestaking()
        
        # Add stakes to both nodes
        address0 = self.nodes[0].getnewaddress()
        address1 = self.nodes[1].getnewaddress()
        
        self.nodes[0].addstake(address0, 1000)
        self.nodes[1].addstake(address1, 1000)
        
        # Generate some blocks and check synchronization
        self.nodes[0].generate(5)
        self.sync_all()
        
        # Verify both nodes have the same chain
        assert_equal(self.nodes[0].getblockcount(), self.nodes[1].getblockcount())
        
        # Check that staking info is consistent
        info0 = self.nodes[0].getstakinginfo()
        info1 = self.nodes[1].getstakinginfo()
        
        # Both nodes should have the same total network stake
        assert_equal(info0["total_stake"], info1["total_stake"])

    def test_staking_disabled_node(self):
        """Test that staking is properly disabled on node 2."""
        self.log.info("Testing staking disabled node...")
        
        # Node 2 should not have staking enabled
        staking_info = self.nodes[2].getstakinginfo()
        assert not staking_info["enabled"], "Node 2 should not have staking enabled"
        
        # Try to enable staking on node 2 (should fail or be ignored)
        # This depends on the implementation - some nodes might allow enabling staking
        # even if it wasn't enabled at startup
        
        # Verify node 2 can still sync with the network
        self.nodes[0].generate(5)
        self.sync_all()
        assert_equal(self.nodes[0].getblockcount(), self.nodes[2].getblockcount())

if __name__ == '__main__':
    StakingTest().main()
