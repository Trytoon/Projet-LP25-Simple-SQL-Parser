#include "check.h"

/*!
 * @brief function check_query is the high level check function, which will call specialized check functions and
 * return their result.
 * @param query a pointer to a query result structure returned after parsing the SQL query (@see sql.h)
 * @return true if the query is valid, false else
 */
bool check_query(query_result_t *query) {

    if (!query) return false;

    switch (query->query_type) {
        case QUERY_SELECT:
            return check_query_select(&query->query_content.select_query);
            break;
        case QUERY_INSERT:
            return check_query_insert(&query->query_content.insert_query);
            break;
        case QUERY_DELETE:
            return check_query_delete(&query->query_content.delete_query);
            break;
        case QUERY_CREATE_TABLE:
            return check_query_create(&query->query_content.create_query);
            break;
        case QUERY_UPDATE:
            return check_query_update(&query->query_content.update_query);
            break;
        case QUERY_DROP_DB:
            return check_query_drop_db((char *) &query->query_content.database_name);
            break;
        case QUERY_DROP_TABLE:
            return check_query_drop_table((char *) &query->query_content.table_name);
            break;
        default:
            printf("Type de Requete inconnue !\n");
    }
    return false;
}

/*!
 * @brief check_query_select checks a select query
 * It relies on check_fields_list (list of fields to display and WHERE clause) and check_value_types (WHERE clause)
 * Also uses get_table_definition from table.h
 * @see check_fields_list
 * @see check_value_types
 * @see get_table_definition
 * @param query a pointer to the select query
 * @return true if valid, false if invalid
 */
bool check_query_select(update_or_select_query_t *query) {

    if (!query) return false;

    if (query->set_clause.fields_count == 1
        && strcmp(query->set_clause.fields[0].column_name, "*") == 0
        && query->where_clause.values.fields_count == 0) return true;

    table_definition_t table;
    get_table_definition(query->table_name, &table);

    return check_fields_list(&query->set_clause, &table) == true
           && check_fields_list(&query->where_clause.values, &table) == true
           && check_value_types(&query->where_clause.values, &table) == true;
}

/*!
 * @brief check_query_update checks an update query
 * It relies on check_value_types and get_table_definition from table.h.
 * @see check_value_types
 * @see get_table_definition
 * @param query a pointer to the update query
 * @return true if valid, false if invalid
 */
bool check_query_update(update_or_select_query_t *query) {

    if (!query) return false;

    table_definition_t table;
    get_table_definition(query->table_name, &table);

    return check_fields_list(&query->set_clause, &table) == true
           && check_fields_list(&query->where_clause.values, &table) == true
           && check_value_types(&query->set_clause, &table) == true
           && check_value_types(&query->where_clause.values, &table) == true;
}

/*!
 * @brief check_query_create checks a create query. It checks if the table doesn't already exist
 * @param query a pointer to the create query
 * @return true if valid, false if invalid
 */
bool check_query_create(create_query_t *query) {

    if (!query) return false;

    char path[] = "..//";
    return !directory_exists(make_full_path(path, query->table_name));
}

/*!
 * @brief check_query_insert checks an insert query
 * It relies on check_value_types and get_table_definition from table.h.
 * Prior to checking the field/value pairs, you must join fields and values (extracted to different table_record_t
 * structs during the SQL parsing) by copying the values strings into the text_value member of the fields names
 * table_record_t structure.
 * @see check_value_types
 * @see get_table_definition
 * @param query a pointer to the insert query
 * @return true if valid, false if invalid
 */
bool check_query_insert(insert_query_t *query) {

    if (!query) return false;

    table_definition_t table;
    get_table_definition(query->table_name, &table);

    for (int i = 0; i < query->fields_values.fields_count; i++) {
        strcpy(query->fields_names.fields[i].field_value.text_value, query->fields_values.fields[i].column_name);
    }

    return check_value_types(&query->fields_names, &table);
}

/*!
 * @brief check_query_delete checks a delete query
 * It relies on check_value_types (WHERE clause) and get_table_definition from table.h.
 * @see check_value_types
 * @see get_table_definition
 * @param query a pointer to the delete query
 * @return true if valid, false if invalid
 */
bool check_query_delete(delete_query_t *query) {
    if (!query) return false;

    table_definition_t table;
    get_table_definition(query->table_name, &table);

    return check_value_types(&query->where_clause.values, &table);
}

/*!
 * @brief check_query_drop_table checks a drop table query: checks that the table exists
 * @param table_name the table name
 * @return true if valid, false if invalid
 */
bool check_query_drop_table(char *table_name) {

    char buffer[1000] = "./";

    if (!table_name) return false;
    return directory_exists(make_full_path(buffer, table_name));
}

/*!
 * @brief check_query_drop_db checks a drop database query: checks that the database exists
 * @param db_name the database name
 * @return true if valid, false if invalid
 */
bool check_query_drop_db(char *db_name) {
    if (!db_name) return false;
    return directory_exists(db_name);
}

/*!
 * @brief function check_fields_list checks if all fields from a fields list exist in a table definition
 * Uses find_field_definition
 * @see find_field_definition
 * @param fields_list list of the fields to check
 * @param table_definition table definition within which to check
 * @return true if all fields belong to table, false else
 */
