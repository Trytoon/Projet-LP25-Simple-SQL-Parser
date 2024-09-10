#include "table.h"

#include <sys/stat.h>
#include <unistd.h>

/*!
 * @brief function open_definition_file opens the table definition file
 * @param table_name the name of the table whose definition file must be opened
 * @param mode the opening mode (as in fopen)
 * @return and pointer to a FILE type, resulting from the fopen function
 */
FILE *open_definition_file(char *table_name, char *mode) {

    char buffer[1000] = {0};
    sprintf(buffer, "%s", make_full_path("./", table_name));
    if (directory_exists(buffer) == true) {
        sprintf(buffer, "%s/%s.def", make_full_path("./", table_name), table_name);
        FILE *f = fopen(buffer, mode);
        if (f) {
            return f;
        }
    }
    return NULL;
}
/*!
 * @brief function open_index_file opens the table key file
 * @param table_name the name of the table whose index file must be opened
 * @param mode the opening mode (as in fopen)
 * @return and pointer to a FILE type, resulting from the fopen function
 */
FILE *open_index_file(char *table_name, char *mode) {

    char buffer[1000] = {0};
    sprintf(buffer, "%s", make_full_path("./", table_name));
    if (directory_exists(buffer) == true) {
        sprintf(buffer, "%s/%s.idx", make_full_path("./", table_name), table_name);
        FILE *f = fopen(buffer, mode);
        if (f) {
            return f;
        }
    }
    return NULL;
}

/*!
 * @brief function open_content_file opens the table key file
 * @param table_name the name of the table whose content file must be opened
 * @param mode the opening mode (as in fopen)
 * @return and pointer to a FILE type, resulting from the fopen function
 */
FILE *open_content_file(char *table_name, char *mode) {
    char buffer[1000] = {0};
    sprintf(buffer, "%s", make_full_path("./", table_name));
    if (directory_exists(buffer) == true) {
        sprintf(buffer, "%s/%s.data", make_full_path("./", table_name), table_name);
        FILE *f = fopen(buffer, mode);
        if (f) {
            return f;
        }
    }
    return NULL;
}

/*!
 * @brief function open_key_file opens the table key file
 * @param table_name the name of the table whose key file must be opened
 * @param mode the opening mode (as in fopen)
 * @return and pointer to a FILE type, resulting from the fopen function
 */
FILE *open_key_file(char *table_name, char *mode) {
    char buffer[1000] = {0};
    sprintf(buffer, "%s", make_full_path("./", table_name));
    if (directory_exists(buffer) == true) {
        sprintf(buffer, "%s/%s.key", make_full_path("./", table_name), table_name);
        FILE *f = fopen(buffer, mode);
        if (f) {
            return f;
        }
    }
    return NULL;
}

/*!
 * @brief function create_table creates the files for a new table. It checks if the table doesn't already exist,
 * creates its directory, its definition file, whose content is written based on the definition. It creates both
 * index and content empty files, and if there is a primary key, it creates the primary key file with a stored value
 * of 1.
 * @param table_definition a pointer to the definition of the new table
 */
void create_table(create_query_t *table_definition) {


    if (!table_definition) return;

    FILE *def;
    FILE *idx;
    FILE *key;
    FILE *data;
    char table_path[1000];

    int field_type;
    char *field_name;

    char buffer[1000];
    char path[] = "./";
    char *nom = table_definition->table_name;

    unsigned long long primary_key = 1;

    strcpy(table_path, make_full_path(path, nom));

    if (directory_exists(table_path) == true) {
        printf("Impossible de creer la table, elle existe deja !");
    } else {

        mkdir(table_path, S_IRWXU);

        //On va creer la chaines de caracteres corresondant au fichier possedant le meme nom que la table et son extension data
        sprintf(buffer, "%s/%s.data", table_path, nom);

        data = fopen(buffer, "w");
        fclose(data);

        //On va creer la chaines de caracteres corresondant au fichier possedant le meme nom que la table et son extension idx
        sprintf(buffer, "%s/%s.idx", table_path, nom);
        idx = fopen(buffer, "w");
        fclose(idx);

        //On va creer la chaines de caracteres corresondant au fichier possedant le meme nom que la table et son extension
        sprintf(buffer, "%s/%s.def", table_path, nom);
        def = fopen(buffer, "w");

        for (int i = 0; i < table_definition->table_definition.fields_count; i++) {
            if (table_definition->table_definition.definitions[i].column_type == TYPE_PRIMARY_KEY) {
                sprintf(buffer, "%s/%s.key", table_path, table_definition->table_name);
                key = fopen(buffer, "w");
                fwrite("1" , 1 , sizeof(char) , key);
                fclose(key);
            } else {
                // On creer une chaine de caractere du type "X Nom_Du_Champ"
                field_type = table_definition->table_definition.definitions[i].column_type;
                field_name = table_definition->table_definition.definitions[i].column_name;
                sprintf(buffer, "%d %s\n", field_type, field_name);
                fprintf(def, buffer);
            }
        }
        fclose(def);
    }
}

