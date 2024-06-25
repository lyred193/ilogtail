//
// Created by lurious on 2024/6/17.
//

#include "serializer/ArmsSerializer.h"

#include "arms_metrics_pb/MeasureBatches.pb.h"


namespace logtail {

bool ArmsMetricsEventGroupListSerializer::Serialize(std::vector<BatchedEventsList>&& v,
                                                    std::string& res,
                                                    std::string& errorMsg) {
    auto measureBatches = new proto::MeasureBatches();
    for (auto& batchedEventsList : v) {
        ConvertBatchedEventsListToMeasureBatch(std::move(batchedEventsList), measureBatches);
    }
    res = measureBatches->SerializeAsString();
    return true;
}

void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsListToMeasureBatch(
    BatchedEventsList&& batchedEventsList, proto::MeasureBatches* measureBatches) {
    for (auto& batchedEvents : batchedEventsList) {
        auto tags = batchedEvents.mTags.mInner;
        auto measureBatch = measureBatches->add_measurebatches();
        measureBatch->set_type("app");
        measureBatch->set_ip(GetIpFromTags(batchedEvents.mTags));
        measureBatch->set_time(GetMeasureTimestamp(batchedEvents));
        measureBatch->set_version("v1");
        measureBatch->set_pid(GetAppIdFromTags(batchedEvents.mTags));
        ConvertBatchedEventsToMeasures(std::move(batchedEvents), measureBatch);
    }
}

int64_t ArmsMetricsEventGroupListSerializer::GetMeasureTimestamp(BatchedEvents& batchedEvents) {
    for (auto&& event : batchedEvents.mEvents) {
        auto timestamp = event->GetTimestamp();
        return static_cast<int64_t>(timestamp);
    }
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    int64_t current_time_millis = static_cast<int64_t>(millis);
    return current_time_millis;
}

void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasures(BatchedEvents&& batchedEvents,
                                                                         proto::MeasureBatch* measureBatch) {
    auto measures = measureBatch->add_measures();
    for (auto& kv : batchedEvents.mTags.mInner) {
        LOG_INFO(sLogger, (kv.first.to_string(), kv.second.to_string()));
        measures->mutable_labels()->insert({kv.first.to_string(), kv.second.to_string()});
    }
    ConvertEventsToMeasure(std::move(batchedEvents.mEvents), measures);
}


void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasure(EventsContainer&& events, proto::Measures* measures) {
    for (const auto& event : events) {
        auto measure = measures->add_measures();
        auto& eventData = event.Cast<MetricEvent>();
        eventData.GetTimestamp();
        std::string metricName(eventData.GetName().data(), eventData.GetName().size());
        measure->set_name(metricName);
        measure->set_valuetype("");
        if (eventData.Is<UntypedSingleValue>()) {
            auto value = eventData.GetValue<UntypedSingleValue>()->mValue;
            measure->set_value(value);
        }
        measure->set_unit(proto::EnumUnit::COUNT);
    }
}

std::string ArmsMetricsEventGroupListSerializer::GetIpFromTags(SizedMap& mTags) {
    auto& mTagsInner = mTags.mInner;
    auto it = mTagsInner.find("source_ip");
    if (it != mTagsInner.end()) {
        return it->second.to_string();
    } else {
        LOG_WARNING(sLogger, ("GetIpFromTags", "do not find source_ip, no tag"));
    }
    return "unkown";
}


std::string ArmsMetricsEventGroupListSerializer::GetAppIdFromTags(SizedMap& mTags) {
    auto& mTagsInner = mTags.mInner;
    auto it = mTagsInner.find("appId");
    if (it != mTagsInner.end()) {
        return it->second.to_string();
    } else {
        LOG_WARNING(sLogger, ("GetAppIdFromTags", "do not find appId, no tag!"));
    }
    return "unkown";
}


} // namespace logtail