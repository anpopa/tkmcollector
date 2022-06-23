#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "IClient.h"
#include "Options.h"

#include "Control.pb.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class ControlClient : public IClient, public std::enable_shared_from_this<ControlClient>
{
public:
  explicit ControlClient(int clientFd);

public:
  auto getShared() -> std::shared_ptr<ControlClient> { return shared_from_this(); }
  void enableEvents();

public:
  ControlClient(ControlClient const &) = delete;
  void operator=(ControlClient const &) = delete;
};

} // namespace tkm::collector
