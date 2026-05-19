#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <slim/common/memory/mapper.h>

using namespace slim::common;

// ─── Helpers ────────────────────────────────────────────────────────────────

static void ensure_map(const std::string& name) {
    memory_mapper::create(name);
}

// ─── map lifecycle ───────────────────────────────────────────────────────────

TEST_CASE("map lifecycle", "[memory_mapper]") {

    SECTION("create makes map exist") {
        memory_mapper::create("lc_map");
        REQUIRE(memory_mapper::map_exists("lc_map"));
    }

    SECTION("create is idempotent") {
        memory_mapper::create("lc_idem");
        memory_mapper::create("lc_idem");          // second call must not throw
        REQUIRE(memory_mapper::map_exists("lc_idem"));
    }

    SECTION("unknown map does not exist") {
        REQUIRE_FALSE(memory_mapper::map_exists("lc_nonexistent_xyzzy"));
    }

    SECTION("create with empty name throws") {
        REQUIRE_THROWS(memory_mapper::create(""));
    }

    SECTION("map_exists with empty name throws") {
        REQUIRE_THROWS(memory_mapper::map_exists(""));
    }
}

// ─── attach ──────────────────────────────────────────────────────────────────

TEST_CASE("attach", "[memory_mapper]") {

    SECTION("attaching a valid pointer makes map exist") {
        auto container = std::make_shared<memory_mapper::map_container>();
        REQUIRE(memory_mapper::attach("att_map", container));
        REQUIRE(memory_mapper::map_exists("att_map"));
    }

    SECTION("attaching nullptr throws") {
        REQUIRE_THROWS(memory_mapper::attach("att_null", nullptr));
    }

    SECTION("attaching with empty name throws") {
        auto container = std::make_shared<memory_mapper::map_container>();
        REQUIRE_THROWS(memory_mapper::attach("", container));
    }
}

// ─── write / variable_exists ─────────────────────────────────────────────────

TEST_CASE("write and variable_exists", "[memory_mapper]") {
    ensure_map("we_map");

    SECTION("written bool variable exists") {
        memory_mapper::write("we_map", "flag", true);
        REQUIRE(memory_mapper::variable_exists("we_map", "flag"));
    }

    SECTION("written string variable exists") {
        memory_mapper::write("we_map", "greeting", std::string("hello"));
        REQUIRE(memory_mapper::variable_exists("we_map", "greeting"));
    }

    SECTION("written shared_ptr<string> variable exists") {
        memory_mapper::write("we_map", "ptr_val", std::make_shared<std::string>("world"));
        REQUIRE(memory_mapper::variable_exists("we_map", "ptr_val"));
    }

    SECTION("unknown variable does not exist") {
        REQUIRE_FALSE(memory_mapper::variable_exists("we_map", "no_such_var_xyzzy"));
    }

    SECTION("variable_exists on unknown map returns false") {
        REQUIRE_FALSE(memory_mapper::variable_exists("we_no_map_xyzzy", "anything"));
    }

    SECTION("write with empty map name throws") {
        REQUIRE_THROWS(memory_mapper::write("", "v", std::string("x")));
    }

    SECTION("write with empty variable name throws") {
        REQUIRE_THROWS(memory_mapper::write("we_map", "", std::string("x")));
    }

    SECTION("variable_exists with empty map name throws") {
        REQUIRE_THROWS(memory_mapper::variable_exists("", "v"));
    }

    SECTION("variable_exists with empty variable name throws") {
        REQUIRE_THROWS(memory_mapper::variable_exists("we_map", ""));
    }

    SECTION("write auto-creates map if absent") {
        REQUIRE_FALSE(memory_mapper::map_exists("we_autocreate"));
        memory_mapper::write("we_autocreate", "x", std::string("y"));
        REQUIRE(memory_mapper::map_exists("we_autocreate"));
    }
}

// ─── read_bool ───────────────────────────────────────────────────────────────

TEST_CASE("read_bool", "[memory_mapper]") {
    ensure_map("rb_map");

    SECTION("reads back true") {
        memory_mapper::write("rb_map", "t", true);
        REQUIRE(memory_mapper::read_bool("rb_map", "t") == true);
    }

    SECTION("reads back false") {
        memory_mapper::write("rb_map", "f", false);
        REQUIRE(memory_mapper::read_bool("rb_map", "f") == false);
    }

    SECTION("overwrite updates value") {
        memory_mapper::write("rb_map", "toggle", true);
        memory_mapper::write("rb_map", "toggle", false);
        REQUIRE(memory_mapper::read_bool("rb_map", "toggle") == false);
    }

    SECTION("missing variable returns default false") {
        REQUIRE(memory_mapper::read_bool("rb_map", "rb_missing_xyzzy") == false);
    }

    SECTION("missing map returns default false") {
        REQUIRE(memory_mapper::read_bool("rb_no_map_xyzzy", "any") == false);
    }
}

// ─── read_string ─────────────────────────────────────────────────────────────

