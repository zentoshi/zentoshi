// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2014-2019 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <clientversion.h>
#include <core_io.h>
#include <crypto/ripemd160.h>
#include <key_io.h>
#include <validation.h>
#include <httpserver.h>
#include <net.h>
#include <netbase.h>
#include <outputtype.h>
#include <rpc/blockchain.h>
#include <rpc/server.h>
#include <rpc/util.h>
#include <script/descriptor.h>
#include <timedata.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <util/validation.h>
#include <warnings.h>

#include <stdint.h>
#include <tuple>
#ifdef HAVE_MALLOC_INFO
#include <malloc.h>
#endif

#include <univalue.h>

extern int64_t nLastCoinStakeSearchInterval;

class CWallet;

/*
    Used for updating/reading spork settings on the network
*/
UniValue spork(const JSONRPCRequest& request)
{
    if (request.params.size() == 1) {
        // basic mode, show info
        std:: string strCommand = request.params[0].get_str();
        if (strCommand == "show") {
            UniValue ret(UniValue::VOBJ);
            for (const auto& sporkDef : sporkDefs) {
                ret.push_back(Pair(sporkDef.name, sporkManager.GetSporkValue(sporkDef.sporkId)));
            }
            return ret;
        } else if(strCommand == "active"){
            UniValue ret(UniValue::VOBJ);
            for (const auto& sporkDef : sporkDefs) {
                ret.push_back(Pair(sporkDef.name, sporkManager.IsSporkActive(sporkDef.sporkId)));
            }
            return ret;
        }
    }

    if (request.fHelp || request.params.size() != 2) {
        // default help, for basic mode
        throw std::runtime_error(
            "spork \"command\"\n"
            "\nShows information about current state of sporks\n"
            "\nArguments:\n"
            "1. \"command\"                     (string, required) 'show' to show all current spork values, 'active' to show which sporks are active\n"
            "\nResult:\n"
            "For 'show':\n"
            "{\n"
            "  \"SPORK_NAME\" : spork_value,    (number) The value of the specific spork with the name SPORK_NAME\n"
            "  ...\n"
            "}\n"
            "For 'active':\n"
            "{\n"
            "  \"SPORK_NAME\" : true|false,     (boolean) 'true' for time-based sporks if spork is active and 'false' otherwise\n"
            "  ...\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("spork", "show")
            + HelpExampleRpc("spork", "\"show\""));
    } else {
        // advanced mode, update spork values
        SporkId nSporkID = sporkManager.GetSporkIDByName(request.params[0].get_str());
        if(nSporkID == SPORK_INVALID)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid spork name");

        if (!g_connman)
            throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

        // SPORK VALUE
        int64_t nValue = request.params[1].get_int64();

        //broadcast new spork
        if(sporkManager.UpdateSpork(nSporkID, nValue, *g_connman)){
            return "success";
        } else {
            throw std::runtime_error(
                "spork \"name\" value\n"
                "\nUpdate the value of the specific spork. Requires \"-sporkkey\" to be set to sign the message.\n"
                "\nArguments:\n"
                "1. \"name\"              (string, required) The name of the spork to update\n"
                "2. value               (number, required) The new desired value of the spork\n"
                "\nResult:\n"
                "  result               (string) \"success\" if spork value was updated or this help otherwise\n"
                "\nExamples:\n"
                + HelpExampleCli("spork", "SPORK_2_INSTANTSEND_ENABLED 4070908800")
                + HelpExampleRpc("spork", "\"SPORK_2_INSTANTSEND_ENABLED\", 4070908800"));
        }
    }

}

UniValue mnsync(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "mnsync [status|next|reset]\n"
            "Returns the sync status, updates to the next step or resets it entirely.\n"
        );

    std::string strMode = request.params[0].get_str();

    if(strMode == "status") {
        UniValue objStatus(UniValue::VOBJ);
        objStatus.push_back(Pair("AssetID", masternodeSync.GetAssetID()));
        objStatus.push_back(Pair("AssetName", masternodeSync.GetAssetName()));
        objStatus.push_back(Pair("AssetStartTime", masternodeSync.GetAssetStartTime()));
        objStatus.push_back(Pair("Attempt", masternodeSync.GetAttempt()));
        objStatus.push_back(Pair("IsBlockchainSynced", masternodeSync.IsBlockchainSynced()));
        objStatus.push_back(Pair("IsSynced", masternodeSync.IsSynced()));
        objStatus.push_back(Pair("IsFailed", masternodeSync.IsFailed()));
        return objStatus;
    }

    if(strMode == "next")
    {
        masternodeSync.SwitchToNextAsset(*g_connman);
        return "sync updated to " + masternodeSync.GetAssetName();
    }

    if(strMode == "reset")
    {
        masternodeSync.Reset();
        masternodeSync.SwitchToNextAsset(*g_connman);
        return "success";
    }
    return "failure";
}

