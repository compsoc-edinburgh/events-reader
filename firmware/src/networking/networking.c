#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/http_client.h"
#include "lwip/stats.h"

#include "mbedtls_config.h"

#include "src/networking/networking.h"
#include "src/sched/sched.h"

#define MAX_WIFI_CONNECT_RETRIES 5
#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAX_PASS_LEN 64

// Note, this should only ever be used internally, so this won't be in the
// networking.h header
typedef struct {
    char ssid[WIFI_MAX_SSID_LEN];
    char password[WIFI_MAX_PASS_LEN];
} wifi_creds_t;

typedef enum {
    WIFI_STATE_INIT = 0,
    WIFI_STATE_CONNECT,
    WIFI_STATE_CLEANUP,
    WIFI_STATE_FATAL_NO_CREDS,
    WIFI_STATE_FATAL_MAX_RETRIES
} wifi_state_t;

typedef struct {
    wifi_state_t state;
    uint8_t retries;
    wifi_creds_t creds;
} wifi_connect_ctx_t;

NETWORK_RESPONSE_CODE connect_to_wifi(wifi_creds_t* creds) {
    if (cyw43_arch_wifi_connect_timeout_ms(creds->ssid, creds->password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connected!\n");
    } else {
        printf("Failed to connect.\n");
    }
}

void wifi_connect_task(void* arg) {
    wifi_connect_ctx_t* ctx = (wifi_connect_ctx_t*) arg;
    switch (ctx->state) {
        case WIFI_STATE_INIT:
            // Queues storage task with callback to next stage
            puts("WIFI_STATE_INIT");
            ctx->state = WIFI_STATE_CONNECT;
            enqueue_task(wifi_connect_task, arg);
            break;
        case WIFI_STATE_CONNECT:
            // Connect to wifi
            puts("WIFI_STATE_CONNECT");
            wifi_creds_t creds = {.ssid = "ssid", .password = "password"};
            connect_to_wifi(&creds);
            ctx->state = WIFI_STATE_CLEANUP;
            enqueue_task(wifi_connect_task, arg);
            break;
        case WIFI_STATE_FATAL_NO_CREDS:
            // Log error
            goto cleanup;
        case WIFI_STATE_FATAL_MAX_RETRIES:
            // Log error
            goto cleanup;
        case WIFI_STATE_CLEANUP:
            puts("WIFI_STATE_CLEANUP");
            cleanup:
            scfree(ctx);
            break;
        default:
            break;
    }
}

NETWORK_RESPONSE_CODE init_networking() {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed!\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();  // enable station mode

    wifi_connect_ctx_t* ctx = szalloc(sizeof(wifi_connect_ctx_t));
    ctx->state = WIFI_STATE_INIT;
    if (enqueue_task(wifi_connect_task, (void*) ctx))
        return PENDING;
    return FAILURE;
}



#define HTTP_INFO(x, ...) printf(x, ##__VA_ARGS__)
#define HTTP_DEBUG(x, ...) printf(x, ##__VA_ARGS__)
#define HTTP_ERROR(x, ...) printf(x, ##__VA_ARGS__)
#define HTTP_INFOC(x) printf("%c", x)

typedef struct EXAMPLE_HTTP_REQUEST {
    /*!
     * The name of the host, e.g. www.raspberrypi.com
     */
    const char *hostname;
    /*!
     * The url to request, e.g. /favicon.ico
     */
    const char *url;
    /*!
     * Function to callback with headers, can be null
     * @see httpc_headers_done_fn
     */
    httpc_headers_done_fn headers_fn;
    /*!
     * Function to callback with results from the server, can be null
     * @see altcp_recv_fn
     */
    altcp_recv_fn recv_fn;
    /*!
     * Function to callback with final results of the request, can be null
     * @see httpc_result_fn
     */
    httpc_result_fn result_fn;
    /*!
     * Callback to pass to calback functions
     */
    void *callback_arg;
    /*!
     * The port to use. A default port is chosen if this is set to zero
     */
    uint16_t port;
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    /*!
     * TLS configuration, can be null or set to a correctly configured tls configuration.
     * e.g altcp_tls_create_config_client(NULL, 0) would use https without a certificate
     */
    struct altcp_tls_config *tls_config;
    /*!
     * TLS allocator, used internall for setting TLS server name indication
     */
    altcp_allocator_t tls_allocator;
#endif
    /*!
     * LwIP HTTP client settings
     */
    httpc_connection_t settings;
    /*!
     * Flag to indicate when the request is complete
     */
    int complete;
    /*!
     * Overall result of http request, only valid when complete is set
     */
    httpc_result_t result;

} EXAMPLE_HTTP_REQUEST_T;

// Print headers to stdout
err_t http_client_header_print_fn(__unused httpc_state_t *connection, __unused void *arg, struct pbuf *hdr, u16_t hdr_len, __unused u32_t content_len) {
    HTTP_INFO("\nheaders %u\n", hdr_len);
    u16_t offset = 0;
    while (offset < hdr->tot_len && offset < hdr_len) {
        char c = (char)pbuf_get_at(hdr, offset++);
        HTTP_INFOC(c);
    }
    return ERR_OK;
}

// Print body to stdout
err_t http_client_receive_print_fn(__unused void *arg, __unused struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    HTTP_INFO("\ncontent err %d\n", err);
    u16_t offset = 0;
    while (offset < p->tot_len) {
        char c = (char)pbuf_get_at(p, offset++);
        HTTP_INFOC(c);
    }
    return ERR_OK;
}


static err_t internal_header_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len) {
    assert(arg);
    EXAMPLE_HTTP_REQUEST_T *req = (EXAMPLE_HTTP_REQUEST_T*)arg;
    if (req->headers_fn) {
        return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
    }
    return ERR_OK;
}

