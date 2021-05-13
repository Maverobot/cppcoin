#include <chrono>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "sha256.h"

std::ostream& operator<<(std::ostream& os, const struct Transaction& transaction);

struct Transaction {
  std::string from_address;
  std::string to_address;
  int32_t amount;
};

using Clock = std::chrono::system_clock;
using TimeStamp = std::chrono::time_point<Clock>;
using Hash = std::string;
using Transactions = std::vector<Transaction>;

std::string str(const Transactions& transactions) {
  std::ostringstream oss;
  for (const auto& trans : transactions) {
    oss << trans << "\n";
  }
  return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Transaction& transaction) {
  return os << "from_adress: " << transaction.from_address
            << ", to_address: " << transaction.to_address << ", amount: " << transaction.amount;
}

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

std::ostream& operator<<(std::ostream& o, const std::vector<Transaction>& vec) {
  o << "\n\t";
  copy(vec.cbegin(), vec.cend(), std::ostream_iterator<Transaction>(o, "\n\t"));
  return o;
}

class Block {
 public:
  Block(TimeStamp timestamp, Transactions transactions) {
    timestamp_ = timestamp;
    transactions_ = std::move(transactions);
    refreshHash();
  }

  void setPreviousHash(std::string previous_hash) {
    previous_hash_ = std::move(previous_hash);
    refreshHash();
  }

  [[nodiscard]] const Hash& previousHash() const { return previous_hash_; }

  [[nodiscard]] const Hash& hash() const { return hash_; }

  [[nodiscard]] const Transactions& transactions() const { return transactions_; }

  void mineHash(uint32_t difficulty) {
    while (hash_.rfind(std::string(difficulty, '0'), 0) != 0) {
      nonce_++;
      refreshHash();
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const Block& block) {
    os << "timestamp: " << block.timestamp_ << "\n";
    os << "transactions: " << block.transactions_ << "\n";
    os << "previous hash: " << block.previous_hash_ << "\n";
    os << "hash: " << block.hash() << "\n";
    return os;
  }

 private:
  TimeStamp timestamp_;
  Transactions transactions_;
  Hash hash_;
  Hash previous_hash_;
  size_t nonce_{0};

  void refreshHash() {
    std::optional<std::string> hash;
    do {
      hash = sha256(std::to_string(timestamp_.time_since_epoch().count()) + str(transactions_) +
                    previous_hash_ + std::to_string(nonce_));
    } while (!hash);
    hash_ = hash.value();
  }
};

class Blockchain {
 public:
  Blockchain() { chain_.push_back(createGenesisBlock()); }

  void minePendingTransactions(std::string mining_reward_address) {
    Block b(Clock::now(), pending_transactions_);
    b.setPreviousHash(chain_.back().hash());
    b.mineHash(mining_difficulty);
    chain_.push_back(b);

    this->pending_transactions_ = {{"", std::move(mining_reward_address), mining_reward}};
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

  void addTransaction(const Transaction& transaction) {
    pending_transactions_.push_back(transaction);
  }

  [[nodiscard]] double getBalance(const std::string& address) const {
    double balance = 0;
    for (const auto& block : chain_) {
      for (const auto& trans : block.transactions()) {
        if (trans.from_address == address) {
          balance -= trans.amount;
        }

        if (trans.to_address == address) {
          balance += trans.amount;
        }
      }
    }
    return balance;
  }

  friend std::ostream& operator<<(std::ostream& os, const Blockchain& blockchain) {
    copy(blockchain.chain_.cbegin(), blockchain.chain_.cend() - 1,
         std::ostream_iterator<Block>(os, "\n-------------------------------------------------\n"));
    os << blockchain.chain_.back();
    return os;
  }

 private:
  std::vector<Block> chain_;
  Transactions pending_transactions_;

  static Block createGenesisBlock() {
    Block block(Clock::now(), {});
    block.setPreviousHash("0");
    return block;
  }

  const uint32_t mining_difficulty = 2;
  const int32_t mining_reward = 100;
};

int main(int /*argc*/, char* /*argv*/[]) {
  Blockchain cppcoin;

  cppcoin.addTransaction({"address1", "address2", 100});
  cppcoin.addTransaction({"address2", "address1", 50});

  std::cout << "Starting the miner...\n";
  cppcoin.minePendingTransactions("reward_adress");

  std::cout << "cppcoin: \n\n" << cppcoin << "\n";
  std::cout << "cppcoin is valid: " << std::boolalpha << cppcoin.isValid() << "\n\n";

  std::cout << "balance [address1]: " << cppcoin.getBalance("address1") << "\n";
  std::cout << "balance [address2]: " << cppcoin.getBalance("address2") << "\n";
  std::cout << "balance [reward_adress]: " << cppcoin.getBalance("reward_adress") << "\n";

  cppcoin.minePendingTransactions("reward_adress");
  std::cout << "balance [reward_address]: " << cppcoin.getBalance("reward_adress") << "\n";
  cppcoin.minePendingTransactions("reward_adress");
  std::cout << "balance [reward_address]: " << cppcoin.getBalance("reward_adress") << "\n";

  std::cout << "cppcoin: \n\n" << cppcoin << "\n";

  return 0;
}