/*!
 * @brief function get_table_definition reads and returns a table definition
 * @param table_name the name of the target table
 * @param result a pointer to a previously allocated table_definition_t structure where the result will be stored.
 * @return the pointer to result, NULL if the function failed
 */
table_definition_t *get_table_definition(char *table_name, table_definition_t *result) {

    if (!result) return NULL;

    result->fields_count = 0;

    char buffer[TEXT_LENGTH + 3];

    FILE *f = open_definition_file(table_name, "r");

    if (f) {
        while (fgets(buffer, TEXT_LENGTH + 3, f)) {
            result->definitions[result->fields_count].column_type = atoi(&buffer[0]);
            memcpy(result->definitions[result->fields_count].column_name, &buffer[2], TEXT_LENGTH);
            result->definitions[result->fields_count].column_name[strlen(result->definitions[result->fields_count].column_name) - 1] = '\0';
            result->fields_count++;
        }

        return result;
    }

    return NULL;
}

/*!
 * @brief function compute_record_length computed the in-memory length of a table record. It sums all fields sizes
 * by their respective sizes. Text is always stored on TEXT_LENGTH chars so a text field in-memory size is always
 * TEXT_LENGTH, whatever its actual content strlen.
 * @param definition the table definition whose record length we need
 * @return the value of the record length.
 */
uint16_t compute_record_length(table_definition_t *definition) {
    uint16_t length = 0;
    for (int i = 0; i < definition->fields_count; i++) {
        switch (definition->definitions[i].column_type) {
            case TYPE_PRIMARY_KEY:
                length += sizeof (unsigned long long);
                break;
            case TYPE_INTEGER:
                length += sizeof (long long);
                break;
            case TYPE_FLOAT:
                length += sizeof (double);
                break;
            case TYPE_TEXT:
                length += TEXT_LENGTH;
                break;
            default:
                break;
        }
    }
    return length;
}

/*!
 * @brief function find_first_free_record finds the first free record in the index file. The first free record is:
 * - the first index_record_t occurrence in the index file whose active field is 0. Its active field must be put to 1
 * before ending the function (replace value inplace)
 * - if none exists: a new field_record_t added to the end of the file, with active set to 1, and others fields to 0.
 * @param table_name the name of the target table
 * @return the offset of the free index in the index file.
 */
uint32_t find_first_free_record(char *table_name) {
    uint8_t active = 1;
    uint32_t offset = 0; //offset dans le fichier de données
    uint16_t length = 0;

    int place_to_write = 0; //decalage en bits à partir du debut du fichier d'index

    uint32_t zero_32 = 0;
    uint16_t zero_16 = 0;

    FILE *index = open_index_file(table_name, "r+");

    // On lit tant qu'on peut trouver une valeur "active" dans le fichier (une nouvelle ligne)
    while (fread(&active, sizeof(uint8_t), 1, index) == 1 && active == 1) {
        fread(&offset, sizeof(uint32_t), 1, index);
        fread(&length, sizeof(uint16_t), 1, index);
        place_to_write += (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t));
    }

    fseek(index, place_to_write, SEEK_SET);

    if (active == 0) {
        active = 1;
        fwrite(&active, sizeof(uint8_t), 1, index);
        fread(&offset, sizeof(uint32_t), 1, index);
        fclose(index);
        return offset;
    } else if (active == 1) {
        fwrite(&active, sizeof(uint8_t), 1, index);
        fwrite(&zero_32, sizeof(uint32_t), 1, index);
        fwrite(&zero_16, sizeof(uint16_t), 1, index);
        fclose(index);
        return offset + length;
    }
    return -1;
}


