// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INDEX_TXINDEX_H
#define BITCOIN_INDEX_TXINDEX_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <threadinterrupt.h>
#include <txdb.h>
#include <uint256.h>
#include <validationinterface.h>

class CBlockIndex;

/**
 * TxIndex is used to look up transactions included in the blockchain by hash.
 * The index is written to a LevelDB database and records the filesystem
 * location of each transaction by transaction hash.
 */
class TxIndex final : public CValidationInterface
{
private:
    const std::unique_ptr<TxIndexDB> m_db;

public:
    /// Constructs the TxIndex, which becomes available to be queried.
    explicit TxIndex(std::unique_ptr<TxIndexDB> db);

    /// Destructor interrupts sync thread if running and blocks until it exits.
    ~TxIndex();

    /// Look up a transaction by hash.
    ///
    /// @param[in]   tx_hash  The hash of the transaction to be returned.
    /// @param[out]  block_hash  The hash of the block the transaction is found in.
    /// @param[out]  tx  The transaction itself.
    /// @return  true if transaction is found, false otherwise
    bool FindTx(const uint256& tx_hash, uint256& block_hash, CTransactionRef& tx) const;
    using IndexEntry = std::pair<uint256, CDiskTxPos>;
    bool WriteIndex(const std::vector<IndexEntry> &list);
};

/// The global transaction index, used in GetTransaction. May be null.
extern std::unique_ptr<TxIndex> g_txindex;

#endif // BITCOIN_INDEX_TXINDEX_H
