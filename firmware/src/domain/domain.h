enum DOMAIN_LOGIC_RESPONSE_CODE {
    SUCCESS,
    AUTHORIZED,
    UNAUTHORIZED,
    FAILURE,
    NONE
};

DOMAIN_LOGIC_RESPONSE_CODE lookup_uid(const char* uid, char* student_id, size_t buff_size);
DOMAIN_LOGIC_RESPONSE_CODE register_uid(const char* uid, const char* student_id);

DOMAIN_LOGIC_RESPONSE_CODE validate_student_id(const char* student_id);
DOMAIN_LOGIC_RESPONSE_CODE check_in_student_id(const char* student_id);
