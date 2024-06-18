//
// Created by lurious on 2024/6/17.
//
#pragma once

#include "json/json.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "batch/BatchStatus.h"
#include "batch/Batcher.h"
#include "common/LogstoreFeedbackKey.h"
#include "compression/Compressor.h"
#include "models/PipelineEventGroup.h"
#include "plugin/interface/Flusher.h"
#include "serializer/ArmsSerializer.h"


namespace logtail {

class FlusherArmsMetrics : Flusher {
public:
    bool Init(const Json::Value& config, Json::Value& optionalGoPipeline) override;
    bool Register() override;
    bool Unregister(bool isPipelineRemoving) override;
    void Send(PipelineEventGroup&& g) override;
    void Flush(size_t key) override;
    void FlushAll() override;
    sdk::AsynRequest* BuildRequest(SenderQueueItem* item) const override;


    SingleLogstoreSenderManager<SenderQueueParam>* GetSenderQueue() const { return mSenderQueue; }

private:
    void SerializeAndPush(std::vector<BatchedEventsList>&& groupLists);
    void SerializeAndPush(BatchedEventsList&& groupList);

    std::unique_ptr<ArmsMetricsEventGroupListSerializer> mGroupListSerializer;

};


} // namespace logtail
