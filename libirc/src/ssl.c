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


#if defined (ENABLE_SSL)

#include "polarssl/config.h"

#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/error.h"


#if defined (_WIN32)
#include <windows.h>

/*
static int mycount=0;
//#undef malloc
#undef free
int my_free(void *x)
{
	mycount--;
	printf("my count=%i\n",mycount);
	free(x);
}
//#define free my_free

#undef malloc
int my_malloc(int size)
{
	mycount++;
	return malloc(size);
}
//#define malloc my_malloc

#undef _strdup
int mystrdup(char *s)
{
	mycount++;
	return _strdup(s);
}
#define _strdup mystrdup
*/
#else

// This array will store all of the mutexes available to OpenSSL
static pthread_mutex_t * mutex_buf = 0;



static int alloc_mutexes( unsigned int total )
{
	int i;
	
	// Enable thread safety in OpenSSL
	mutex_buf = (pthread_mutex_t*) malloc( total * sizeof(pthread_mutex_t) );

	if ( !mutex_buf )
		return -1;

	for ( i = 0;  i < total;  i++)
		pthread_mutex_init( &(mutex_buf[i]), 0 );
	
	return 0;
}

#endif

int DEBUG_LEVEL=1;
int set_debug_level(int i)
{ DEBUG_LEVEL=i;return 0; }
void my_debug(void *ctx,int level,const char *str)
{
	if(level<=DEBUG_LEVEL){
		fprintf((FILE*)ctx,"%s",str);
		fflush((FILE*)ctx);
	}
}

// Initializes the SSL context. Must be called after the socket is created.
static int libirc_ssl_init(ssl_context *ssl)
{

    memset(ssl,0,sizeof(ssl_context));
	return 0;
}

static int libirc_ssl_disconnect(ssl_context *ssl,int *socket)
{
	ssl_close_notify(ssl);
	net_close(ssl->p_recv);
	ssl_free(ssl);
	socket[0]=-1;
	return 0;
}

static int libirc_ssl_connect(ssl_context *ssl,const int options,const char *host,const int port,int *c_socket)
{
	int ret;
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_session ssn;
	int socket=0;
    char *pers="irc connection";

	memset(&ssn,0,sizeof(ssl_session));
    entropy_init(&entropy);
    if((ret=ctr_drbg_init(&ctr_drbg,entropy_func,&entropy,
                               (unsigned char *)pers,strlen(pers)))!=0){
		return LIBIRC_ERR_SSL_INIT_FAILED;
    }
	if((ret=net_connect(&socket,host,port))!=0)
		return LIBIRC_ERR_CONNECT_SSL_FAILED;

	if((ret=ssl_init(ssl))!=0){
		net_close(socket);
		return LIBIRC_ERR_CONNECT_SSL_FAILED;
	}
	ssl_set_endpoint(ssl,SSL_IS_CLIENT);
	ssl_set_authmode(ssl,options&LIBIRC_OPTION_SSL_NO_VERIFY ? SSL_VERIFY_NONE:SSL_VERIFY_REQUIRED);
	ssl_set_authmode(ssl,SSL_VERIFY_NONE); //for now
	
	ssl_set_rng(ssl,ctr_drbg_random,&ctr_drbg);
	ssl_set_dbg(ssl,my_debug,stdout);
	ssl_set_bio(ssl,net_recv,&socket,net_send,&socket);
	c_socket[0]=socket;
	//socket_make_nonblocking(&socket);
	
	ssl_set_ciphersuites(ssl,ssl_default_ciphersuites);
	ssl_set_session(ssl,1,600,&ssn);
	ssl->read_timeout=30;
	get_ini_value("SSL_OPTIONS","read_timeout",&ssl->read_timeout);
	return 0;

}
static void ssl_handle_error( irc_session_t * session, int ssl_error )
{
/*
	if ( ERR_GET_LIB(ssl_error) == ERR_LIB_SSL )
	{
		if ( ERR_GET_REASON(ssl_error) == SSL_R_CERTIFICATE_VERIFY_FAILED )
		{
			session->lasterror = LIBIRC_ERR_SSL_CERT_VERIFY_FAILED;
			return;
		}
		
		if ( ERR_GET_REASON(ssl_error) == SSL_R_UNKNOWN_PROTOCOL )
		{
			session->lasterror = LIBIRC_ERR_CONNECT_SSL_FAILED;
			return;
		}
	}
*/
}

