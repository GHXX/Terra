#include "stdafx.h"

#include "tthread.h"
#include "tnetwork.h"

#include "ttest.h"

#include "test.h"

typedef int(*TNetworkExecFunc)(TSocket *);

int TNetworkSetupServer(TNetworkExecFunc data) {
	TSocket *server;
	TSocket *server_connect;
	int res;
	TUInt8 received;

	server = TSocketNew();
	TTestMustValidate(server);

	TTestMustValidate(!TSocketBind(server, "23", T_SOCKET_PROTOCOL_TCP));

	server_connect = TSocketListen(server);
	if (!server_connect) {
		TSocketFree(server);
		TTestMustValidate(1);
	}

	res = 0;
	if(data) res = data(server_connect);

	TStream *stream = TSocketGetStream(server_connect);
	received = TStreamRead8(stream);

	TStreamFree(stream);
	TSocketFree(server_connect);
	TSocketFree(server);

	return res;
}

int TNetworkTestConnect(void) {
	TThread *server;

	TSocket *client;
	TStream *clientStream;

	server = TThreadCreate(TNetworkSetupServer, 0);

	TThreadSleep(50);

	client = TSocketNew();
	TTestMustValidate(client);
	if (!client) {
		TThreadJoin(server);
		TTestMustValidate(1);
	}
	clientStream = TSocketConnect(client, "localhost", "23", T_SOCKET_PROTOCOL_TCP);

	TStreamWrite8(clientStream, 128);

	TStreamFree(clientStream);
	TSocketFree(client);
	return TThreadJoin(server);
}


void TNetworkTest(void) {
	TestFunc tests[] = {
		TNetworkTestConnect

	};

	TNetworkInitialize();

	TTestRun("Network", tests, sizeof(tests) / sizeof(TestFunc));

	TNetworkDestroy();
}
