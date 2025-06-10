# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libshahcoin_cli*         | RPC client functionality used by *shahcoin-cli* executable |
| *libshahcoin_common*      | Home for common functionality shared by different executables and libraries. Similar to *libshahcoin_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libshahcoin_consensus*   | Stable, backwards-compatible consensus functionality used by *libshahcoin_node* and *libshahcoin_wallet* and also exposed as a [shared library](../shared-libraries.md). |
| *libshahcoinconsensus*    | Shared library build of static *libshahcoin_consensus* library |
| *libshahcoin_kernel*      | Consensus engine and support library used for validation by *libshahcoin_node* and also exposed as a [shared library](../shared-libraries.md). |
| *libshahcoinqt*           | GUI functionality used by *shahcoin-qt* and *shahcoin-gui* executables |
| *libshahcoin_ipc*         | IPC functionality used by *shahcoin-node*, *shahcoin-wallet*, *shahcoin-gui* executables to communicate when [`--enable-multiprocess`](multiprocess.md) is used. |
| *libshahcoin_node*        | P2P and RPC server functionality used by *shahcoind* and *shahcoin-qt* executables. |
| *libshahcoin_util*        | Home for common functionality shared by different executables and libraries. Similar to *libshahcoin_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libshahcoin_wallet*      | Wallet functionality used by *shahcoind* and *shahcoin-wallet* executables. |
| *libshahcoin_wallet_tool* | Lower-level wallet functionality used by *shahcoin-wallet* executable. |
| *libshahcoin_zmq*         | [ZeroMQ](../zmq.md) functionality used by *shahcoind* and *shahcoin-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. Exceptions are *libshahcoin_consensus* and *libshahcoin_kernel* which have external interfaces documented at [../shared-libraries.md](../shared-libraries.md).

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`libshahcoin_*_SOURCES`](../../src/Makefile.am) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libshahcoin_node* code lives in `src/node/` in the `node::` namespace
  - *libshahcoin_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libshahcoin_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libshahcoin_util* code lives in `src/util/` in the `util::` namespace
  - *libshahcoin_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

shahcoin-cli[shahcoin-cli]-->libshahcoin_cli;

shahcoind[shahcoind]-->libshahcoin_node;
shahcoind[shahcoind]-->libshahcoin_wallet;

shahcoin-qt[shahcoin-qt]-->libshahcoin_node;
shahcoin-qt[shahcoin-qt]-->libshahcoinqt;
shahcoin-qt[shahcoin-qt]-->libshahcoin_wallet;

shahcoin-wallet[shahcoin-wallet]-->libshahcoin_wallet;
shahcoin-wallet[shahcoin-wallet]-->libshahcoin_wallet_tool;

libshahcoin_cli-->libshahcoin_util;
libshahcoin_cli-->libshahcoin_common;

libshahcoin_common-->libshahcoin_consensus;
libshahcoin_common-->libshahcoin_util;

libshahcoin_kernel-->libshahcoin_consensus;
libshahcoin_kernel-->libshahcoin_util;

libshahcoin_node-->libshahcoin_consensus;
libshahcoin_node-->libshahcoin_kernel;
libshahcoin_node-->libshahcoin_common;
libshahcoin_node-->libshahcoin_util;

libshahcoinqt-->libshahcoin_common;
libshahcoinqt-->libshahcoin_util;

libshahcoin_wallet-->libshahcoin_common;
libshahcoin_wallet-->libshahcoin_util;

libshahcoin_wallet_tool-->libshahcoin_wallet;
libshahcoin_wallet_tool-->libshahcoin_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class shahcoin-qt,shahcoind,shahcoin-cli,shahcoin-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Consensus* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libshahcoin_wallet* and *libshahcoin_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libshahcoin_consensus* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libshahcoin_util* should also be a standalone dependency that any library can depend on, and it should not depend on other internal libraries.

- *libshahcoin_common* should serve a similar function as *libshahcoin_util* and be a place for miscellaneous code used by various daemon, GUI, and CLI applications and libraries to live. It should not depend on anything other than *libshahcoin_util* and *libshahcoin_consensus*. The boundary between _util_ and _common_ is a little fuzzy but historically _util_ has been used for more generic, lower-level things like parsing hex, and _common_ has been used for shahcoin-specific, higher-level things like parsing base58. The difference between util and common is mostly important because *libshahcoin_kernel* is not supposed to depend on *libshahcoin_common*, only *libshahcoin_util*. In general, if it is ever unclear whether it is better to add code to *util* or *common*, it is probably better to add it to *common* unless it is very generically useful or useful particularly to include in the kernel.


- *libshahcoin_kernel* should only depend on *libshahcoin_util* and *libshahcoin_consensus*.

- The only thing that should depend on *libshahcoin_kernel* internally should be *libshahcoin_node*. GUI and wallet libraries *libshahcoinqt* and *libshahcoin_wallet* in particular should not depend on *libshahcoin_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be get able it from *libshahcoin_consensus*, *libshahcoin_common*, and *libshahcoin_util*, instead of *libshahcoin_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libshahcoinqt*, *libshahcoin_node*, *libshahcoin_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](`../../src/interfaces/`) abstract interfaces.

## Work in progress

- Validation code is moving from *libshahcoin_node* to *libshahcoin_kernel* as part of [The libshahcoinkernel Project #24303](https://github.com/shahcoin/shahcoin/issues/24303)
- Source code organization is discussed in general in [Library source code organization #15732](https://github.com/shahcoin/shahcoin/issues/15732)
