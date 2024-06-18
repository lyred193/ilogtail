//
// Created by lurious on 2024/6/17.
//

#include "serializer/ArmsSerializer.h"

// #include "arms_metrics_pb/MeasureBatches.pb.h"

namespace logtail {

bool ArmsMetricsEventGroupListSerializer::Serialize(std::vector<BatchedEvents>&& v,
                                                    std::string& res,
                                                    std::string& errorMsg) {
}


void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasureBathch(BatchedEvents&& BatchedEvents) {
}
void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasures(EventsContainer&& events) {
}

void ArmsMetricsEventGroupListSerializer::ConvertEventToMeasure(PipelineEventPtr&& event) {
    // convert to metric event
    auto& eventData = event.Cast<MetricEvent>();
    for (const auto& kv : eventData) {
        contPtr->set_key(kv.first.to_string());
        contPtr->set_value(kv.second.to_string());
    }
}


} // namespace logtail