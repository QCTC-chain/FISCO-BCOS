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
 * @brief matain the sync status
 * @file SyncPeerStatus.h
 * @author: jimmyshi
 * @date 2021-05-24
 */
#pragma once
#include "bcos-sync/BlockSyncConfig.h"
#include "bcos-sync/interfaces/BlockSyncStatusInterface.h"
#include "bcos-sync/state/DownloadRequestQueue.h"
#include "bcos-sync/utilities/Common.h"
namespace bcos::sync
{
class PeerStatus
{
public:
    using Ptr = std::shared_ptr<PeerStatus>;
    PeerStatus(BlockSyncConfig::Ptr _config, bcos::crypto::PublicPtr _nodeId,
        bcos::protocol::BlockNumber _number, bcos::crypto::HashType const& _hash,
        bcos::crypto::HashType const& _genesisHash);
    PeerStatus(BlockSyncConfig::Ptr _config, bcos::crypto::PublicPtr _nodeId);

    PeerStatus(BlockSyncConfig::Ptr _config, bcos::crypto::PublicPtr _nodeId,
        BlockSyncStatusInterface::ConstPtr _status);

    virtual ~PeerStatus() = default;

    virtual bool update(BlockSyncStatusInterface::ConstPtr _status);

    bcos::crypto::PublicPtr nodeId() { return m_nodeId; }

    bcos::protocol::BlockNumber number() const
    {
        std::lock_guard<std::mutex> lock(x_mutex);
        return m_number;
    }

    bcos::protocol::BlockNumber archivedBlockNumber() const
    {
        std::lock_guard<std::mutex> lock(x_mutex);
        return m_archivedNumber;
    }

    bcos::crypto::HashType const& hash() const
    {
        std::lock_guard<std::mutex> lock(x_mutex);
        return m_hash;
    }

    bcos::crypto::HashType const& genesisHash() const
    {
        std::lock_guard<std::mutex> lock(x_mutex);
        return m_genesisHash;
    }

    DownloadRequestQueue::Ptr downloadRequests() { return m_downloadRequests; }

private:
    bcos::crypto::PublicPtr m_nodeId;
    bcos::protocol::BlockNumber m_number;
    bcos::protocol::BlockNumber m_archivedNumber;
    bcos::crypto::HashType m_hash;
    bcos::crypto::HashType m_genesisHash;

    mutable std::mutex x_mutex;
    DownloadRequestQueue::Ptr m_downloadRequests;
};

class SyncPeerStatus
{
public:
    using Ptr = std::shared_ptr<SyncPeerStatus>;
    explicit SyncPeerStatus(BlockSyncConfig::Ptr _config) : m_config(std::move(_config)) {}
    virtual ~SyncPeerStatus() = default;

    virtual bool hasPeer(bcos::crypto::PublicPtr _peer);
    virtual PeerStatus::Ptr peerStatus(bcos::crypto::PublicPtr _peer);
    virtual bool updatePeerStatus(
        bcos::crypto::PublicPtr _peer, BlockSyncStatusInterface::ConstPtr _peerStatus);
    virtual void deletePeer(bcos::crypto::PublicPtr _peer);

    void foreachPeerRandom(std::function<bool(PeerStatus::Ptr)> const&) const;
    void foreachPeer(std::function<bool(PeerStatus::Ptr)> const&) const;
    std::shared_ptr<bcos::crypto::NodeIDs> peers();
    size_t peersSize() const
    {
        std::lock_guard<std::mutex> lock(x_peersStatus);
        return m_peersStatus.size();
    }
    PeerStatus::Ptr insertEmptyPeer(bcos::crypto::PublicPtr _peer);

protected:
    virtual void updateKnownMaxBlockInfo(BlockSyncStatusInterface::ConstPtr _peerStatus);

private:
    std::map<bcos::crypto::PublicPtr, PeerStatus::Ptr, bcos::crypto::KeyCompare> m_peersStatus;
    mutable std::mutex x_peersStatus;

    BlockSyncConfig::Ptr m_config;
};
}  // namespace bcos::sync