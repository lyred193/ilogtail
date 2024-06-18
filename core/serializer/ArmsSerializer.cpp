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
}


void ArmsMetricsEventGroupListSerializer::ConvertBatchedEventsToMeasureBathch(BatchedEvents&& BatchedEvents) {
}
void ArmsMetricsEventGroupListSerializer::ConvertEventsToMeasures(EventsContainer&& events) {
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