#include <openssl/ssl.h>
#include <openssl/err.h>

#define USESSL

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
};

void cleanup_openssl() {
    EVP_cleanup();
};

SSL_CTX* create_context() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    // Set client cert and key
    if (SSL_CTX_use_certificate_file(ctx, "./certs/client-cert.pem", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "./certs/client-key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Trust root certificate
    SSL_CTX_load_verify_locations(ctx, "./certs/intermediate-cert.pem", NULL);
}