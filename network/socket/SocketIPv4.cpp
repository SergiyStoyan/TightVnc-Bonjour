// Copyright (C) 2008,2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the CisteraVNC software.  Please visit our Web site:
//
//                       http://www.cistera.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

//#include <winsock2.h>//added for WSASetSocketSecurity()
//#include <mstcpip.h>//added for WSASetSocketSecurity()
//#include <Ws2tcpip.h>//added for WSASetSocketSecurity()

#include <stdlib.h>
#include "SocketAddressIPv4.h"
#include "SocketAddressIPv4.h"
#include "SocketIPv4.h"
#include "win-system/Environment.h"

#include "thread/AutoLock.h"

#include <crtdbg.h>

SocketIPv4::SocketIPv4(bool useSsl)
	: m_localAddr(NULL), m_peerAddr(NULL), m_isBound(false), m_wsaStartup(1, 2)
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_isClosed = false;

	if (m_socket == INVALID_SOCKET) {
		throw SocketException();
	}

	m_useSsl = useSsl;
	m_ssl = NULL;
}

bool SocketIPv4::sslInitialized = false;
int SocketIPv4::sslSocketCount = 0;
SSL_CTX* SocketIPv4::m_sslCtx = NULL;

SocketIPv4::~SocketIPv4()
{
	destroySslSocket();

#ifdef _WIN32
	::closesocket(m_socket);
#else
	::close(m_socket);
#endif

	AutoLock l(&m_mutex);

	if (m_peerAddr) {
		delete m_peerAddr;
	}

	if (m_localAddr) {
		delete m_localAddr;
	}
}

void SocketIPv4::createSslSocket(bool server)
{
	if (m_ssl != NULL)
		return;

	sslSocketCount++;
	if (sslSocketCount == 1)
		initializeSsl(server);

	m_ssl = SSL_new(m_sslCtx);
	if(m_ssl == NULL)
		throwSslException();
	SSL_set_fd(m_ssl, m_socket);
	if (server)
	{
		if (SSL_accept(m_ssl) <= 0)
			throwSslException();
	}
	else
	{
		if (SSL_connect(m_ssl) <= 0)
			throwSslException();
	}
}

void SocketIPv4::destroySslSocket()
{
	if (m_ssl == NULL)
		return;

	SSL_free(m_ssl);
	m_ssl = NULL;
	sslSocketCount--;
	if (sslSocketCount == 0)
		shutdownSsl();
}

void SocketIPv4::initializeSsl(bool server)
{
	if (!sslInitialized)
	{
		sslInitialized = true;
		ERR_load_crypto_strings();
		SSL_load_error_strings();
		//SSL_library_init();
		OpenSSL_add_ssl_algorithms();
		//OPENSSL_config(NULL);
	}
	if (m_sslCtx == NULL)
		m_sslCtx = createSslContext(server);
}

void SocketIPv4::shutdownSsl()
{
	//Not really necessary, since resources will be freed on process termination
	ERR_free_strings();
	//RAND_cleanup();
	EVP_cleanup();
	// this seems to cause double-frees for me: ENGINE_cleanup ();
	//CONF_modules_free();
	ERR_remove_state(0);
	sslInitialized = false;

	if (m_sslCtx != NULL)
	{
		SSL_CTX_free(m_sslCtx);
		m_sslCtx = NULL;
	}
}

void SocketIPv4::getSslErrors(TCHAR* m, int size)
{
	BIO *bio = BIO_new(BIO_s_mem());
	ERR_print_errors(bio);
	//char buffer[2000];
	char* buffer = NULL;
	size_t bl = BIO_get_mem_data(bio, &buffer);
	size_t string_length = bl < size - 1 ? bl : size - 1;
#ifdef UNICODE
	//TCHAR == WCHAR
	mbstowcs(m, buffer, string_length);
#else
	//TCHAR == char	
	memcpy(m, buffer, string_length);
#endif
	m[string_length] = '\0';
	BIO_free(bio);
}

