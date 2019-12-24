// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>
#include <consensus/validation.h>
#include <policy/policy.h>
#include <primitives/transaction.h>

bool CheckTransaction(const CTransaction& tx, CValidationState &state, bool fCheckDuplicateInputs)
{
    bool allowEmptyTxInOut = false;
    if (tx.nType == TRANSACTION_COINBASE || tx.nType == TRANSACTION_QUORUM_COMMITMENT) {
        allowEmptyTxInOut = true;
    }

    // Basic checks that don't depend on any context
    if (!allowEmptyTxInOut && tx.vin.empty()) {
        LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vin-empty");
    }
    if (!allowEmptyTxInOut && tx.vout.empty()) {
        LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-empty");
    }
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT) {
        LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-oversize");
    }
    if (tx.vExtraPayload.size() > MAX_TX_EXTRA_PAYLOAD) {
        LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-payload-oversize");
    }

    // Check for negative or overflow output values (see CVE-2010-5139)
    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout)
    {
        if (txout.nValue < 0) {
            LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-negative");
        }
        if (txout.nValue > MAX_MONEY) {
            LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-toolarge");
        }
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut)) {
            LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
        }
    }

    // Check for duplicate inputs - note that this check is slow so we skip it in CheckBlock
    if (fCheckDuplicateInputs) {
        std::set<COutPoint> vInOutPoints;
        for (const auto& txin : tx.vin)
        {
            if (!vInOutPoints.insert(txin.prevout).second) {
                LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
                return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
            }
        }
    }

    if (tx.IsCoinBase()) {
        if (tx.vin[0].scriptSig.size() < 1 || tx.vin[0].scriptSig.size() > 100) {
            LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-cb-length");
        }
    }
    else
    {
        for (const auto& txin : tx.vin) {
            if (txin.prevout.IsNull()) {
                LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
                return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-prevout-null");
            }
        }
    }
    return true;
}

bool ContextualCheckTransaction(const CTransaction& tx, CValidationState &state, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{
    int nHeight = pindexPrev == nullptr ? 0 : pindexPrev->nHeight + 1;
    bool fDIP0001Active_context = nHeight >= consensusParams.DIP0001Height;
    bool fDIP0003Active_context = nHeight >= consensusParams.DIP0003Height;

    if (fDIP0003Active_context) {
        // check version 3 transaction types
        if (tx.nVersion >= 3) {
            if (tx.nType != TRANSACTION_NORMAL &&
                tx.nType != TRANSACTION_PROVIDER_REGISTER &&
                tx.nType != TRANSACTION_PROVIDER_UPDATE_SERVICE &&
                tx.nType != TRANSACTION_PROVIDER_UPDATE_REGISTRAR &&
                tx.nType != TRANSACTION_PROVIDER_UPDATE_REVOKE &&
                tx.nType != TRANSACTION_COINBASE &&
                tx.nType != TRANSACTION_QUORUM_COMMITMENT) {
                LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
                return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-type");
            }
            if (tx.IsCoinBase() && tx.nType != TRANSACTION_COINBASE) {
                LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
                return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-cb-type");
            }
        } else if (tx.nType != TRANSACTION_NORMAL) {
            LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
            return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-type");
        }
    }

    // Size limits
    if (fDIP0001Active_context && ::GetSerializeSize(tx) > MAX_STANDARD_TX_WEIGHT) {
        LogPrintf("Invalid State at line %d in file %s\n", __LINE__, __FILE__);
        return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-oversize");
    }

    return true;
}
