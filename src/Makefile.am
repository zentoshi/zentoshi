# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(WARN_CXXFLAGS) $(NOWARN_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
AM_LIBTOOLFLAGS = --preserve-dup-deps
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

BITCOIN_CONFIG_INCLUDES=-I$(builddir)/config
BITCOIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/include
BITCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

BLS_LIBS=-lchiabls -lgmp

LIBBITCOIN_SERVER=libzentoshi_server.a
LIBBITCOIN_COMMON=libzentoshi_common.a
LIBBITCOIN_CONSENSUS=libzentoshi_consensus.a
LIBBITCOIN_CLI=libzentoshi_cli.a
LIBBITCOIN_UTIL=libzentoshi_util.a
LIBBITCOIN_CRYPTO_BASE=crypto/libzentoshi_crypto_base.a
LIBBITCOINQT=qt/libbitcoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBBITCOIN_ZMQ=libzentoshi_zmq.a
endif
if BUILD_BITCOIN_LIBS
LIBBITCOINCONSENSUS=libzentoshiconsensus.la
endif
if ENABLE_WALLET
LIBBITCOIN_WALLET=libzentoshi_wallet.a
LIBBITCOIN_WALLET_TOOL=libzentoshi_wallet_tool.a
endif

LIBBITCOIN_CRYPTO= $(LIBBITCOIN_CRYPTO_BASE)
if ENABLE_SSE41
LIBBITCOIN_CRYPTO_SSE41 = crypto/libzentoshi_crypto_sse41.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_SSE41)
endif
if ENABLE_AVX2
LIBBITCOIN_CRYPTO_AVX2 = crypto/libzentoshi_crypto_avx2.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_AVX2)
endif
if ENABLE_SHANI
LIBBITCOIN_CRYPTO_SHANI = crypto/libzentoshi_crypto_shani.a
LIBBITCOIN_CRYPTO += $(LIBBITCOIN_CRYPTO_SHANI)
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_CLI) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_WALLET_TOOL) \
  $(LIBBITCOIN_ZMQ)

