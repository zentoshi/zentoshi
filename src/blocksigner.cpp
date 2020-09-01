#include <blocksigner.h>
#include <messagesigner.h>
#include <primitives/block.h>
#include <script/sign.h>
#include <script/signingprovider.h>
#include <util/strencodings.h>
#include <util/system.h>
#include <wallet/wallet.h>

typedef std::vector<unsigned char> valtype;
bool SignBlock(CBlock& block, const CWallet& pwallet)
{
    std::vector<valtype> vSolutions;
    const CTxOut& txout = block.IsProofOfStake()? block.vtx[1]->vout[1] : block.vtx[0]->vout[0];
    txnouttype whichType = Solver(txout.scriptPubKey, vSolutions);

    CKeyID keyID;

    if (whichType == TX_PUBKEYHASH)
        keyID = CKeyID(uint160(vSolutions[0]));
    else if (whichType == TX_PUBKEY)
        keyID = CPubKey(vSolutions[0]).GetID();

    CKey key;
    if (!pwallet.GetKey(keyID, key)) return false;
    if (!key.Sign(block.GetHash(), block.vchBlockSig)) return false;
    return true;
}

bool CheckBlockSignature(const CBlock& block)
{
    if (block.IsProofOfWork())
        return block.vchBlockSig.empty();

    std::vector<valtype> vSolutions;
    txnouttype whichType;
    const CTxOut& txout = block.vtx[1]->vout[1];

    whichType = Solver(txout.scriptPubKey, vSolutions);
    valtype& vchPubKey = vSolutions[0];
    if (whichType == TX_PUBKEY)
    {
        CPubKey key(vchPubKey);
        if (block.vchBlockSig.empty()) return false;
        return key.Verify(block.GetHash(), block.vchBlockSig);
    }
    else if (whichType == TX_PUBKEYHASH)
    {
        CKeyID keyID;
        keyID = CKeyID(uint160(vchPubKey));
        CPubKey pubkey(vchPubKey);

        if (!pubkey.IsValid()) return false;
        if (block.vchBlockSig.empty()) return false;
        return pubkey.Verify(block.GetHash(), block.vchBlockSig);
    }

    return false;
}

