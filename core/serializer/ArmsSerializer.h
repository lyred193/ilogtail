//
// Created by lurious on 2024/6/17.
//

#pragma once

#include <string>
#include <vector>

#include "arms_metrics_pb/MeasureBatches.pb.h"
#include "serializer/Serializer.h"

namespace logtail {

class ArmsMetricsEventGroupListSerializer : public Serializer<std::vector<BatchedEvents>> {
public:
    ArmsMetricsEventGroupListSerializer(Flusher* f) : Serializer<std::vector<BatchedEvents>>(f) {}

    bool Serialize(std::vector<BatchedEvents>&& v, std::string& res, std::string& errorMsg) override;

private:
    void ConvertBatchedEventsToMeasureBathch(BatchedEvents&& BatchedEvents);
    void ConvertEventsToMeasures(EventsContainer&& events);
    void ConvertEventToMeasure(PipelineEventPtr&& event);
};

} // namespace logtail