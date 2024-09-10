#include "sql.h"


bool has_reached_sql_end(char *sql) {

    if (!sql) return false;

    char* copy_SQL = sql;

    while (copy_SQL[0] == ' ') {
        copy_SQL++;
    }

    if (copy_SQL[0] == ' ' || copy_SQL[0] == '\0' || copy_SQL[0] == ';') {
        return true;
    }
    return false;
}

char *get_sep_space(char *sql) {
    if (!sql) return NULL;

    char* copy_SQL = sql;

    while (copy_SQL[0] == ' ') {
        copy_SQL++;
    }

    return copy_SQL;
}

char *get_keyword(char *sql, char *keyword) {
    if (!sql) return NULL;

    unsigned int len_keyword = strlen(keyword);
    char* copy_SQL = sql;

    while (copy_SQL[0] != '\0' && toupper(copy_SQL[0]) == toupper(keyword[0])) {
        copy_SQL++;
        keyword++;
    }

    //On a bien trouvé le mot clé dans le cas ou tous les caracteres du mot clé etaient les mêmes
    if (keyword[0] != '\0') {
        return NULL;
    }

    return get_sep_space(sql + len_keyword);
}

char *get_sep_space_and_char(char *sql, char c) {

    if (!sql) return NULL;

    char* copy_SQL = sql;

    bool separator_found = false;

    while (copy_SQL[0] == ' ' || copy_SQL[0] == c) {

        //Permet de savoir si le carctere de séparation se trouve bien donne une séquence donnée
        if (copy_SQL[0] == c) {
            separator_found = true;
        }
        copy_SQL++;
    }

    if (!separator_found) {
        return NULL;
    }

    return get_sep_space(copy_SQL);
}

char *get_field_name(char *sql, char *field_name) {

    if (!sql) return NULL;
    if (has_reached_sql_end(sql)) return NULL;

    char *copy_SQL = sql;

    // Un champ ne peut pas commencer par une virgule!
    if(copy_SQL[0] == ',') {
        return NULL;
    }

    int i = 0;
    int j = 0;

    if (copy_SQL[0] == '\'') {
        copy_SQL++;
        while (copy_SQL[i] != '\'' && copy_SQL[i] != '\0') {
            i++;
        }
    } else {
        while (copy_SQL[i] != ' ' && copy_SQL[i] != ',' && copy_SQL[i] != ';' && copy_SQL[i] != '=' && copy_SQL[i] != ')' && copy_SQL[i] != '\0') {
            i++;
        }
    }

    while (j < i) {
        field_name[j] = copy_SQL[0];
        copy_SQL++;
        j++;
    }
    field_name[i] ='\0';

    //Si il y a un ' alors il y a deux caracteres en plus qui n'ont pas été copiés dans field_name
    if (sql[0] == '\'') {
        i += 2;
    }

    return get_sep_space(sql + i);
}

char *parse_fields_or_values_list(char *sql, table_record_t *result) {

    if (!sql) return NULL;

    char *copy_SQL = sql;

    char buffer[TEXT_LENGTH] = {0};

    field_record_t field;

    result->fields_count = 0;

    while (!has_reached_sql_end(copy_SQL)) {

        copy_SQL = get_field_name(copy_SQL, buffer);

        strcpy(field.column_name, buffer);
        field.field_type = TYPE_UNKNOWN;

        result->fields[result->fields_count] = field;

        result->fields_count++;


        if (get_sep_space_and_char(copy_SQL, ',') != NULL) {
            copy_SQL = get_sep_space_and_char(copy_SQL, ',');
        } else {
            break;
        }
    }
    return get_sep_space(copy_SQL);
}

char *parse_equality(char *sql, field_record_t *equality) {

    if (!sql) return NULL;

    char *copy_SQL = sql;

    char field[TEXT_LENGTH];
    char value[TEXT_LENGTH];

    copy_SQL = get_field_name(copy_SQL, field);
    copy_SQL = get_sep_space_and_char(copy_SQL, '=');

    //La requete peut etre mal formée, il peut ne pas y avoir le caractere = voire de deuxieme champs
    if (copy_SQL != NULL) {
        copy_SQL = get_field_name(copy_SQL, value);

        //Il y a une erreur de syntaxe, le nom du champ n'est pas valide
        if(copy_SQL == NULL) {
            return NULL;
        }

        strcpy(equality->column_name, field);
        strcpy(equality->field_value.text_value, value);
        equality->field_type = TYPE_UNKNOWN;
    }

    return get_sep_space(copy_SQL);
}

char *parse_set_clause(char *sql, table_record_t *result) {

    field_record_t equality_field;

    char *copy_SQL = sql;

    while (!get_sep_space_and_char(copy_SQL, ',')) {

        copy_SQL = parse_equality(copy_SQL, &equality_field);
        result->fields[result->fields_count] = equality_field;
        result->fields_count++;

        //Verifie quand on est arrivé à la fin de la liste de champs et casse la boucle
        if (get_sep_space_and_char(copy_SQL, ',') != NULL) {
            copy_SQL = get_sep_space_and_char(copy_SQL, ',');
        } else {
            break;
        }
    }

    if (get_keyword(copy_SQL, "WHERE") == NULL && !has_reached_sql_end(copy_SQL)) {
        return NULL;
    }

    return get_sep_space(copy_SQL);
}

