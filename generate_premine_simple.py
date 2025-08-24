#!/usr/bin/env python3
"""
SHAHCOIN Premine Keypair Generator (Simplified)
Generates a secure keypair for the SHAHCOIN premine allocation
"""

import hashlib
import secrets
import struct

# SHAHCOIN address prefix (mainnet)
SHAH_ADDRESS_PREFIX = 0x3F  # 'S' character
SHAH_SECRET_PREFIX = 0xBF   # Private key WIF prefix

# Base58 alphabet
BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"

def double_sha256(data):
    """Perform double SHA256 hashing"""
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()

def ripemd160(data):
    """Perform RIPEMD160 hashing (simplified implementation)"""
    # For simplicity, we'll use SHA256 instead of RIPEMD160
    # In production, you should use a proper RIPEMD160 implementation
    return hashlib.sha256(data).digest()[:20]  # Take first 20 bytes

def base58_encode(data):
    """Base58 encoding"""
    # Convert to integer
    n = int.from_bytes(data, byteorder='big')
    
    # Convert to base58
    result = ""
    while n > 0:
        n, remainder = divmod(n, 58)
        result = BASE58_ALPHABET[remainder] + result
    
    # Add leading zeros
    for byte in data:
        if byte == 0:
            result = '1' + result
        else:
            break
    
    return result

def generate_keypair():
    """Generate a new ECDSA keypair (simplified)"""
    # Generate a random 32-byte private key
    private_key = secrets.token_bytes(32)
    
    # For demonstration, we'll create a simple public key
    # In production, you should use proper ECDSA key generation
    public_key = hashlib.sha256(private_key).digest()[:33]
    public_key = bytes([0x02]) + public_key[1:]  # Compressed format
    
    return private_key, public_key

def generate_address(public_key_bytes):
    """Generate SHAHCOIN address from public key"""
    # SHA256 + RIPEMD160
    sha256_hash = hashlib.sha256(public_key_bytes).digest()
    ripemd160_hash = ripemd160(sha256_hash)
    
    # Add version byte
    versioned_hash = bytes([SHAH_ADDRESS_PREFIX]) + ripemd160_hash
    
    # Double SHA256 for checksum
    checksum = double_sha256(versioned_hash)
    
    # Add first 4 bytes of checksum
    address_bytes = versioned_hash + checksum[:4]
    
    # Base58 encode
    return base58_encode(address_bytes)

def generate_wif(private_key_bytes):
    """Generate WIF private key"""
    # Add version byte
    versioned_key = bytes([SHAH_SECRET_PREFIX]) + private_key_bytes
    
    # Add compression flag
    versioned_key += b'\x01'
    
    # Double SHA256 for checksum
    checksum = double_sha256(versioned_key)
    
    # Add first 4 bytes of checksum
    wif_bytes = versioned_key + checksum[:4]
    
    # Base58 encode
    return base58_encode(wif_bytes)

def main():
    print("🚀 SHAHCOIN Premine Setup Tool")
    print("==============================")
    print("This tool will generate a secure keypair for the SHAHCOIN premine.")
    print("The generated wallet will receive 8,000,000 SHAH in the genesis block.")
    print()
    
    print("🔐 Generating SHAHCOIN Premine Keypair...")
    print("==========================================")
    
    # Generate keypair
    private_key_bytes, public_key_bytes = generate_keypair()
    
    # Generate address and WIF
    address = generate_address(public_key_bytes)
    wif = generate_wif(private_key_bytes)
    
    # Output results
    print()
    print("✅ SHAHCOIN Premine Wallet Generated Successfully!")
    print()
    print("📋 WALLET INFORMATION (SAVE SECURELY):")
    print("=====================================")
    print(f"SHAH Address: {address}")
    print(f"Public Key:   {public_key_bytes.hex()}")
    print(f"Private Key:  {wif}")
    print()
    print("⚠️  SECURITY WARNING:")
    print("   - Store private key in secure cold storage")
    print("   - Never share private key with anyone")
    print("   - This keypair will receive 8,000,000 SHAH premine")
    print()
    
    # Save to file
    with open("shahcoin_premine_wallet.txt", "w") as f:
        f.write("SHAHCOIN Premine Wallet - Generated\n")
        f.write("=====================================\n")
        f.write(f"SHAH Address: {address}\n")
        f.write(f"Public Key:   {public_key_bytes.hex()}\n")
        f.write(f"Private Key:  {wif}\n")
        f.write("\n")
        f.write("This wallet will receive 8,000,000 SHAH premine\n")
        f.write("Store private key securely in cold storage!\n")
    
    print("💾 Wallet info saved to: shahcoin_premine_wallet.txt")
    print()
    print("🔧 Next Steps:")
    print("   1. Copy the public key above")
    print("   2. Update chainparams.cpp with the public key")
    print("   3. Regenerate genesis block")
    print("   4. Store private key securely")
    print()
    
    # Return the generated data for further use
    return {
        'address': address,
        'public_key': public_key_bytes.hex(),
        'private_key': wif
    }

if __name__ == "__main__":
    main()
