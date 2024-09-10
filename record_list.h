#ifndef LP25_PROJET_WIP_RECORD_LIST_H
#define LP25_PROJET_WIP_RECORD_LIST_H

#include "utils.h"

typedef struct record_list_node {
    table_record_t record;
    struct record_list_node *next;
    struct record_list_node *previous;
} record_list_node_t;

typedef struct {
    record_list_node_t *head;
    record_list_node_t *tail;
} record_list_t;

void clear_list(record_list_t *record_list);
void add_record(record_list_t *record_list, table_record_t *record);

int field_record_length(field_record_t *field_record);
void display_table_record_list(record_list_t *record_list);


#endif //LP25_PROJET_WIP_RECORD_LIST_H
