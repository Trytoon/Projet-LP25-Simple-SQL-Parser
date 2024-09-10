#include "expand.h"
#include "table.h"

void expand(query_result_t *query) {

    if (!query) return;

    if (query->query_type == QUERY_SELECT) {
        expand_select(&query->query_content.select_query);
    } else if (query->query_type == QUERY_INSERT) {
        expand_insert(&query->query_content.insert_query);
    }
}

void expand_select(update_or_select_query_t *query) {

    if (!query) return;

    if (query->set_clause.fields_count == 1 && strcmp(query->set_clause.fields[0].column_name, "*") == 0) {
        FILE *f = open_definition_file(query->table_name, "r");
        char line[165] = {0}; //Buffer
        query->set_clause.fields_count = 0;
        int i;

        if (f) {
            while (fgets(line, sizeof(line), f)) {
                i = query->set_clause.fields_count;
                memcpy(query->set_clause.fields[i].column_name,&line[2], 160);
                query->set_clause.fields_count++;
            }
        }

        fclose(f);
    }
}

void expand_insert(insert_query_t *query) {

    if (query) {
        table_definition_t result;
        get_table_definition(query->table_name, &result);
        bool is_in;

        for (int i = 0; i < result.fields_count; i = i + 1) {
            is_in = is_field_in_record(&query->fields_names, result.definitions[i].column_name);

            if (!is_in) {
                strcpy(query->fields_names.fields[i].column_name, result.definitions[i].column_name);
                make_default_value(&query->fields_names.fields[i], query->table_name);
            }
        }

    }
}


bool is_field_in_record(table_record_t *record, char *field_name) {

    if (!record || !field_name) return false;

    int i = 0;

    while (i < record->fields_count && strcmp(field_name, record->fields[i].column_name) != 0) {
        i++;
    }

    if (i >= record->fields_count) {
        return false;
    }

    return true;
}

void make_default_value(field_record_t *field, char *table_name) {

    if (!field || !table_name) return;

    table_definition_t table;
    get_table_definition(table_name, &table);
    int i = 0;
    bool trouve = false;

    while (i < table.fields_count && !trouve) {
        if (strcmp(table.definitions[i].column_name, field->column_name) == 0) {
            field->field_type = table.definitions[i].column_type;
            trouve = true;
        }
        i++;
    }

    switch (field->field_type) {
        case TYPE_PRIMARY_KEY:
            field->field_value.primary_key_value = field->field_value.primary_key_value +1; //Pour chaque nouvelle insertion on ajoute 1 à la clé primaire
            break;
        case TYPE_INTEGER:
            field->field_value.int_value = 0;
            break;
        case TYPE_FLOAT:
            field->field_value.float_value = 0.;
            break;
        case TYPE_TEXT:
            strcpy(field->field_value.text_value, "");
            break;
        default:
            break;
    }
}