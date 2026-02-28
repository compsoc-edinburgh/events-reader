enum STORAGE_RESPONSE_CODE {
    SUCCESS,
    FAILURE,
    NONE
}

STORAGE_RESPONSE_CODE lookup_entry(char* entry_name, char* buffer, size_t buff_size);
STORAGE_RESPONSE_CODE write_entry(char* entry_name, char* entry_contents);
STORAGE_RESPONSE_CODE delete_entry(char* entry_name);

STORAGE_RESPONSE_CODE read_file(char* filename, char* buffer, size_t buff_size);
STORAGE_RESPONSE_CODE append_file(char* filename, char* entry);
STORAGE_RESPONSE_CODE delete_file(char* filename);
