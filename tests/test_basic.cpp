#include <iostream>
#include <cassert>
#include "../src/data/hashtable.hpp"
#include "../src/protocol/protocol.hpp"
#include "../src/data/ttl_manager.hpp"

using namespace scuffedredis;

void test_hashtable() {
    std::cout << "Testing HashTable..." << std::endl;
    
    HashTable table;
    
    // Test basic operations
    assert(table.set("key1", "value1"));
    assert(table.get("key1").value() == "value1");
    assert(table.exists("key1"));
    assert(table.size() == 1);
    
    // Test update
    assert(!table.set("key1", "value2"));  // Should return false for update
    assert(table.get("key1").value() == "value2");
    
    // Test deletion
    assert(table.del("key1"));
    assert(!table.exists("key1"));
    assert(table.size() == 0);
    
    // Test multiple keys
    table.set("a", "1");
    table.set("b", "2");
    table.set("c", "3");
    assert(table.size() == 3);
    
    auto keys = table.keys("*");
    assert(keys.size() == 3);
    
    std::cout << "HashTable tests passed!" << std::endl;
}

void test_protocol() {
    std::cout << "Testing Protocol..." << std::endl;
    
    // Test message creation
    auto msg = protocol::Message::make_simple_string("OK");
    assert(msg->is_string());
    assert(msg->as_string() == "OK");
    
    // Test serialization
    auto data = msg->serialize();
    assert(data.size() > 0);
    
    // Test parsing
    protocol::Parser parser;
    parser.feed(data);
    assert(parser.has_message());
    
    auto parsed = parser.parse_message();
    assert(parsed);
    assert(parsed->as_string() == "OK");
    
    // Test command creation
    auto cmd = protocol::utils::make_command({"SET", "key", "value"});
    assert(cmd->is_array());
    
    auto args = protocol::utils::parse_command(cmd);
    assert(args.size() == 3);
    assert(args[0] == "SET");
    assert(args[1] == "key");
    assert(args[2] == "value");
    
    std::cout << "Protocol tests passed!" << std::endl;
}

void test_ttl_manager() {
    std::cout << "Testing TTL Manager..." << std::endl;
    
    TTLManager ttl;
    
    // Test setting TTL
    assert(ttl.set_ttl("key1", 5));
    assert(ttl.has_ttl("key1"));
    assert(ttl.get_ttl("key1") > 0);
    
    // Test removing TTL
    assert(ttl.remove_ttl("key1"));
    assert(!ttl.has_ttl("key1"));
    assert(ttl.get_ttl("key1") == -1);
    
    // Test expiration (with very short TTL)
    assert(ttl.set_ttl("key2", 1));
    
    // Wait a bit and check expiration
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    size_t expired = ttl.check_expirations();
    assert(expired == 1);
    assert(!ttl.has_ttl("key2"));
    
    std::cout << "TTL Manager tests passed!" << std::endl;
}

int main() {
    std::cout << "Running ScuffedRedis tests..." << std::endl;
    std::cout << "==============================" << std::endl;
    
    try {
        test_hashtable();
        test_protocol();
        test_ttl_manager();
        
        std::cout << "==============================" << std::endl;
        std::cout << "All tests passed! ✅" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
