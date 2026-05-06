#include <slim/common/memory/mapper.h>
#include <slim/common/log.h>
#include <cassert>

using namespace slim::common;

void test_create_and_exists() {
    memory_mapper::create("test_map");
    assert(memory_mapper::map_exists("test_map"));
    log::info("test_create_and_exists passed");
}

void test_write_and_read_bool() {
    memory_mapper::write("test_map", "my_bool", true);
    assert(memory_mapper::variable_exists("test_map", "my_bool"));
    assert(memory_mapper::read_bool("test_map", "my_bool") == true);
    log::info("test_write_and_read_bool passed");
}

void test_write_and_read_string() {
    memory_mapper::write("test_map", "my_string", std::string("hello"));
    assert(memory_mapper::read_string("test_map", "my_string") == "hello");
    log::info("test_write_and_read_string passed");
}

void test_write_and_read_shared_ptr_string() {
    memory_mapper::write("test_map", "my_ptr", std::make_shared<std::string>("world"));
    auto result = memory_mapper::read("test_map", "my_ptr");
    assert(result != nullptr);
    assert(*result == "world");
    log::info("test_write_and_read_shared_ptr_string passed");
}

void test_list_keys() {
    auto keys = memory_mapper::list_keys("test_map");
    assert(keys.size() >= 3);
    log::info("test_list_keys passed");
}

void test_variable_not_exists() {
    assert(!memory_mapper::variable_exists("test_map", "nonexistent"));
    log::info("test_variable_not_exists passed");
}

void test_map_not_exists() {
    assert(!memory_mapper::map_exists("nonexistent_map"));
    log::info("test_map_not_exists passed");
}

int main() {
    test_create_and_exists();
    test_write_and_read_bool();
    test_write_and_read_string();
    test_write_and_read_shared_ptr_string();
    test_list_keys();
    test_variable_not_exists();
    test_map_not_exists();

    log::info("all tests passed");
    return 0;
}