TEST_CASE("read_string", "[memory_mapper]") {
    ensure_map("rs_map");

    SECTION("reads back a plain string") {
        memory_mapper::write("rs_map", "key", std::string("value"));
        REQUIRE(memory_mapper::read_string("rs_map", "key") == "value");
    }

    SECTION("reads back a shared_ptr<string> as string") {
        memory_mapper::write("rs_map", "ptr_key", std::make_shared<std::string>("from_ptr"));
        REQUIRE(memory_mapper::read_string("rs_map", "ptr_key") == "from_ptr");
    }

    SECTION("overwrite updates value") {
        memory_mapper::write("rs_map", "mutable", std::string("first"));
        memory_mapper::write("rs_map", "mutable", std::string("second"));
        REQUIRE(memory_mapper::read_string("rs_map", "mutable") == "second");
    }

    SECTION("missing variable returns empty string") {
        REQUIRE(memory_mapper::read_string("rs_map", "rs_missing_xyzzy").empty());
    }

    SECTION("missing map returns empty string") {
        REQUIRE(memory_mapper::read_string("rs_no_map_xyzzy", "any").empty());
    }

    SECTION("empty string value round-trips") {
        memory_mapper::write("rs_map", "empty_val", std::string(""));
        REQUIRE(memory_mapper::read_string("rs_map", "empty_val").empty());
    }
}

// ─── read (shared_ptr<string>) ───────────────────────────────────────────────

TEST_CASE("read returns shared_ptr<string>", "[memory_mapper]") {
    ensure_map("rp_map");

    SECTION("plain string yields non-null pointer with correct content") {
        memory_mapper::write("rp_map", "plain", std::string("hello"));
        auto ptr = memory_mapper::read("rp_map", "plain");
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == "hello");
    }

    SECTION("shared_ptr<string> value yields correct content") {
        memory_mapper::write("rp_map", "shared", std::make_shared<std::string>("world"));
        auto ptr = memory_mapper::read("rp_map", "shared");
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == "world");
    }

    SECTION("missing variable yields nullptr") {
        auto ptr = memory_mapper::read("rp_map", "rp_missing_xyzzy");
        REQUIRE(ptr == nullptr);
    }

    SECTION("missing map yields nullptr") {
        auto ptr = memory_mapper::read("rp_no_map_xyzzy", "any");
        REQUIRE(ptr == nullptr);
    }
}

// ─── read_string_view ────────────────────────────────────────────────────────

TEST_CASE("read_string_view", "[memory_mapper]") {
    ensure_map("rv_map");

    SECTION("plain string yields correct view") {
        memory_mapper::write("rv_map", "sv_key", std::string("viewable"));
        auto sv = memory_mapper::read_string_view("rv_map", "sv_key");
        REQUIRE(sv == "viewable");
    }

    SECTION("shared_ptr<string> value yields correct view") {
        memory_mapper::write("rv_map", "sv_ptr", std::make_shared<std::string>("ptr_view"));
        auto sv = memory_mapper::read_string_view("rv_map", "sv_ptr");
        REQUIRE(sv == "ptr_view");
    }

    SECTION("missing variable yields empty view") {
        auto sv = memory_mapper::read_string_view("rv_map", "rv_missing_xyzzy");
        REQUIRE(sv.empty());
    }
}

// ─── list_keys ───────────────────────────────────────────────────────────────

TEST_CASE("list_keys", "[memory_mapper]") {
    ensure_map("lk_map");

    SECTION("empty map yields empty key list") {
        memory_mapper::create("lk_empty");
        auto keys = memory_mapper::list_keys("lk_empty");
        REQUIRE(keys.empty());
    }

    SECTION("returns all written keys") {
        memory_mapper::write("lk_map", "a", std::string("1"));
        memory_mapper::write("lk_map", "b", std::string("2"));
        memory_mapper::write("lk_map", "c", true);
        auto keys = memory_mapper::list_keys("lk_map");
        REQUIRE(keys.size() >= 3);
        auto has = [&](const std::string& k) {
            return std::find(keys.begin(), keys.end(), k) != keys.end();
        };
        REQUIRE(has("a"));
        REQUIRE(has("b"));
        REQUIRE(has("c"));
    }

    SECTION("overwriting a key does not duplicate it") {
        memory_mapper::create("lk_dedup");
        memory_mapper::write("lk_dedup", "x", std::string("v1"));
        memory_mapper::write("lk_dedup", "x", std::string("v2"));
        auto keys = memory_mapper::list_keys("lk_dedup");
        auto count = std::count(keys.begin(), keys.end(), "x");
        REQUIRE(count == 1);
    }

    SECTION("unknown map yields empty key list") {
        auto keys = memory_mapper::list_keys("lk_no_map_xyzzy");
        REQUIRE(keys.empty());
    }

    SECTION("empty map name throws") {
        REQUIRE_THROWS(memory_mapper::list_keys(""));
    }
}

// ─── erase ───────────────────────────────────────────────────────────────────

TEST_CASE("erase", "[memory_mapper]") {

    SECTION("erase with empty name throws") {
        REQUIRE_THROWS(memory_mapper::erase(""));
    }

    // NOTE: the erase body is currently commented out in the implementation.
    // The tests below document the *intended* post-erase behaviour and are
    // written to be enabled once the implementation is uncommented.

    // SECTION("erased map no longer exists") {
    //     memory_mapper::create("er_map");
    //     memory_mapper::erase("er_map");
    //     REQUIRE_FALSE(memory_mapper::map_exists("er_map"));
    // }

    // SECTION("erase on unknown map is a no-op") {
    //     REQUIRE_NOTHROW(memory_mapper::erase("er_nonexistent_xyzzy"));
    // }
}
