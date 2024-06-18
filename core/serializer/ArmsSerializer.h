//
// Created by lurious on 2024/6/17.
//

#pragma once

#include <string>
#include <vector>

#include "serializer/Serializer.h"

namespace logtail {

class ArmsMetricsEventGroupListSerializer : public Serializer<std::vector<>> {
public:
    ArmsMetricsEventGroupListSerializer(Flusher* f) : Serializer<std::vector<BatchedEvents>>(f) {}

    bool Serialize(std::vector<BatchedEvents>&& v, std::string& res, std::string& errorMsg) override;
};

} // namespace logtail