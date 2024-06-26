//
// Created by lurious on 2024/6/17.
//

#include "serializer/ArmsSerializer.h"

#include "arms_metrics_pb/MeasureBatches.pb.h"

const std::map<std::string, proto::EnumUnit> metricUnitMap{
    {"arms_rpc_requests_count", proto::EnumUnit::COUNT},
    {"arms_rpc_requests_error_count", proto::EnumUnit::COUNT},
    {"arms_rpc_requests_seconds", proto::EnumUnit::MILLISECONDS},
    {"arms_rpc_requests_slow_count", proto::EnumUnit::COUNT},
    {"arms_rpc_requests_by_status_count", proto::EnumUnit::COUNT},
    {"arms_npm_sent_bytes_total", proto::EnumUnit::BYTE},
    {"arms_npm_recv_bytes_total", proto::EnumUnit::BYTE},
    {"arms_npm_sent_packets_total", proto::EnumUnit::COUNT},
    {"arms_npm_recv_packets_total", proto::EnumUnit::COUNT},
    {"arms_npm_tcp_retrans_total", proto::EnumUnit::COUNT},
    {"arms_npm_tcp_drop_count", proto::EnumUnit::COUNT},
    {"arms_npm_tcp_conn_stats_count", proto::EnumUnit::COUNT},
    {"arms_npm_tcp_count_by_state", proto::EnumUnit::COUNT},
    {"arms_npm_tcp_rtt_avg", proto::EnumUnit::MILLISECONDS},
};

const std::string COUNT = "count";
const std::string SUM = "SUM";


const std::map<std::string, std::string> metricValueTypeMap{
    {"arms_rpc_requests_count", COUNT},
    {"arms_rpc_requests_error_count", COUNT},
    {"arms_rpc_requests_seconds", SUM},
    {"arms_rpc_requests_slow_count", COUNT},
    {"arms_rpc_requests_by_status_count", COUNT},
    {"arms_npm_sent_bytes_total", COUNT},
    {"arms_npm_recv_bytes_total", COUNT},
    {"arms_npm_sent_packets_total", COUNT},
    {"arms_npm_recv_packets_total", COUNT},
    {"arms_npm_tcp_retrans_total", COUNT},
    {"arms_npm_tcp_drop_count", COUNT},
    {"arms_npm_tcp_conn_stats_count", COUNT},
    {"arms_npm_tcp_count_by_state", COUNT},
    {"arms_npm_tcp_rtt_avg", SUM},
};


namespace logtail {

bool ArmsMetricsEventGroupListSerializer::Serialize(std::vector<BatchedEventsList>&& v,
                                                    std::string& res,
                                                    std::string& errorMsg) {
    // auto measureBatches = new proto::MeasureBatches();
    std::shared_ptr<proto::MeasureBatches> measureBatches = std::make_shared<proto::MeasureBatches>();
    for (auto& batchedEventsList : v) {
        ConvertBatchedEventsListToMeasureBatch(std::move(batchedEventsList), measureBatches.get());
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
        measureBatch->mutable_commontags()->insert({"source", "ebpf"});
        measureBatch->mutable_commontags()->insert({"appType", "EBPF"});
        for (auto& kv : batchedEvents.mTags.mInner) {
            measureBatch->mutable_commontags()->insert({kv.first.to_string(), kv.second.to_string()});
        }
        ConvertBatchedEventsToMeasures(std::move(batchedEvents), measureBatch);
    }
}

int64_t ArmsMetricsEventGroupListSerializer::GetMeasureTimestamp(BatchedEvents& batchedEvents) {
    for (auto&& event : batchedEvents.mEvents) {
        auto timestamp = event->GetTimestamp();
        if (timestamp != 0) {
            return static_cast<int64_t>(timestamp);
        }
    }
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    int64_t current_time_millis = static_cast<int64_t>(millis);
    return current_time_millis;
}

void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasures(BatchedEvents&& batchedEvents,
                                                                         proto::MeasureBatch* measureBatch) {
    for (const auto& event : batchedEvents.mEvents) {
        auto measures = measureBatch->add_measures();
        auto measure = measures->add_measures();
        auto& eventData = event.Cast<MetricEvent>();
        for (auto& kv : eventData.GetTags()) {
            measures->mutable_labels()->insert({kv.first.to_string(), kv.second.to_string()});
        }
        eventData.GetTimestamp();
        std::string metricName(eventData.GetName().data(), eventData.GetName().size());
        measure->set_name(metricName);
        measure->set_valuetype(GetValueTypeByMetricName(metricName));
        if (eventData.Is<UntypedSingleValue>()) {
            auto value = eventData.GetValue<UntypedSingleValue>()->mValue;
            measure->set_value(value);
        }
        measure->set_unit(GetUnitByMetricName(metricName));
    }
}


void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasure(EventsContainer&& events, proto::Measures* measures) {
    for (const auto& event : events) {
        auto measure = measures->add_measures();
        auto& eventData = event.Cast<MetricEvent>();
        eventData.GetTimestamp();
        std::string metricName(eventData.GetName().data(), eventData.GetName().size());
        measure->set_name(metricName);
        measure->set_valuetype(GetValueTypeByMetricName(metricName));
        if (eventData.Is<UntypedSingleValue>()) {
            auto value = eventData.GetValue<UntypedSingleValue>()->mValue;
            measure->set_value(value);
        }
        measure->set_unit(GetUnitByMetricName(metricName));
    }
}

proto::EnumUnit ArmsMetricsEventGroupListSerializer::GetUnitByMetricName(std::string metricName) {
    auto it = metricUnitMap.find(metricName);
    if (it != metricUnitMap.end()) {
        return it->second;
    } else {
        return proto::EnumUnit::UNKNOWN;
    }
}

std::string ArmsMetricsEventGroupListSerializer::GetValueTypeByMetricName(std::string metricName) {
    auto it = metricValueTypeMap.find(metricName);
    if (it != metricValueTypeMap.end()) {
        return it->second;
    } else {
        return COUNT;
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