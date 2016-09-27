/*
 * testnetworksocket.cpp
 *
 *  Created on: 2016年8月3日
 *      Author: hui
 */

#include "testnetworksocket.h"
#include "networksocket.h"

void TestNetworkSocket::ntopAndpton_test()
{
	NetworkSocket ns;
	ns.set_portAndAddr(5432 , "127.0.0.1");
	ns.addr_pton();
	ns.addr_ntop();

	TEST_ASSERT(!ns.get_address().compare("127.0.0.1"));
}

void TestNetworkSocket::isValidAddress_test()
{
	NetworkSocket ns;
	ns.set_portAndAddr(10, "127.0.0.1");//is not valid port
	TEST_ASSERT(ns.get_port() > 0); //test not assig port
	TEST_ASSERT(ns.is_validAddress() == 1);

	ns.set_portAndAddr(20000, std::string());//is not valid address
	TEST_ASSERT(ns.is_validAddress() == 0);

	ns.set_portAndAddr(20000, std::string("127.0.0.1"));//is not valid address
	TEST_ASSERT(ns.is_validAddress() == 1);

}

