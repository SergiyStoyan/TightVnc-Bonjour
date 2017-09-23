#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
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

int connect_socket(char* ip, int port)
{
	WSADATA wData;
	WSAStartup(MAKEWORD(2, 2), &wData);

	sockaddr_in da;
	memset(&da, 0, sizeof(da));
	da.sin_family = AF_INET;
	unsigned long inaddr = inet_addr(ip);
	if (inaddr != INADDR_NONE)
		memcpy(&da.sin_addr, &inaddr, sizeof(inaddr));
	else
	{
		hostent* hp = gethostbyname(ip);
		if (!hp)
		{
			printf("--- Remote system not found !\r\n");
			return 2;
		}
		memcpy(&da.sin_addr, hp->h_addr, hp->h_length);
	}
	da.sin_port = htons(port);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(s, (sockaddr*)&da, sizeof(da)) < 0)
	{
		printf("--- Cannot connect !\r\n");
		exit(EXIT_FAILURE);
	}

	printf("OK , connected with %s:%u...\r\n\r\n", inet_ntoa(da.sin_addr), ntohs(da.sin_port));
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
	const SSL_METHOD* method = SSLv23_server_method();
	SSL_CTX* ctx = SSL_CTX_new(method);
	if (!ctx) {
		printf(ossl_err_as_string());
		exit(EXIT_FAILURE);
	}
	return ctx;
}

void configure_context(SSL_CTX *ctx)
{
	//SSL_CTX_set_ecdh_auto(ctx, 1);

	/* Set the key and cert */
	//if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
	//	//ERR_print_errors_fp(stderr);
	//	exit(EXIT_FAILURE);
	//}

	//if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
	//	//ERR_print_errors_fp(stderr);
	//	exit(EXIT_FAILURE);
	//}
}

int main(int argc, char **argv)
{
	ERR_load_crypto_strings();
	init_openssl();
	SSL_CTX* ctx = create_context();

	configure_context(ctx);

	SOCKET sock = connect_socket("127.0.0.1", 1111);
	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sock);

	if (SSL_connect(ssl) <= 0) 
	{	
		printf(ossl_err_as_string());
	}
	else 
		looping(ssl);

	SSL_free(ssl);
	closesocket(sock);

	SSL_CTX_free(ctx);
	cleanup_openssl();

	getc(stdin);
}