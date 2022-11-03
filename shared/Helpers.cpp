/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helper methods
 * @details   Verious helper methods
 *-
 */

#include "Helpers.h"

#include <cstring>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

constexpr size_t GDescBufferSize = 8192;
namespace pbio = google::protobuf::io;

namespace tkm
{

auto hashForDevice(const tkm::msg::control::DeviceData &data) -> std::string
{
  std::string tmp = data.address();
  tmp += std::to_string(data.port());
  return std::to_string(jnkHsh(tmp.c_str()));
}

bool sendControlDescriptor(int fd, tkm::msg::control::Descriptor &descriptor)
{
  tkm::msg::control::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::control::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  unsigned char buffer[GDescBufferSize]{};
  pbio::ArrayOutputStream outputArray(buffer, sizeof(buffer));
  pbio::CodedOutputStream codedOutput(&outputArray);

  size_t envelopeSize = envelope.ByteSizeLong();
  if ((envelopeSize > UINT32_MAX) || (envelopeSize > sizeof(buffer))) {
    return false;
  }
  codedOutput.WriteVarint32(static_cast<uint32_t>(envelopeSize));

  if (!envelope.SerializeToCodedStream(&codedOutput)) {
    return false;
  }

  if (::send(fd, buffer, envelopeSize + sizeof(uint64_t), MSG_WAITALL) !=
      (static_cast<ssize_t>(envelopeSize + sizeof(uint64_t)))) {
    return false;
  }

  return true;
}

bool readControlDescriptor(int fd, tkm::msg::control::Descriptor &descriptor)
{
  tkm::msg::control::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::control::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Control);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  unsigned char buffer[GDescBufferSize]{};
  pbio::ArrayInputStream inputArray(buffer, sizeof(buffer));
  pbio::CodedInputStream codedInput(&inputArray);

  if (recv(fd, buffer, sizeof(uint64_t), MSG_WAITALL) != static_cast<ssize_t>(sizeof(uint64_t))) {
    return false;
  }

  uint32_t messageSize;
  codedInput.ReadVarint32(&messageSize);
  if (messageSize > (sizeof(buffer) - sizeof(uint64_t))) {
    return false;
  }

  if (recv(fd, buffer + sizeof(uint64_t), messageSize, MSG_WAITALL) != messageSize) {
    return false;
  }

  codedInput.PushLimit(static_cast<int>(messageSize));
  if (!envelope.ParseFromCodedStream(&codedInput)) {
    return false;
  }

  envelope.mesg().UnpackTo(&message);
  if (message.type() != tkm::msg::control::Message_Type_Descriptor) {
    return false;
  }

  message.data().UnpackTo(&descriptor);
  return true;
}

} // namespace tkm
