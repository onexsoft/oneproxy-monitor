/*
 * testnetworksocket.h
 *
 *  Created on: 2016年8月3日
 *      Author: hui
 */

#ifndef TEST_TESTNETWORKSOCKET_H_
#define TEST_TESTNETWORKSOCKET_H_

#include "unittest/cpptest.h"

class TestNetworkSocket: public Test::Suite{

public:
	TestNetworkSocket() {
		TEST_ADD(TestNetworkSocket::ntopAndpton_test);
		TEST_ADD(TestNetworkSocket::isValidAddress_test);
	}
private:
	void ntopAndpton_test();
	void isValidAddress_test();
};

#endif /* TEST_TESTNETWORKSOCKET_H_ */
