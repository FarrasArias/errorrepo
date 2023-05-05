#include <iostream>
#include "../build/test.pb.h"

namespace mmmProto {
	void testMmmProto()
	{
		std::cout << "mmmProto test function called" << std::endl;
		protoTest::Test testCall;
		testCall.set_var_1(5);
		testCall.set_var_2(6);
	}
}

/*int main() {
	mmmProto::testMmmProto();
	std::cout << "BARUGAAA" << std::endl;
}*/