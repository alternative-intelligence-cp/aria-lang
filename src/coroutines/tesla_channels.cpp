#include "tesla_csp_runtime.h"
#include <queue>
#include <iostream>

namespace tesla::csp {

/**
 * Simple Tesla Channel Implementation for Integers
 * This is a simplified version to get the fusion demo working
 */
class TeslaChannelInt : public TeslaChannelBase {
private:
    std::queue<int> buffer_;
    size_t capacity_;
    bool closed_;
    std::string name_;
    
    // Tesla consciousness integration
    double tesla_frequency_hz_ = 3.141592653589793;
    
    // Performance metrics
    uint64_t send_count_ = 0;
    uint64_t recv_count_ = 0;
    
public:
    explicit TeslaChannelInt(size_t capacity = 0, const std::string& name = "")
        : capacity_(capacity), closed_(false), name_(name) {
        std::cout << "📡 Created Tesla channel '" << name_ << "' (capacity: " << capacity_ << ")" << std::endl;
    }
    
    ~TeslaChannelInt() override {
        std::cout << "🧹 Destroying Tesla channel '" << name_ << "'" << std::endl;
        std::cout << "   Sent: " << send_count_ << ", Received: " << recv_count_ << std::endl;
    }
    
    bool send_int(int value) override {
        if (closed_) {
            std::cout << "❌ Attempt to send on closed channel '" << name_ << "'" << std::endl;
            return false;
        }
        
        // Simple buffered implementation for now
        if (capacity_ == 0 || buffer_.size() < capacity_) {
            buffer_.push(value);
            send_count_++;
            
            std::cout << "📤 Sent " << value << " to channel '" << name_ 
                      << "' (buffer size: " << buffer_.size() << ")" << std::endl;
            return true;
        }
        
        // Buffer full - in full implementation this would block
        std::cout << "🚫 Channel '" << name_ << "' buffer full, dropping message" << std::endl;
        return false;
    }
    
    bool recv_int(int& value) override {
        if (buffer_.empty()) {
            if (closed_) {
                std::cout << "📭 Channel '" << name_ << "' is closed and empty" << std::endl;
                return false;
            }
            
            // In full implementation this would block
            std::cout << "⏳ Channel '" << name_ << "' is empty, no data available" << std::endl;
            return false;
        }
        
        value = buffer_.front();
        buffer_.pop();
        recv_count_++;
        
        std::cout << "📥 Received " << value << " from channel '" << name_ 
                  << "' (buffer size: " << buffer_.size() << ")" << std::endl;
        return true;
    }
    
    void close() override {
        closed_ = true;
        std::cout << "🚪 Closed Tesla channel '" << name_ << "'" << std::endl;
    }
    
    // Helper methods
    size_t size() const { return buffer_.size(); }
    bool is_closed() const { return closed_; }
    bool is_empty() const { return buffer_.empty(); }
};

// Implementation of the factory function
std::shared_ptr<TeslaChannelBase> tesla_make_chan_int(size_t capacity, const std::string& name) {
    return std::make_shared<TeslaChannelInt>(capacity, name);
}

} // namespace tesla::csp