static int ssl_recv( irc_session_t * session )
{
	int count;
	unsigned int amount = (sizeof (session->incoming_buf) - 1) - session->incoming_offset;
	
	count=ssl_read(&session->ssl,session->incoming_buf+session->incoming_offset,amount);

    if ( count > 0 )
		return count;
	else if ( count == 0 )
		return -1; // remote connection closed
	else
	{
		int ssl_error = 0; //SSL_get_error( session->ssl, count );
		
		// Handle SSL error since not all of them are actually errors
        switch ( ssl_error )
        {
/*
		case SSL_ERROR_WANT_READ:
                // This is not really an error. We received something, but
                // OpenSSL gave nothing to us because all it read was
                // internal data. Repeat the same read.
				return 0;

            case SSL_ERROR_WANT_WRITE:
                // This is not really an error. We received something, but
                // now OpenSSL needs to send the data before returning any
                // data to us (like negotiations). This means we'd need
                // to wait for WRITE event, but call SSL_read() again.
                session->flags |= SESSIONFL_SSL_READ_WANTS_WRITE;
				return 0;
*/
		}

		// This is an SSL error, handle it
		//ssl_handle_error( session, ERR_get_error() ); 
	}
	
	return -1;
}


static int ssl_send( irc_session_t * session )
{
	int count;

	count=ssl_write(&session->ssl,session->outgoing_buf,session->outgoing_offset);

    if ( count > 0 )
		return count;
    else if ( count == 0 )
		return -1;
    else
    {
		int ssl_error = 0; //SSL_get_error( session->ssl, count );
		
        switch ( ssl_error )
        {
/*
            case SSL_ERROR_WANT_READ:
                // This is not really an error. We sent some internal OpenSSL data,
                // but now it needs to read more data before it can send anything.
                // Thus we wait for READ event, but will call SSL_write() again.
                session->flags |= SESSIONFL_SSL_WRITE_WANTS_READ;
				return 0;

           case SSL_ERROR_WANT_WRITE:
                // This is not really an error. We sent some data, but now OpenSSL
                // wants to send some internal data before sending ours.
                // Repeat the same write.
				return 0;
*/
        }
        
		// This is an SSL error, handle it
		//ssl_handle_error( session, ERR_get_error() ); 
    }

	return -1;
}

#endif


// Handles both SSL and non-SSL reads.
// Returns -1 in case there is an error and socket should be closed/connection terminated
// Returns 0 in case there is a temporary error and the call should be retried (SSL_WANTS_WRITE case)
// Returns a positive number if we actually read something
static int session_socket_read( irc_session_t * session )
{
	int length;

#if defined (ENABLE_SSL)
	if (session->flags & SESSIONFL_SSL_CONNECTION)
	{
		// Yes, I know this is tricky
		if ( session->flags & SESSIONFL_SSL_READ_WANTS_WRITE )
		{
			session->flags &= ~SESSIONFL_SSL_READ_WANTS_WRITE;
			ssl_send( session );
			return 0;
		}
		
		return ssl_recv( session );
	}
#endif
	
	length = socket_recv( &session->sock, 
						session->incoming_buf + session->incoming_offset, 
					    (sizeof (session->incoming_buf) - 1) - session->incoming_offset );
	
	// There is no "retry" errors for regular sockets
	if ( length <= 0 )
		return -1;
	
	return length;
}

// Handles both SSL and non-SSL writes.
// Returns -1 in case there is an error and socket should be closed/connection terminated
// Returns 0 in case there is a temporary error and the call should be retried (SSL_WANTS_WRITE case)
// Returns a positive number if we actually sent something
static int session_socket_write( irc_session_t * session )
{
	int length;

#if defined (ENABLE_SSL)
	if (session->flags & SESSIONFL_SSL_CONNECTION)
	{
		// Yep
		if ( session->flags & SESSIONFL_SSL_WRITE_WANTS_READ )
		{
			session->flags &= ~SESSIONFL_SSL_WRITE_WANTS_READ;
			ssl_recv( session );
			return 0;
		}
		
		return ssl_send( session );
	}
#endif
	
	length = socket_send (&session->sock, session->outgoing_buf, session->outgoing_offset);
	
	// There is no "retry" errors for regular sockets
	if ( length <= 0 )
		return -1;
	
	return length;
}
