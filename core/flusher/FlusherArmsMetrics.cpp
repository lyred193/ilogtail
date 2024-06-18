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
#include "sender/Sender.h"
#include "serializer/ArmsSerializer.h"


using namespace std;

namespace logtail {


bool FlusherArmsMetrics::Init(const Json::Value& config, Json::Value& optionalGoPipeline) {
    //
}

bool FlusherArmsMetrics::Register() {
}
bool FlusherArmsMetrics::Unregister(bool isPipelineRemoving) {
}
void FlusherArmsMetrics::Send(PipelineEventGroup&& g) {
}
void FlusherArmsMetrics::Flush(size_t key) {
}
void FlusherArmsMetrics::FlushAll() {
}


void FlusherArmsMetrics::SerializeAndPush(BatchedEventsList&& groupList) {
    string serializeArmsMetricData, serializeErrMsg;
    mGroupListSerializer->Serialize(std::move(groupList), serializeArmsMetricData, serializeErrMsg);
}


void FlusherArmsMetrics::SerializeAndPush(vector<BatchedEventsList>&& groupLists) {
    for (auto& groupList : groupLists) {
        SerializeAndPush(std::move(groupList));
    }
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
    //  snappy compress
    // snappy::Compress(body,body, compressBody);
    string HTTP_POST = "POST";
    string host = "";
    int32_t port = 443;
    string operation = "";
    string queryString = "";
    int32_t mTimeout = 600;
    sdk::Response* response = new sdk::PostLogStoreLogsResponse();
    SendClosure* callBack = new SendClosure;
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
                                true,
                                callBack,
                                response);
}

} // namespace logtail

int main() {
    printf("test .. ");
}