/*!
 * @brief function update_key updates the key value in the key file. Key value is updated if and only if the new value
 * if higher than the stored value. The value sent to the function is the last value inserted into the table, so the
 * function must increment it before comparing to the key file content.
 * @param table_name the name of the table whose key file must be updated
 * @param value the new key value
 */
void update_key(char *table_name, unsigned long long value) {

    value++;
    FILE *f = open_key_file(table_name, "r");
    char buffer[21] = {0};
    unsigned long long key_value = 0;


    if (f) {
        fgets( buffer, sizeof (unsigned long long), f); //Il n'y a qu'une seule ligne
        key_value = strtoull(buffer, NULL, 10);

        if (value > key_value) {
            fwrite(&value, 1, sizeof (unsigned long long), f);
        }

        fclose(f);
    }
}

/*!
 * @brief function get_next_key extract the next key value from a table key file.
 * @param table_name the name of the table whose key value we need.
 * @return the next value of the key is it exists, 0 else
 */
unsigned long long get_next_key(char *table_name) {
    FILE *f = open_key_file(table_name, "r");
    char buffer[21] = {0};
    unsigned long long key_value = 0;


    if (f) {
        fgets( buffer, sizeof (unsigned long long), f); //Il n'y a qu'une seule ligne
        key_value = strtoull(buffer, NULL, 10);
        fclose(f);
    } else {
        return 0;
    }

    return key_value++;
}

/*!
 * @brief function add_row_to_table adds a row to a table. Relies on format_row, compute_record_length,
 * find_first_free_record, get_table_definition, open_content_file, open_index_file
 * @param table_name the name of the target table
 * @param record the record to add
 */
void add_row_to_table(char *table_name, table_record_t *record) {

    if (!table_name || !record) return;

    char *buffer;
    uint8_t active = 1;

    FILE *data = open_content_file(table_name, "r+");
    FILE *index = open_index_file(table_name, "r+");

    u_int32_t data_offset;
    unsigned int index_offset = 0;

    //Construire le buffer à écrire
    table_definition_t table;
    get_table_definition(table_name, &table);
    buffer = format_row(table_name, buffer, &table, record);


    //Chercher un index de libre
    data_offset = find_first_free_record(table_name);
    uint16_t record_length = compute_record_length(&table);

    //Modifier le fichier d'index avec l'index courant
    if (record_length != 0) {
        index_offset = (data_offset/record_length) * (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t));
    }
    fseek(index, index_offset, SEEK_SET);
    fwrite(&active, sizeof(uint8_t), 1, index);
    fwrite(&data_offset, sizeof(uint32_t), 1, index);
    fwrite(&record_length, sizeof(uint16_t), 1, index);

    //Il suffit d'ecrire le buffer binaire au bon emplacement dans le fichier de données
    fseek(data, data_offset, SEEK_SET);
    fwrite(buffer, record_length, 1, data);

    fclose(data);
    fclose(index);
    free(buffer);
}

/*!
 * @brief function get_filtered_records gets a list (as a linked list) of table records limited to the fields
 * specified to the function and filtered by the specified WHERE clause.
 * @param table_name table from which to fetch data
 * @param required_fields the list of fields to be returned (cannot be NULL)
 * @param filter the WHERE clause filter. NULL if no filter must be applied
 * @param result a pointer to a previously allocated record_list_t. List shall be empty
 * @return a pointer to the first element of the resulting linked list. Shall be freed by the calling function
 */
