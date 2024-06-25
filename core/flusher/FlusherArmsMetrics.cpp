//
// Created by lurious on 2024/6/17.
//
#include "flusher/FlusherArmsMetrics.h"

#include "batch/FlushStrategy.h"
#include "common/EndpointUtil.h"
#include "common/HashUtil.h"
#include "common/LogtailCommonFlags.h"
#include "common/ParamExtractor.h"
#include "compression/CompressorFactory.h"
#include "logger/Logger.h"
#include "pipeline/Pipeline.h"
#include "queue/SenderQueueItem.h"
#include "sdk/Common.h"
#include "sender/PackIdManager.h"
#include "sender/SLSClientManager.h"
#include "sender/Sender.h"
#include "serializer/ArmsSerializer.h"

using namespace std;
DEFINE_FLAG_INT32(arms_metrics_batch_send_interval, "batch sender interval (second)(default 3)", 3);
DEFINE_FLAG_INT32(arms_metrics_merge_count_limit, "log count in one logGroup at most", 100);
DEFINE_FLAG_INT32(arms_metrics_batch_send_metric_size,
                  "batch send metric size limit(bytes)(default 256KB)",
                  256 * 1024);

namespace logtail {

DEFINE_FLAG_INT32(arms_test_merged_buffer_interval, "default flush merged buffer, seconds", 2);


const string FlusherArmsMetrics::sName = "flusher_arms_metrics";

FlusherArmsMetrics::FlusherArmsMetrics() : mRegion(Sender::Instance()->GetDefaultRegion()) {
    LOG_INFO(sLogger, ("start test arms metrics flusher", "CREATE by lurious"));
    // FIXME must be remove this in release batch
    // new Thread(bind(&FlusherArmsMetrics::MockMetricsEvent, this));
    // MockMetricsEvent();
}

bool FlusherArmsMetrics::Init(const Json::Value& config, Json::Value& optionalGoPipeline) {
    mRegion = "cn-hangzhou";
    string errorMsg;
    // Region
    if (!GetOptionalStringParam(config, "Region", mRegion, errorMsg)) {
        PARAM_WARNING_DEFAULT(mContext->GetLogger(),
                              mContext->GetAlarm(),
                              errorMsg,
                              mRegion,
                              sName,
                              mContext->GetConfigName(),
                              mContext->GetProjectName(),
                              mContext->GetLogstoreName(),
                              mContext->GetRegion());
    }
    // licenseKey
    if (!GetOptionalStringParam(config, "Licensekey", licenseKey, errorMsg)) {
        PARAM_WARNING_DEFAULT(mContext->GetLogger(),
                              mContext->GetAlarm(),
                              errorMsg,
                              mRegion,
                              sName,
                              mContext->GetConfigName(),
                              mContext->GetProjectName(),
                              mContext->GetLogstoreName(),
                              mContext->GetRegion());
    }
    // pushAppId
    if (!GetOptionalStringParam(config, "PushAppId", pushAppId, errorMsg)) {
        PARAM_WARNING_DEFAULT(mContext->GetLogger(),
                              mContext->GetAlarm(),
                              errorMsg,
                              mRegion,
                              sName,
                              mContext->GetConfigName(),
                              mContext->GetProjectName(),
                              mContext->GetLogstoreName(),
                              mContext->GetRegion());
    }
    mCompressor = CompressorFactory::GetInstance()->Create(config, *mContext, sName, CompressType::SNAPPY);
    //
    mGroupListSerializer = make_unique<ArmsMetricsEventGroupListSerializer>(this);

    DefaultFlushStrategyOptions strategy{static_cast<uint32_t>(INT32_FLAG(arms_metrics_batch_send_metric_size)),
                                         static_cast<uint32_t>(INT32_FLAG(arms_metrics_merge_count_limit)),
                                         static_cast<uint32_t>(INT32_FLAG(arms_metrics_batch_send_interval))};
    if (!mBatcher.Init(Json::Value(), this, strategy, true)) {
        // when either exactly once is enabled or ShardHashKeys is not empty, we don't enable group batch
        LOG_WARNING(sLogger, ("mBatcher init info: ", "init err!"));
        return false;
    }
    LOG_INFO(sLogger, ("init info: ", "arms metrics init successful !"));

    return true;
}

bool FlusherArmsMetrics::Register() {
    Sender::Instance()->IncreaseProjectReferenceCnt(mProject);
    Sender::Instance()->IncreaseRegionReferenceCnt(mRegion);
    SLSClientManager::GetInstance()->IncreaseAliuidReferenceCntForRegion(mRegion, mAliuid);
    return true;
}
bool FlusherArmsMetrics::Unregister(bool isPipelineRemoving) {
    Sender::Instance()->DecreaseProjectReferenceCnt(mProject);
    Sender::Instance()->DecreaseRegionReferenceCnt(mRegion);
    SLSClientManager::GetInstance()->DecreaseAliuidReferenceCntForRegion(mRegion, mAliuid);
    return true;
}

void FlusherArmsMetrics::MockMetricsEvent() {
    static int32_t lastMergeTime = 0;
    while (true) {
        int32_t curTime = time(NULL);
        if (curTime - lastMergeTime >= INT32_FLAG(arms_test_merged_buffer_interval)) {
            std::unique_ptr<PipelineEventGroup> mEventGroup;
            for (int i = 100 - 1; i >= 0; i--) {
                auto metricsEvent = mEventGroup->AddMetricEvent();
                metricsEvent->SetName("arms_request_count");
                metricsEvent->SetTag(StringView("workloadName"), StringView("cmonitor_agent"));
                metricsEvent->SetTag(StringView("workloadKind"), StringView("deployment"));
                metricsEvent->SetTag(StringView("app"), StringView("cmonitor_agent"));
                // metricsEvent->SetValue<double>(120.0);
            }
            Send(std::move(*mEventGroup));
            lastMergeTime = curTime;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void FlusherArmsMetrics::Send(PipelineEventGroup&& g) {
    LOG_INFO(sLogger, ("FlusherArmsMetrics Send receiver Data", "CREATE by lurious"));

    if (g.IsReplay()) {
        // SerializeAndPush(std::move(g));
        LOG_INFO(sLogger, (" IsReplay ", "true"));
    } else {
        vector<BatchedEventsList> res;
        mBatcher.Add(std::move(g), res);
        SerializeAndPush(std::move(res));
    }
}
void FlusherArmsMetrics::Flush(size_t key) {
    // BatchedEventsList res;
    // mBatcher.FlushQueue(key, res);
    // SerializeAndPush(std::move(res));
}
void FlusherArmsMetrics::FlushAll() {
    LOG_INFO(sLogger, ("FlusherArmsMetrics FlushAll flusher all Data", "CREATE by lurious"));

    vector<BatchedEventsList> res;
    mBatcher.FlushAll(res);
    SerializeAndPush(std::move(res));
}


void FlusherArmsMetrics::SerializeAndPush(BatchedEventsList&& groupList) {
}


void FlusherArmsMetrics::SerializeAndPush(vector<BatchedEventsList>&& groupLists) {
    LOG_INFO(sLogger, ("SerializeAndPush :", "start to serialed res info"));
    LOG_INFO(sLogger, ("SerializeAndPush groupLists size :", groupLists.size()));
    string serializeArmsMetricData, compressedData, serializeErrMsg;
    mGroupListSerializer->Serialize(std::move(groupLists), serializeArmsMetricData, serializeErrMsg);
    // LOG_INFO(sLogger, ("serialed res info:", serializeArmsMetricData));
    size_t packageSize = 0;
    packageSize += serializeArmsMetricData.size();
    if (mCompressor) {
        if (!mCompressor->Compress(serializeArmsMetricData, compressedData, serializeErrMsg)) {
            LOG_WARNING(mContext->GetLogger(),
                        ("failed to compress arms metrics event group", serializeErrMsg)("action", "discard data")(
                            "plugin", sName)("config", mContext->GetConfigName()));
            mContext->GetAlarm().SendAlarm(COMPRESS_FAIL_ALARM,
                                           "failed to compress arms event group: " + serializeErrMsg
                                               + "\taction: discard data\tplugin: " + sName
                                               + "\tconfig: " + mContext->GetConfigName(),
                                           mContext->GetProjectName(),
                                           mContext->GetLogstoreName(),
                                           mContext->GetRegion());
            return;
        }
    } else {
        compressedData = serializeArmsMetricData;
    }
    // LOG_INFO(sLogger, ("compressedData serialed res info:", compressedData));

    PushToQueue(std::move(compressedData), packageSize, RawDataType::EVENT_GROUP_LIST);
}

void FlusherArmsMetrics::PushToQueue(string&& data,
                                     size_t rawSize,
                                     RawDataType type,
                                     const string& logstore,
                                     const string& shardHashKey,
                                     RangeCheckpointPtr&& eoo) {
    SLSSenderQueueItem* item = new SLSSenderQueueItem(std::move(data),
                                                      rawSize,
                                                      this,
                                                      eoo ? eoo->fbKey : 0,
                                                      shardHashKey,
                                                      std::move(eoo),
                                                      type,
                                                      eoo ? false : true,
                                                      logstore.empty() ? "" : logstore);
    Sender::Instance()->PutIntoBatchMap(item, mRegion);
    // auto asynRequest = BuildRequest(item);
    // LOG_INFO(sLogger, ("asynRequest info:", asynRequest->mHost));
    // LOG_INFO(sLogger, ("asynRequest request body:", asynRequest->mBody));
}

sdk::AsynRequest* FlusherArmsMetrics::BuildRequest(SenderQueueItem* item) const {
    map<string, string> httpHeader;
    httpHeader[logtail::sdk::CONTENT_TYPE] = "text/plain";
    httpHeader["content.encoding"] = "snappy";
    string body = item->mData;
    // string compressBody;
    string HTTP_POST = "POST";
    auto host = GetArmsPrometheusGatewayHost();
    LOG_INFO(sLogger, ("BuildRequest host ...", host));
    int32_t port = 80;
    auto operation = GetArmsPrometheusGatewayOperation();
    LOG_INFO(sLogger, ("BuildRequest operation ...", operation));

    string queryString = "";
    int32_t mTimeout = 600;
    sdk::Response* response = new sdk::PostLogStoreLogsResponse();
    SendClosure* sendClosure = new SendClosure;
    sendClosure->mDataPtr = item;
    string mInterface = "";

    return new sdk::AsynRequest(HTTP_POST,
                                host,
                                port,
                                operation,
                                queryString,
                                httpHeader,
                                body,
                                mTimeout,
                                mInterface,
                                false,
                                sendClosure,
                                response);
}

std::string FlusherArmsMetrics::GetArmsPrometheusGatewayHost() const {
    std::string urlPrefix = "http://";
    std::string inner = "-intranet";
    std::string urlCommon = ".arms.aliyuncs.com";
    std::string host = mRegion + urlCommon;
    return host;
}

std::string FlusherArmsMetrics::GetArmsPrometheusGatewayOperation() const {
    std::string operation = "/collector/arms/ebpf/";
    operation.append(licenseKey);
    operation.append("/");
    operation.append(pushAppId);
    return operation;
}


} // namespace logtail
