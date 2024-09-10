#include "sql.h"
#include "check.h"
#include "query_exec.h"
#include "expand.h"
#include "database.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#define SQL_COMMAND_MAX_SIZE 1500

int main(int argc, char *argv[]) {

    /**
    * Peut poser des problemes en fonction du nombres d'arguments. Veuillez tester individuellement
    */
    int opt;
    while((opt = getopt(argc, argv, "d:l:")) != -1) {
        switch (opt) {
            case 'd':
                printf("%s", optarg);
                if (directory_exists(optarg)) {
                    chdir(optarg);
                } else {
                    mkdir(optarg, S_IRWXU);
                }
                break;
            case 'l':
                chdir(optarg);
                break;
            case '?':
                printf("Il manque des arguments!");
                break;
        }
    }

    char sql[SQL_COMMAND_MAX_SIZE];
    do {
        printf("> ");
        fflush(stdin);
        if (fgets(sql, SQL_COMMAND_MAX_SIZE, stdin) == NULL)
            continue;
        sql[strlen(sql)-1] = '\0';
        if (strcmp(sql, "exit") == 0)
            break;



        /**
         * Requete fonctionnant entierement
         */
        //char *sql = "CREATE TABLE Etudiant (Nom TEXT, Prenom TEXT, Age INT);";
        //char *sql = "INSERT INTO Etudiant (Nom, Prenom, Age) VALUES (Jean, Michel, 45);";
        //char *sql = "DROP TABLE Etudiant;"; Probleme dans recursive_rmdir();
        //char *sql = "SELECT * FROM Etudiant"; Probleme avec la clause where, le filtre n'est pas pris en compte

        /**
         * Requete dont l'execution n'est pas finie / testée.
         * Cependant, la partie parse / check / expand fonctionne.
         * Execution impossible car necessite de recuperer une liste d'enregistrements, chose que l'on a pas réussie
         */
        //char *sql = "UPDATE ETUDIANT SET Age = 19 WHERE Age = 18;
        //char *sql = "DELETE FROM Etudiant";

        bool can_be_executed = false;

        query_result_t result;
        parse(sql, &result);

        can_be_executed = check_query(&result);

        if(can_be_executed) {
            if (result.query_type == QUERY_SELECT || result.query_type == QUERY_INSERT) {
                expand(&result);
            }

            execute(&result);
        }
    } while (true);
    return(0);

}

