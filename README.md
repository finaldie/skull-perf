# skull-perf
Performance Test Framework for Skull

## Preparation
First, this project assumed that the _Skull_ has already been installed in your environment.

## Build
* Clone and Build
```console
git submodule update --init --recursive
```

## Run Perf Case
```console
make case=echo_back
```
**Notes:** After you tested one case, run `killall skull-engine` and then start next case.

### Cases
Case Name              | Description
-----------------------|------------
echo_back              | A simple echo back server, which accept HTTP GET request and return the response
module_call_service    | A module call a service api, then response to client
module_call_service_ep | A module call a service api, and in the service api it create a ep client to callout another service
