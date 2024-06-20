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
        measureBatch->set_pid(GetAppIdFromTags(batchedEvents.mTags));
        ConvertBatchedEventsToMeasures(std::move(batchedEvents), measureBatch);
    }
}


void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasures(BatchedEvents&& batchedEvents,
                                                                         proto::MeasureBatch* measureBatch) {
    auto measures = measureBatch->add_measures();
    for (auto& kv : batchedEvents.mTags.mInner) {
        measures->mutable_labels()->insert({kv.first.to_string(), kv.second.to_string()});
    }
}


void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasure(EventsContainer&& events, proto::Measures* measures) {
    for (const auto& event : events) {
        // auto tags = event.GetSizedTags();
        // add labels
        // for (auto& kv : tags.mInner) {
        //     measures->mutable_labels()->insert({kv.first.to_string(), kv.second.to_string()});
        // }
        auto measure = measures->add_measures();
        auto& eventData = event.Cast<MetricEvent>();
        eventData.GetTimestamp();
        auto measurePtr = measure;
        // measurePtr.set_name(eventData.GetName());
        measurePtr->set_valuetype("");
        // measurePtr.set_value(static_cast<double>(eventData.GetValue()));
        // measurePtr.set_desc(eventData.GetDesc());
        measurePtr->set_unit(proto::EnumUnit::COUNT);
    }
}

std::string ArmsMetricsEventGroupListSerializer::GetIpFromTags(SizedMap& mTags) {
    auto& mTagsInner = mTags.mInner;
    auto it = mTagsInner.find("source_ip");
    if (it != mTagsInner.end()) {
        return it->second.to_string();
    }
    return "ip";
}


std::string ArmsMetricsEventGroupListSerializer::GetAppIdFromTags(SizedMap& mTags) {
    auto& mTagsInner = mTags.mInner;
    auto it = mTagsInner.find("appId");
    if (it != mTagsInner.end()) {
        return it->second.to_string();
    }
    return "appId";
}


} // namespace logtail