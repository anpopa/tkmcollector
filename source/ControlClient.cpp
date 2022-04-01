#include <unistd.h>

#include "Application.h"
#include "ControlClient.h"
#include "Defaults.h"
#include "Helpers.h"

#include "Collector.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector {

static auto doInitDatabase(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq) -> bool;
static auto doRequestSession(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq) -> bool;
static auto doTerminateCollector(const shared_ptr<ControlClient> &client,
                                 tkm::msg::collector::Request &rq) -> bool;
static auto doGetDevices(const shared_ptr<ControlClient> &client,
                         tkm::msg::collector::Request &rq) -> bool;
static auto doAddDevice(const shared_ptr<ControlClient> &client,
                        tkm::msg::collector::Request &rq) -> bool;
static auto doRemoveDevice(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq) -> bool;
static auto doConnectDevice(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq) -> bool;
static auto doDisconnectDevice(const shared_ptr<ControlClient> &client,
                               tkm::msg::collector::Request &rq) -> bool;
static auto doStartDeviceSession(const shared_ptr<ControlClient> &client,
                                 tkm::msg::collector::Request &rq) -> bool;
static auto doStopDeviceSession(const shared_ptr<ControlClient> &client,
                                tkm::msg::collector::Request &rq) -> bool;

ControlClient::ControlClient(int clientFd)
    : IClient("ControlClient", clientFd) {
  bswi::event::Pollable::lateSetup(
      [this]() {
        auto status = true;

        do {
          tkm::msg::Envelope envelope;

          // Read next message
          auto readStatus = readEnvelope(envelope);
          if (readStatus == IAsyncEnvelope::Status::Again) {
            return true;
          } else if (readStatus == IAsyncEnvelope::Status::Error) {
            logDebug() << "ControlClient read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "ControlClient read end of file";
            return false;
          }

          // Check for valid origin
          if (envelope.origin() != tkm::msg::Envelope_Recipient_Control) {
            continue;
          }

          tkm::msg::collector::Request rq;
          envelope.mesg().UnpackTo(&rq);

          switch (rq.type()) {
          case tkm::msg::collector::Request_Type_RequestSession:
            status = doRequestSession(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_InitDatabase:
            status = doInitDatabase(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_TerminateCollector:
            status = doTerminateCollector(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_GetDevices:
            status = doGetDevices(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_AddDevice:
            status = doAddDevice(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_RemoveDevice:
            status = doRemoveDevice(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_ConnectDevice:
            status = doConnectDevice(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_DisconnectDevice:
            status = doDisconnectDevice(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_StartDeviceSession:
            status = doStartDeviceSession(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_StopDeviceSession:
            status = doStopDeviceSession(getShared(), rq);
            break;
          default:
            status = false;
            break;
          }
        } while (status);

        return status;
      },
      getFD(), bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  setFinalize([this]() {
    logDebug() << "Ended connection with client: " << getName();
  });
}

void ControlClient::enableEvents() {
  CollectorApp()->addEventSource(getShared());
}

ControlClient::~ControlClient() {
  if (m_clientFd > 0) {
    ::close(m_clientFd);
    m_clientFd = -1;
  }
}

static auto doRequestSession(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq) -> bool {
  tkm::msg::collector::SessionInfo sessionInfo;
  rq.data().UnpackTo(&sessionInfo);

  tkm::msg::Envelope requestEnvelope;
  tkm::msg::collector::Message requestMessage;

  requestMessage.set_type(tkm::msg::collector::Message_Type_SetSession);
  requestMessage.mutable_data()->PackFrom(sessionInfo);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Control);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  auto status = client->writeEnvelope(requestEnvelope);

  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::SendStatus};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());
  if (!status) {
    nrq.args.emplace(Defaults::Arg::Status,
                     tkmDefaults.valFor(Defaults::Val::StatusError));
    nrq.args.emplace(Defaults::Arg::Reason, "Failed to set session");
  } else {
    nrq.args.emplace(Defaults::Arg::Status,
                     tkmDefaults.valFor(Defaults::Val::StatusOkay));
    nrq.args.emplace(Defaults::Arg::Reason, "Session set");
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doInitDatabase(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::InitDatabase};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doTerminateCollector(const shared_ptr<ControlClient> &client,
                                 tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::TerminateCollector};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doGetDevices(const shared_ptr<ControlClient> &client,
                         tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::GetDevices};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());
  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doAddDevice(const shared_ptr<ControlClient> &client,
                        tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::AddDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doRemoveDevice(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::RemoveDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doConnectDevice(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::ConnectDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doDisconnectDevice(const shared_ptr<ControlClient> &client,
                               tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::DisconnectDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doStartDeviceSession(const shared_ptr<ControlClient> &client,
                                 tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::StartDeviceSession};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doStopDeviceSession(const shared_ptr<ControlClient> &client,
                                tkm::msg::collector::Request &rq) -> bool {
  Dispatcher::Request nrq{.client = client,
                          .action = Dispatcher::Action::StopDeviceSession};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced,
                     tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

} // namespace tkm::collector