record_list_t *get_filtered_records(char *table_name, table_record_t *required_fields, filter_t *filter, record_list_t *result) {
    clear_list(result);

    FILE *idx = open_index_file(table_name, "r+");

    if (idx){
        index_record_t index_record;
        table_record_t record;
        table_record_t result_record;

        table_definition_t table;
        get_table_definition(table_name, &table);

        while (fread(&index_record.is_active, sizeof(uint8_t), 1, idx) == 1) {
            if (index_record.is_active) {
                fread(&index_record.record_offset, sizeof(uint32_t), 1, idx);
                record = *get_table_record(table_name, index_record.record_offset, &table, &record);

                if (is_matching_filter(&record, filter)) {

                    for (int num_required_field = 0;
                         num_required_field < required_fields->fields_count; ++num_required_field) {
                        result_record.fields[num_required_field] = *(find_field_in_table_record(required_fields->fields[num_required_field].column_name, &record));
                        result_record.fields_count = num_required_field;
                    }
                    result_record.fields_count++;
                    add_record(result, &result_record);
                }
                fseek(idx, 3, SEEK_CUR);
            } else {
                fseek(idx, 7, SEEK_CUR);
            }
        }

        fclose(idx);
    }

    return result;
}

/*!
 * @brief function format_row prepares the buffer to be written to the content file
 * @param table_name the name of the target table
 * @param buffer the buffer where the row content is prepared
 * @param table_definition the table definition for fields ordering and formatting
 * @param record the record to write
 * @return a pointer to buffer in case of success, NULL else.
 */
char *format_row(char *table_name, char *buffer, table_definition_t *table_definition, table_record_t *record) {

    if (!table_definition || !table_name) return NULL;
    get_table_definition(table_name, table_definition);

    //On cree un buffer dynamique, on ne connais à priori pas d'avance la taille occupée en memeoire par un enregistrement
    uint16_t len = compute_record_length(table_definition);

    buffer = malloc(len + 1);
    field_record_t champ_a_ecrire;

    int offset = 0; //Une chaine de caracteres de 150 octets peut contenir '\0'. On doit donc savoir à partir de ou modifier la chaine de caratceres

    for (int i = 0;  i < record->fields_count; i++) {
        champ_a_ecrire = *find_field_in_table_record(record->fields[i].column_name, record);

        switch (champ_a_ecrire.field_type) {

            case TYPE_INTEGER:
                memcpy(buffer + offset, (char*) &champ_a_ecrire.field_value.int_value, sizeof(long long));
                offset += sizeof (long long);
                break;
            case TYPE_FLOAT:
                memcpy(buffer + offset, (char*) &champ_a_ecrire.field_value.float_value, sizeof(double));
                offset += sizeof (double);
                break;
            case TYPE_TEXT:
                memcpy(buffer + offset, (char*) &champ_a_ecrire.field_value.text_value, TEXT_LENGTH);
                offset += TEXT_LENGTH;
                break;
            case TYPE_PRIMARY_KEY:
                //Le buffer dans le fichier de contenu n'est pas modifié. Attention cependant à ne pas oublier le fichier de clé
                update_key(table_name, get_next_key(table_name));
                break;
            default:
                break;
        }
    }

    return buffer;
}


/*!
 * @brief function find_field_in_table_record looks up for a field by its name in a table_record_t structure
 * @param field_name the name of the field to look for
 * @param record a pointer to the table_record_t structure
 * @return a pointer to the matching field_record_t if found, NULL else
 */
field_record_t *find_field_in_table_record(char *field_name, table_record_t *record) {

    if (!record) return NULL;

    if(field_name[strlen(field_name) - 1] == '\n') {
        field_name[strlen((field_name)) - 1] = '\0';
    }

    int i = 0;
    bool trouve = false;

    while (i < record->fields_count && trouve == false) {
        if(strcmp(field_name, record->fields[i].column_name) == 0) {
            trouve = true;
        }
        i++;
    }

    if (trouve) {
        return &record->fields[i-1];
    }

    return NULL; // Checking the query shall avoid this
}

/*!
 * @brief function is_matching_filter tests a table record against a filter to check if the record matches the filter,
 * i.e. conditions in the filter are verified by the record. The filter contains one or more equalities to check.
 * Tests are all following a single AND or OR logical operator.
 * @param record the record to be checked
 * @param filter the filter to check against (a NULL filter means no filtering must be done -> returns 0)
 * @return true if the record matches the filter, false else
 */
