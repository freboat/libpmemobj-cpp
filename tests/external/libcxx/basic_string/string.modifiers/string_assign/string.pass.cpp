//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Copyright 2019, Intel Corporation
//
// Modified to test pmem::obj containers
//

#include "unittest.hpp"

#include <array>
#include <libpmemobj++/experimental/string.hpp>

namespace nvobj = pmem::obj;
namespace pmem_exp = pmem::obj::experimental;
using S = pmem_exp::string;

struct root {
	nvobj::persistent_ptr<S> s;
	nvobj::persistent_ptr<S> s_arr[5];
};

template <class T>
void
test(nvobj::pool<struct root> &pop, const S &s1, const T &str,
     const S &expected)
{
	auto r = pop.root();

	nvobj::transaction::run(pop,
				[&] { r->s = nvobj::make_persistent<S>(s1); });

	auto &s = *r->s;

	s.assign(str);
	UT_ASSERT(s == expected);

	nvobj::transaction::run(pop,
				[&] { nvobj::delete_persistent<S>(r->s); });
}

void
test_self_assignment(nvobj::pool<struct root> &pop, const S &s1,
		     const S &expected)
{
	auto r = pop.root();

	nvobj::transaction::run(pop,
				[&] { r->s = nvobj::make_persistent<S>(s1); });

	auto &s = *r->s;

	s.assign(s);
	UT_ASSERT(s == expected);

	nvobj::transaction::run(pop,
				[&] { nvobj::delete_persistent<S>(r->s); });
}

int
main(int argc, char *argv[])
{
	START();

	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " file-name" << std::endl;
		return 1;
	}

	auto path = argv[1];
	auto pop = nvobj::pool<root>::create(
		path, "string_test", PMEMOBJ_MIN_POOL, S_IWUSR | S_IRUSR);

	auto &s_arr = pop.root()->s_arr;
	{
		try {
			nvobj::transaction::run(pop, [&] {
				s_arr[0] = nvobj::make_persistent<S>();
				s_arr[1] = nvobj::make_persistent<S>("12345");
				s_arr[2] =
					nvobj::make_persistent<S>("1234567890");
				s_arr[3] = nvobj::make_persistent<S>(
					"12345678901234567890");
				s_arr[4] = nvobj::make_persistent<S>(
					"12345678901234567890123456789012345678901234567890123456789012345678901234567890");
			});

			/* assign from pmem::string */
			test(pop, *s_arr[0], *s_arr[0], *s_arr[0]);
			test(pop, *s_arr[0], *s_arr[1], *s_arr[1]);
			test(pop, *s_arr[0], *s_arr[2], *s_arr[2]);
			test(pop, *s_arr[0], *s_arr[3], *s_arr[3]);
			test(pop, *s_arr[0], *s_arr[4], *s_arr[4]);

			test(pop, *s_arr[1], *s_arr[0], *s_arr[0]);
			test(pop, *s_arr[1], *s_arr[1], *s_arr[1]);
			test(pop, *s_arr[1], *s_arr[2], *s_arr[2]);
			test(pop, *s_arr[1], *s_arr[3], *s_arr[3]);
			test(pop, *s_arr[1], *s_arr[4], *s_arr[4]);

			test(pop, *s_arr[2], *s_arr[0], *s_arr[0]);
			test(pop, *s_arr[2], *s_arr[1], *s_arr[1]);
			test(pop, *s_arr[2], *s_arr[2], *s_arr[2]);
			test(pop, *s_arr[2], *s_arr[3], *s_arr[3]);
			test(pop, *s_arr[2], *s_arr[4], *s_arr[4]);

			test(pop, *s_arr[3], *s_arr[0], *s_arr[0]);
			test(pop, *s_arr[3], *s_arr[1], *s_arr[1]);
			test(pop, *s_arr[3], *s_arr[2], *s_arr[2]);
			test(pop, *s_arr[3], *s_arr[3], *s_arr[3]);
			test(pop, *s_arr[3], *s_arr[4], *s_arr[4]);

			test_self_assignment(pop, *s_arr[3], *s_arr[3]);

			/* assign from std::string */
			std::array<std::string, 5> std_str_arr = {
				"", "12345", "1234567890",
				"12345678901234567890",
				"12345678901234567890123456789012345678901234567890123456789012345678901234567890"};

			test(pop, *s_arr[0], std_str_arr[0], *s_arr[0]);
			test(pop, *s_arr[0], std_str_arr[1], *s_arr[1]);
			test(pop, *s_arr[0], std_str_arr[2], *s_arr[2]);
			test(pop, *s_arr[0], std_str_arr[3], *s_arr[3]);
			test(pop, *s_arr[0], std_str_arr[4], *s_arr[4]);

			test(pop, *s_arr[1], std_str_arr[0], *s_arr[0]);
			test(pop, *s_arr[1], std_str_arr[1], *s_arr[1]);
			test(pop, *s_arr[1], std_str_arr[2], *s_arr[2]);
			test(pop, *s_arr[1], std_str_arr[3], *s_arr[3]);
			test(pop, *s_arr[1], std_str_arr[4], *s_arr[4]);

			test(pop, *s_arr[2], std_str_arr[0], *s_arr[0]);
			test(pop, *s_arr[2], std_str_arr[1], *s_arr[1]);
			test(pop, *s_arr[2], std_str_arr[2], *s_arr[2]);
			test(pop, *s_arr[2], std_str_arr[3], *s_arr[3]);
			test(pop, *s_arr[2], std_str_arr[4], *s_arr[4]);

			test(pop, *s_arr[3], std_str_arr[0], *s_arr[0]);
			test(pop, *s_arr[3], std_str_arr[1], *s_arr[1]);
			test(pop, *s_arr[3], std_str_arr[2], *s_arr[2]);
			test(pop, *s_arr[3], std_str_arr[3], *s_arr[3]);
			test(pop, *s_arr[3], std_str_arr[4], *s_arr[4]);

			nvobj::transaction::run(pop, [&] {
				for (unsigned i = 0; i < 5; ++i) {
					nvobj::delete_persistent<S>(s_arr[i]);
				}
			});
		} catch (std::exception &e) {
			UT_FATALexc(e);
		}
	}

	pop.close();

	return 0;
}
