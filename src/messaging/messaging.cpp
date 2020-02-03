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

#ifdef ENABLE_WALLET
#include <wallet/wallet.h>
#endif

bool CMessagingParticipant::ValidateFundingTx(CFundingTransaction& pfundingTx, std::string& error) const
{
    pDestination = DecodeDestination(pAddress);
    if (!IsValidDestination(pDestination)) {
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
    const PKHash* pkhash = boost::get<PKHash>(&pSender.pDestination);
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

bool CMessaging::IsMine() const
{
#ifndef ENABLE_WALLET
    return false;
#else
    bool hasKey = false;
    auto wallets = GetWallets();
    for (std::shared_ptr<CWallet> wallet : wallets) {
        auto spk_man = wallet->GetLegacyScriptPubKeyMan();
        LOCK(wallet->cs_wallet);
        AssertLockHeld(spk_man->cs_wallet);

        auto script = GetScriptForDestination(pRecipient.pDestination);
        if (spk_man->IsMine(script) | ISMINE_SPENDABLE) {
            hasKey = true;
            break;
        }
    }

    return hasKey;
#endif
}

void CMessaging::SaveMessage() const
{
#ifdef ENABLE_WALLET
    // TODO: Decrypt message and save somewhere
#endif
}