void SocketIPv4::throwSslException()
{
	TCHAR m[2000];
	getSslErrors(m, sizeof(m));
	throw SocketException(m);
}

SSL_CTX* SocketIPv4::createSslContext(bool server)
{
	const SSL_METHOD* method = server ? SSLv23_server_method(): SSLv23_client_method();
	SSL_CTX* ctx = SSL_CTX_new(method);
	if (ctx == NULL)
		throwSslException();

	if (!server)
		return ctx;

	//#####################create self-signed certificate and key######################
	//>openssl.exe req -newkey rsa:2048 -config cnf/openssl.cnf  -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
	
	SSL_CTX_set_ecdh_auto(ctx, 1);
	
	StringStorage currentModuleFolderPath_;
	Environment::getCurrentModuleFolderPath(&currentModuleFolderPath_);
	char currentModuleFolderPath[2000];
#ifdef UNICODE
	//TCHAR == WCHAR
	wcstombs(currentModuleFolderPath, currentModuleFolderPath_.getString(), sizeof(currentModuleFolderPath));
#else
	//TCHAR == char	
	strcpy(currentModuleFolderPath, (char *)currentModuleFolderPath_.getString());
#endif
	char buffer[2000];
	sprintf(buffer, "%s\\%s", currentModuleFolderPath, "server_certificate.pem");
	if (SSL_CTX_use_certificate_file(ctx, buffer, SSL_FILETYPE_PEM) <= 0)
		throwSslException();

	sprintf(buffer, "%s\\%s", currentModuleFolderPath, "server_key.pem");
	if (SSL_CTX_use_PrivateKey_file(ctx, buffer, SSL_FILETYPE_PEM) <= 0)
		throwSslException();

	return ctx;
}

void SocketIPv4::connect(const TCHAR *host, unsigned short port)
{
  SocketAddressIPv4 address(host, port);

  connect(address);
}

