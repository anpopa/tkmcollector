/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Command Class
 * @details   Process commands from cli arguments
 *-
 */

#include "Command.h"
#include "Application.h"

namespace tkm::control
{

Command::Command()
: UserEvent("Command")
{
  setCallback([this]() {
    for (const Command::Request &request : m_requests) {
      switch (request.action) {
      case Command::Action::InitDatabase: {
        Dispatcher::Request rq{.action = Dispatcher::Action::InitDatabase,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};
        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::QuitCollector: {
        Dispatcher::Request rq{.action = Dispatcher::Action::QuitCollector,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};
        if (request.args.count(tkm::Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::GetDevices: {
        Dispatcher::Request rq{.action = Dispatcher::Action::GetDevices,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};
        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::AddDevice: {
        Dispatcher::Request rq{.action = Dispatcher::Action::AddDevice,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceName, request.args.at(Defaults::Arg::DeviceName));
        rq.args.emplace(Defaults::Arg::DeviceAddress,
                        request.args.at(Defaults::Arg::DeviceAddress));
        rq.args.emplace(Defaults::Arg::DevicePort, request.args.at(Defaults::Arg::DevicePort));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::RemoveDevice: {
        Dispatcher::Request rq{.action = Dispatcher::Action::RemoveDevice,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::ConnectDevice: {
        Dispatcher::Request rq{.action = Dispatcher::Action::ConnectDevice,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::DisconnectDevice: {
        Dispatcher::Request rq{.action = Dispatcher::Action::DisconnectDevice,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::StartCollecting: {
        Dispatcher::Request rq{.action = Dispatcher::Action::StartCollecting,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::StopCollecting: {
        Dispatcher::Request rq{.action = Dispatcher::Action::StopCollecting,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::GetSessions: {
        Dispatcher::Request rq{.action = Dispatcher::Action::GetSessions,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }

        if (request.args.count(Defaults::Arg::DeviceHash)) {
          rq.args.emplace(Defaults::Arg::DeviceHash, request.args.at(Defaults::Arg::DeviceHash));
        }

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::RemoveSession: {
        Dispatcher::Request rq{.action = Dispatcher::Action::RemoveSession,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};

        if (request.args.count(Defaults::Arg::Forced)) {
          rq.args.emplace(tkm::Defaults::Arg::Forced, request.args.at(tkm::Defaults::Arg::Forced));
        }
        rq.args.emplace(Defaults::Arg::SessionHash, request.args.at(Defaults::Arg::SessionHash));

        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      case Command::Action::Quit: {
        Dispatcher::Request rq{.action = Dispatcher::Action::Quit,
                               .bulkData = std::make_any<int>(0),
                               .args = std::map<Defaults::Arg, std::string>()};
        ControlApp()->getDispatcher()->pushRequest(rq);
        break;
      }
      default:
        logError() << "Unknown command request";
        break;
      }
    }

    return false; // Once we are done we remove ourself from event loop
  });
}

void Command::enableEvents()
{
  ControlApp()->addEventSource(getShared());
}

} // namespace tkm::control
