typedef enum {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    PENDING,
    NONE
} NETWORK_RESPONSE_CODE;

NETWORK_RESPONSE_CODE init_networking();
NETWORK_RESPONSE_CODE get_request(const char* uri, const char* headers,
                                  char* reply, size_t reply_size);
NETWORK_RESPONSE_CODE post_request(const char* uri, const char* headers,
                                   char* reply, size_t reply_size);
