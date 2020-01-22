// Copyright (c) 2020 The Níckolas Goline
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <messaging/messaging.h>

#include <chainparams.h>
#include <key_io.h>
#include <script/standard.h>
#include <util/time.h>
#include <util/validation.h>
#include <validation.h>

bool CMessagingParticipant::ValidateFundingTx(CFundingTransaction& pfundingTx, std::string& error) const
{
    if (!pAddress.IsValid()) {
        error = "Address is not valid!";
        return false;
    }

    if (nFundingBlk == 0) {
        error = "Funding block height can't be zero!";
        return false;
    }

    CBlockIndex* pblockindex = ::ChainActive()[nFundingBlk];
    if (pblockindex == nullptr) {
        error = "Funding block height doesn't exist!";
        return false;
    }

    if (nFundingTxPos >= pblockindex->nTx) {
        error = "Funding tx pos is bigger than number of tx in block!";
        return false;
    }

    CBlock block;
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus())) {
        error = "Block not found in disk!";
        return false;
    }

    CTransactionRef txRef = block.vtx[nFundingTxPos];
    if (nFundingTxPos >= txRef->vout.size()) {
        error = "Funding tx vout n is bigger than number of vout in tx!";
        return false;
    }

    // TODO: Get current lock from script
    pfundingTx = CFundingTransaction(txRef->vout[nFundingTxPos].nValue, 10);

    return true;
}

bool CMessaging::CheckSignature() const
{
    CTxDestination destination = DecodeDestination(pSender.pAddress.ToString());
    const PKHash* pkhash = boost::get<PKHash>(&destination);
    if (!pkhash) return false;


    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(pSignature.c_str(), &fInvalid);
    if (fInvalid) return false;

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << pMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey.GetID() == *pkhash);
}

void CMessaging::GetTimeToLive(int* pTimeToLive) const
{
    *pTimeToLive = GetSystemTimeInSeconds() - nSentOn;
}