void SocketIPv4::connect(const SocketAddressIPv4 &addr)
{
  struct sockaddr_in targetSockAddr = addr.getSockAddr();

  if (::connect(m_socket, (const sockaddr *)&targetSockAddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  if (m_useSsl)
	  createSslSocket(false);

  AutoLock l(&m_mutex);

  if (m_peerAddr) {
    delete m_peerAddr;
  }

  m_peerAddr = new SocketAddressIPv4(*(struct sockaddr_in *)&targetSockAddr);

  m_isBound = false;
}

void SocketIPv4::close()
{
  m_isClosed = true;
}

void SocketIPv4::shutdown(int how)
{
	if (::shutdown(m_socket, how) == SOCKET_ERROR) {
		throw SocketException();
	}
}

void SocketIPv4::bind(const TCHAR *bindHost, unsigned int bindPort)
{
  SocketAddressIPv4 address(bindHost, bindPort);

  bind(address);
}

void SocketIPv4::bind(const SocketAddressIPv4 &addr)
{
  struct sockaddr_in bindSockaddr = addr.getSockAddr();

  if (::bind(m_socket, (const sockaddr *)&bindSockaddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  AutoLock l(&m_mutex);

  if (m_localAddr) {
    delete m_localAddr;
  }

  m_localAddr = new SocketAddressIPv4(*(struct sockaddr_in*)&bindSockaddr);

  m_isBound = true;
}

bool SocketIPv4::isBound()
{
  AutoLock l(&m_mutex);

  return m_isBound;
}

void SocketIPv4::listen(int backlog)
{
  if (::listen(m_socket, backlog) == SOCKET_ERROR) {
    throw SocketException();
  }
}

SocketIPv4 *SocketIPv4::accept()
{
  struct sockaddr_in addr;

  SOCKET result = getAcceptedSocket(&addr);

  SocketIPv4 *accepted;

  try {
    accepted = new SocketIPv4(m_useSsl); 
    accepted->close();
  } catch(...) {
    // Cleanup and throw further
#ifdef _WIN32
    ::closesocket(result);
#else
    ::close(result);
#endif
    throw SocketException();
  }

  // Fall out with exception, no need to check if accepted is NULL
  accepted->set(result); 
  return accepted; // Valid and initialized
}

void SocketIPv4::set(SOCKET socket)
{
	AutoLock l(&m_mutex);

#ifdef _WIN32
	::closesocket(m_socket);
#else
	::close(m_socket);
#endif
	m_socket = socket;

	if (m_useSsl)
		createSslSocket(true);

	// Set local and peer addresses for new socket
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (getsockname(socket, (struct sockaddr *)&addr, &addrlen) == 0) {
		m_localAddr = new SocketAddressIPv4(addr);
	}

	if (getpeername(socket, (struct sockaddr *)&addr, &addrlen) == 0) {
		m_peerAddr = new SocketAddressIPv4(addr);
	}
}

SOCKET SocketIPv4::getAcceptedSocket(struct sockaddr_in *addr)
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	fd_set afd;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;
	SOCKET result = INVALID_SOCKET;

	while (true) {
		FD_ZERO(&afd);
		FD_SET(m_socket, &afd);

		// FIXME: The select() and accept() function can provoke an system
		// exception, if it allows by project settings and closesocket() has alredy
		// been called.
		int ret = select((int)m_socket + 1, &afd, NULL, NULL, &timeout);

		if (m_isClosed || ret == SOCKET_ERROR)
			throw SocketException();
		else if (ret == 0)
			continue;
		else if (ret > 0)
		{
			if (FD_ISSET(m_socket, &afd))
			{
				result = ::accept(m_socket, (struct sockaddr*)addr, &addrlen);
				if (result == INVALID_SOCKET)
					throw SocketException();
				break;
			} // if.
		} // if select ret > 0.
	} // while waiting for incoming connection.
	return result;
}

int SocketIPv4::send(const char *data, int size, int flags)
{
	int result;

	if (m_useSsl)
	{
		result = SSL_write(m_ssl, data, size);
		if (result == 0)
			throw IOException(_T("Connection has been gracefully closed"));
		if (result < 0)
			throw IOException(_T("Failed to write data to SSL socket."));
	}
	else
	{
		result = ::send(m_socket, data, size, flags);
		if (result == -1)
			throw IOException(_T("Failed to send data to socket."));
	}

	return result;
}

int SocketIPv4::recv(char *buffer, int size, int flags)
{
	int result;

	if (m_useSsl)
	{
		result = SSL_read(m_ssl, buffer, size);
		if (result == 0)
			throw IOException(_T("Connection has been gracefully closed"));
		if (result < 0)
			throw IOException(_T("Failed to read data from SSL socket."));
	}
	else
	{
		result = ::recv(m_socket, buffer, size, flags);
		if (result == 0)
			throw IOException(_T("Connection has been gracefully closed"));
		if (result == SOCKET_ERROR)
			throw IOException(_T("Failed to recv data from socket.")); 
	}
	return result;
}

bool SocketIPv4::getLocalAddr(SocketAddressIPv4 *addr)
{
  AutoLock l(&m_mutex);

  if (m_localAddr == 0) {
    return false;
  }

  *addr = *m_localAddr;

  return true;
}

bool SocketIPv4::getPeerAddr(SocketAddressIPv4 *addr)
{
  AutoLock l(&m_mutex);

  if (m_peerAddr == 0) {
    return false;
  }

  *addr = *m_peerAddr;

  return true;
}

/* Auxiliary */
void SocketIPv4::setSocketOptions(int level, int name, void *value, socklen_t len)
{
  if (setsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::getSocketOptions(int level, int name, void *value, socklen_t *len)
{
  if (getsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::enableNaggleAlgorithm(bool enabled)
{
  BOOL disabled = enabled ? 0 : 1;

  setSocketOptions(IPPROTO_TCP, TCP_NODELAY, &disabled, sizeof(disabled));
}

void SocketIPv4::setExclusiveAddrUse()
{
  int val = 1;

  setSocketOptions(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &val, sizeof(val));
}
