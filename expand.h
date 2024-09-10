#ifndef LP25_PROJET_WIP_EXPAND_H
#define LP25_PROJET_WIP_EXPAND_H

#include "sql.h"

void expand(query_result_t *query);

void expand_select(update_or_select_query_t *query);
void expand_insert(insert_query_t *query);

bool is_field_in_record(table_record_t *record, char *field_name);
void make_default_value(field_record_t *field, char *table_name);

#endif //LP25_PROJET_WIP_EXPAND_H
