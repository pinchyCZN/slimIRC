/*
 * Copyright (C) 2004-2012 George Yunaev gyunaev@ulduzsoft.com
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

/*
 * The sockets interface was moved out to simplify going OpenSSL integration.
 */
#if !defined (_WIN32)
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <fcntl.h>

	#define IS_SOCKET_ERROR(a)	((a)<0)
	typedef int				socket_t;

#else
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>

	#define IS_SOCKET_ERROR(a)	((a)==SOCKET_ERROR)

	#define EWOULDBLOCK		WSAEWOULDBLOCK
	#define EINPROGRESS		WSAEINPROGRESS
	#define EINTR			WSAEINTR

	typedef SOCKET			socket_t;
	typedef int				socklen_t;
	typedef unsigned char	sa_family_t;
	#define _SS_MAXSIZE 304
	#define _SS_ALIGNSIZE (sizeof (char*))
	#define _SS_PAD1SIZE (_SS_ALIGNSIZE - (sizeof(unsigned char) + sizeof(sa_family_t)))
	#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof(unsigned char) + sizeof(sa_family_t)+_SS_PAD1SIZE + _SS_ALIGNSIZE))
#ifndef	_GNU_H_WINDOWS32_SOCKETS
	typedef struct sockaddr_storage {
	  short   ss_family;
	  char    __ss_pad1[_SS_PAD1SIZE];
	  __int64 __ss_align;
	  char    __ss_pad2[_SS_PAD2SIZE];
	} SOCKADDR_STORAGE, *PSOCKADDR_STORAGE;

	typedef struct addrinfo {
	  int             ai_flags;
	  int             ai_family;
	  int             ai_socktype;
	  int             ai_protocol;
	  size_t          ai_addrlen;
	  char            *ai_canonname;
	  struct sockaddr  *ai_addr;
	  struct addrinfo  *ai_next;
	} ADDRINFOA, *PADDRINFOA;
#endif
	typedef int (__stdcall * getaddrinfo_ptr_t) (const char *nodename, const char *servname,const struct addrinfo * hints, struct addrinfo ** res);
	typedef void (__stdcall * freeaddrinfo_ptr_t) (struct addrinfo * ai);

#endif

#ifndef INADDR_NONE
	#define INADDR_NONE 	0xFFFFFFFF
#endif


static int socket_error()
{
#if !defined (_WIN32)
	return errno;
#else
	return WSAGetLastError();
#endif
}


static int socket_create (int domain, int type, socket_t * sock)
{
	*sock = socket (domain, type, 0);
	return IS_SOCKET_ERROR(*sock) ? 1 : 0;
}


static int socket_make_nonblocking (socket_t * sock)
{
#if !defined (_WIN32)
	return fcntl (*sock, F_SETFL, fcntl (*sock, F_GETFL,0 ) | O_NONBLOCK) != 0;
#else
	unsigned long mode = 0;
	return ioctlsocket (*sock, FIONBIO, &mode) == SOCKET_ERROR;
#endif
}


static int socket_close (socket_t * sock)
{
#if !defined (_WIN32)
	close (*sock);
#else
	closesocket (*sock);
#endif

	*sock = -1;
	return 0;
}


static int socket_connect (socket_t * sock, const struct sockaddr *saddr, socklen_t len)
{
	while ( 1 )
	{
	    if ( connect (*sock, saddr, len) < 0 )
	    {
	    	if ( socket_error() == EINTR )
	    		continue;

			if ( socket_error() != EINPROGRESS && socket_error() != EWOULDBLOCK )
				return 1;
		}

		return 0;
	}
}


static int socket_accept (socket_t * sock, socket_t * newsock, struct sockaddr *saddr, socklen_t * len)
{
	while ( IS_SOCKET_ERROR(*newsock = accept (*sock, saddr, len)) )
	{
    	if ( socket_error() == EINTR )
    		continue;

		return 1;
	}

	return 0;
}


static int socket_recv (socket_t * sock, void * buf, size_t len)
{
	int length;

	while ( (length = recv (*sock, buf, len, 0)) < 0 )
	{
		int err = socket_error();

		if ( err != EINTR && err != EAGAIN )
			break;
	}

	return length;
}


static int socket_send (socket_t * sock, const void *buf, size_t len)
{
	int length;

	while ( (length = send (*sock, buf, len, 0)) < 0 )
	{
		int err = socket_error();

		if ( err != EINTR && err != EAGAIN )
			break;
	}

	return length;
}
