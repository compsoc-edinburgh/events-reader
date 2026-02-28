enum NFC_RETURN_CODE {
    SUCCESS,
    FAILURE,
    NONE
}

NFC_RETURN_CODE read_nfc_tag(char* uid_buffer, size_t buffer_size);