char *parse_create_fields_list(char *sql, table_definition_t *result) {

    //);

    if (!sql) return NULL;

    char valid_types[4][12] = {"INT", "PRIMARY KEY", "FLOAT", "TEXT"};

    field_definition_t field;
    result->fields_count = 0;

    char field_name[TEXT_LENGTH] = {0};

    int temp;
    bool correct_type;

    char *copy_SQL = sql;

    if (copy_SQL[0] == '(') {
        copy_SQL++;
    } else {
        return NULL;
    }

    while (!get_sep_space_and_char(sql, ',')) {

        temp = 0;
        correct_type = false;

        copy_SQL = get_field_name(copy_SQL, field_name);
        copy_SQL = get_sep_space(copy_SQL);

        //Cherche un type parmi tous ceux qui existant.
        while (temp < 4 && correct_type == false) {
            if(get_keyword(copy_SQL, valid_types[temp]) == NULL) {
                temp++;
            } else {
                correct_type = true;
                copy_SQL = get_keyword(copy_SQL, valid_types[temp]);
                copy_SQL = get_sep_space(copy_SQL);
            }
        }

        //La variable ne possède pas de type valide, la requete est mal formée
        if (!correct_type) {
            return NULL;
        }

        strcpy(field.column_name, field_name);

        switch (temp) {
            case 0:
                field.column_type = TYPE_INTEGER;
                break;
            case 1:
                field.column_type = TYPE_PRIMARY_KEY;
                break;
            case 2:
                field.column_type = TYPE_FLOAT;
                break;
            case 3:
                field.column_type = TYPE_TEXT;
                break;
            default:
                field.column_type = TYPE_UNKNOWN;
                break;
        }

        result->definitions[result->fields_count] = field;
        result->fields_count++;

        //Verifie quand on est arrivé à la fin de la liste de champs et casse la boucle
        if (get_sep_space_and_char(copy_SQL, ',') != NULL) {
            copy_SQL = get_sep_space_and_char(copy_SQL, ',');
        } else {
            break;
        }
    }


    return get_sep_space_and_char(copy_SQL, ')');
}

char *parse_where_clause(char *sql, filter_t *filter) {

    if (!sql) return NULL;
    char *copy_SQL = sql;

    char field_name[TEXT_LENGTH] = {0};

    bool stop = false;

    operator_t operator;
    field_record_t equality;

    table_record_t records;

    records.fields_count = 0;

    do {
        copy_SQL = get_sep_space(copy_SQL);
        copy_SQL = parse_equality(copy_SQL, &equality);
        copy_SQL = get_sep_space(copy_SQL);

        records.fields[records.fields_count] = equality;
        records.fields_count++;

        if (get_keyword(copy_SQL, "AND") != NULL) {

            copy_SQL = get_keyword(copy_SQL, "AND");
            operator = OP_AND;

        } else if(get_keyword(copy_SQL, "OR") != NULL) {

            copy_SQL = get_keyword(copy_SQL, "OR");
            operator = OP_OR;

        } else {
            stop = true;
        }

    } while (!stop);

    filter->logic_operator = operator;
    filter->values = records;

    return get_sep_space(copy_SQL);
}

query_result_t *parse_create(char *sql, query_result_t *result) {

    create_query_t requete_creer;
    table_definition_t nouvelle_table;
    char nom_table[TEXT_LENGTH] = {0};

    sql = get_sep_space(sql);
    sql = get_keyword(sql, "CREATE TABLE");
    sql = get_field_name(sql, nom_table);
    sql = parse_create_fields_list(sql, &nouvelle_table);

    if (!has_reached_sql_end(sql)) {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    } else {
        result->query_type = QUERY_CREATE_TABLE;
        strcpy(requete_creer.table_name, nom_table);
        requete_creer.table_definition = nouvelle_table;
        result->query_content.create_query = requete_creer;
        printf("%s", "Table analysee avec succes!\n");
    }

    return result;
}

query_result_t *parse_drop_db(char *sql, query_result_t *result) {

    char nom_bdd[TEXT_LENGTH] = {0};
    sql = get_sep_space(sql);
    sql = get_keyword(sql, "DROP DATABASE");
    sql = get_field_name(sql, nom_bdd);

    if (!has_reached_sql_end(sql)) {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    } else {
        result->query_type = QUERY_DROP_DB;
        strcpy(result->query_content.database_name, nom_bdd);
        printf("%s", "Table analysee avec succes!\n");
    }

    return result;
}

query_result_t *parse_drop_table(char *sql, query_result_t *result) {

    char nom_table[TEXT_LENGTH] = {0};
    sql = get_sep_space(sql);
    sql = get_keyword(sql, "DROP TABLE");
    sql = get_field_name(sql, nom_table);

    if (!has_reached_sql_end(sql)) {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    } else {
        result->query_type = QUERY_DROP_TABLE;
        strcpy(result->query_content.table_name, nom_table);
        printf("%s", "Table analysee avec succes!\n");
    }

    return result;
}

