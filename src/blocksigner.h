#ifndef BLOCKSIGNER_H
#define BLOCKSIGNER_H

#include <key.h>
#include <primitives/block.h>
#include <script/signingprovider.h>

class CBlock;
class CPubKey;
class CKey;
class CWallet;

bool SignBlock(CBlock& block, const CWallet& pwallet);
bool CheckBlockSignature(const CBlock& block);

#endif // BLOCKSIGNER_H
