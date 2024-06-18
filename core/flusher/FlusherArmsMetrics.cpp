//
// Created by lurious on 2024/6/17.
//
#include "flusher/FlusherArmsMetrics.h"

#include <snappy.h>

#include "batch/FlushStrategy.h"
#include "common/EndpointUtil.h"
#include "common/HashUtil.h"
#include "common/LogtailCommonFlags.h"
#include "common/ParamExtractor.h"
#include "compression/CompressorFactory.h"
#include "pipeline/Pipeline.h"
#include "queue/SenderQueueItem.h"
#include "sdk/Common.h"
#include "sender/PackIdManager.h"
#include "sender/SLSClientManager.h"
#include "sender/Sender.h"
#include "serializer/ArmsSerializer.h"


using namespace std;

namespace logtail {

const string FlusherArmsMetrics::sName = "flusher_arms_metrics";

FlusherArmsMetrics::FlusherArmsMetrics() : mRegion(Sender::Instance()->GetDefaultRegion()) {
    std::cout << "start test arms metrics flusher" << std::endl;
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

    //
    // mGroupListSerializer = make_unique<ArmsMetricsEventGroupListSerializer>(this);
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
void FlusherArmsMetrics::Send(PipelineEventGroup&& g) {
    if (g.IsReplay()) {
        // SerializeAndPush(std::move(g));
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
    vector<BatchedEventsList> res;
    mBatcher.FlushAll(res);
    SerializeAndPush(std::move(res));
}


void FlusherArmsMetrics::SerializeAndPush(BatchedEventsList&& groupList) {
}


void FlusherArmsMetrics::SerializeAndPush(vector<BatchedEventsList>&& groupLists) {
    string serializeArmsMetricData, serializeErrMsg;
    mGroupListSerializer->Serialize(std::move(groupLists), serializeArmsMetricData, serializeErrMsg);
    size_t packageSize = 0;
    packageSize += serializeArmsMetricData.size();
    PushToQueue(std::move(serializeArmsMetricData), packageSize, RawDataType::EVENT_GROUP_LIST);
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
}


// req.Header.Set("Content-Type", "text/plain")
// req.Header.Set("content.encoding", "snappy")
// req.Header.Set("User-Agent", e.userAgent)
sdk::AsynRequest* FlusherArmsMetrics::BuildRequest(SenderQueueItem* item) const {
    map<string, string> httpHeader;
    httpHeader[logtail::sdk::CONTENT_TYPE] = "text/plain";
    httpHeader["content.encoding"] = "snappy";
    string body = "";
    string compressBody;
    // snappy compress
    // snappy::Compress(body, body.length(), compressBody);
    string HTTP_POST = "POST";
    auto host = GetArmsPrometheusGatewayHost();
    std::cout << "BuildRequest host ..." << std::endl;
    std::cout << host << std::endl;
    int32_t port = 80;
    auto operation = GetArmsPrometheusGatewayOperation();
    std::cout << "BuildRequest operation ..." << std::endl;
    std::cout << operation << std::endl;
    string queryString = "";
    int32_t mTimeout = 600;
    sdk::Response* response = new sdk::PostLogStoreLogsResponse();
    SendClosure* callBack = new SendClosure;
    string mInterface = "";
    std::cout << "BuildRequest end ..." << std::endl;

    return new sdk::AsynRequest(HTTP_POST,
                                host,
                                port,
                                operation,
                                queryString,
                                httpHeader,
                                compressBody,
                                mTimeout,
                                mInterface,
                                true,
                                callBack,
                                response);
}

std::string FlusherArmsMetrics::GetArmsPrometheusGatewayHost() const {
    std::string urlPrefix = "http://";
    std::string inner = "-intranet";
    std::string urlCommon = ".arms.aliyuncs.com";
    std::string host = urlPrefix + mRegion + urlCommon;
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
