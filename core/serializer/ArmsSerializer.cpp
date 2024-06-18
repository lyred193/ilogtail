//
// Created by lurious on 2024/6/17.
//

#include "serializer/ArmsSerializer.h"

#include "arms_metrics_pb/MeasureBatches.pb.h"
#include "log_pb/sls_logs.pb.h"

// #include "arms_metrics_pb/MeasureBatches.pb.h"

namespace logtail {

bool ArmsMetricsEventGroupListSerializer::Serialize(std::vector<BatchedEvents>&& v,
                                                    std::string& res,
                                                    std::string& errorMsg) {
    for (auto& batchedEvents : v) {
        ConvertBatchedEventsToMeasureBathch(batchedEvents);
    }
    arms_metrics::MeasureBatches measureBatches = new arms_metrics::MeasureBatches();
    res = measureBatches.SerializeAsString();
}


void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasureBathch(BatchedEvents&& batchedEvents) {
    for (const auto& events : batchedEvents) {
        ConvertEventsToMeasures(events);
    }
}
void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasures(EventsContainer&& events) {
    for (const auto& e : events) {
        arms_metrics::Measures measures = new arms_metrics::Measures();
        auto measuresPtr = measures.add_measures();
        auto& measure = ConvertEventToMeasure(std::move(e));
    }
}

std::unique_ptr<Measure> ArmsMetricsEventGroupListSerializer::ConvertEventToMeasure(PipelineEventPtr&& event) {
    // convert to metric event
    auto& eventData = event.Cast<MetricEvent>();
    arms_metrics::MeasureBatch measureBath = new MeasureBatch();
    arms_metrics::Measures measures = new Measures();
    auto measurePtr = measures.add_measures();
    measurePtr.set_name(eventData.GetName());
    measurePtr.set_valuetype("");
    measurePtr.set_value(eventData.GetValue());
    measurePtr.set_desc(eventData.GetDesc());
    measurePtr.set_unit(arms_metrics::EnumUnit::COUNT);
    return make_unique<arms_metrics::Measure>(measurePtr);
}


} // namespace logtail