lib_LTLIBRARIES = $(LIBBITCOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BITCOIND
  bin_PROGRAMS += zentoshid
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += zentoshi-cli
endif
if BUILD_BITCOIN_TX
  bin_PROGRAMS += zentoshi-tx
endif

.PHONY: FORCE check-symbols check-security
# bitcoin core #
BITCOIN_CORE_H = \
  activemasternode.h \
  addrdb.h \
  addrman.h \
  alert.h \
  attributes.h \
  banman.h \
  base58.h \
  bech32.h \
  bloom.h \
  blocksigner.h \
  blockencodings.h \
  blockfilter.h \
  cachemap.h \
  cachemultimap.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/assumptions.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_verify.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  ctpl.h \
  cxxtimer.hpp \
  evo/evodb.h \
  evo/specialtx.h \
  evo/providertx.h \
  evo/deterministicmns.h \
  evo/cbtx.h \
  evo/simplifiedmns.h \
  dsnotificationinterface.h \
  fs.h \
  flat-database.h \
  governance/governance.h \
  governance/governance-classes.h \
  governance/governance-exceptions.h \
  governance/governance-object.h \
  governance/governance-validators.h \
  governance/governance-vote.h \
  governance/governance-votedb.h \
  httprpc.h \
  httpserver.h \
  index/base.h \
  index/txindex.h \
  indirectmap.h \
  init.h \
  instantx.h \
  interfaces/chain.h \
  interfaces/handler.h \
  interfaces/node.h \
  interfaces/wallet.h \
  key.h \
  key_io.h \
  keystore.h \
  kernel.h \
  dbwrapper.h \
  limitedmap.h \
  logging.h \
  llmq/quorums_commitment.h \
  llmq/quorums_blockprocessor.h \
  llmq/quorums_dummydkg.h \
  llmq/quorums_utils.h \
  llmq/quorums_init.h \
  masternode.h \
  masternode-payments.h \
  masternode-sync.h \
  masternodeman.h \
  masternodeconfig.h \
  messagesigner.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_processing.h \
  netfulfilledman.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  node/transaction.h \
  noui.h \
  optional.h \
  outputtype.h \
  policy/feerate.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  pow.h \
  protocol.h \
  psbt.h \
  privatesend/privatesend.h \
  privatesend/privatesend-client.h \
  privatesend/privatesend-server.h \
  privatesend/privatesend-util.h \
  random.h \
  reverse_iterator.h \
  reverselock.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/mining.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/rawtransaction.h \
  rpc/register.h \
  rpc/util.h \
  scheduler.h \
  script/descriptor.h \
  script/ismine.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  shutdown.h \
  streams.h \
  support/allocators/mt_pooled_secure.h \
  support/allocators/pooled_secure.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  spork.h \
  threadsafety.h \
  threadinterrupt.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  undo.h \
  util/bip32.h \
  util/bytevectorhash.h \
  util/system.h \
  util/memory.h \
  util/moneystr.h \
  util/time.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  versionbitsinfo.h \
  walletinitinterface.h \
  wallet/coincontrol.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/feebumper.h \
  wallet/fees.h \
  wallet/psbtwallet.h \
  wallet/rpcwallet.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallettool.h \
  wallet/walletutil.h \
  wallet/coinselection.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  zmq/zmqrpc.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libzentoshi_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between zentoshid and zentoshi-qt
libzentoshi_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libzentoshi_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_server_a_SOURCES = \
  activemasternode.cpp \
  addrdb.cpp \
  addrman.cpp \
  alert.cpp \
  banman.cpp \
  bloom.cpp \
  blocksigner.cpp \
  blockencodings.cpp \
  blockfilter.cpp \
  chain.cpp \
  checkpoints.cpp \
  consensus/tx_verify.cpp \
  dsnotificationinterface.cpp \
  evo/evodb.cpp \
  evo/specialtx.cpp \
  evo/providertx.cpp \
  evo/deterministicmns.cpp \
  evo/cbtx.cpp \
  evo/simplifiedmns.cpp \
  httprpc.cpp \
  httpserver.cpp \
  index/base.cpp \
  index/txindex.cpp \
  interfaces/chain.cpp \
  interfaces/handler.cpp \
  interfaces/node.cpp \
  init.cpp \
  instantx.cpp \
  kernel.cpp \
  dbwrapper.cpp \
  governance/governance.cpp \
  governance/governance-classes.cpp \
  governance/governance-object.cpp \
  governance/governance-validators.cpp \
  governance/governance-vote.cpp \
  governance/governance-votedb.cpp \
  llmq/quorums_commitment.cpp \
  llmq/quorums_blockprocessor.cpp \
  llmq/quorums_dummydkg.cpp \
  llmq/quorums_utils.cpp \
  llmq/quorums_init.cpp \
  masternode.cpp \
  masternode-payments.cpp \
  masternode-sync.cpp \
  masternodeconfig.cpp \
  masternodeman.cpp \
  merkleblock.cpp \
  messagesigner.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  netfulfilledman.cpp \
  node/transaction.cpp \
  noui.cpp \
  outputtype.cpp \
  privatesend/privatesend.cpp \
  privatesend/privatesend-client.cpp \
  privatesend/privatesend-server.cpp \
  privatesend/privatesend-util.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  policy/rbf.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/governance.cpp\
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/masternode.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  rpc/util.cpp \
  script/sigcache.cpp \
  shutdown.cpp \
  spork.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(BITCOIN_CORE_H)

if !ENABLE_WALLET
libzentoshi_server_a_SOURCES += dummywallet.cpp
endif

if ENABLE_ZMQ
libzentoshi_zmq_a_CPPFLAGS = $(BITCOIN_INCLUDES) $(ZMQ_CFLAGS)
libzentoshi_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp \
  zmq/zmqrpc.cpp
endif


# wallet: shared between zentoshid and zentoshi-qt, but only linked
# when wallet enabled
libzentoshi_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_wallet_a_SOURCES = \
  outputtype.cpp \
  interfaces/wallet.cpp \
  wallet/coincontrol.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/feebumper.cpp \
  wallet/fees.cpp \
  wallet/init.cpp \
  wallet/psbtwallet.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  wallet/walletutil.cpp \
  wallet/coinselection.cpp \
  $(BITCOIN_CORE_H)

libzentoshi_wallet_tool_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_wallet_tool_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_wallet_tool_a_SOURCES = \
  wallet/wallettool.cpp \
  $(BITCOIN_CORE_H)

# crypto primitives library
crypto_libzentoshi_crypto_base_a_CPPFLAGS = $(AM_CPPFLAGS) $(SSL_CFLAGS)
crypto_libzentoshi_crypto_base_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libzentoshi_crypto_base_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/siphash.cpp \
  crypto/siphash.h

if USE_ASM
crypto_libzentoshi_crypto_base_a_SOURCES += crypto/sha256_sse4.cpp
endif

crypto_libzentoshi_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libzentoshi_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libzentoshi_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libzentoshi_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
crypto_libzentoshi_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libzentoshi_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libzentoshi_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libzentoshi_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libzentoshi_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
crypto_libzentoshi_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

crypto_libzentoshi_crypto_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libzentoshi_crypto_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libzentoshi_crypto_shani_a_CXXFLAGS += $(SHANI_CXXFLAGS)
crypto_libzentoshi_crypto_shani_a_CPPFLAGS += -DENABLE_SHANI
crypto_libzentoshi_crypto_shani_a_SOURCES = crypto/sha256_shani.cpp

# consensus: shared between all executables that validate any consensus rules.
libzentoshi_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  bls/bls.cpp \
  bls/bls.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/bitcoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  span.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h

# common: shared between zentoshid, and zentoshi-qt and non-server tools
libzentoshi_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  key_io.cpp \
  keystore.cpp \
  netaddress.cpp \
  netbase.cpp \
  policy/feerate.cpp \
  psbt.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/descriptor.cpp \
  script/ismine.cpp \
  script/sign.cpp \
  script/standard.cpp \
  versionbitsinfo.cpp \
  warnings.cpp \
  $(BITCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libzentoshi_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_util_a_SOURCES = \
  bls/bls_ies.cpp \
  bls/bls_ies.h \
  bls/bls_worker.cpp \
  bls/bls_worker.h \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  logging.cpp \
  random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util/bip32.cpp \
  util/bytevectorhash.cpp \
  util/system.cpp \
  util/moneystr.cpp \
  util/strencodings.cpp \
  util/time.cpp \
  $(BITCOIN_CORE_H)

if GLIBC_BACK_COMPAT
libzentoshi_util_a_SOURCES += compat/glibc_compat.cpp
AM_LDFLAGS += $(COMPAT_LDFLAGS)
endif

# cli: shared between zentoshi-cli and zentoshi-qt
libzentoshi_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libzentoshi_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libzentoshi_cli_a_SOURCES = \
  rpc/client.cpp \
  $(BITCOIN_CORE_H)

nodist_libzentoshi_util_a_SOURCES = $(srcdir)/obj/build.h
#

# zentoshid binary #
zentoshid_SOURCES = zentoshid.cpp
zentoshid_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
zentoshid_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zentoshid_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zentoshid_SOURCES += zentoshid-res.rc
endif

zentoshid_LDADD = \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

zentoshid_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(BLS_LIBS)

# zentoshi-cli binary #
zentoshi_cli_SOURCES = zentoshi-cli.cpp
zentoshi_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS)
zentoshi_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zentoshi_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zentoshi_cli_SOURCES += zentoshi-cli-res.rc
endif

zentoshi_cli_LDADD = \
  $(LIBBITCOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CRYPTO)

zentoshi_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS) $(BLS_LIBS)
#

