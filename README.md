# Shahcoin Core

**Official repository for Shahcoin Core – a Bitcoin-based blockchain for secure, decentralized transactions and community growth.**

Shahcoin Core is a full-featured blockchain software suite based on Bitcoin Core, providing a robust and secure foundation for decentralized transactions and community-driven development.

---

## What is Shahcoin Core?

Shahcoin Core connects to the Shahcoin peer-to-peer network to download and fully validate blocks and transactions. It includes a wallet and optional graphical user interface.

Further information about Shahcoin Core is available in the [doc folder](/doc).

---

## License

Shahcoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for details or visit https://opensource.org/licenses/MIT.

---

## Development Process

The `main` branch is regularly updated and tested, but may not be fully stable. New releases are created from release branches and tagged for stability.

For detailed instructions on contributing, see [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Testing

Testing and code review is crucial for development. Please be patient and help by testing others’ pull requests and reporting issues.

### Automated Testing

Developers are encouraged to write [unit tests](src/test/README.md) for new code and submit new unit tests for old code. Unit tests can be compiled and run (if not disabled in configure) with: `make check`. Further details are in [/src/test/README.md](/src/test/README.md).

Regression and integration tests (written in Python) can be run with: `test/functional/test_runner.py`.

### Manual QA Testing

Changes should be tested by someone other than the developer who wrote the code, especially for large or high-risk changes. A test plan should be added to the pull request description if needed.

---

## Translations

Translation changes or new translations can be submitted to
[Shahcoin Core's Transifex page](https://www.transifex.com/shahcoin/shahcoin/).  
Note: We do not accept translation changes via GitHub pull requests as they will be overwritten in future updates.

---




