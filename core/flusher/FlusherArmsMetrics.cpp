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
#include "pipeline/Pipeline.h"
#include "queue/SenderQueueItem.h"
#include "sdk/Common.h"
#include "sender/PackIdManager.h"
#include "sender/Sender.h"
#include "serializer/SLSSerializer.h"
#include <snappy.h>


using namespace std;

namespace logtail {


bool FlusherArmsMetrics::Init(const Json::Value& config, Json::Value& optionalGoPipeline) {
    //
    mGroupListSerializer = make_unique<SLSEventGroupListSerializer>(this);
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
    httpHeader[CONTENT_TYPE] = "text/plain";
    httpHeader["content.encoding"] = "snappy";
    string body = "";
    bool mUsingHTTPS = true;
    string host = "";
    int32_t port = 443;
    string operation = "";
    string queryString = "";
    int32_t mTimeout = 600;
    Response* response = new PostLogStoreLogsResponse();
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
                                mUsingHTTPS,
                                callBack,
                                response);
}

} // namespace logtail

int main() {
    printf("test .. ")
}