static err_t internal_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    assert(arg);
    EXAMPLE_HTTP_REQUEST_T *req = (EXAMPLE_HTTP_REQUEST_T*)arg;
    if (req->recv_fn) {
        return req->recv_fn(req->callback_arg, conn, p, err);
    }
    return ERR_OK;
}

static void internal_result_fn(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
    assert(arg);
    EXAMPLE_HTTP_REQUEST_T *req = (EXAMPLE_HTTP_REQUEST_T*)arg;
    HTTP_DEBUG("result %d len %u server_response %u err %d\n", httpc_result, rx_content_len, srv_res, err);
    req->complete = true;
    req->result = httpc_result;
    if (req->result_fn) {
        req->result_fn(req->callback_arg, httpc_result, rx_content_len, srv_res, err);
    }
}

// Override altcp_tls_alloc to set sni
static struct altcp_pcb *altcp_tls_alloc_sni(void *arg, u8_t ip_type) {
    assert(arg);
    EXAMPLE_HTTP_REQUEST_T *req = (EXAMPLE_HTTP_REQUEST_T*)arg;
    struct altcp_pcb *pcb = altcp_tls_alloc(req->tls_config, ip_type);
    if (!pcb) {
        HTTP_ERROR("Failed to allocate PCB\n");
        return NULL;
    }
    mbedtls_ssl_set_hostname(altcp_tls_context(pcb), req->hostname);
    return pcb;
}

// Make a http request, complete when req->complete returns true
int http_client_request_async(async_context_t *context, EXAMPLE_HTTP_REQUEST_T *req) {
#if LWIP_ALTCP
    const uint16_t default_port = req->tls_config ? 443 : 80;
    if (req->tls_config) {
        if (!req->tls_allocator.alloc) {
            req->tls_allocator.alloc = altcp_tls_alloc_sni;
            req->tls_allocator.arg = req;
        }
        req->settings.altcp_allocator = &req->tls_allocator;
    }
#else
    const uint16_t default_port = 80;
#endif
    req->complete = false;
    req->settings.headers_done_fn = req->headers_fn ? internal_header_fn : NULL;
    req->settings.result_fn = internal_result_fn;
    async_context_acquire_lock_blocking(context);
    err_t ret = httpc_get_file_dns(req->hostname, req->port ? req->port : default_port, req->url, &req->settings, internal_recv_fn, req, NULL);
    async_context_release_lock(context);
    if (ret != ERR_OK) {
        HTTP_ERROR("http request failed: %d", ret);
    }
    return ret;
}

// Make a http request and only return when it has completed. Returns true on success
int http_client_request_sync(async_context_t *context, EXAMPLE_HTTP_REQUEST_T *req) {
    assert(req);
    int ret = http_client_request_async(context, req);
    if (ret != 0) {
        return ret;
    }
    while(!req->complete) {
        async_context_poll(context);
        stats_display();
        async_context_wait_for_work_ms(context, 1000);
    }
    return req->result;
}

NETWORK_RESPONSE_CODE get_request(const char* url, const char* headers,
                                  char* reply, size_t reply_size) {
    EXAMPLE_HTTP_REQUEST_T req1 = {0};
    req1.hostname = "www.example.com";
    req1.url = "/";
    req1.headers_fn = http_client_header_print_fn;
    req1.recv_fn = http_client_receive_print_fn;
    req1.tls_config = altcp_tls_create_config_client(NULL, 0); // https
    int result = http_client_request_sync(cyw43_arch_async_context(), &req1); // repeat
    altcp_tls_free_config(req1.tls_config);
}

NETWORK_RESPONSE_CODE post_request(const char* url, const char* headers,
                                   char* reply, size_t reply_size) {

}
