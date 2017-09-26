#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <commctrl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

char *ossl_err_as_string(void)
{
	BIO *bio = BIO_new(BIO_s_mem());
	ERR_print_errors(bio);
	char *buf = NULL;
	size_t len = BIO_get_mem_data(bio, &buf);
	char *ret = (char *)calloc(1, 1 + len);
	if (ret)
		memcpy(ret, buf, len);
	BIO_free(bio);
	return ret;
}

void receiving(void* ssl_)
{
	SSL* ssl = (SSL*)ssl_;
	for (;;)
	{
		char buffer[1000];
		int rb = 0;

		rb = SSL_read(ssl, buffer, sizeof(buffer));
		if (rb <= 0)
		{
			printf("--- Disconnected !\r\n\r\n");
			exit(0);
		}
	}
}

void looping(SSL* ssl)
{
	_beginthread(receiving, 4096, ssl);
	for (; ; )
	{
		char c = getc(stdin);
		int wb = SSL_write(ssl, &c, 1);
		if (wb <= 0)
		{
			printf("--- Disconnected !\r\n\r\n");
			exit(0);
		}
	}
}

int create_socket(int port)
{
	WSADATA wData;
	WSAStartup(MAKEWORD(2, 2), &wData);

	sockaddr_in dA;
	memset(&dA, 0, sizeof(dA));
	dA.sin_family = AF_INET;

	dA.sin_port = htons(port);
	dA.sin_addr.s_addr = INADDR_ANY;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	int slen = sizeof(sockaddr_in);
	if (bind(s, (sockaddr*)&dA, slen) < 0)
	{
		printf("--- Cannot bind !\r\n");
		return 4;
	}
	if (listen(s, 3) < 0)
	{
		printf("--- Cannot listen !\r\n");
		return 5;
	}
	return s;
}

void init_openssl()
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
	EVP_cleanup();
}

SSL_CTX *create_context()
{
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	method = SSLv23_server_method();

	ctx = SSL_CTX_new(method);
	if (!ctx) 
	{
		printf(ossl_err_as_string());
	}

	return ctx;
}

void configure_context(SSL_CTX *ctx)
{//openssl.exe req -newkey rsa:2048 -config cnf/openssl.cnf  -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
	SSL_CTX_set_ecdh_auto(ctx, 1);

	if (SSL_CTX_use_certificate_file(ctx, "certificate.pem", SSL_FILETYPE_PEM) <= 0)
	{
		printf(ossl_err_as_string());
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) 
	{
		printf(ossl_err_as_string());
	}
}

int main(int argc, char **argv)
{
	SOCKET sock;
	SSL_CTX *ctx;

	init_openssl();
	ctx = create_context();

	configure_context(ctx);

	sock = create_socket(1111);

	/* Handle connections */
	while (1) {
		struct sockaddr_in addr;
		int len = sizeof(addr);
		SSL *ssl;

		SOCKET client = accept(sock, (struct sockaddr*)&addr, &len);
		if (client < 0) {
			printf(ossl_err_as_string());
		}

		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, client);

		int c = SSL_accept(ssl);
		if (c <= 0) {
			printf(ossl_err_as_string());
		}
		else
		{
			printf("OK , connected with %s:%u...\r\n\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			looping(ssl);
		}

		SSL_free(ssl);
		closesocket(client);
	}

	closesocket(sock);
	SSL_CTX_free(ctx);
	cleanup_openssl();

	getc(stdin);
}