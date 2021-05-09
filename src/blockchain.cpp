#include <chrono>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "sha256.h"

using Clock = std::chrono::system_clock;
using TimeStamp = std::chrono::time_point<Clock>;
using Data = std::string;
using Hash = std::string;

std::ostream& operator<<(std::ostream& os, const TimeStamp& timestamp) {
  std::time_t t = Clock::to_time_t(timestamp);
  std::string ts = std::ctime(&t);
  ts.resize(ts.size() - 1);
  os << ts << " "
     << (timestamp - std::chrono::duration_cast<std::chrono::seconds>(timestamp.time_since_epoch()))
            .time_since_epoch()
            .count();
  return os;
}

class Block {
 public:
  Block(TimeStamp timestamp, Data data) {
    timestamp_ = timestamp;
    data_ = std::move(data);
    refreshHash();
  }

  void setPreviousHash(std::string previous_hash) {
    previous_hash_ = std::move(previous_hash);
    refreshHash();
  }

  [[nodiscard]] const Hash& previousHash() const { return previous_hash_; }

  [[nodiscard]] const Hash& hash() const { return hash_; }

  friend std::ostream& operator<<(std::ostream& os, const Block& block) {
    os << "timestamp: " << block.timestamp_ << "\n";
    os << "data: " << block.data_ << "\n";
    os << "previous hash: " << block.previous_hash_ << "\n";
    os << "hash: " << block.hash() << "\n";
    return os;
  }

 private:
  TimeStamp timestamp_;
  Data data_;
  Hash hash_;
  Hash previous_hash_;

  void refreshHash() {
    std::optional<std::string> hash;
    do {
      hash = sha256(std::to_string(timestamp_.time_since_epoch().count()) + data_ + previous_hash_);
    } while (!hash);
    hash_ = hash.value();
  }
};

class Blockchain {
 public:
  Blockchain() { chain_.push_back(createGenesisBlock()); }

  void addBlock(Block new_block) {
    new_block.setPreviousHash(chain_.back().hash());
    chain_.push_back(std::move(new_block));
  }

  [[nodiscard]] bool isValid() const {
    for (size_t i = 1; i < chain_.size(); i++) {
      const auto& current_block = chain_.at(i);
      const auto& previous_block = chain_.at(i - 1);
      if (current_block.previousHash() != previous_block.hash()) {
        return false;
      }
    }
    return true;
  }

  friend std::ostream& operator<<(std::ostream& os, const Blockchain& blockchain) {
    copy(blockchain.chain_.cbegin(), blockchain.chain_.cend() - 1,
         std::ostream_iterator<Block>(os, "\n"));
    os << blockchain.chain_.back();
    return os;
  }

 private:
  std::vector<Block> chain_;

  static Block createGenesisBlock() {
    Block block(Clock::now(), "Genesis block");
    block.setPreviousHash("0");
    return block;
  }
};

int main(int /*argc*/, char* /*argv*/[]) {
  Blockchain cppcoin;
  cppcoin.addBlock(Block{Clock::now(), "transaction: 100"});
  cppcoin.addBlock(Block{Clock::now(), "transaction: 10"});

  std::cout << "cppcoin: \n\n" << cppcoin << "\n";
  std::cout << "cppcoin is valid: " << std::boolalpha << cppcoin.isValid() << "\n";

  return 0;
}
