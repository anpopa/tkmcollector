/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Main function
 * @details   Application main function
 *-
 */

#include "Application.h"
#include "Defaults.h"

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>

#include <pwd.h>
#include <taskmonitor/Helpers.h>
#include <unistd.h>

static void terminate(int signum)
{
  static_cast<void>(signum); // UNUSED
  exit(EXIT_SUCCESS);
}

auto main(int argc, char **argv) -> int
{
  const char *config_path = nullptr;
  const char *unique_id = nullptr;
  const char *device_name = nullptr;
  const char *device_address = nullptr;
  const char *device_port = nullptr;

  bool help = false;
  bool force = false;
  bool init_database = false;
  bool quit = false;
  bool list_devices = false;
  bool list_sessions = false;
  bool add_device = false;
  bool remove_device = false;
  bool connect_device = false;
  bool disconnect_device = false;
  bool remove_session = false;
  bool start_collecting = false;
  bool stop_collecting = false;
  int long_index = 0;
  int c;

  struct option longopts[] = {{"help", no_argument, nullptr, 'h'},
                              {"force", no_argument, nullptr, 'f'},
                              {"initDatabase", no_argument, nullptr, 'i'},
                              {"quit", no_argument, nullptr, 'q'},
                              {"listDevices", no_argument, nullptr, 'l'},
                              {"listSessions", no_argument, nullptr, 'j'},
                              {"addDevice", no_argument, nullptr, 'a'},
                              {"remDevice", no_argument, nullptr, 'r'},
                              {"remSession", no_argument, nullptr, 'g'},
                              {"config", required_argument, nullptr, 'o'},
                              {"Id", required_argument, nullptr, 'I'},
                              {"Name", required_argument, nullptr, 'N'},
                              {"Address", required_argument, nullptr, 'A'},
                              {"Port", required_argument, nullptr, 'P'},
                              {"connect", required_argument, nullptr, 'c'},
                              {"disconnect", required_argument, nullptr, 'd'},
                              {"startCollecting", required_argument, nullptr, 's'},
                              {"stopCollecting", required_argument, nullptr, 'x'},
                              {nullptr, 0, nullptr, 0}};

  while ((c = getopt_long(argc, argv, "hfiqljarcdsxgo:I:N:A:P:", longopts, &long_index)) != -1) {
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
    case 'q':
      quit = true;
      break;
    case 'l':
      list_devices = true;
      break;
    case 'j':
      list_sessions = true;
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
    case 'g':
      remove_session = true;
      break;
    case 's':
      start_collecting = true;
      break;
    case 'x':
      stop_collecting = true;
      break;
    case 'I':
      unique_id = optarg;
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
    default:
      help = true;
      break;
    }
  }

  // Check for valid options
  if (!add_device && !remove_device && !connect_device && !disconnect_device && !start_collecting &&
      !stop_collecting && !init_database && !quit && !list_devices && !list_sessions &&
      !remove_session && !help) {
    std::cout << "Please select one top level option" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (add_device && (remove_device || connect_device || disconnect_device || start_collecting ||
                     stop_collecting || init_database || quit || list_devices || list_sessions)) {
    std::cout << "Add device option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (remove_device &&
      (add_device || connect_device || disconnect_device || start_collecting || stop_collecting ||
       init_database || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Remove device option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (connect_device &&
      (add_device || remove_device || disconnect_device || start_collecting || stop_collecting ||
       init_database || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Connect device option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (disconnect_device &&
      (add_device || remove_device || connect_device || start_collecting || stop_collecting ||
       init_database || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Disconnect device option cannot be used with other top level options"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  if (start_collecting &&
      (add_device || remove_device || connect_device || disconnect_device || stop_collecting ||
       init_database || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Start connecting option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (stop_collecting &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       init_database || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Start connecting option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (init_database &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       stop_collecting || quit || list_devices || list_sessions || remove_session)) {
    std::cout << "Stop connecting option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (quit &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       stop_collecting || init_database || list_devices || list_sessions || remove_session)) {
    std::cout << "Quit collector option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (list_devices &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       stop_collecting || init_database || quit || list_sessions || remove_session)) {
    std::cout << "List devices option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (list_sessions &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       stop_collecting || init_database || quit || list_devices || remove_session)) {
    std::cout << "List sessions option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (remove_session &&
      (add_device || remove_device || connect_device || disconnect_device || start_collecting ||
       stop_collecting || init_database || quit || list_devices || list_sessions)) {
    std::cout << "List sessions option cannot be used with other top level options" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (quit) {
    if (!force) {
      std::cout << "Quit collector can only be used with force option" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (add_device) {
    if (!device_name || !device_address || !device_port) {
      std::cout << "Please provide the complete device data" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (remove_device || connect_device || disconnect_device || start_collecting || stop_collecting) {
    if (!unique_id) {
      std::cout << "Please provide the device hash id" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (remove_session && !unique_id) {
    std::cout << "Please provide the device hash id" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (help) {
    std::cout << "TaskMonitorCollector-Control: TaskMonitor collector control utility\n"
              << "Version: " << tkm::tkmDefaults.getFor(tkm::Defaults::Default::Version)
              << " libtkm: " << TKMLIB_VERSION << "\n\n";
    std::cout << "Usage: tkmcontrol [OPTIONS] \n\n";
    std::cout << "  General:\n";
    std::cout << "     --config, -o              <string>  Configuration file path\n";
    std::cout << "     --force, -f               <noarg>   Force actions\n";
    std::cout << "     --quit, -q                <noarg>   Ask tkm-collector to terminate\n";
    std::cout << "  Database:\n";
    std::cout << "     --initDatabase, -i        <noarg>   Initialize database\n";
    std::cout << "  Devices:\n";
    std::cout << "     --listDevices, -l         <noarg>   Get list of devices from database\n";
    std::cout << "     --listSessions, -j        <noarg>   Get list of sessions for device\n";
    std::cout << "        Optional:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "     --addDevice,  -a          <noarg>   Add a new device to the database\n";
    std::cout << "        Require:\n";
    std::cout << "         --Name, -N            <string>  Device name\n";
    std::cout << "         --Address, -A         <string>  Device IP address\n";
    std::cout << "         --Port, -P            <int>     Device port number\n";
    std::cout << "     --remDevice,  -r          <noarg>   Remove user from database\n";
    std::cout << "        Require:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "     --remSession, -g          <noarg>   Remove session from database\n";
    std::cout << "        Require:\n";
    std::cout << "         --Id, -I              <string>  Session ID\n";
    std::cout << "     --connect, -c             <noarg>   Connect device to taskmonitor\n";
    std::cout << "       Require:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "     --disconnect, -d          <noarg>   Disconnect device from taskmonitor\n";
    std::cout << "       Require:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "     --startCollecting, -s     <noarg>   Start collecting data from device\n";
    std::cout << "       Require:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "     --stopCollecting, -x      <noarg>   Stop collecting data from device\n";
    std::cout << "       Require:\n";
    std::cout << "         --Id, -I              <string>  Device ID\n";
    std::cout << "  Help:\n";
    std::cout << "     --help, -h                          Print this help\n\n";

    exit(EXIT_SUCCESS);
  }

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);

  std::filesystem::path configPath(tkm::tkmDefaults.getFor(tkm::Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!std::filesystem::exists(config_path)) {
      std::cout << "Provided configuration file cannot be accessed: " << config_path << std::endl;
      return EXIT_FAILURE;
    }
    configPath = std::filesystem::path(config_path);
  }

  try {
    tkm::control::Application app{"TKM-Control", "TKM Control", configPath.string()};

    if (init_database) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::InitDatabase,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }
    if (list_devices) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::GetDevices,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      app.getCommand()->addRequest(rq);
    }
    if (add_device) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::AddDevice,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceName, device_name);
      rq.args.emplace(tkm::Defaults::Arg::DeviceAddress, device_address);
      rq.args.emplace(tkm::Defaults::Arg::DevicePort, device_port);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (list_sessions) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::GetSessions,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      if (unique_id != nullptr) {
        rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      }
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (remove_device) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::RemoveDevice,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (remove_session) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::RemoveSession,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::SessionHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (connect_device) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::ConnectDevice,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (disconnect_device) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::DisconnectDevice,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (start_collecting) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::StartCollecting,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (stop_collecting) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::StopCollecting,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      rq.args.emplace(tkm::Defaults::Arg::DeviceHash, unique_id);
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    if (quit) {
      tkm::control::Command::Request rq{.action = tkm::control::Command::Action::QuitCollector,
                                        .args = std::map<tkm::Defaults::Arg, std::string>()};
      if (force) {
        rq.args.emplace(tkm::Defaults::Arg::Forced,
                        tkm::tkmDefaults.valFor(tkm::Defaults::Val::True));
      }
      app.getCommand()->addRequest(rq);
    }

    // Request initial connection
    tkm::control::Dispatcher::Request connectRequest{
        .action = tkm::control::Dispatcher::Action::Connect,
        .bulkData = std::make_any<int>(0),
        .args = std::map<tkm::Defaults::Arg, std::string>()};
    app.getDispatcher()->pushRequest(connectRequest);

    app.run();
  } catch (std::exception &e) {
    std::cout << "Application start failed. " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
