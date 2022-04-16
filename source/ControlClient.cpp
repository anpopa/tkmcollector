#include <unistd.h>

#include "Application.h"
#include "ControlClient.h"
#include "Defaults.h"
#include "Helpers.h"

#include "Collector.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{

static bool doInitDatabase(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq);
static bool doRequestSession(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq);
static bool doQuitCollector(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq);
static bool doGetDevices(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq);
static bool doGetSessions(const shared_ptr<ControlClient> &client,
                          tkm::msg::collector::Request &rq);
static bool doRemoveSession(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq);
static bool doAddDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq);
static bool doRemoveDevice(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq);
static bool doConnectDevice(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq);
static bool doDisconnectDevice(const shared_ptr<ControlClient> &client,
                               tkm::msg::collector::Request &rq);
static bool doStartCollecting(const shared_ptr<ControlClient> &client,
                              tkm::msg::collector::Request &rq);
static bool doStopCollecting(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq);

ControlClient::ControlClient(int clientFd)
: IClient("ControlClient", clientFd)
{
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
          case tkm::msg::collector::Request_Type_QuitCollector:
            status = doQuitCollector(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_GetDevices:
            status = doGetDevices(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_GetSessions:
            status = doGetSessions(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_RemoveSession:
            status = doRemoveSession(getShared(), rq);
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
          case tkm::msg::collector::Request_Type_StartCollecting:
            status = doStartCollecting(getShared(), rq);
            break;
          case tkm::msg::collector::Request_Type_StopCollecting:
            status = doStopCollecting(getShared(), rq);
            break;
          default:
            status = false;
            break;
          }
        } while (status);

        return status;
      },
      getFD(),
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  setFinalize([this]() { logDebug() << "Ended connection with client: " << getName(); });
}

void ControlClient::enableEvents()
{
  CollectorApp()->addEventSource(getShared());
}

ControlClient::~ControlClient()
{
  if (m_clientFd > 0) {
    ::close(m_clientFd);
    m_clientFd = -1;
  }
}

static bool doRequestSession(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq)
{
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

  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::SendStatus};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());
  if (!status) {
    nrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    nrq.args.emplace(Defaults::Arg::Reason, "Failed to set session");
  } else {
    nrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    nrq.args.emplace(Defaults::Arg::Reason, "Control client session set");
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doInitDatabase(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::InitDatabase};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doQuitCollector(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::QuitCollector};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doGetDevices(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::GetDevices};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());
  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doRemoveSession(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::RemoveSession};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::SessionData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::SessionData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doAddDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::AddDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doRemoveDevice(const shared_ptr<ControlClient> &client,
                           tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::RemoveDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doConnectDevice(const shared_ptr<ControlClient> &client,
                            tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::ConnectDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doDisconnectDevice(const shared_ptr<ControlClient> &client,
                               tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::DisconnectDevice};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doStartCollecting(const shared_ptr<ControlClient> &client,
                              tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::StartCollecting};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doStopCollecting(const shared_ptr<ControlClient> &client,
                             tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::StopCollecting};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static bool doGetSessions(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
{
  Dispatcher::Request nrq{.client = client, .action = Dispatcher::Action::GetSessions};
  nrq.args.emplace(Defaults::Arg::RequestId, rq.id());

  if (rq.forced() == tkm::msg::collector::Request_Forced_Enforced) {
    nrq.args.emplace(Defaults::Arg::Forced, tkmDefaults.valFor(Defaults::Val::True));
  }

  tkm::msg::collector::DeviceData data;
  rq.data().UnpackTo(&data);
  nrq.bulkData = std::make_any<tkm::msg::collector::DeviceData>(data);

  return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

} // namespace tkm::collector
