//
// Created by flassabe on 19/11/2021.
//

#ifndef _TABLE_H
#define _TABLE_H

#include <stdint.h>
#include <stdio.h>

#include "sql.h"

typedef struct __attribute__((__packed__)) {
    uint8_t is_active;
    uint32_t record_offset;
    uint16_t record_length;
} index_record_t;

FILE *open_definition_file(char *table_name, char *mode);

table_definition_t *get_table_definition(char *table_name, table_definition_t *result);

#endif //_TABLE_H
