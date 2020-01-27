// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>
#include <consensus/validation.h>
#include <policy/policy.h>
#include <primitives/transaction.h>
#include <spork.h>

bool CheckTransaction(const CTransaction& tx, CValidationState &state, bool fCheckDuplicateInputs)
{
    bool allowEmptyTxInOut = false;
    if (tx.nType == TRANSACTION_COINBASE || tx.nType == TRANSACTION_STAKE || tx.nType == TRANSACTION_QUORUM_COMMITMENT) {
        allowEmptyTxInOut = true;
    }
    // Basic checks that don't depend on any context
    if (!allowEmptyTxInOut && tx.vin.empty()) {
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vin-empty");
    }
    if (!allowEmptyTxInOut && tx.vout.empty()) {
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-empty");
    }
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT) {
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-oversize");
    }
    if (tx.vExtraPayload.size() > MAX_TX_EXTRA_PAYLOAD) {
        return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-payload-oversize");
    }
    // Check for negative or overflow output values (see CVE-2010-5139)
    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout)
    {
        if (txout.nValue < 0)
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
    }
    // Check for duplicate inputs - note that this check is slow so we skip it in CheckBlock
    if (fCheckDuplicateInputs) {
        std::set<COutPoint> vInOutPoints;
        for (const auto& txin : tx.vin)
        {
            if (!vInOutPoints.insert(txin.prevout).second) {
                return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
            }
        }
    }

    if (tx.IsCoinBase()) {
        if (tx.vin[0].scriptSig.size() < 1 || tx.vin[0].scriptSig.size() > 100) {
            return state.Invalid(ValidationInvalidReason::CONSENSUS, false, REJECT_INVALID, "bad-cb-length");
        }
    }
    else
    {
        for (const auto& txin : tx.vin) {
            if (txin.prevout.IsNull()) {
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
                tx.nType != TRANSACTION_QUORUM_COMMITMENT &&
                tx.nType != TRANSACTION_STAKE) {
                return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-type");
            }
            if (tx.IsCoinStake() && tx.nType != TRANSACTION_STAKE) {
                return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-cb-type");
            }
        }
        else if (tx.nType != TRANSACTION_NORMAL) {
            if (sporkManager.IsSporkActive(SPORK_27_ENFORCE_STRICT_CBTX)) {
                return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-type");
            } else {
                LogPrintf("ContextualCheckTransaction(ZENTOSHI): spork is off, standard CBTX rules skipped\n");
            }
        }
    }

    // Size limits
    if (fDIP0001Active_context && ::GetSerializeSize(tx) > MAX_STANDARD_TX_WEIGHT) {
        return state.Invalid(ValidationInvalidReason::CBTX_INVALID, false, REJECT_INVALID, "bad-txns-oversize");
    }

    return true;
}
