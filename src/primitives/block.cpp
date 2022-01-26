// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <streams.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <crypto/balloon.h>
#include <primitives/fastsync.h>

uint256 CBlockHeader::GetHash() const
{
    if (this->nTime < 1643211906) {
        if (this->nTime < 1643042620) {
            if (this->hashMerkleRoot == uint256S("c54b432a10f01d5085395cfaf713fb4a512d4de58ef005180de44046d917fc88")) {
                return uint256S("00000df39444f013a2c22a9d25f74952dfc9c148dec9254a45d93ad093dad799");
            }
            return blockHashFromData(this->hashPrevBlock);
        }
        std::vector<unsigned char> vch(80);
        CVectorWriter ss(SER_NETWORK, PROTOCOL_VERSION, vch, 0);
        ss << *this;
	    if (this->nTime < 1624122000) { // OLD: 1624122000
	    	return HashX11((const char *)vch.data(), (const char *)vch.data() + vch.size());
	    } else {
	    	uint256 thash;
	    	alx_balloon((char*)vch.data(), &thash);
	    	return thash;
	    }
    } else {
        std::vector<unsigned char> vch(176);
        CVectorWriter ss(SER_NETWORK, PROTOCOL_VERSION, vch, 0);
        ss << *this;
        return HashX11((const char *)vch.data(), (const char *)vch.data() + vch.size());
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
