/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#include <errno.h>
#include <stdio.h>

#if (!defined(_WIN32)) && (!defined(__wasi__))
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#if (!defined(_WIN32)) && (!defined(__wasi__))
static lVal lnfSocketConect(lVal host, lVal port){
	reqString(host);
	reqInt(port);
	if((port.vInt < 0) || (port.vInt > 0xFFFF)){
		return lValException(lSymError, "Port numbers need to be between 0-65535", port);
	}

	struct addrinfo hints, *res, *result;
	bzero(&hints, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags |= AI_CANONNAME;

	int errcode = getaddrinfo(lBufferData(host.vBuffer), NULL, &hints, &result);
	if(errcode != 0){
		return NIL;
	}
	res = result;
	while (res){
		if(res->ai_family == AF_INET){
			((struct sockaddr_in*)((void *)res->ai_addr))->sin_port = htons(port.vInt);
		} else if(res->ai_family == AF_INET6){
			((struct sockaddr_in6*)((void *)res->ai_addr))->sin6_port = htons(port.vInt);
		} else {
			continue;
		}
		int fd = socket(res->ai_family, res->ai_socktype, 0);
		if(fd < 0){
			continue;
		}

		int rc = connect(fd, res->ai_addr, res->ai_addrlen);
		if(rc == 0){
			freeaddrinfo(result);
			FILE *fh = fdopen(fd, "rb+");
			if(fh == NULL){
				return NIL;
			}
			return lValFileHandle(fh);
		}
		res = res->ai_next;
	}

	freeaddrinfo(result);
	return NIL;
}
#endif


void lOperationsNet(){
	#if (!defined(_WIN32)) && (!defined(__wasi__))
	lAddNativeFuncVV("socket/connect", "(host port)", "Quits with code a", lnfSocketConect, 0);
	#endif
}
