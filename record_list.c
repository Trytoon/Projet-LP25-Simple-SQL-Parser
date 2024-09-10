#include "record_list.h"
#include <string.h>

void clear_list(record_list_t *record_list) {
    if (record_list) {
        record_list_node_t *tmp = record_list->head;
        while (tmp) {
            record_list->head = tmp->next;
            free(tmp);
            tmp = record_list->head;
        }
        record_list->head = record_list->tail = NULL;
    }
}

void add_record(record_list_t *record_list, table_record_t *record) {
    if (!record_list) return;

    record_list_node_t *new_node = malloc(sizeof(record_list_node_t));
    memcpy(&new_node->record, record, sizeof (table_record_t));
    new_node->next = NULL;

    if (record_list->head == NULL) {
        record_list->head = record_list->tail = new_node;
        new_node->previous = NULL;
    } else {
        record_list->tail->next = new_node;
        new_node->previous = record_list->tail;
        record_list->tail = new_node;
    }
}

/*!
 * @brief function field_record_length returns a field display length, i.e. the characters count it requires to be
 * displayed (equal to number of digits, signs, and '.' for numbers, number of characters (excluding the trailing '\0')
 * for strings)
 * @param field_record the field value whose display length must be computed
 * @return the display length of the field
 */
int field_record_length(field_record_t *field_record) {

    if (!field_record) return 0;

    char buffer[21] = {0};

    if (field_record->field_type == TYPE_TEXT) {
        return strlen(field_record->field_value.text_value);
    } else if (field_record->field_type == TYPE_INTEGER) {
        sprintf(buffer, "%lld", field_record->field_value.int_value);
    } else if (field_record->field_type == TYPE_PRIMARY_KEY) {
        sprintf(buffer, "%lld", field_record->field_value.primary_key_value);
    } else if (field_record->field_type == TYPE_FLOAT) {
        sprintf(buffer, "%f", field_record->field_value.float_value);
    }
    return strlen(buffer);
}

/*!
 * @brief function display_table_record_list displays a select query result. It consists of 4 steps:
 * - Step 1: go through the whole result and compute the maximum size for each field (use an array of MAX_FIELDS_COUNT
 * integers). Also update max lengths with column_names values.
 * - Step 2: display result header: make a line formatted as +----+----+---+------+ etc. with '+' for columns limits,
 * a number of '-' equal to the maximum size of current field + 2, and so on.
 * - Step 2 bis: display the columns names, right-aligned, separated by char '|' and with a one space left-padding.
 * - Step 2 tes: add the same line as in step 2.
 * - Step 3: for each record in the result, display its values like you did with header column names.
 * Step 2 and 3 require that you add extra space padding to the left of the values for those to be aligned.
 * - Step 4: add a line as in step 2.
 * @param record_list the record list to display
 *
 * For instance, a record list with two fields named 'id' and 'label' and two records (1, 'blah'), and (2, 'foo') will
 * display:
  +----+-------+
  | id | label |
  +----+-------+
  |  1 |  blah |
  |  2 |   foo |
  +----+-------+
 */
void display_table_record_list(record_list_t *record_list) {

    if (!record_list)  {
        printf("\nAucune donnee a traiter !\n");
        return;
    }

    /**
     * Step 1
     */

    int max_sizes[MAX_FIELDS_COUNT] = {0}; //On stocke la taille de chaque colonne
    record_list_node_t *element = record_list->head;

    field_record_t un_champs; //Un champs de la liste de champs donné
    table_record_t un_enregistrement = element->record; //Correspond à un enregistrement de la liste de champs (le premier ici)
    int nb_columns = un_enregistrement.fields_count;
    int size; //La taille de ce champs

    while (element != NULL) {

        for (int i = 0; i < nb_columns; i++) {
            un_champs = element->record.fields[i];
            size = field_record_length(&un_champs);
            if (size > max_sizes[i]) {
                max_sizes[i] = size;
            } else if (strlen(un_champs.column_name) > max_sizes[i]) {
                max_sizes[i] = strlen(un_champs.column_name);
            }
        }

        element = element->next;
    }

    /**
   * Step 2
   */

    printf("+");
    for (int i = 0; i < nb_columns; i++) {
        for (int j = 0; j < max_sizes[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }

    /**
    * Step 2 bis
    * Aligne les valeurs centrées par rapport à la colonne (et pas à droite)
    */

    printf("\n/");

    for (int i = 0; i < nb_columns; i++) {
        int word_length = strlen(un_enregistrement.fields[i].column_name);

        for (int j = 0; j < (max_sizes[i] - word_length)/2 + 1; j++) {
            printf(" ");
        }
        printf("%s", un_enregistrement.fields[i].column_name);
        for (int k = 0; k < (max_sizes[i] - word_length)/2 + 1; k++) {
            printf(" ");
        }

        printf("/");
    }


    /**
    * Step 2 tes
    */
    printf("\n");
    printf("+");
    for (int i = 0; i < nb_columns; i++) {
        for (int j = 0; j < max_sizes[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }

    /**
    * Step 3
    */
    printf("\n");

    //On se replace au debut de la liste pour pouvoir la reparcourir
    element = record_list->head;

    while (element != NULL) {

        un_enregistrement = element->record;

        printf("/");

        for (int i = 0; i < nb_columns; i++) {
            un_champs = un_enregistrement.fields[i];
            int word_length = field_record_length(&un_champs);

            for (int j = 0; j < (max_sizes[i] - word_length) / 2 + 1; j++) {
                printf(" ");
            }

            switch (un_champs.field_type) {

                case TYPE_INTEGER:
                    printf("%lld", un_champs.field_value.int_value);
                    break;
                case TYPE_FLOAT:
                    printf("%f", un_champs.field_value.float_value);
                    break;
                case TYPE_TEXT:
                    printf("%s", un_champs.field_value.text_value);
                default:
                    break;
            }

            for (int j = 0; j < (max_sizes[i] - word_length) / 2 + 1; j++) {
                printf(" ");
            }
        }
        printf("/");
        printf("\n");
        element = element->next;
    }



    /**
    * Step 4
    */
    printf("+");
    for (int i = 0; i < nb_columns; i++) {
        for (int j = 0; j < max_sizes[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }
}