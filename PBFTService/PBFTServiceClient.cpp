/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief client for the PBFTService
 * @file PBFTServiceClient.cpp
 * @author: yujiechen
 * @date 2021-06-29
 */

#include "PBFTServiceClient.h"
#include "../protocols/BlockImpl.h"
#include "Common.h"

using namespace bcostars;

void PBFTServiceClient::asyncSubmitProposal(bcos::bytesConstRef _proposalData,
    bcos::protocol::BlockNumber _proposalIndex, bcos::crypto::HashType const& _proposalHash,
    std::function<void(bcos::Error::Ptr)> _onProposalSubmitted)
{
    m_proxy->async_asyncSubmitProposal(new PBFTServiceCommonCallback(_onProposalSubmitted),
        std::vector<char>(_proposalData.begin(), _proposalData.end()), _proposalIndex, std::vector<char>(_proposalHash.begin(), _proposalHash.end()));
}

void PBFTServiceClient::asyncGetPBFTView(
    std::function<void(bcos::Error::Ptr, bcos::consensus::ViewType)> _onGetView)
{
    class Callback : public PBFTServicePrxCallback
    {
    public:
        explicit Callback(
            std::function<void(bcos::Error::Ptr, bcos::consensus::ViewType)> _callback)
          : PBFTServicePrxCallback(), m_callback(_callback)
        {}
        ~Callback() override {}

        void callback_asyncGetPBFTView(const bcostars::Error& ret, tars::Int64 _view) override
        {
            m_callback(toBcosError(ret), _view);
        }
        void callback_asyncGetPBFTView_exception(tars::Int32 ret) override
        {
            m_callback(toBcosError(ret), 0);
        }

    private:
        std::function<void(bcos::Error::Ptr, bcos::consensus::ViewType)> m_callback;
    };
    m_proxy->async_asyncGetPBFTView(new Callback(_onGetView));
}

// the sync module calls this interface to check block
void PBFTServiceClient::asyncCheckBlock(
    bcos::protocol::Block::Ptr _block, std::function<void(bcos::Error::Ptr, bool)> _onVerifyFinish)
{
    class Callback : public PBFTServicePrxCallback
    {
    public:
        explicit Callback(std::function<void(bcos::Error::Ptr, bool)> _callback)
          : PBFTServicePrxCallback(), m_callback(_callback)
        {}
        ~Callback() override {}

        void callback_asyncCheckBlock(const bcostars::Error& ret, tars::Bool _verifyResult) override
        {
            m_callback(toBcosError(ret), _verifyResult);
        }
        void callback_asyncCheckBlock_exception(tars::Int32 ret) override
        {
            m_callback(toBcosError(ret), false);
        }

    private:
        std::function<void(bcos::Error::Ptr, bool)> m_callback;
    };

    auto blockImpl = std::dynamic_pointer_cast<bcostars::protocol::BlockImpl>(_block);
    m_proxy->async_asyncCheckBlock(new Callback(_onVerifyFinish), blockImpl->inner());
}

// the sync module calls this interface to notify new block
void PBFTServiceClient::asyncNotifyNewBlock(
    bcos::ledger::LedgerConfig::Ptr _ledgerConfig, std::function<void(bcos::Error::Ptr)> _onRecv)
{
    m_proxy->async_asyncNotifyNewBlock(
        new PBFTServiceCommonCallback(_onRecv), toTarsLedgerConfig(_ledgerConfig));
}

// called by frontService to dispatch message
void PBFTServiceClient::asyncNotifyConsensusMessage(bcos::Error::Ptr _error,
    std::string const& _uuid, bcos::crypto::NodeIDPtr _nodeID, bcos::bytesConstRef _data,
    std::function<void(bcos::Error::Ptr _error)> _onRecv)
{
    auto nodeIDData = _nodeID->data();
    m_proxy->async_asyncNotifyConsensusMessage(
        new PBFTServiceCommonCallback(_onRecv), _uuid, std::vector<char>(nodeIDData.begin(), nodeIDData.end()), std::vector<char>(_data.begin(), _data.end()));
}

// Note: used for the txpool notify the unsealed txsSize
void PBFTServiceClient::asyncNoteUnSealedTxsSize(
    size_t _unsealedTxsSize, std::function<void(bcos::Error::Ptr)> _onRecv)
{
    m_proxy->async_asyncNoteUnSealedTxsSize(
        new PBFTServiceCommonCallback(_onRecv), _unsealedTxsSize);
}

void BlockSyncServiceClient::asyncGetSyncInfo(
    std::function<void(bcos::Error::Ptr, std::string)> _onGetSyncInfo)
{
    class Callback : public PBFTServicePrxCallback
    {
    public:
        explicit Callback(std::function<void(bcos::Error::Ptr, std::string)> _callback)
          : PBFTServicePrxCallback(), m_callback(_callback)
        {}
        ~Callback() override {}

        void callback_asyncGetSyncInfo(
            const bcostars::Error& ret, const std::string& _syncInfo) override
        {
            m_callback(toBcosError(ret), _syncInfo);
        }
        void callback_asyncGetSyncInfo_exception(tars::Int32 ret) override
        {
            m_callback(toBcosError(ret), "callback_asyncGetSyncInfo_exception");
        }

    private:
        std::function<void(bcos::Error::Ptr, std::string)> m_callback;
    };
    m_proxy->async_asyncGetSyncInfo(new Callback(_onGetSyncInfo));
}