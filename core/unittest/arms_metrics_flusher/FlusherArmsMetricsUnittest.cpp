
#include <json/json.h>

#include <memory>
#include <string>

#include "common/JsonUtil.h"
#include "common/LogstoreFeedbackKey.h"
#include "common/LogtailCommonFlags.h"
#ifdef __ENTERPRISE__
#include "config/provider/EnterpriseConfigProvider.h"
#endif
#include "flusher/FlusherArmsMetrics.h"
#include "pipeline/PipelineContext.h"
#include "sender/SLSClientManager.h"
#include "sender/Sender.h"
#include "unittest/Unittest.h"

DECLARE_FLAG_INT32(batch_send_interval);
DECLARE_FLAG_INT32(merge_log_count_limit);
DECLARE_FLAG_INT32(batch_send_metric_size);

using namespace std;

namespace logtail {

class FlusherArmsMetricsUnittest : public testing::Test {
public:
    void OnSuccessfulInit();

protected:
    void SetUp() override { ctx.SetConfigName("test_config"); }

private:
    PipelineContext ctx;
};


void FlusherArmsMetricsUnittest::OnSuccessfulInit() {
    unique_ptr<FlusherArmsMetrics> flusher;
    Json::Value configJson, optionalGoPipelineJson, optionalGoPipeline;
    string configStr, optionalGoPipelineStr, errorMsg;

    // only mandatory param
    configStr = R"(
        {
            "Type": "flusher_arms",
            "Project": "test_project",
            "Logstore": "test_logstore",
            "Region": "cn-hk",
            "Licensekey": "test-licenseKey",
            "PushAppId": "test-push-appId",
            "Endpoint": "cn-hangzhou.log.aliyuncs.com"
        }
    )";
    APSARA_TEST_TRUE(ParseJsonTable(configStr, configJson, errorMsg));
    flusher.reset(new FlusherArmsMetrics());
    flusher->Init(configJson, optionalGoPipeline);
    // auto item = new SenderQueueItem();
    auto requestItem = flusher->BuildRequest(nullptr);
    std::cout << &requestItem << std::endl;

    // // flusher = new
    // // flusher->SetContext(ctx);
    // // flusher->SetMetricsRecordRef(FlusherArmsMetrics::sName, "1");
    // APSARA_TEST_TRUE(flusher->Init(configJson, optionalGoPipeline));
    // APSARA_TEST_TRUE(optionalGoPipeline.isNull());
    // APSARA_TEST_EQUAL("test_project", flusher->mProject);
    // // APSARA_TEST_EQUAL("test_logstore", flusher->mLogstore);
    // // APSARA_TEST_EQUAL(GenerateLogstoreFeedBackKey("test_project", "test_logstore"), flusher->GetLogstoreKey());
    // APSARA_TEST_EQUAL(STRING_FLAG(default_region_name), flusher->mRegion);
    // // APSARA_TEST_EQUAL("cn-hangzhou.log.aliyuncs.com", flusher->mEndpoint);
    APSARA_TEST_EQUAL("", flusher->mAliuid);
    // APSARA_TEST_EQUAL(FlusherArmsMetrics::TelemetryType::LOG, flusher->mTelemetryType);
    // APSARA_TEST_EQUAL(0U, flusher->mFlowControlExpireTime);
    // APSARA_TEST_EQUAL(-1, flusher->mMaxSendRate);
    // APSARA_TEST_TRUE(flusher->mShardHashKeys.empty());
    // APSARA_TEST_EQUAL(CompressType::LZ4, flusher->mCompressor->GetCompressType());
    // APSARA_TEST_TRUE(flusher->mGroupSerializer);
    // APSARA_TEST_TRUE(flusher->mGroupListSerializer);
    // APSARA_TEST_EQUAL(static_cast<uint32_t>(INT32_FLAG(merge_log_count_limit)),
    //                   flusher->mBatcher.mEventFlushStrategy.GetMaxCnt());
    // APSARA_TEST_EQUAL(static_cast<uint32_t>(INT32_FLAG(batch_send_metric_size)),
    //                   flusher->mBatcher.mEventFlushStrategy.GetMaxSizeBytes());
    // uint32_t timeout = static_cast<uint32_t>(INT32_FLAG(batch_send_interval)) / 2;
    // APSARA_TEST_EQUAL(static_cast<uint32_t>(INT32_FLAG(batch_send_interval)) - timeout,
    //                   flusher->mBatcher.mEventFlushStrategy.GetTimeoutSecs());
    // APSARA_TEST_EQUAL(static_cast<uint32_t>(INT32_FLAG(batch_send_metric_size)),
    //                   flusher->mBatcher.mGroupFlushStrategy->GetMaxSizeBytes());
    // APSARA_TEST_EQUAL(timeout, flusher->mBatcher.mGroupFlushStrategy->GetTimeoutSecs());
}


UNIT_TEST_CASE(FlusherArmsMetricsUnittest, OnSuccessfulInit)
} // namespace logtail

int main(int argc, char** argv) {
    InitUnittestMain();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