bool is_matching_filter(table_record_t *record, filter_t *filter) {

    if (!filter) return false;

    field_record_t *a_field_in_records;

    bool une_egalite_dans_filtre_ou = false;

    for (int i = 0; i < filter->values.fields_count && une_egalite_dans_filtre_ou == false; i++) {
        // On recupere le champ et sa valeur defini dans la liste de champs. Valeur qu'on compare au filtre
        a_field_in_records = find_field_in_table_record(filter->values.fields[i].column_name, record);


        switch (filter->values.fields[i].field_type) {

            case TYPE_INTEGER:
                if (filter->logic_operator == OP_AND) {
                    if (a_field_in_records->field_value.int_value != filter->values.fields[i].field_value.int_value) {
                        return false;
                    } else {
                        if (a_field_in_records->field_value.int_value == filter->values.fields[i].field_value.int_value) {
                            une_egalite_dans_filtre_ou = true;
                        }
                    }
                }
                break;
            case TYPE_FLOAT:
                if (filter->logic_operator == OP_AND) {
                    if (a_field_in_records->field_value.float_value != filter->values.fields[i].field_value.float_value) {
                        return false;
                    } else {
                        if (a_field_in_records->field_value.float_value == filter->values.fields[i].field_value.float_value) {
                            une_egalite_dans_filtre_ou = true;
                        }
                    }
                }
                break;
            case TYPE_TEXT:

                if (filter->logic_operator == OP_AND) {
                    if (strcmp(a_field_in_records->field_value.text_value, filter->values.fields[i].field_value.text_value) != 0) {
                        return false;
                    } else {
                        if (strcmp(a_field_in_records->field_value.text_value, filter->values.fields[i].field_value.text_value) == 0) {
                            une_egalite_dans_filtre_ou = true;
                        }
                    }
                }

                break;
            default:
                break;

        }
    }

    if (filter->logic_operator == OP_AND) {
        return une_egalite_dans_filtre_ou;
    }

        return true;
}



/*!
 * @brief function get_table_record reads a table record from its content file
 * @param table_name the table whose record must be read from
 * @param offset the position of the beginning of the record in the content file (found in the index file)
 * @param def the table definition
 * @param result a pointer to a previously allocated table_record_t structure
 * @return the pointer to to result if succeeded, NULL else.
 */
table_record_t *get_table_record(char *table_name, uint32_t offset, table_definition_t *def, table_record_t *result) {

    get_table_definition(table_name, def);

    //On se place dans le bon endroit pour la lecture dans le fichier de conetenu
    FILE *contenu = open_content_file(table_name, "r");
    fseek(contenu, offset, SEEK_SET);

    //Champ N du tableau de  champs result
    field_record_t champs_actuel;

    char buffer[TEXT_LENGTH];
    long long an_integer_value; //Pour stocker une valeur numérique issue de la lecture
    double a_double_value; //Pour stocker une valeur double issue de la lecture

    if (contenu) {
        for (int i = 0; i < def->fields_count; i++) {
            strcpy(champs_actuel.column_name, def->definitions[i].column_name);

            switch (def->definitions[i].column_type) {

                case TYPE_INTEGER:
                    fread(&an_integer_value, sizeof(long long), 1, contenu);
                    champs_actuel.field_value.int_value = an_integer_value;
                    champs_actuel.field_type = TYPE_INTEGER;
                    break;
                case TYPE_FLOAT:
                    fread(&a_double_value, sizeof(double), 1, contenu);
                    champs_actuel.field_value.float_value = a_double_value;
                    champs_actuel.field_type = TYPE_FLOAT;
                    break;
                case TYPE_TEXT:
                    fread(buffer, TEXT_LENGTH, 1, contenu);
                    strcpy(champs_actuel.field_value.text_value, buffer);
                    champs_actuel.field_type = TYPE_TEXT;
                    break;
                default:
                    break;
            }

            result->fields[i] = champs_actuel;
            result->fields_count++;
        }
    } else {
        result = NULL;
    }

    return result;
}
