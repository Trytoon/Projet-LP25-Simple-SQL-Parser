#include "query_exec.h"

#include <dirent.h>
#include <unistd.h>

#include "database.h"
#include "table.h"
#include "record_list.h"

void execute(query_result_t *query) {

    if (!query) return;

    switch (query->query_type) {
        case QUERY_CREATE_TABLE:
            execute_create(&query->query_content.create_query);
            break;
        case QUERY_INSERT:
            execute_insert(&query->query_content.insert_query);
            break;
        case QUERY_SELECT:
            execute_select(&query->query_content.select_query);
            break;
        case QUERY_UPDATE:
            execute_update(&query->query_content.update_query);
            break;
        case QUERY_DELETE:
            execute_delete(&query->query_content.delete_query);
            break;
        case QUERY_DROP_TABLE:
            execute_drop_table(query->query_content.table_name);
            break;
        case QUERY_DROP_DB:
            execute_drop_database(query->query_content.database_name);
            break;
        default:
            break;
    }
}

void execute_create(create_query_t *query) {

    if (!query) return;
    create_table(query);
}
void execute_insert(insert_query_t *query) {
    if (!query) return;
    add_row_to_table(query->table_name, &query->fields_names);
}

void execute_drop_table(char *table_name) {
    if (!table_name) return;

    char buffer[200];
    sprintf(buffer, "./%s", table_name);

    recursive_rmdir(buffer);
}

void execute_drop_database(char *db_name) {
    if (!db_name) return;
    char buffer[200];
    sprintf(buffer, "./%s", db_name);
    recursive_rmdir(buffer);
}

void execute_select(update_or_select_query_t *query) {
    if (!query) return;

    record_list_t liste_enregistrements;
    liste_enregistrements = *get_filtered_records(query->table_name, &query->set_clause, &query->where_clause, &liste_enregistrements);
    display_table_record_list(&liste_enregistrements);
}

//todo
void execute_update(update_or_select_query_t *query) {

    if (!query) return;

}

//todo
void execute_delete(delete_query_t *query) {

    if (!query) return;


}