#ifdef ENABLE_WALLET
class DescribeAddressVisitor : public boost::static_visitor<UniValue>
{
public:
    CWallet * const pwallet;

    DescribeAddressVisitor(CWallet *_pwallet) : pwallet(_pwallet) {}

    UniValue operator()(const CNoDestination &dest) const { return UniValue(UniValue::VOBJ); }

    UniValue operator()(const CKeyID &keyID) const {
        UniValue obj(UniValue::VOBJ);
        CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
        if (pwallet && pwallet->GetPubKey(keyID, vchPubKey)) {
            obj.push_back(Pair("pubkey", HexStr(vchPubKey)));
            obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
        }
        return obj;
    }

    FlatSigningProvider key_provider;
    auto desc = Parse(desc_str, key_provider, /* require_checksum = */ true);
    if (!desc) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Invalid descriptor"));
    }
};
#endif

/**
 * Used by addmultisigaddress / createmultisig:
 */
CScript _createmultisig_redeemScript(CWallet * const pwallet, const UniValue& params)
{
    int nRequired = params[0].get_int();
    const UniValue& keys = params[1].get_array();

    // Gather public keys
    if (nRequired < 1)
        throw std::runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw std::runtime_error(
            strprintf("not enough keys supplied "
                      "(got %u keys, but need at least %d to redeem)", keys.size(), nRequired));
    if (keys.size() > 16)
        throw std::runtime_error("Number of addresses involved in the multisignature address creation > 16\nReduce the number");
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        const std::string& ks = keys[i].get_str();
#ifdef ENABLE_WALLET
        // Case 1: Dash address and we have full public key:
        CBitcoinAddress address(ks);
        if (pwallet && address.IsValid()) {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
                throw std::runtime_error(
                    strprintf("%s does not refer to a key",ks));
            CPubKey vchPubKey;
            if (!pwallet->GetPubKey(keyID, vchPubKey)) {
                throw std::runtime_error(
                    strprintf("no full public key for address %s",ks));
            }
            if (!vchPubKey.IsFullyValid())
                throw std::runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }

        // Case 2: hex public key
        else
#endif
        if (IsHex(ks))
        {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsFullyValid())
                throw std::runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }
        else
        {
            throw std::runtime_error(" Invalid public key: "+ks);
        }
    }
    CScript result = GetScriptForMultisig(nRequired, pubkeys);

    if (result.size() > MAX_SCRIPT_ELEMENT_SIZE)
        throw std::runtime_error(
                strprintf("redeemScript exceeds size limit: %d > %d", result.size(), MAX_SCRIPT_ELEMENT_SIZE));

    return result;
}

static UniValue createmultisig(const JSONRPCRequest& request)
{
            RPCHelpMan{"createmultisig",
                "\nCreates a multi-signature address with n signature of m keys required.\n"
                "It returns a json object with the address and redeemScript.\n",
                {
                    {"nrequired", RPCArg::Type::NUM, RPCArg::Optional::NO, "The number of required signatures out of the n keys."},
                    {"keys", RPCArg::Type::ARR, RPCArg::Optional::NO, "A json array of hex-encoded public keys.",
                        {
                            {"key", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "The hex-encoded public key"},
                        }},
                    {"address_type", RPCArg::Type::STR, /* default */ "legacy", "The address type to use. Options are \"legacy\", \"p2sh-segwit\", and \"bech32\"."},
                },
                RPCResult{
            "{\n"
            "  \"address\":\"multisigaddress\",  (string) The value of the new multisig address.\n"
            "  \"redeemScript\":\"script\"       (string) The string value of the hex-encoded redemption script.\n"
            "}\n"
                },
                RPCExamples{
            "\nCreate a multisig address from 2 public keys\n"
            + HelpExampleCli("createmultisig", "2 \"[\\\"03789ed0bb717d88f7d321a368d905e7430207ebbd82bd342cf11ae157a7ace5fd\\\",\\\"03dbc6764b8884a92e871274b87583e6d5c2a58819473e17e107ef3f6aa5a61626\\\"]\"") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("createmultisig", "2, \"[\\\"03789ed0bb717d88f7d321a368d905e7430207ebbd82bd342cf11ae157a7ace5fd\\\",\\\"03dbc6764b8884a92e871274b87583e6d5c2a58819473e17e107ef3f6aa5a61626\\\"]\"")
                },
            }.Check(request);

    int required = request.params[0].get_int();

    // Get the public keys
    const UniValue& keys = request.params[1].get_array();
    std::vector<CPubKey> pubkeys;
    for (unsigned int i = 0; i < keys.size(); ++i) {
        if (IsHex(keys[i].get_str()) && (keys[i].get_str().length() == 66 || keys[i].get_str().length() == 130)) {
            pubkeys.push_back(HexToPubKey(keys[i].get_str()));
        } else {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Invalid public key: %s\n.", keys[i].get_str()));
        }
    }

    // Get the output type
    OutputType output_type = OutputType::LEGACY;
    if (!request.params[2].isNull()) {
        if (!ParseOutputType(request.params[2].get_str(), output_type)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Unknown address type '%s'", request.params[2].get_str()));
        }
    }

    // Construct using pay-to-script-hash:
    FillableSigningProvider keystore;
    CScript inner;
    const CTxDestination dest = AddAndGetMultisigDestination(required, pubkeys, output_type, keystore, inner);

    UniValue result(UniValue::VOBJ);
    result.pushKV("address", EncodeDestination(dest));
    result.pushKV("redeemScript", HexStr(inner.begin(), inner.end()));

    return result;
}

