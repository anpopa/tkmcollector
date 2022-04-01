#pragma once

#include <any>
#include <string>

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

#include "Client.pb.h"
#include "Collector.pb.h"

namespace tkm
{

auto pskHsh(const std::string &password) -> std::string;
auto jnkHsh(const char *key) -> uint64_t;
auto base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len) -> std::string;
auto base64Decode(std::string const &encoded_string) -> std::string;
bool sendControlDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);
bool readControlDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);
bool sendClientDescriptor(int fd, tkm::msg::client::Descriptor &descriptor);
bool readClientDescriptor(int fd, tkm::msg::client::Descriptor &descriptor);

} // namespace tkm
