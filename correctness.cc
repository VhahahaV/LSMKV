#include <iostream>
#include <cstdint>
#include <string>

#include "test.h"

class CorrectnessTest : public Test {
private:
	const uint64_t SIMPLE_TEST_MAX = 512;
	const uint64_t LARGE_TEST_MAX = 1024 * 1;

	void regular_test(uint64_t max)
	{
		uint64_t i;

		// Test a single key
		EXPECT(not_found, store.get(1));
		store.put(1, "SE");
		EXPECT("SE", store.get(1));
		EXPECT(true, store.del(1));
		EXPECT(not_found, store.get(1));
		EXPECT(false, store.del(1));

		phase();

		// Test multiple key-value pairs
		for (i = 0; i < max; ++i) {
			store.put(i, std::string(i+1, 's'));
            std::string ans = store.get(i);
			EXPECT(std::string(i+1, 's'), ans);
		}
		phase();
//        store.test();
		// Test after all insertions
		for (i = 0; i < max; ++i){

            EXPECT(std::string(i+1, 's'), store.get(i));

        }
		phase();

		// Test scan
//		std::list<std::pair<uint64_t, std::string> > list_ans;
//		std::list<std::pair<uint64_t, std::string> > list_stu;
//
//		for (i = 0; i < max / 2; ++i) {
//			list_ans.emplace_back(std::make_pair(i, std::string(i+1, 's')));
//		}
//
//		store.scan(0, max / 2 - 1, list_stu);
//		EXPECT(list_ans.mSize(), list_stu.mSize());
//
//		auto ap = list_ans.begin();
//		auto sp = list_stu.begin();
//		while(ap != list_ans.end()) {
//			if (sp == list_stu.end()) {
//				EXPECT((*ap).first, -1);
//				EXPECT((*ap).second, not_found);
//				ap++;
//			}
//			else {
//				EXPECT((*ap).first, (*sp).first);
//				EXPECT((*ap).second, (*sp).second);
//				ap++;
//				sp++;
//			}
//		}
//
//		phase();

		// Test deletions
		for (i = 0; i < max; i+=2){
            EXPECT(true, store.del(i));
            EXPECT((i & 1) ? std::string(i+1, 's') : not_found,
                   store.get(i));
        }

		for (i = 0; i < max; ++i)
			EXPECT((i & 1) ? std::string(i+1, 's') : not_found,
			       store.get(i));

		for (i = 1; i < max; ++i)
			EXPECT(i & 1, store.del(i));

		phase();

		report();
	}

public:
	CorrectnessTest(const std::string &dir, bool v=true) : Test(dir, v)
	{
	}

	void start_test(void *args = NULL) override
	{
		std::cout << "KVStore Correctness Test" << std::endl;

		store.reset();
		
//		std::cout << "[Simple Test]" << std::endl;
//		regular_test(SIMPLE_TEST_MAX);
		
//		store.reset();

		std::cout << "[Large Test]" << std::endl;
		regular_test(LARGE_TEST_MAX);
	}
};

int main(int argc, char *argv[])
{
	bool verbose = (argc == 2 && std::string(argv[1]) == "-v");

	std::cout << "Usage: " << argv[0] << " [-v]" << std::endl;
	std::cout << "  -v: print extra info for failed tests [currently ";
	std::cout << (verbose ? "ON" : "OFF")<< "]" << std::endl;
	std::cout << std::endl;
	std::cout.flush();

	CorrectnessTest test("./data", true);

	test.start_test();

	return 0;
}
