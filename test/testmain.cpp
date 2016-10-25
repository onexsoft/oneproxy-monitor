/*
 * testmain.cpp
 *
 *  Created on: 2016年8月1日
 *      Author: hui
 */

#include "cpptest.h"
#include "testlogger.h"
#include "testnetworksocket.h"
#include "teststringbuf.h"
#include "testtcpclient.h"
#include "testtcpserver.h"
#include "testtool.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

enum OutputType
{
	Compiler,
	Html,
	TextTerse,
	TextVerbose
};

static void
usage()
{
	cout << "usage: mytest [MODE]\n"
		 << "where MODE may be one of:\n"
		 << "  --compiler\n"
		 << "  --html\n"
		 << "  --text-terse (default)\n"
		 << "  --text-verbose\n";
	exit(0);
}

static auto_ptr<Test::Output>
cmdline(int argc, char* argv[])
{
	if (argc > 2)
		usage(); // will not return

	Test::Output* output = 0;

	if (argc == 1)
		output = new Test::TextOutput(Test::TextOutput::Verbose);
	else
	{
		const char* arg = argv[1];
		if (strcmp(arg, "--compiler") == 0)
			output = new Test::CompilerOutput;
		else if (strcmp(arg, "--html") == 0)
			output =  new Test::HtmlOutput;
		else if (strcmp(arg, "--text-terse") == 0)
			output = new Test::TextOutput(Test::TextOutput::Terse);
		else if (strcmp(arg, "--text-verbose") == 0)
			output = new Test::TextOutput(Test::TextOutput::Verbose);
		else
		{
			cout << "invalid commandline argument: " << arg << endl;
			usage(); // will not return
		}
	}

	return auto_ptr<Test::Output>(output);
}

int main(int argc, char* argv[])
{
	try {
		Test::Suite ts;
		//ts.add(auto_ptr<Test::Suite>(new TestLogger));
		//ts.add(auto_ptr<Test::Suite>(new TestNetworkSocket));
		//ts.add(auto_ptr<Test::Suite>(new TestStringBuf));
		ts.add(auto_ptr<Test::Suite>(new TestTcpClient));
//		ts.add(auto_ptr<Test::Suite>(new TestTcpServer));
//		ts.add(auto_ptr<Test::Suite>(new TestTool));

		auto_ptr<Test::Output> output(cmdline(argc, argv));
		ts.run(*output, true);

		Test::HtmlOutput* const html = dynamic_cast<Test::HtmlOutput*>(output.get());
		if (html)
			html->generate(cout, true, "MyTest");
	}
	catch (...)
	{
		cout << "unexpected exception encountered\n";
		return EXIT_FAILURE;
	}

	system("pause");

	return EXIT_SUCCESS;
}


