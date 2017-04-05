
#include "stdafx.h"

#include "tnetwork.h"

#ifndef _WINDOWS
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

#include "talloc.h"
#include "terror.h"


//--- TNetwork              ------------------------------//

void TNetworkInitialize(void) {
#ifdef _WINDOWS
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void TNetworkDestroy(void) {
#ifdef _WINDOWS
	WSACleanup();
#endif
}

//--- TSocket               ------------------------------//

struct TSocket {
#ifdef _WINDOWS
	SOCKET sockfd;
#else
	int sockfd;
#endif
};

TSocket *TSocketNew(void) {
	TSocket *socket = TAllocData(TSocket);

	if (socket) memset(socket, 0, sizeof(TSocket));

	return socket;
}

TSocket *TSocketListen(TSocket *context) {
	if (context && context->sockfd) {
		TSocket *client;
		int clientfd;
#ifdef _WINDOWS
		if (!(clientfd = accept(context->sockfd, 0, 0))) return 0;
#else

#endif
		client = TSocketNew();
		client->sockfd = clientfd;
		return client;
	}

	return 0;
}

void TSocketFree(TSocket *context) {
	if (context) {
#ifdef _WINDOWS
		if (context->sockfd) closesocket(context->sockfd);
#else
		close(context->sockfd);
#endif
		TFree(context);
	}
}

TStream *TSocketConnect(TSocket *context, const char *address, const char *port, int protocol) {
	if (context) {
#ifdef _WINDOWS
		struct addrinfo *result, *ptr, hint;
		int connected;

		memset(&hint, 0, sizeof(hint));
		hint.ai_family = AF_UNSPEC;
		if (protocol == T_SOCKET_PROTOCOL_TCP) {
			hint.ai_socktype = SOCK_STREAM;
			hint.ai_protocol = IPPROTO_TCP;
		} else {
			hint.ai_socktype = SOCK_DGRAM;
			hint.ai_protocol = IPPROTO_UDP;
		}

		result = ptr = 0;

		if (getaddrinfo(address, port, &hint, &result)) return 0;

		ptr = result;
		do {
			connected = 1;

			context->sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (context->sockfd) {
				if (connect(context->sockfd, ptr->ai_addr, (int)ptr->ai_addrlen)) {
					TErrorSet(WSAGetLastError());
					closesocket(context->sockfd);
					ptr++;
					connected = 0;
				}
			} else {
				ptr++;
				connected = 0;
			}
		} while (ptr && !connected);

		freeaddrinfo(result);

		if (!connected) return 0;
#else

#endif
		return TSocketGetStream(context);
	}

	return 0;
}

int TSocketBind(TSocket *context, const char *port, int protocol) {
	if (context) {
#ifdef _WINDOWS
		struct addrinfo *result, *ptr, hint;
		int connected;

		memset(&hint, 0, sizeof(hint));
		hint.ai_family = AF_UNSPEC;
		if (protocol == T_SOCKET_PROTOCOL_TCP) {
			hint.ai_socktype = SOCK_STREAM;
			hint.ai_protocol = IPPROTO_TCP;
		} else {
			hint.ai_socktype = SOCK_DGRAM;
			hint.ai_protocol = IPPROTO_UDP;
		}

		result = ptr = 0;

		if (getaddrinfo(0, port, &hint, &result)) return 1;

		ptr = result;
		do {
			connected = 1;

			context->sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (context->sockfd) {
				if (bind(context->sockfd, ptr->ai_addr, (int)ptr->ai_addrlen)) {
					closesocket(context->sockfd);
					ptr++;
					connected = 0;
				}
			} else {
				TErrorSet(WSAGetLastError());
				ptr++;
				connected = 0;
			}
		} while (ptr && !connected);

		freeaddrinfo(result);

		if (!connected) return 1;

		if (listen(context->sockfd, SOMAXCONN)) { closesocket(context->sockfd); return 1; }
#else

#endif
	}

	return 0;
}

//--- Default Socket Operations ------------------------------//

static TSize TStreamSocketRead(TStreamContent *content, TPtr buffer, TSize size) {
	TSocket *socket;
	socket = (TSocket *)content;

#ifdef _WINDOWS
	return recv(socket->sockfd, buffer, size, 0);
#else
#endif
}

static TSize TStreamSocketWrite(TStreamContent *content, TCPtr buffer, TSize size) {
	TSocket *socket;
	socket = (TSocket *)content;

#ifdef _WINDOWS
	return send(socket->sockfd, buffer, size, 0);
#else
#endif
}

static int TStreamSocketClose(TStreamContent *content) {
	TSocket *socket;
	socket = (TSocket *)content;

#ifdef _WINDOWS
	closesocket(socket->sockfd);
	socket->sockfd = 0;
#else
#endif

	return 0;
}

static TStreamOps TStreamSocketOps = {
	0,
	0,
	0,
	0,
	TStreamSocketRead,
	TStreamSocketWrite,
	TStreamSocketClose,
};

TStream *TSocketGetStream(TSocket *context) {
	return TStreamFromContent(context, &TStreamSocketOps, 0);
}
