/* sas_support.h 
 * Judson Cooper
 * MP3
 *
 * Purpose: A template for MP3
 *
 * Assumptions: structure definitions and public functions as defined for
 * assignment.  
 *
 * Bugs:
 */

#define MAXLINE 180

// prototype function definitions 

// function to compare secondary user records 
int sas_compare(const su_info_t *rec_a, const su_info_t *rec_b);

// functions to create and cleanup a list 
ListPtr sas_create(ListPtr, const char *);
void sas_cleanup(ListPtr);

void sas_print(ListPtr list_ptr, char *);      // print list of records 
void sas_stats(ListPtr, int, ListPtr);
int sas_invalid_ch(int);

void sas_add(ListPtr, int, ListPtr);
void sas_lookup(ListPtr , int);
void sas_remove(ListPtr, ListPtr, int);
void sas_move(ListPtr, int, ListPtr);
void sas_change(ListPtr, int, int);
void sas_assign(ListPtr, int, ListPtr, int);

// new functions for lab 3
void sas_sort(ListPtr, int);
void sas_addtail(ListPtr, int);

