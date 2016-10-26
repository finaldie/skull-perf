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
    skullcpp::Config::instance().load(config);
}

static
void skull_service_release(skullcpp::Service& service)
{
    std::cout << "skull service release" << std::endl;
}

static
void _timerjob(skullcpp::Service& service, skullcpp::ServiceApiData& apiData) {
    //std::cout << "timer job triggered" << std::endl;
}

static
void _timerjob_error(const skullcpp::Service& service, skullcpp::ServiceApiData& apiData) {
    //std::cout << "timer job error occurred (service busy)" << std::endl;

    SKULLCPP_LOG_WARN("_timerjob_error", "service busy",
                      "Please increase the max value of the service queue");
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

    auto& apiResp = (skull::service::s1::get_resp&)response;
    apiResp.set_response("Hi new bie");

    // Create a service job which is used for delay the whole process for
    //  X ms
    int ret = service.createJob((uint32_t)10, skull_BindSvcJobW(_timerjob),
                                skull_BindSvcJobError(_timerjob_error));
    if (ret) {
        SKULLCPP_LOG_ERROR("svc.test.get-2", "service job creation failure", "perf-test");
    }
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