query_result_t *parse_delete(char *sql, query_result_t *result) {

    char nom_table[TEXT_LENGTH] = {0};
    filter_t filtre;
    delete_query_t requete_supprimer;

    sql = get_sep_space(sql);
    sql = get_keyword(sql, "DELETE FROM");
    sql = get_field_name(sql, nom_table);
    sql = get_keyword(sql, "WHERE");
    sql = parse_where_clause(sql, &filtre);

    if (!has_reached_sql_end(sql)) {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    } else {
        result->query_type = QUERY_DELETE;
        strcpy(requete_supprimer.table_name, nom_table);

        requete_supprimer.where_clause = filtre;
        result->query_content.delete_query = requete_supprimer;

        printf("%s", "Table analysee avec succes!\n");
    }
    return result;
}

query_result_t *parse_update(char *sql, query_result_t *result) {

    char nom_table[TEXT_LENGTH] = {0};
    update_or_select_query_t requete_update;

    table_record_t champs_set;
    filter_t champs_where;

    result->query_type = QUERY_UPDATE;


    sql = get_sep_space(sql);
    sql = get_keyword(sql, "UPDATE");
    sql = get_field_name(sql, nom_table);
    sql = get_keyword(sql, "SET");
    sql = parse_set_clause(sql, &champs_set);


    //il n'y a pas de clause where
    if (sql && has_reached_sql_end(sql)) {
        requete_update.set_clause = champs_set;

    } else if (sql) { //Il y a une clause where
        sql = get_keyword(sql, "WHERE");
        sql = parse_where_clause(sql, &champs_where);


    } if (sql) {
        requete_update.set_clause = champs_set;
        strcpy(requete_update.table_name, nom_table);
        requete_update.where_clause = champs_where;
        printf("%s", "Table analysee avec succes!\n");
    } else {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    }

    return result;
}

query_result_t *parse_insert(char *sql, query_result_t *result) {

    char nom_table[TEXT_LENGTH] = {0};
    insert_query_t requete_inserer;

    table_record_t noms_champs;
    table_record_t valeurs_champs;

    result->query_type = QUERY_INSERT;


    sql = get_sep_space(sql);
    sql = get_keyword(sql, "INSERT INTO");
    sql = get_field_name(sql, nom_table);
    sql = get_sep_space_and_char(sql, '(');
    sql = parse_fields_or_values_list(sql, &noms_champs);
    sql = get_sep_space_and_char(sql, ')');
    sql = get_keyword(sql, "VALUES");
    sql = get_sep_space_and_char(sql, '(');
    sql = parse_fields_or_values_list(sql, &valeurs_champs);
    sql = get_sep_space_and_char(sql, ')');

    if (sql) {
        strcpy(requete_inserer.table_name, nom_table);
        requete_inserer.fields_names = noms_champs;
        requete_inserer.fields_values = valeurs_champs;
        result->query_content.insert_query = requete_inserer;
        printf("%s", "Table analysee avec succes!\n");
    } else {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    }

    return result;
}

query_result_t *parse_select(char *sql, query_result_t *result) {

    char nom_table[TEXT_LENGTH] = {0};

    update_or_select_query_t requete_selectionner;

    table_record_t champs_liste;
    filter_t filtre;

    sql = get_sep_space(sql);
    sql = get_keyword(sql, "SELECT");
    sql = parse_fields_or_values_list(sql, &champs_liste);
    sql = get_keyword(sql, "FROM");
    sql = get_field_name(sql, nom_table);

    if (sql) {
        sql = get_keyword(sql, "WHERE");
        sql = parse_where_clause(sql, &filtre);
        strcpy(requete_selectionner.table_name, nom_table);
        requete_selectionner.set_clause = champs_liste;
        requete_selectionner.where_clause = filtre;
        result->query_type = QUERY_SELECT;
        result->query_content.update_query = requete_selectionner;
    } else {
        printf("%s", "Erreur de syntaxe dans la requete!\n");
        return NULL;
    }

    return result;
}

query_result_t *parse(char *sql, query_result_t *result) {

    sql = get_sep_space(sql);
    if (get_keyword(sql, "SELECT")) {
        parse_select(sql, result);
    } else if (get_keyword(sql, "INSERT")) {
        parse_insert(sql, result);
    } else if (get_keyword(sql, "UPDATE")) {
        parse_update(sql, result);
    } else if (get_keyword(sql, "DELETE")) {
        parse_delete(sql ,result);
    } else if (get_keyword(sql,"DROP DATABASE")) {
        parse_drop_db(sql, result);
    } else if (get_keyword(sql, "DROP TABLE")) {
        parse_drop_table(sql, result);
    } else if (get_keyword(sql, "CREATE TABLE")) {
        parse_create(sql, result);
    } else {
        result = NULL;
    }
    return result;
}