bool check_fields_list(table_record_t *fields_list, table_definition_t *table_definition) {

    if (!fields_list || !table_definition) return false;

    int i = 0;
    int valid_values = 0;

    while(i < fields_list->fields_count) {
        if (find_field_definition(fields_list->fields[i].column_name, table_definition) != NULL) {
            valid_values++;
        }
        i++;
    }

    return valid_values == fields_list->fields_count;
}

/*!
 * @brief function check_value_types checks if all fields in a record list belong to a table (through its definition)
 * AND if the corresponding value can be converted to the field type as specified in the table definition.
 * Uses find_field_definition and is_value_valid
 * @see find_field_definition
 * @see is_value_valid
 * @param fields_list list of the fields and their values (as text)
 * @param table_definition table definition
 * @return true if all fields belong to table and their value types are correct, false else
 */
bool check_value_types(table_record_t *fields_list, table_definition_t *table_definition) {

    if (!fields_list || !table_definition) return false;

    for (int i = 0; i < fields_list->fields_count; i++) {
        field_definition_t *champ = find_field_definition(fields_list->fields[i].column_name, table_definition);

        if (!champ) { //Un des champs ne correspond pas dans la table
            return false;
        } else {
            if (!is_value_valid(&fields_list->fields[i], champ)) { //La valeur du champ n'est pas coherante
                return false;
            }
        }
    }
    return true;
}

/*!
 * @brief function find_field_definition looks up for a field name in a table definition.
 * @param field_name the field name to lookup for.
 * @param table_definition the table definition in which to search.
 * @return a pointer to the field definition structure if the field name exists, NULL if it doesn't.
 */
field_definition_t *find_field_definition(char *field_name, table_definition_t *table_definition) {

    if (!field_name || !table_definition) return false;

    bool stop = false;
    int i = 0;
    while (i < table_definition->fields_count && !stop) {

        if (strcmp(table_definition->definitions[i].column_name, field_name) != 0) {
            i++;
        } else {
            stop = true;
        }
    }

    if (i >= table_definition->fields_count) {
        return NULL;
    }

    return  &table_definition->definitions[i];
}

/*!
 * @brief function is_value_valid checks if a field_record_t holding a field name and a text value matches a field definition:
 * it checks fields names, and tests if the text value can be converted into the table defined type.
 * /!\ If it succeeded, this function must modify the value in the field_record_t structure by converting and setting
 * the value to the proper type!
 * Uses is_int, is_float, is_key
 * @see is_int
 * @see is_float
 * @see is_key
 * @param value the field and value record
 * @param field_definition the field definition to test against
 * @return true if valid (and converted), false if invalid
 */
bool is_value_valid(field_record_t *value, field_definition_t *field_definition) {

    if (!value || !field_definition) return false;

    if (strcmp(value->column_name, field_definition->column_name) != 0) {
        return false;
    }

    switch (field_definition->column_type) {
        case TYPE_INTEGER:
            if (is_int(value->field_value.text_value)) {
                value->field_type = TYPE_INTEGER;
                value->field_value.int_value = atoi(value->field_value.text_value);
                return true;
            }
            break;
        case TYPE_FLOAT:
            if (is_float(value->field_value.text_value)) {
                value->field_type = TYPE_FLOAT;
                value->field_value.float_value = atof(value->field_value.text_value);
                return true;
            }
            break;
        case TYPE_PRIMARY_KEY:
            if (is_key(value->field_value.text_value)) {
                value->field_type = TYPE_PRIMARY_KEY;
                value->field_value.primary_key_value = strtoull(value->field_value.text_value, NULL, 10);
                return true;
            }
        case TYPE_TEXT:
            value->field_type = TYPE_TEXT;
            break;
        default:
            return false;
    }

    return true;
}

/*!
 * @brief function is_int tests if the string value is a text representation of an integer value.
 * @param value the text representation to test
 * @return true if value can be converted into an integer, false if it cannot
 */
bool is_int(char *value) {

    if (!value) return false;

    char *end;

    strtoll(value, &end, 10);

    if (end[0] != '\0') {
        return false;
    }

    return true;
}

/*!
 * @brief function is_float tests if the string value is a text representation of a double value.
 * @param value the text representation to test
 * @return true if value can be converted into a double, false if it cannot
 */
bool is_float(char *value) {

    if (!value) return false;

    bool valide = true;

    do {
        if ((value[0] < '0' || value[0] > '9') && value[0] != '.') {
            valide = false;
        }
        value++;

    } while (valide && value[0] != '\0');

    return valide;
}

/*!
 * @brief function is_int tests if the string value is a text representation of a key value.
 * You may use strtoull for this test.
 * @param value the text representation to test
 * @return true if value can be converted into a key, false if it cannot
 */
bool is_key(char *value) {

    if (!value) return false;

    char *end;
    unsigned long long  numerical_value = strtoull(value, &end, 10);

    if (end[0] != '\0' || value[0] == '-' || numerical_value > ULONG_MAX) {
        return false;
    }
    return true;
}