# zentoshi-tx binary #
zentoshi_tx_SOURCES = zentoshi-tx.cpp
zentoshi_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
zentoshi_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
zentoshi_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
zentoshi_tx_SOURCES += zentoshi-tx-res.rc
endif

zentoshi_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBSECP256K1)

zentoshi_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS) $(BLS_LIBS)

# zentoshi-wallet binary #
#zentoshi_wallet_SOURCES = zentoshi-wallet.cpp
#zentoshi_wallet_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
#zentoshi_wallet_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
#zentoshi_wallet_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

#if TARGET_WINDOWS
#zentoshi_wallet_SOURCES += zentoshi-wallet-res.rc
#endif

#zentoshi_wallet_LDADD = \
# $(LIBBITCOIN_WALLET_TOOL) \
# $(LIBBITCOIN_WALLET) \
# $(LIBBITCOIN_SERVER) \
# $(LIBBITCOIN_COMMON) \
# $(LIBBITCOIN_CONSENSUS) \
# $(LIBBITCOIN_UTIL) \
# $(LIBBITCOIN_CRYPTO) \
# $(LIBLEVELDB) \
# $(LIBLEVELDB_SSE42) \
# $(LIBMEMENV) \
# $(LIBSECP256K1)

#zentoshi_wallet_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS)

# bitcoinconsensus library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/bitcoinconsensus.h
libzentoshiconsensus_la_SOURCES = $(crypto_libzentoshi_crypto_base_a_SOURCES) $(libzentoshi_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libzentoshiconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libzentoshiconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libzentoshiconsensus_la_LIBADD = $(LIBSECP256K1) $(BLS_LIBS)
libzentoshiconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BITCOIN_INTERNAL $(SSL_CFLAGS)
libzentoshiconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += obj/build.h

IMMER_DIST = immer
EXTRA_DIST = $(CTAES_DIST)
EXTRA_DIST += $(IMMER_DIST)


config/zentoshi-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/zentoshi-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/zentoshi-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/zentoshi-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

if ENABLE_BIP70
%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<
endif

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_QT
include Makefile.qt.include
endif
