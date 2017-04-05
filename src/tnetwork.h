
#ifndef _included_terra_network_h
#define _included_terra_network_h

#include "io/tstream.h"

/**
* Terra Network
*
*   The purpose of this file is to provide network
*   support.
*
*/

//--- TNetwork              ------------------------------//

void TNetworkInitialize(void);
void TNetworkDestroy(void);

//--- TSocket               ------------------------------//

enum T_SOCKET_PROTOCOL {
	T_SOCKET_PROTOCOL_TCP,
	T_SOCKET_PROTOCOL_UDP
};

typedef struct TSocket TSocket;

TSocket *TSocketNew(void);
TSocket *TSocketListen(TSocket *context);
void TSocketFree(TSocket *context);

TStream *TSocketConnect(TSocket *context, const char *address, const char *port, int protocol);
int TSocketBind(TSocket *context, const char *port, int protocol);

TStream *TSocketGetStream(TSocket *context);

#endif
