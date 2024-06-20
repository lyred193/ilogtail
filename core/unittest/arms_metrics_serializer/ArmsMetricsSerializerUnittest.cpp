// Copyright 2024 iLogtail Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <snappy.h>

#include "flusher/FlusherArmsMetrics.h"
#include "serializer/ArmsSerializer.h"
#include "unittest/Unittest.h"

DECLARE_FLAG_INT32(max_send_log_group_size);

using namespace std;

namespace logtail {

class ArmsMetricsSerializerUnittest : public ::testing::Test {
public:
    void TestSerializeEventGroupList();

protected:
    static void SetUpTestCase() { armsFlusher = make_unique<FlusherArmsMetrics>(); }

    void SetUp() override {
        // mCtx.SetConfigName("test_config");
        // sFlusher->SetContext(mCtx);
        // sFlusher->SetMetricsRecordRef(FlusherArmsMetrics::sName, "1");
    }

private:
    BatchedEvents CreateBatchedEvents(bool enableNanosecond);

    static unique_ptr<FlusherArmsMetrics> armsFlusher;

    PipelineContext mCtx;
};

unique_ptr<FlusherArmsMetrics> ArmsMetricsSerializerUnittest::armsFlusher;


void ArmsMetricsSerializerUnittest::TestSerializeEventGroupList() {
    // std::vector<BatchedEvents> v;
    // v.emplace_back("data1", 10);
    std::cout << "start test ArmsMetricsSerializerUnittest" << std::endl;
    std::vector<BatchedEventsList> vec(1);
    std::vector<BatchedEvents> batchedEventList(10);
    for (int i = 0; i < 10; i++) {
        auto events = CreateBatchedEvents(true);
        batchedEventList.emplace_back(std::move(events));
    }
    vec.emplace_back(std::move(batchedEventList));
    armsFlusher.reset(new FlusherArmsMetrics());
    auto serializer = new ArmsMetricsEventGroupListSerializer(armsFlusher.get());
    std::string res, errorMsg;
    APSARA_TEST_TRUE(serializer->Serialize(std::move(vec), res, errorMsg));
    std::cout << res << std::endl;
    // std::string compressData;
    // snappy::Compress(res.data(), res.size(), &compressData);

    // sls_logs::SlsLogPackageList logPackageList;
    // APSARA_TEST_TRUE(logPackageList.ParseFromString(res));
    // APSARA_TEST_EQUAL(1, logPackageList.packages_size());
    // APSARA_TEST_STREQ("data1", logPackageList.packages(0).data().c_str());
    // APSARA_TEST_EQUAL(10, logPackageList.packages(0).uncompress_size());
    // APSARA_TEST_EQUAL(sls_logs::SlsCompressType::SLS_CMP_NONE, logPackageList.packages(0).compress_type());
}

BatchedEvents ArmsMetricsSerializerUnittest::CreateBatchedEvents(bool enableNanosecond) {
    PipelineEventGroup group(make_shared<SourceBuffer>());
    // group.SetTag(LOG_RESERVED_KEY_TOPIC, "topic");
    // group.SetTag(LOG_RESERVED_KEY_SOURCE, "source");
    // group.SetTag(LOG_RESERVED_KEY_MACHINE_UUID, "machine_uuid");
    // group.SetTag(LOG_RESERVED_KEY_PACKAGE_ID, "pack_id");
    group.SetTag(string("appName"), "cmonitor-test");
    group.SetTag(string("workloadName"), "cmonitor-test");
    group.SetTag(string("workloadKind"), "deployment");
    group.SetTag(string("appId"), "xxxxdsgejosldie");
    group.SetTag(string("source_ip"), "192.168.88.11");
    StringBuffer b = group.GetSourceBuffer()->CopyString(string("pack_id"));
    group.SetMetadataNoCopy(EventGroupMetaKey::SOURCE_ID, StringView(b.data, b.size));
    group.SetExactlyOnceCheckpoint(RangeCheckpointPtr(new RangeCheckpoint));
    LogEvent* e = group.AddLogEvent();
    e->SetContent(string("key"), string("value"));
    if (enableNanosecond) {
        e->SetTimestamp(1234567890, 1);
    } else {
        e->SetTimestamp(1234567890);
    }
    BatchedEvents batch(std::move(group.MutableEvents()),
                        std::move(group.GetSizedTags()),
                        std::move(group.GetSourceBuffer()),
                        group.GetMetadata(EventGroupMetaKey::SOURCE_ID),
                        std::move(group.GetExactlyOnceCheckpoint()));
    std::cout << "CreateBatchedEvents end..." << std::endl;

    return batch;
}

UNIT_TEST_CASE(ArmsMetricsSerializerUnittest, TestSerializeEventGroupList)

} // namespace logtail

UNIT_TEST_MAIN
