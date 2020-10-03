#include "catch.hpp"

#include <limits>

uint64_t u64(double v) { return *reinterpret_cast<uint64_t const*>(&v); }

#include <lang/vm/lang_value.h>

TEST_CASE("lang_value", "[lang_value][vm]") {
	CHECK_FALSE(lang_is_object(u64( 1.0 )));
	CHECK_FALSE(lang_is_object(u64( 1.0/0.0 ))); // INF
	CHECK_FALSE(lang_is_object(u64( 0.0/0.0 ))); // NaN
	CHECK_FALSE(lang_is_object(u64( std::numeric_limits<double>::quiet_NaN()     )));
	CHECK_FALSE(lang_is_object(u64( std::numeric_limits<double>::signaling_NaN() )));
	CHECK_FALSE(lang_is_object(u64( std::numeric_limits<double>::infinity()      )));

	CHECK(lang_is_object( lang_object2val(0) ));
	CHECK(lang_is_object( lang_object2val(lang_value_max_object) ));
}
