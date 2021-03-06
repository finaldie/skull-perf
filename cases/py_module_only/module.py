import yaml
import pprint

from skull import txn     as Txn
from skull import txndata as TxnData
from skull import logger  as Logger

from common import protos  as Protos
from common import metrics as Metrics
from common.proto import *

RESPONSE_CONTENT ="\
HTTP/1.1 200 OK\r\n\
Server: Skull-Engine\r\n\
Content-Length: 43\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html\r\n\
\r\n\
<html><body><h1>It works</h1></body></html>"

##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    print("py module init")
    Logger.info('0', 'config: {}'.format(pprint.pformat(config)))

    Logger.trace('py module init: trace test')
    Logger.debug('py module init: debug test')
    Logger.info('1', 'py module init: info test')
    Logger.warn('2', 'py module init: warn test', 'no suggestion')
    Logger.error('3', 'py module init: error test', 'no solution')
    Logger.fatal('4', 'py module init: fatal test', 'no solution')
    return

##
# Module Release Function, be called when shutdown phase
#
def module_release():
    print("py module release")
    return

##
# Input data unpack function, be called if this module is the 'first' module in
#  the workflow and there is input data incoming
#
# @param txn  Transcation context which is used for get shared transcation data,
#              invoke `iocall`
# @param data Input data
#
# @return How many bytes be consumed
#
def module_unpack(txn, data):
    #print "py module unpack"
    #Logger.info('5', 'receive data: {}'.format(data))

    # Parse the request
    consumed = 0
    idx = str(data).rfind('\r\n\r\n')
    if idx != -1:
       consumed = idx + 4
    else:
       return 0

    # Store data into txn sharedData
    example_msg = txn.data()
    example_msg.data = data[:consumed]
    #print "consumed: {}, len of data: {}".format(consumed, len(data))
    return consumed

##
# Input data unpack function, be called if this module is the 'last' module in
#  the workflow (It would no effect if there is no response needed)
#
# @param txn  Transcation context which is used for get shared transcation data,
#              invoke `iocall`
# @param data Input data
#
# @return How many bytes be consumed
#
def module_pack(txn, txndata):
    #print "py module pack"

    # Increase counters
    mod_metrics = Metrics.module()
    mod_metrics.response.inc(1)

    # Assemble response
    if txn.status() != Txn.Txn.TXN_OK:
        txndata.append('error')
    else:
        #example_msg = txn.data()
        #print "pack data: %s" % example_msg.data
        #Logger.info('7', 'module_pack: data sz: {}'.format(len(example_msg.data)))

        txndata.append(RESPONSE_CONTENT)

##
# Module Runnable Entry, be called when this module be picked up in current
#  workflow
#
# @param txn Transcation context
#
# @return - True if no error
#         - False if error occurred
def module_run(txn):
    #print "py module run"

    # Increase counters
    mod_metrics = Metrics.module()
    mod_metrics.request.inc(1)
    return True
