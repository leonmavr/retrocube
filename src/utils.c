#include "utils.h"
#include <stdbool.h>

bool ut_is_decimal(char* string) {
    bool ret = false;
    for (char* s = string; *s != '\0'; ++s) {
        if (((*s >= '0') && (*s <= '9')) ||
            (*s == '.') || (*s == ',') ||
            (*s == '\n'))
            ret = true;
        else
            return false;
    }
    return ret;
}
