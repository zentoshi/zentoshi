// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <chainparams.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/system.h>
#include <validation.h>

const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake)) {
        pindex = pindex->pprev;
    }
    return pindex;
}

unsigned int DualKGW3(const CBlockIndex* pindexLast, const Consensus::Params& params, bool fProofOfStake)
{
    const CBlockIndex* BlockLastSolved = GetLastBlockIndex(pindexLast, fProofOfStake);
    const CBlockIndex* BlockReading = GetLastBlockIndex(pindexLast, fProofOfStake);
    int64_t PastBlocksMass = 0;
    int64_t PastRateActualSeconds = 0;
    int64_t PastRateTargetSeconds = 0;
    double PastRateAdjustmentRatio = double(1);
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;
    double EventHorizonDeviation;
    double EventHorizonDeviationFast;
    double EventHorizonDeviationSlow;
    static const int64_t Resolution = 20;
    static const int64_t Blocktime = fProofOfStake ? params.nPosTargetSpacing : params.nPowTargetSpacing;
    static const unsigned int timeDaySeconds = 86400;
    uint64_t pastSecondsMin = timeDaySeconds * 0.025;
    uint64_t pastSecondsMax = timeDaySeconds * 7;
    uint64_t PastBlocksMin = pastSecondsMin / Blocktime;
    uint64_t PastBlocksMax = pastSecondsMax / Blocktime;
    const arith_uint256 bnLimit = fProofOfStake ? UintToArith256(params.posLimit) : UintToArith256(params.powLimit);

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 ||
            (uint64_t)BlockLastSolved->nHeight < PastBlocksMin)
        return bnLimit.GetCompact();

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) break;
        PastBlocksMass++;
        PastDifficultyAverage.SetCompact(BlockReading->nBits);
        if (i > 1) {
            if (PastDifficultyAverage >= PastDifficultyAveragePrev)
                PastDifficultyAverage =
                        ((PastDifficultyAverage - PastDifficultyAveragePrev) / i) +
                        PastDifficultyAveragePrev;
            else
                PastDifficultyAverage =
                        PastDifficultyAveragePrev -
                        ((PastDifficultyAveragePrev - PastDifficultyAverage) / i);
        }
        PastDifficultyAveragePrev = PastDifficultyAverage;
        PastRateActualSeconds =
                BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds = Blocktime * PastBlocksMass;
        PastRateAdjustmentRatio = double(1);
        if (PastRateActualSeconds < 0) PastRateActualSeconds = 0;
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0)
            PastRateAdjustmentRatio =
                    double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        EventHorizonDeviation =
                1 + (0.7084 * pow((double(PastBlocksMass) / double(72)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
            if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) ||
                    (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) {
                assert(BlockReading);
                break;
            }
        }
        if (BlockReading->pprev == nullptr) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 kgw_dual1(PastDifficultyAverage);
    arith_uint256 kgw_dual2;
    kgw_dual2.SetCompact(GetLastBlockIndex(pindexLast, fProofOfStake)->nBits);
    if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
        kgw_dual1 *= PastRateActualSeconds;
        kgw_dual1 /= PastRateTargetSeconds;
    }

    int64_t nActualTime = GetLastBlockIndex(pindexLast, fProofOfStake)->GetBlockTime() -
                          GetLastBlockIndex(pindexLast, fProofOfStake)->pprev->GetBlockTime();
    int64_t nActualTimespanshort = nActualTime;
    if (nActualTime < 0)
        nActualTime = Blocktime;
    if (nActualTime < Blocktime / Resolution)
        nActualTime = Blocktime / Resolution;
    if (nActualTime > Blocktime * Resolution)
        nActualTime = Blocktime * Resolution;
    kgw_dual2 *= nActualTime;
    kgw_dual2 /= Blocktime;
    arith_uint256 bnNew;
    bnNew = ((kgw_dual2 + kgw_dual1) / 2);

    if (nActualTimespanshort < Blocktime / 6) {
        const int nLongShortNew1 = 85;
        const int nLongShortNew2 = 100;
        bnNew = bnNew * nLongShortNew1;
        bnNew = bnNew / nLongShortNew2;
    }

    if (bnNew > bnLimit) bnNew = bnLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params, bool fProofOfStake)
{
    return DualKGW3(pindexLast, params, fProofOfStake);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // LogPrintf("hash %s target %s\n", hash.ToString().c_str(), bnTarget.ToString().c_str());

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget) {
        return false;
    }

    return true;
}