UniValue getdescriptorinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1) {
        throw std::runtime_error(
            RPCHelpMan{"getdescriptorinfo",
            {"\nAnalyses a descriptor.\n"},
            {
                {"descriptor", RPCArg::Type::STR, RPCArg::Optional::NO, "The descriptor."},
            },
            RPCResult{
            "{\n"
            "  \"descriptor\" : \"desc\",         (string) The descriptor in canonical form, without private keys\n"
            "  \"isrange\" : true|false,        (boolean) Whether the descriptor is ranged\n"
            "  \"issolvable\" : true|false,     (boolean) Whether the descriptor is solvable\n"
            "  \"hasprivatekeys\" : true|false, (boolean) Whether the input descriptor contained at least one private key\n"
            "}\n"
            },
            RPCExamples{
                "Analyse a descriptor\n" +
                HelpExampleCli("getdescriptorinfo", "\"wpkh([d34db33f/84h/0h/0h]0279be667ef9dcbbac55a06295Ce870b07029Bfcdb2dce28d959f2815b16f81798)\"")
            }}.ToString()
        );
    }

    RPCTypeCheck(request.params, {UniValue::VSTR});

    FlatSigningProvider provider;
    auto desc = Parse(request.params[0].get_str(), provider);
    if (!desc) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Invalid descriptor"));
    }

    UniValue result(UniValue::VOBJ);
    result.pushKV("descriptor", desc->ToString());
    result.pushKV("isrange", desc->IsRange());
    result.pushKV("issolvable", desc->IsSolvable());
    result.pushKV("hasprivatekeys", provider.keys.size() > 0);
    return result;
}

