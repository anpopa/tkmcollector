#include "Application.h"
#include "Defaults.h"

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>

#include <pwd.h>
#include <unistd.h>

using namespace std;
using namespace tkm::control;
namespace fs = std::filesystem;

static void terminate(int signum) { exit(EXIT_SUCCESS); }

auto main(int argc, char **argv) -> int {
  const char *config_path = nullptr;
  const char *device_id = nullptr;
  const char *device_name = nullptr;
  const char *device_address = nullptr;
  const char *device_port = nullptr;

  bool help = false;
  bool force = false;
  bool init_database = false;
  bool terminate_collector = false;
  bool list_devices = false;
  bool add_device = false;
  bool remove_device = false;
  bool connect_device = false;
  bool disconnect_device = false;
  bool start_device_session = false;
  bool stop_device_session = false;
  int long_index = 0;
  int c;

  struct option longopts[] = {{"help", no_argument, nullptr, 'h'},
                              {"force", no_argument, nullptr, 'f'},
                              {"initDatabase", no_argument, nullptr, 'i'},
                              {"terminateCollector", no_argument, nullptr, 't'},
                              {"listDevices", no_argument, nullptr, 'l'},
                              {"addDevice", no_argument, nullptr, 'a'},
                              {"remDevice", no_argument, nullptr, 'r'},
                              {"config", required_argument, nullptr, 'o'},
                              {"Id", required_argument, nullptr, 'I'},
                              {"Name", required_argument, nullptr, 'N'},
                              {"Address", required_argument, nullptr, 'A'},
                              {"Port", required_argument, nullptr, 'P'},
                              {"connect", required_argument, nullptr, 'c'},
                              {"disconnect", required_argument, nullptr, 'd'},
                              {"startSession", required_argument, nullptr, 's'},
                              {"stopSession", required_argument, nullptr, 'x'},
                              {nullptr, 0, nullptr, 0}};

  while ((c = getopt_long(argc, argv, "hfitlarocdsxI:N:A:P:", longopts,
                          &long_index)) != -1) {
    switch (c) {
    case 'o':
      config_path = optarg;
      break;
    case 'f':
      force = true;
      break;
    case 'i':
      init_database = true;
      break;
    case 't':
      terminate_collector = true;
      break;
    case 'l':
      list_devices = true;
      break;
    case 'a':
      add_device = true;
      break;
    case 'r':
      remove_device = true;
      break;
    case 'c':
      connect_device = true;
      break;
    case 'd':
      disconnect_device = true;
      break;
    case 's':
      start_device_session = true;
      break;
    case 'x':
      stop_device_session = true;
      break;
    case 'I':
      device_id = optarg;
      break;
    case 'N':
      device_name = optarg;
      break;
    case 'A':
      device_address = optarg;
      break;
    case 'P':
      device_port = optarg;
      break;
    case 'h':
      help = true;
      break;
    default:
      break;
    }
  }

  if (terminate_collector) {
    if (!force) {
      cout << "Terminate server can only be used with force option" << endl;
      exit(EXIT_FAILURE);
    }
  }

  if (help) {
    cout << "TKM-Control: TaskMonitor collector control version: "
         << tkm::tkmDefaults.getFor(tkm::Defaults::Default::Version) << "\n\n";
    cout << "Usage: tkm-control [OPTIONS] \n\n";
    cout << "  General:\n";
    cout
        << "     --config, -o              <string>  Configuration file path\n";
    cout << "     --force, -f               <noarg>   Force actions\n";
    cout << "     --terminateCollector, -t  <noarg>   Ask tkm-collector to "
            "terminate\n";
    cout << "  Database:\n";
    cout << "     --initDatabase, -i        <noarg>   Initialize database\n";
    cout << "  Devices:\n";
    cout << "     --listDevices, -l         <noarg>   Get list of devices from "
            "database\n";
    cout << "     --addDevice,  -a          <noarg>   Add a new device to the "
            "database. "
            "Require:\n";
    cout << "         --Name, -N            <string>  Device name\n";
    cout << "         --Address, -A         <string>  Device IP address\n";
    cout << "         --Port, -P            <int>     Device port number\n";
    cout << "     --removeDevice,  -r       <noarg>   Remove user from "
            "database. Require:\n";
    cout << "         --Id, -I              <string>  Device ID\n";
    cout << "     --connect, -c             <noarg>   Connect device to "
            "taskmonitor"
            "Require:\n";
    cout << "         --Id, -I              <string>  Device ID\n";
    cout << "     --disconnect, -d          <noarg>   Disconnect device from "
            "taskmonitor"
            "Require:\n";
    cout << "         --Id, -I              <string>  Device ID\n";
    cout << "     --startSession, -s        <noarg>   Start collecting data "
            "from device"
            "Require:\n";
    cout << "         --Id, -I              <string>  Device ID\n";
    cout << "     --stopSession, -x         <noarg>   Stop collecting data "
            "from device"
            "Require:\n";
    cout << "         --Id, -I              <string>  Device ID\n";
    cout << "  Help:\n";
    cout << "     --help, -h                          Print this help\n\n";

    exit(EXIT_SUCCESS);
  }

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);

  fs::path configPath(
      tkm::tkmDefaults.getFor(tkm::Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!fs::exists(config_path)) {
      cout << "Provided configuration file cannot be accessed: " << config_path
           << endl;
      return EXIT_FAILURE;
    }
    configPath = string(config_path);
  }

  try {
    tkm::control::Application app{"TKM-Control", "TKM Control", configPath};

    if (init_database) {
      Command::Request rq{.action = Command::Action::InitDatabase};
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }
    if (list_devices) {
      Command::Request rq{.action = Command::Action::GetDevices};
      app.getCommand()->addRequest(rq);
    }
    if (add_device) {
      Command::Request rq{.action = Command::Action::AddDevice};
      rq.args.emplace(tkm::Defaults::Arg::DeviceName, device_name);
      rq.args.emplace(tkm::Defaults::Arg::DeviceAddress, device_address);
      rq.args.emplace(tkm::Defaults::Arg::DevicePort, device_port);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (remove_device) {
      Command::Request rq{.action = Command::Action::RemoveDevice};
      rq.args.emplace(tkm::Defaults::Arg::DeviceId, device_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (connect_device) {
      Command::Request rq{.action = Command::Action::ConnectDevice};
      rq.args.emplace(tkm::Defaults::Arg::DeviceId, device_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (disconnect_device) {
      Command::Request rq{.action = Command::Action::DisconnectDevice};
      rq.args.emplace(tkm::Defaults::Arg::DeviceId, device_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (start_device_session) {
      Command::Request rq{.action = Command::Action::StartDeviceSession};
      rq.args.emplace(tkm::Defaults::Arg::DeviceId, device_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (stop_device_session) {
      Command::Request rq{.action = Command::Action::StopDeviceSession};
      rq.args.emplace(tkm::Defaults::Arg::DeviceId, device_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (terminate_collector) {
      Command::Request rq{.action = Command::Action::TerminateCollector};
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    // Request initial connection
    Dispatcher::Request connectRequest{
        .action = Dispatcher::Action::Connect,
    };
    app.getDispatcher()->pushRequest(connectRequest);

    app.run();
  } catch (std::exception &e) {
    cout << "Application start failed. " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
