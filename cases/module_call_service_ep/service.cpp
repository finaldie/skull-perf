#include <stdlib.h>

#include <iostream>
#include <string>
#include <google/protobuf/message.h>

#include <skullcpp/api.h>
#include "skull_protos.h"
#include "config.h"

// ====================== Service Init/Release =================================
/**
 * Initialize the service
 */
static
void skull_service_init(skullcpp::Service& service, const skull_config_t* config)
{
    std::cout << "skull service init" << std::endl;

    // Load skull_config to skullcpp::Config
    auto& conf = skullcpp::Config::instance();
    conf.load(config);

    SKULLCPP_LOG_INFO("svc.test.init", "Endpoint IP: " << conf.ep_ip());
    SKULLCPP_LOG_INFO("svc.test.init", "Endpoint port: " << conf.ep_port());
}

static
void skull_service_release(skullcpp::Service& service)
{
    std::cout << "skull service release" << std::endl;
}

// Since we send out 'hello ep' to echo back server, we expect to get
//  the same content returned.
static
ssize_t _ep_unpack(const void* data, size_t len)
{
    //std::cout << "ep received data len: " << len << std::endl;

    // Here '8' is the same of strlen("hello ep")
    if (len >= 8) {
        return 8;
    } else {
        return 0;
    }
}

static
void _ep_cb(const skullcpp::Service&, skullcpp::EPClientRet& ret)
{
    //std::cout << "in _ep_cb data len: " << ret.responseSize() << std::endl;
    //SKULL_LOG_INFO("svc.test.ep_cb", "status: %d, responseSize: %zu, latency: %d",
    //               ret.status(), ret.responseSize(), ret.latency());

    //SKULLCPP_LOG_INFO("svc.test.ep_cb1", "ip: " << ret.ip() << ", "
    //                  << "port: "    << ret.port()    << ", "
    //                  << "timeout: " << ret.timeout() << ", "
    //                  << "type: "    << ret.type()    << ", "
    //                  << "flags: "   << ret.flags());

    auto& apiData = ret.apiData();
    //apiData.request().PrintDebugString();
    auto& apiResp = (skull::service::s1::get_resp&)apiData.response();

    const std::string& rawResponse = apiResp.response();
    apiResp.set_response(rawResponse + ", " + std::string((const char*)ret.response(), ret.responseSize()));

    skullcpp::EPClient::Status st = ret.status();
    if (st == skullcpp::EPClient::Status::ERROR) {
        SKULLCPP_LOG_ERROR("svc.test.ep_cb-2", "ep cb error", "perf-test");
    } else if (st == skullcpp::EPClient::Status::TIMEOUT) {
        SKULLCPP_LOG_WARN("svc.test.ep_cb-3", "ep cb timeout", "perf-test");
    }
}

// ====================== Service APIs Calls ===================================
/**
 * Service API implementation. For the api which has read access, you can call
 *  `service.get()` to fetch the service data. For the api which has write
 *  access, you can also call the `service.set()` to store your service data.
 *
 * @note  DO NOT carry over the service data to another place, the only safe
 *        place for it is leaving it inside the service api scope
 */
static
void skull_service_getdata(const skullcpp::Service& service,
                           const google::protobuf::Message& request,
                           google::protobuf::Message& response)
{
    //std::cout << "skull service api: getdata" << std::endl;
    //SKULLCPP_LOG_INFO("svc.test.get-1", "service get data");

    auto& conf = skullcpp::Config::instance();
    auto& apiResp = (skull::service::s1::get_resp&)response;
    apiResp.set_response("Hi new bie");

    skullcpp::EPClient epClient;
    epClient.setType(skullcpp::EPClient::TCP);
    epClient.setIP(conf.ep_ip());
    epClient.setPort(conf.ep_port());
    epClient.setTimeout(50);
    epClient.setUnpack(skull_BindEpUnpack(_ep_unpack));

    skullcpp::EPClient::Status st =
        epClient.send(service, "hello ep", skull_BindEpCb(_ep_cb));

    if (st == skullcpp::EPClient::Status::ERROR) {
        SKULLCPP_LOG_ERROR("svc.test-get-2", "ep send error", "perf-test");
    } else if (st == skullcpp::EPClient::Status::TIMEOUT) {
        SKULLCPP_LOG_WARN("svc.test-get-3", "ep send timeout", "perf-test");
    }

    //std::cout << "ep status: " << st << std::endl;
    //SKULL_LOG_INFO("svc.test-get-2", "ep status: %d", st);
}

// ====================== Register Service =====================================
static skullcpp::ServiceReadApi api_read_tbl[] = {
    {"get", skull_service_getdata},
    {NULL, NULL}
};

static skullcpp::ServiceWriteApi api_write_tbl[] = {
    {NULL, NULL}
};

static skullcpp::ServiceEntry service_entry = {
    skull_service_init,
    skull_service_release,
    api_read_tbl,
    api_write_tbl
};

SKULLCPP_SERVICE_REGISTER(&service_entry)
