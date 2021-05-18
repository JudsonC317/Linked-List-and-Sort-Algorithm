/* lab3.c
 * Judson Cooper
 * MP3
 *
 * Purpose: This file contains the main() function for the SAS application.
 *		It provides the commands that are available to the users and handles the
 *		input and output for the application. Users are able to perform actions such
 *		as adding, deleting, and editing items in the list.
 *
 * Assumptions: This file is simple.  Input is collected and the
 *		appropriate SAS function is called. It does not rely on the specific
 *		details of the type of list being implemented.
 *
 * Bugs: None known.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "datatypes.h"
#include "list.h"
#include "sas_support.h"

int main(int argc, char * argv[])
{
    ListPtr assigned_list;
    ListPtr waiting_list;
    char line[MAXLINE];
    char command[MAXLINE];
    char junk[MAXLINE];
    int num_items;
    int sorted_size = -1;
    int input_1 = -1; 
    int input_2 = -1;

    if (argc != 1) {
        printf("Usage: ./lab2\n");
        exit(1);
    }
    printf("Welcome to lab2. Possible commands:\n");
    printf("CREATE x; ADDSU; DELSU x\nLISTCH x; CLEARCH x; ");
    printf("CHANGECH old new; ASSIGNSU x\nPRINTASSIGNED; PRINTWAITING\n");
    printf("STATS; SORT x; ADDTAIL su_id; QUIT\n");

    // this list should be sorted and the size of the list is limited
    assigned_list = NULL;

    // this list is unsorted and the list size is not limited
    waiting_list = NULL;

    // main loop to collect user input, call appropriate function for each command
    // remember fgets includes newline \n unless line too long
    while (fgets(line, MAXLINE, stdin) != NULL) {
        num_items = sscanf(line, "%s%d%d%s", command, &input_1, &input_2, junk);
        if (num_items == 2 && strcmp(command, "CREATE") == 0) {
            sorted_size = input_1;
            if (sorted_size < 1) {
                printf("Cannot create list: invalid size %d\n", sorted_size);
            } else {
                // clean up old list if necessary and create a new empty one
                printf("Assignment list size limit: %d\n", sorted_size);
                assigned_list = sas_create(assigned_list, "assignment list");
                waiting_list = sas_create(waiting_list, "waiting queue");
            }
        } else if (num_items == 1 && strcmp(command, "ADDSU") == 0) {
            sas_add(assigned_list, sorted_size, waiting_list);
        } else if (num_items == 2 && strcmp(command, "LISTCH") == 0) {
            if (sas_invalid_ch(input_1)) {
                printf("Invalid channel number %d for LISTCH\n", input_1);
            } else {
                sas_lookup(assigned_list, input_1);
            }
        } else if (num_items == 2 && strcmp(command, "DELSU") == 0) {
            sas_remove(assigned_list, waiting_list, input_1);
        } else if (num_items == 2 && strcmp(command, "CLEARCH") == 0) {
            if (sas_invalid_ch(input_1)) {
                printf("Invalid channel number %d for CLEARCH\n", input_1);
            } else {
                sas_move(assigned_list, input_1, waiting_list);
            }
        } else if (num_items == 3 && strcmp(command, "CHANGECH") == 0) {
            if (sas_invalid_ch(input_1) || sas_invalid_ch(input_2)) {
                printf("Invalid old %d or new %d channel for CHANGECH\n", 
                        input_1, input_2);
            } else {
                sas_change(assigned_list, input_1, input_2);
            }
        } else if (num_items == 2 && strcmp(command, "ASSIGNSU") == 0) {
            if (sas_invalid_ch(input_1)) {
                printf("Invalid channel number %d for ASSIGNSU\n", input_1);
            } else {
                sas_assign(assigned_list, sorted_size, waiting_list, input_1);
            }
        } else if (num_items == 1 && strcmp(command, "PRINTASSIGNED") == 0) {
            sas_print(assigned_list, "Assigned List");
        } else if (num_items == 1 && strcmp(command, "PRINTWAITING") == 0) {
            sas_print(waiting_list, "Waiting Queue");
        } else if (num_items == 1 && strcmp(command, "STATS") == 0) {
            sas_stats(assigned_list, sorted_size, waiting_list);
        } else if (num_items == 2 && strcmp(command, "SORT") == 0) {
			  sas_sort(waiting_list, input_1);
        } else if (num_items == 2 && strcmp(command, "ADDTAIL") == 0) {
			  sas_addtail(waiting_list, input_1);
        } else if (num_items == 1 && strcmp(command, "QUIT") == 0) {
			  sas_cleanup(assigned_list);
			  sas_cleanup(waiting_list);
            printf("Goodbye\n");
            break;
        } else {
            printf("# %s", line);
        }
    }
    exit(0);
}
