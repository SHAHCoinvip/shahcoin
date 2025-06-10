Shared Libraries
================

## shahcoinconsensus

The purpose of this library is to make the verification functionality that is critical to Shahcoin's consensus available to other applications, e.g. to language bindings.

### API

The interface is defined in the C header `shahcoinconsensus.h` located in `src/script/shahcoinconsensus.h`.

#### Version

`shahcoinconsensus_version` returns an `unsigned int` with the API version *(currently `2`)*.

#### Script Validation

`shahcoinconsensus_verify_script`, `shahcoinconsensus_verify_script_with_amount` and `shahcoinconsensus_verify_script_with_spent_outputs` return an `int` with the status of the verification. It will be `1` if the input script correctly spends the previous output `scriptPubKey`.

##### Parameters
###### shahcoinconsensus_verify_script
- `const unsigned char *scriptPubKey` - The previous output script that encumbers spending.
- `unsigned int scriptPubKeyLen` - The number of bytes for the `scriptPubKey`.
- `const unsigned char *txTo` - The transaction with the input that is spending the previous output.
- `unsigned int txToLen` - The number of bytes for the `txTo`.
- `unsigned int nIn` - The index of the input in `txTo` that spends the `scriptPubKey`.
- `unsigned int flags` - The script validation flags *(see below)*.
- `shahcoinconsensus_error* err` - Will have the error/success code for the operation *(see below)*.

###### shahcoinconsensus_verify_script_with_amount
- `const unsigned char *scriptPubKey` - The previous output script that encumbers spending.
- `unsigned int scriptPubKeyLen` - The number of bytes for the `scriptPubKey`.
- `int64_t amount` - The amount spent in the input
- `const unsigned char *txTo` - The transaction with the input that is spending the previous output.
- `unsigned int txToLen` - The number of bytes for the `txTo`.
- `unsigned int nIn` - The index of the input in `txTo` that spends the `scriptPubKey`.
- `unsigned int flags` - The script validation flags *(see below)*.
- `shahcoinconsensus_error* err` - Will have the error/success code for the operation *(see below)*.

###### shahcoinconsensus_verify_script_with_spent_outputs
- `const unsigned char *scriptPubKey` - The previous output script that encumbers spending.
- `unsigned int scriptPubKeyLen` - The number of bytes for the `scriptPubKey`.
- `int64_t amount` - The amount spent in the input
- `const unsigned char *txTo` - The transaction with the input that is spending the previous output.
- `unsigned int txToLen` - The number of bytes for the `txTo`.
- `UTXO *spentOutputs` - Previous outputs spent in the transaction. `UTXO` is a struct composed by `const unsigned char *scriptPubKey`, `unsigned int scriptPubKeySize` (the number of bytes for the `scriptPubKey`) and `unsigned int value`.
- `unsigned int spentOutputsLen` - The number of bytes for the `spentOutputs`.
- `unsigned int nIn` - The index of the input in `txTo` that spends the `scriptPubKey`.
- `unsigned int flags` - The script validation flags *(see below)*.
- `shahcoinconsensus_error* err` - Will have the error/success code for the operation *(see below)*.

##### Script Flags
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_NONE`
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH` - Evaluate P2SH ([BIP16](https://github.com/shahcoin/bips/blob/master/bip-0016.mediawiki)) subscripts
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG` - Enforce strict DER ([BIP66](https://github.com/shahcoin/bips/blob/master/bip-0066.mediawiki)) compliance
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY` - Enforce NULLDUMMY ([BIP147](https://github.com/shahcoin/bips/blob/master/bip-0147.mediawiki))
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY` - Enable CHECKLOCKTIMEVERIFY ([BIP65](https://github.com/shahcoin/bips/blob/master/bip-0065.mediawiki))
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY` - Enable CHECKSEQUENCEVERIFY ([BIP112](https://github.com/shahcoin/bips/blob/master/bip-0112.mediawiki))
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS` - Enable WITNESS ([BIP141](https://github.com/shahcoin/bips/blob/master/bip-0141.mediawiki))
- `shahcoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT` - Enable TAPROOT ([BIP340](https://github.com/shahcoin/bips/blob/master/bip-0340.mediawiki), [BIP341](https://github.com/shahcoin/bips/blob/master/bip-0341.mediawiki), [BIP342](https://github.com/shahcoin/bips/blob/master/bip-0342.mediawiki))

##### Errors
- `shahcoinconsensus_ERR_OK` - No errors with input parameters *(see the return value of `shahcoinconsensus_verify_script` for the verification status)*
- `shahcoinconsensus_ERR_TX_INDEX` - An invalid index for `txTo`
- `shahcoinconsensus_ERR_TX_SIZE_MISMATCH` - `txToLen` did not match with the size of `txTo`
- `shahcoinconsensus_ERR_DESERIALIZE` - An error deserializing `txTo`
- `shahcoinconsensus_ERR_AMOUNT_REQUIRED` - Input amount is required if WITNESS is used
- `shahcoinconsensus_ERR_INVALID_FLAGS` - Script verification `flags` are invalid (i.e. not part of the libconsensus interface)
- `shahcoinconsensus_ERR_SPENT_OUTPUTS_REQUIRED` - Spent outputs are required if TAPROOT is used
- `shahcoinconsensus_ERR_SPENT_OUTPUTS_MISMATCH` - Spent outputs size doesn't match tx inputs size

### Example Implementations
- [NShahcoin](https://github.com/MetacoSA/NShahcoin/blob/5e1055cd7c4186dee4227c344af8892aea54faec/NShahcoin/Script.cs#L979-#L1031) (.NET Bindings)
- [node-libshahcoinconsensus](https://github.com/bitpay/node-libshahcoinconsensus) (Node.js Bindings)
- [java-libshahcoinconsensus](https://github.com/dexX7/java-libshahcoinconsensus) (Java Bindings)
- [shahcoinconsensus-php](https://github.com/Bit-Wasp/shahcoinconsensus-php) (PHP Bindings)
- [rust-shahcoinconsensus](https://github.com/rust-shahcoin/rust-shahcoinconsensus) (Rust Bindings)