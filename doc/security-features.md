# SHAHCOIN Core Security Features

This document describes the three major security and staking extensions implemented in SHAHCOIN Core:

## 1. Honeypot Transaction Filters

### Overview
Honeypot transaction filtering detects and blocks suspicious transactions that match known exploit or spam patterns, protecting the network from various attacks.

### Features
- **OP_RETURN Limits**: Prevents excessive OP_RETURN outputs and data size
- **Script Validation**: Detects malformed scriptSig/scriptPubKey patterns
- **Dust Detection**: Identifies suspicious dust amount patterns
- **Input Validation**: Checks for malformed input structures
- **Spam Pattern Detection**: Recognizes known spam transaction patterns
- **Exploit Prevention**: Blocks potential exploit attempts
- **Size Limits**: Enforces maximum transaction and script sizes
- **Rate Limiting**: Prevents transaction flooding from single sources

### Configuration
Default limits can be configured:
- Maximum OP_RETURN outputs: 10
- Maximum OP_RETURN size: 80 bytes
- Maximum total OP_RETURN size: 400 bytes
- Minimum dust threshold: 546 SHAH
- Maximum transaction size: 1MB
- Rate limit window: 3600 seconds

### RPC Commands
- `gethoneypotstats`: Get filtering statistics
- `resethoneypotstats`: Reset statistics

### Usage
```bash
# Get current statistics
shahcoin-cli gethoneypotstats

# Reset statistics
shahcoin-cli resethoneypotstats
```

## 2. Block Finality Rules

### Overview
Block finality rules implement soft finality logic to prevent deep reorganizations and provide stronger guarantees about transaction confirmation.

### Features
- **Soft Finality**: Blocks become final after configurable confirmations (default: 100)
- **Hard Finality**: Economic finality after more confirmations (default: 1000)
- **Irreversibility**: Deep finality after extensive confirmations (default: 10000)
- **Reorganization Protection**: Prevents reorganizations deeper than finalized blocks
- **Configurable Parameters**: All thresholds can be adjusted

### Finality Levels
1. **PENDING**: Block is not yet finalized
2. **SOFT_FINAL**: Block has sufficient confirmations for basic finality
3. **HARD_FINAL**: Block has economic finality
4. **IRREVERSIBLE**: Block is considered irreversible

### Configuration
- Soft finality confirmations: 100
- Hard finality confirmations: 1000
- Irreversible confirmations: 10000
- Maximum reorganization depth: 100

### RPC Commands
- `getfinalitystatus [blockhash]`: Get finality status for a block
- `getfinalitystats`: Get finality statistics

### Usage
```bash
# Get finality status of current tip
shahcoin-cli getfinalitystatus

# Get finality status of specific block
shahcoin-cli getfinalitystatus "0000000000000000000000000000000000000000000000000000000000000000"

# Get finality statistics
shahcoin-cli getfinalitystats
```

## 3. Cold Staking Support

### Overview
Cold staking allows users to stake from cold wallets securely using delegation, separating fund ownership from staking operations.

### Features
- **Delegation System**: Cold wallet delegates staking rights to hot wallet
- **Secure Separation**: Cold wallet holds funds, hot wallet performs staking
- **Revocation Rights**: Cold wallet can revoke delegation at any time
- **Expiry Support**: Optional delegation expiry times
- **Transaction Types**: Special transaction types for delegation operations

### Transaction Types
1. **DELEGATE**: Create a new delegation
2. **REVOKE**: Revoke an existing delegation
3. **STAKE**: Create stake using delegated funds
4. **WITHDRAW**: Withdraw staking rewards

### Delegation Status
- **ACTIVE**: Delegation is active and can be used for staking
- **REVOKED**: Delegation has been revoked by cold wallet
- **EXPIRED**: Delegation has expired
- **PENDING**: Delegation is pending activation

### RPC Commands
- `getcoldstakingdelegations [address] [type]`: Get delegations for an address
- `getcoldstakingstats`: Get cold staking statistics

### Usage
```bash
# Get all active delegations
shahcoin-cli getcoldstakingdelegations

# Get delegations for specific address
shahcoin-cli getcoldstakingdelegations "S1a2h3c4o5i6n7A8d9d0r1e2s3s4"

# Get cold wallet delegations only
shahcoin-cli getcoldstakingdelegations "S1a2h3c4o5i6n7A8d9d0r1e2s3s4" "cold"

# Get cold staking statistics
shahcoin-cli getcoldstakingstats
```

## Integration

### Initialization
All security systems are automatically initialized when the first PoS block is processed. The systems are designed to work together seamlessly.

### Logging
All security features log their activities to the debug log:
- Honeypot filtering logs suspicious transactions
- Finality rules log reorganization attempts
- Cold staking logs delegation operations

### Configuration Files
Security parameters can be configured in `shahcoin.conf`:

```ini
# Honeypot filtering
honeypot-filtering=1
max-op-return-outputs=10
max-op-return-size=80

# Finality rules
finality-rules=1
soft-finality-confirmations=100
hard-finality-confirmations=1000

# Cold staking
cold-staking=1
```

## Security Considerations

### Honeypot Filtering
- False positives may occur with legitimate transactions
- Patterns are updated based on network analysis
- Filtering can be disabled if needed

### Finality Rules
- Finality is probabilistic, not absolute
- Deep reorganizations are still possible but unlikely
- Rules can be adjusted based on network conditions

### Cold Staking
- Cold wallet must trust hot wallet not to misbehave
- Delegation revocation requires cold wallet signature
- Expired delegations must be manually renewed

## Testing

Run the security tests to verify functionality:

```bash
make check
./src/test/test_shahcoin --run_test=security_tests
```

## Future Enhancements

### Planned Features
- Machine learning-based honeypot detection
- Dynamic finality thresholds based on network conditions
- Multi-signature cold staking support
- GUI integration for all security features

### Monitoring
- Real-time security metrics dashboard
- Alert system for suspicious activity
- Network-wide security statistics

## Support

For questions or issues with these security features:
- Check the debug log for detailed information
- Use RPC commands to query system status
- Review configuration parameters
- Report bugs through the standard channels
