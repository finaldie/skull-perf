#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "skullcpp/api.h"
#include "skull_metrics.h"
#include "skull_protos.h"
#include "config.h"

/**
 * Module Initialization Function.
 *
 * @note This function be called during the starting phase before the server
 *        is ready for serving
 */
static
void module_init(const skull_config_t* config)
{
    std::cout << "module(test): init" << std::endl;
    SKULLCPP_LOG_TRACE("skull trace log test " << 1);
    SKULLCPP_LOG_DEBUG("skull debug log test " << 2);
    SKULLCPP_LOG_INFO("1", "skull info log test " << 3);
    SKULLCPP_LOG_WARN("1", "skull warn log test " << 4, "ignore, this is test");
    SKULLCPP_LOG_ERROR("1", "skull error log test " << 5, "ignore, this is test");
    SKULLCPP_LOG_FATAL("1", "skull fatal log test " << 5, "ignore, this is test");

    // Load skull_config to skullcpp::Config
    auto& conf = skullcpp::Config::instance();
    conf.load(config);

    SKULLCPP_LOG_DEBUG("config test_item: " << conf.test_item());
    SKULLCPP_LOG_DEBUG("config test_rate: " << conf.test_rate());
    SKULLCPP_LOG_DEBUG("config test_name: " << conf.test_name());
    SKULLCPP_LOG_DEBUG("config test_bool: " << conf.test_bool());
}

/**
 * Module Release Function
 *
 * @note This function be called during the server shutdown phase
 */
static
void module_release()
{
    std::cout << "module(test): released" << std::endl;
    SKULLCPP_LOG_INFO("5", "module(test): released");
}

/**
 * Unpack the request from raw bytes stream.
 *
 * @note This function only be executed when this module is the first module
 *        in the workflow
 *
 * @return
 *   - = 0: Still need more data
 *   - > 0: Already unpacked a completed request, the returned value will be consumed
 */
static
size_t module_unpack(skullcpp::Txn& txn, const void* data, size_t data_sz)
{
    skull_metrics_module.request.inc(1);
    //std::cout << "module_unpack(test): data sz: " << data_sz << std::endl;
    //SKULLCPP_LOG_INFO("2", "module_unpack(test): data sz:" << data_sz);

    size_t consumed = 0;

    // parse the request (assume all the request is get/)
    std::string request((const char*)data, data_sz);
    //std::cout << "raw request: [" << request << "]" << std::endl;

    std::string::size_type i = request.rfind("\r\n\r\n");
    if (i != std::string::npos) {
        // the request is a completed one
        consumed = i + 4;
        //std::cout << "found one" << std::endl;
    }

    // deserialize data to transcation data
    auto& example = (skull::workflow::example&)txn.data();
    example.set_data(data, consumed);
    //std::cout << "consumed: " << consumed << std::endl;
    return consumed;
}

static
int svc_api_callback(skullcpp::Txn& txn, const std::string& apiName,
                     const google::protobuf::Message& request,
                     const google::protobuf::Message& response)
{
    const auto& apiReq  = (skull::service::s1::get_req&)request;
    const auto& apiResp = (skull::service::s1::get_resp&)response;

    //std::cout << "svc_api_callback.... apiName: " << apiName << std::endl;
    //std::cout << "svc_api_callback.... request: " << apiReq.name() << std::endl;
    //std::cout << "svc_api_callback.... response: " << apiResp.response() << std::endl;

    auto& example = (skull::workflow::example&)txn.data();
    example.set_data(apiResp.response());
    return 0;
}

/**
 * Module runnable function, when the transaction is moved to this module, this
 *  function will be executed.
 *
 * @return
 *  -     0: Everything is ok, transation will move to next module
 *  - non-0: Error occurred, transcation will be cancelled, and will run the
 *          `pack` function of the last module
 */
static
int module_run(skullcpp::Txn& txn)
{
    //auto& example = (skull::workflow::example&)txn.data();

    //std::cout << "receive data: " << example.data() << std::endl;
    //SKULLCPP_LOG_INFO("3", "receive data: " << example.data());
    
    skull::service::s1::get_req req;
    req.set_name("hello service");

    skullcpp::Txn::IOStatus ret =
        txn.serviceCall("s1", "get", req, 0, svc_api_callback);

    return 0;
}

#define RESPONSE_CONTENT \
"HTTP/1.1 200 OK\r\n" \
"Server: Skull-Engine\r\n" \
"Content-Length: 43\r\n" \
"Connection: Keep-Alive\r\n" \
"Content-Type: text/html\r\n" \
"\r\n" \
"<html><body><h1>It works</h1></body></html>"

/**
 * Pack the data to `TxnData`, the data will be sent back to the sender. This
 *  function only be called when this module is the last module in the workflow
 */
static
void module_pack(skullcpp::Txn& txn, skullcpp::TxnData& txndata)
{
    auto& example = (skull::workflow::example&)txn.data();

    if (txn.status() != skullcpp::Txn::TXN_OK) {
        SKULLCPP_LOG_ERROR("5", "module_pack(test): error status occurred. txn data: "
                            << example.data(), "This error is expected");
        txndata.append("error");
        return;
    }

    skull_metrics_module.response.inc(1);
    //std::cout << "module_pack(test): data sz: " << sizeof(RESPONSE_CONTENT) << std::endl;
    //SKULLCPP_LOG_INFO("4", "module_pack(test): data sz:" << sizeof(RESPONSE_CONTENT));
    txndata.append(RESPONSE_CONTENT);
}

/******************************** Register Module *****************************/
static skullcpp::ModuleEntry module_entry = {
    module_init,
    module_release,
    module_run,
    module_unpack,
    module_pack
};

SKULLCPP_MODULE_REGISTER(&module_entry)