UniValue deriveaddresses(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.empty() || request.params.size() > 2) {
        throw std::runtime_error(
            RPCHelpMan{"deriveaddresses",
            {"\nDerives one or more addresses corresponding to an output descriptor.\n"
            "Examples of output descriptors are:\n"
            "    pkh(<pubkey>)                        P2PKH outputs for the given pubkey\n"
            "    wpkh(<pubkey>)                       Native segwit P2PKH outputs for the given pubkey\n"
            "    sh(multi(<n>,<pubkey>,<pubkey>,...)) P2SH-multisig outputs for the given threshold and pubkeys\n"
            "    raw(<hex script>)                    Outputs whose scriptPubKey equals the specified hex scripts\n"
            "\nIn the above, <pubkey> either refers to a fixed public key in hexadecimal notation, or to an xpub/xprv optionally followed by one\n"
            "or more path elements separated by \"/\", where \"h\" represents a hardened child key.\n"
            "For more information on output descriptors, see the documentation in the doc/descriptors.md file.\n"},
            {
                {"descriptor", RPCArg::Type::STR, RPCArg::Optional::NO, "The descriptor."},
                {"range", RPCArg::Type::RANGE, RPCArg::Optional::OMITTED_NAMED_ARG, "If a ranged descriptor is used, this specifies the end or the range (in [begin,end] notation) to derive."},
            },
            RPCResult{
                "[ address ] (array) the derived addresses\n"
            },
            RPCExamples{
                "First three native segwit receive addresses\n" +
                HelpExampleCli("deriveaddresses", "\"wpkh([d34db33f/84h/0h/0h]xpub6DJ2dNUysrn5Vt36jH2KLBT2i1auw1tTSSomg8PhqNiUtx8QX2SvC9nrHu81fT41fvDUnhMjEzQgXnQjKEu3oaqMSzhSrHMxyyoEAmUHQbY/0/*)#cjjspncu\" \"[0,2]\"")
            }}.ToString()
        );
    }

    RPCTypeCheck(request.params, {UniValue::VSTR, UniValueType()}); // Range argument is checked later
    const std::string desc_str = request.params[0].get_str();

    int64_t range_begin = 0;
    int64_t range_end = 0;

    if (request.params.size() >= 2 && !request.params[1].isNull()) {
        std::tie(range_begin, range_end) = ParseDescriptorRange(request.params[1]);
    }

    FlatSigningProvider provider;
    auto desc = Parse(desc_str, provider, /* require_checksum = */ true);
    if (!desc) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Invalid descriptor"));
    }

    if (!desc->IsRange() && request.params.size() > 1) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Range should not be specified for an un-ranged descriptor");
    }

    if (desc->IsRange() && request.params.size() == 1) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Range must be specified for a ranged descriptor");
    }

    UniValue addresses(UniValue::VARR);

    for (int i = range_begin; i <= range_end; ++i) {
        std::vector<CScript> scripts;
        if (!desc->Expand(i, provider, scripts, provider)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Cannot derive script without private keys"));
        }

        for (const CScript &script : scripts) {
            CTxDestination dest;
            if (!ExtractDestination(script, dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Descriptor does not have a corresponding address"));
            }

            addresses.push_back(EncodeDestination(dest));
        }
    }

    // This should not be possible, but an assert seems overkill:
    if (addresses.empty()) {
        throw JSONRPCError(RPC_MISC_ERROR, "Unexpected empty result");
    }

    return addresses;
}

static UniValue verifymessage(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 3)
        throw std::runtime_error(
            "verifymessage \"address\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The zentoshi address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"
            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"XwnLY9Tf7Zsef8gMGL2fhWA9ZmMjt4KPwG\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"XwnLY9Tf7Zsef8gMGL2fhWA9ZmMjt4KPwG\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("verifymessage", "\"XwnLY9Tf7Zsef8gMGL2fhWA9ZmMjt4KPwG\", \"signature\", \"my message\"")
        );

    LOCK(cs_main);

    std::string strAddress  = request.params[0].get_str();
    std::string strSign     = request.params[1].get_str();
    std::string strMessage  = request.params[2].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    const PKHash *pkhash = boost::get<PKHash>(&destination);
    if (!pkhash) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    CPubKey::InputScriptType inputScriptType;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig, inputScriptType))
        return false;

    return (pubkey.GetID() == *pkhash);
}

UniValue signmessagewithprivkey(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "signmessagewithprivkey \"privkey\" \"message\"\n"
            "\nSign a message with the private key of an address\n"
            "\nArguments:\n"
            "1. \"privkey\"         (string, required) The private key to sign the message with.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nCreate the signature\n"
            + HelpExampleCli("signmessagewithprivkey", "\"privkey\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"XwnLY9Tf7Zsef8gMGL2fhWA9ZmMjt4KPwG\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessagewithprivkey", "\"privkey\", \"my message\"")
        );

    std::string strPrivkey = request.params[0].get_str();
    std::string strMessage = request.params[1].get_str();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strPrivkey);
    if (!fGood)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
    CKey key = vchSecret.GetKey();
    if (!key.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

UniValue setmocktime(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "setmocktime timestamp\n"
            "\nSet the local time to given timestamp (-regtest only)\n"
            "\nArguments:\n"
            "1. timestamp  (integer, required) Unix seconds-since-epoch timestamp\n"
            "   Pass 0 to go back to using the system time."
        );

    if (!Params().MineBlocksOnDemand())
        throw std::runtime_error("setmocktime for regression testing (-regtest mode) only");

    // For now, don't change mocktime if we're in the middle of validation, as
    // this could have an effect on mempool time-based eviction, as well as
    // IsCurrentForFeeEstimation() and IsInitialBlockDownload().
    // TODO: figure out the right way to synchronize around mocktime, and
    // ensure all call sites of GetTime() are accessing this safely.
    LOCK(cs_main);

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));
    SetMockTime(request.params[0].get_int64());

    return NullUniValue;
}

