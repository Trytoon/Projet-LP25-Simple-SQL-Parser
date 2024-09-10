#ifndef LP25_PROJET_WIP_QUERY_EXEC_H
#define LP25_PROJET_WIP_QUERY_EXEC_H

#include "sql.h"

void execute(query_result_t *query);

void execute_create(create_query_t *query);
void execute_insert(insert_query_t *query);
void execute_select(update_or_select_query_t *query);
void execute_update(update_or_select_query_t *query);
void execute_delete(delete_query_t *query);
void execute_drop_table(char *table_name);
void execute_drop_database(char *db_name);

#endif //LP25_PROJET_WIP_QUERY_EXEC_H
