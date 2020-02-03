// Copyright (c) 2020 The Níckolas Goline
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MESSAGING_MESSAGING_H
#define BITCOIN_MESSAGING_MESSAGING_H

#include <amount.h>
#include <uint256.h>

class CFundingTransaction
{
public:
    CFundingTransaction(){};
    CFundingTransaction(CAmount pamount, uint64_t plockedFor)
    {
        pLockedAmount = pamount;
        nLockedFor = plockedFor;
    }

    CAmount pLockedAmount;
    uint64_t nLockedFor;
};

class CMessagingParticipant
{
public:
    CMessagingParticipant()
    {
        nFundingBlk = 0;
        nFundingTxPos = 0;
        nFundingN = 0;
    }

    std::string pAddress;
    uint nFundingBlk;
    uint nFundingTxPos;
    uint nFundingN;

    CTxDestination pDestination;

    bool ValidateFundingTx(CFundingTransaction& pfundingTx, std::string& error) const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(pAddress);
        READWRITE(VARINT(nFundingBlk));
        READWRITE(VARINT(nFundingTxPos));
        READWRITE(VARINT(nFundingN));
    }
};

class CMessaging
{
public:
    CMessaging()
    {
        nSentOn = 0;
    }

    uint64_t nSentOn;
    CMessagingParticipant pSender;
    CMessagingParticipant pRecipient;
    std::string pMessage;
    std::string pSignature;

    void GetTimeToLive(int* pTimeToLive) const;
    bool CheckSignature() const;
    bool IsMine() const;
    void SaveMessage() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(nSentOn);
        READWRITE(pSender);
        READWRITE(pRecipient);
        READWRITE(pMessage);
        READWRITE(pSignature);
    }
};

#endif // BITCOIN_MESSAGING_MESSAGING_H