static UniValue RPCLockedMemoryInfo()
{
    LockedPool::Stats stats = LockedPoolManager::Instance().stats();
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("used", uint64_t(stats.used)));
    obj.push_back(Pair("free", uint64_t(stats.free)));
    obj.push_back(Pair("total", uint64_t(stats.total)));
    obj.push_back(Pair("locked", uint64_t(stats.locked)));
    obj.push_back(Pair("chunks_used", uint64_t(stats.chunks_used)));
    obj.push_back(Pair("chunks_free", uint64_t(stats.chunks_free)));
    return obj;
}

UniValue getmemoryinfo(const JSONRPCRequest& request)
{
    /* Please, avoid using the word "pool" here in the RPC interface or help,
     * as users will undoubtedly confuse it with the other "memory pool"
     */
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getmemoryinfo\n"
            "Returns an object containing information about memory usage.\n"
            "\nResult:\n"
            "{\n"
            "  \"locked\": {               (json object) Information about locked memory manager\n"
            "    \"used\": xxxxx,          (numeric) Number of bytes used\n"
            "    \"free\": xxxxx,          (numeric) Number of bytes available in current arenas\n"
            "    \"total\": xxxxxxx,       (numeric) Total number of bytes managed\n"
            "    \"locked\": xxxxxx,       (numeric) Amount of bytes that succeeded locking. If this number is smaller than total, locking pages failed at some point and key data could be swapped to disk.\n"
            "    \"chunks_used\": xxxxx,   (numeric) Number allocated chunks\n"
            "    \"chunks_free\": xxxxx,   (numeric) Number unused chunks\n"
            "  }\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmemoryinfo", "")
            + HelpExampleRpc("getmemoryinfo", "")
        );
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("locked", RPCLockedMemoryInfo()));
    return obj;
}

UniValue echo(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw std::runtime_error(
            "echo|echojson \"message\" ...\n"
            "\nSimply echo back the input arguments. This command is for testing.\n"
            "\nThe difference between echo and echojson is that echojson has argument conversion enabled in the client-side table in"
            "bitcoin-cli and the GUI. There is no server-side difference."
        );

    return request.params;
}

UniValue getstakingstatus(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getstakingstatus\n"
            "Returns an object containing various staking information.\n"
            "\nResult:\n"
            "{\n"
            "  \"validtime\": true|false,          (boolean) if the chain tip is within staking phases\n"
            "  \"haveconnections\": true|false,    (boolean) if network connections are present\n"
            "  \"walletunlocked\": true|false,     (boolean) if the wallet is unlocked\n"
            "  \"mintablecoins\": true|false,      (boolean) if the wallet has mintable coins\n"
            "  \"enoughcoins\": true|false,        (boolean) if available coins are greater than reserve balance\n"
            "  \"mnsync\": true|false,             (boolean) if masternode data is synced\n"
            "  \"staking status\": true|false,     (boolean) if the wallet is staking or not\n"
            "}\n"
            "\nExamples:\n" +
            HelpExampleCli("getstakingstatus", "") + HelpExampleRpc("getstakingstatus", ""));

    std::shared_ptr<CWallet> const pwallet = GetWalletForJSONRPCRequest(request);

    UniValue obj(UniValue::VOBJ);
    obj.pushKV("validtime", true);
    obj.pushKV("haveconnections", g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) > 0);
    if (pwallet) {
        obj.pushKV("walletunlocked", !pwallet->IsLocked());
        obj.pushKV("mintablecoins", pwallet->MintableCoins());
        obj.pushKV("enoughcoins", pwallet->GetBalance() > 0);
    }
    obj.pushKV("mnsync", masternodeSync.IsSynced());

    bool nStaking = false;

    if (nLastCoinStakeSearchInterval > 0)
        nStaking = true;

    obj.pushKV("staking status", nStaking);

    return obj;
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "control",            "getmemoryinfo",          &getmemoryinfo,          {} },
    { "util",               "createmultisig",         &createmultisig,         {"nrequired","keys"} },
    { "util",               "verifymessage",          &verifymessage,          {"address","signature","message"} },
    { "util",               "signmessagewithprivkey", &signmessagewithprivkey, {"privkey","message"} },
    { "util",               "getstakingstatus",       &getstakingstatus,       {} },

    /* Dash features */
    { "zentoshi",           "mnsync",                 &mnsync,                 {} },
    { "zentoshi",           "spork",                   &spork,                    {"mode"} },

    /* Not shown in help */
    { "hidden",             "setmocktime",            &setmocktime,            {"timestamp"}},
    { "hidden",             "echo",                   &echo,                   {"arg0","arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9"}},
    { "hidden",             "echojson",               &echo,                   {"arg0","arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9"}},
};

void RegisterMiscRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
