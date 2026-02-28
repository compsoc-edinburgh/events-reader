enum NETWORK_RESPONSE_CODE {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    PENDING,
    NONE
};

NETWORK_RESPONSE_CODE connect_to_wifi(char* ssid, char* password);
NETWORK_RESPONSE_CODE get_request(const char* uri, const char* headers,
                                  char* reply, size_t reply_size);
NETWORK_RESPONSE_CODE post_request(const char* uri, const char* headers,
                                   char* reply, size_t reply_size);
