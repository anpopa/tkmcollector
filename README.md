# Task Monitor Collector

Data collector for remote taskmonitor devices

## Description

The project implements a host service that can be used to collect TaskMonitor traces from remote devices and store the output in a database. This method allows post events data analyses and data visualization.

## Download
`# git clone --recurse-submodules https://gitlab.com/taskmonitor/tkm-collector.git`

## Dependencies

TKM-Collector depends on the following libraries

| Library | Reference | Info |
| ------ | ------ | ------ |
| protobuf | https://developers.google.com/protocol-buffers | Data serialization |
| libsystemd | https://github.com/systemd/systemd/tree/main/src/libsystemd | Optional if WITH_SYSTEMD is ON 

## Build
### Compile options

| Option | Default | Info |
| ------ | ------ | ------ |
| WITH_SYSTEMD | ON | Enable systemd service and watchdog support |

### Local Build
`mkdir build && cd build && cmake .. && make `