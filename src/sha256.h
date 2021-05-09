#pragma once

#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <openssl/sha.h>

std::optional<std::string> sha256(const std::string& str) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;

  if (!SHA256_Init(&sha256)) {
    printf("init failed\n");
    return std::nullopt;
  }
  if (!SHA256_Update(&sha256, str.c_str(), str.size())) {
    printf("update failed\n");
    return std::nullopt;
  }

  if (!SHA256_Final(hash, &sha256)) {
    printf("final failed\n");
    return std::nullopt;
  };

  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss.str();
}
