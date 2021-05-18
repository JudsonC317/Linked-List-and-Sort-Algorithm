/* sas_support.c 
 * Judson Cooper
 * MP3
 *
 * Purpose: This file provides the functions that handle interaction
 *		with the SAS records. It uses the functions provided in the list.c
 *		file to accomplish this.
 *
 * Assumptions: The members of the list_t and list_node_t structures cannot
 *		be directly interacted with in this file. The code in sas_support.c
 *		should be independent of the type of list being implemented. This file
 *		implements the interface defined in sas_support.h.
 *
 * Bugs: None known.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "datatypes.h"
#include "list.h"
#include "sas_support.h"
 
// Private functions used in this file only.  Do not include prototype
// in the header file
void sas_record_fill(su_info_t *rec);   // collect input from user 
void sas_record_print(su_info_t *rec);  // print one record 

/* sas_compare is required by the list ADT for sorted lists. 
 *
 * This function returns 
 *     1 if rec_a should be closer to the head than rec_b,
 *    -1 if rec_b is to be considered closer to the head, and 
 *     0 if the records are equal.
 *
 * For the secondary user data we want to sort from lowest ID up, so
 * closer to the head means a smaller ID.
 *
 * The function expects pointers to two record structures, and it is an error
 * if either is NULL
 */
int sas_compare(const su_info_t *record_a, const su_info_t *record_b)
{
    assert(record_a != NULL && record_b !=NULL);
    if (record_a->su_id < record_b->su_id)
        return 1;
    else if (record_a->su_id > record_b->su_id)
        return -1;
    else
        return 0;
}


/* print the secondary user record list 
 *
 * This function provides an example on how to use the
 * Iterator pointers.
 */
void sas_print(ListPtr list_ptr, char *type_of_list)
{
	// NULL list will cause assert to fail in list functions called below
	if (list_ptr == NULL) {
		return;
	}
    assert(strcmp(type_of_list, "Assigned List")==0 
            || strcmp(type_of_list, "Waiting Queue")==0);
    IteratorPtr index;
    int num_in_list = list_size(list_ptr);
    int counter = 0;

    if (num_in_list == 0) {
        printf("%s empty\n", type_of_list);
    } else {
        printf("%s has %d records\n", type_of_list, num_in_list);
        index = list_iter_front(list_ptr);
        while (index != NULL)
        {
            printf("%d: ", ++counter);
            sas_record_print(list_access(list_ptr, index));
            index = list_iter_next(index);
        }
        assert(num_in_list == counter);
    }
    printf("\n");
}

/* This creates a list for storing secondary user records.  Based on
 * the type, the list can either store items in sorted or unsorted order.
 *
 * The only difference is if a comparison function pointer is installed
 * in the list header block.  Unsorted lists do not need a comparison
 * function, so the pointer is null.
 *
 * Notice this function has a list point as input.  If the pointer is
 * null, then this is the first time a list is created.  If it is not
 * null, then the old list should be destructed and a pointer to the
 * new list is returned.  This interface is similar to how the 
 * recalloc() function works.
 */
ListPtr sas_create(ListPtr list_ptr, const char *type)
{
    // function to describe how the data will be compared for sorting
    int (*comp_function) (const su_info_t *, const su_info_t *);
    comp_function = sas_compare;

    if (list_ptr != NULL) {
         printf("Replacing existing %s\n", type);
		  list_destruct(list_ptr);
    } else {
         printf("New %s\n", type);
    }
    return list_construct(comp_function);
}

/* This function adds the secondary record to one of the lists.  
 *
 * If the secondary user ID is found in the assigned list, then two cases 
 * are possible.  
 *    If the channel ID is the same, then simply update all the other 
 *      information for the secondary user.  
 *    If the channel ID is different, then remove the secondary user 
 *      from the assigned list and place it at the tail of the waiting 
 *      queue (and update all the secondary information).  
 * If the secondary user is not in the assigned list, then determine 
 *      if the secondary user is in the waiting queue, and if so update 
 *      the secondary user information.  
 * If the secondary user is not found in either the assigned list 
 *      or waiting queue, then add the secondary user to the tail of 
 *      the waiting queue
 */
void sas_add(ListPtr assn_ptr, int size, ListPtr wait_ptr)
{
    int add_action = -2;
    su_info_t *rec_ptr;
    rec_ptr = (su_info_t *) calloc(1, sizeof(su_info_t));
    sas_record_fill(rec_ptr);

	// don't let it add something to null list
	 if ((assn_ptr == NULL) || (wait_ptr == NULL)) {
		 free(rec_ptr);
		 rec_ptr = NULL;
		 return;
	 }
 
    // figure out how to add and set add_action
	 IteratorPtr match = list_elem_find(assn_ptr, rec_ptr);
	 if (match != NULL) { // found in assigned list
		 if (rec_ptr->channel == list_access(assn_ptr, match)->channel) { // same channel
			 // remove old entry, put in new for update
			 free(list_remove(assn_ptr, match));
			 match = NULL;
			 list_insert_sorted(assn_ptr, rec_ptr);
			 add_action = 0;
		 }
		 else { // different channel
			 // remove old entry, put new into waiting
			 free(list_remove(assn_ptr, match));
			 match = NULL;
			 list_insert(wait_ptr, rec_ptr, NULL);
			 add_action = 1;
		 }
	 }
	 else {
		 match = list_elem_find(wait_ptr, rec_ptr); 
		 if (match != NULL) { // found in waiting list
			 // update all info
			 su_info_t *foundData = list_access(wait_ptr, match);
			 assert(rec_ptr->su_id == foundData->su_id);
			 foundData->ip_address = rec_ptr->ip_address;
			 foundData->access_point = rec_ptr->access_point;
			 foundData->authenticated = rec_ptr->authenticated;
			 foundData->privacy = rec_ptr->privacy;
			 foundData->band = rec_ptr->band;
			 foundData->channel = rec_ptr->channel;
			 foundData->data_rate = rec_ptr->data_rate;
			 foundData->time_received = rec_ptr->time_received;
			 // free(rec_ptr);
			 // rec_ptr = NULL;
			 add_action = 2;
		 }
		 else { // completely new entry		 	
			 list_insert(wait_ptr, rec_ptr, NULL);
			 add_action = 3;
		 }
	 }

    if (add_action == 3) {
        printf("Inserted new waiting SU: %d\n", rec_ptr->su_id);
    } else if (add_action == 0) {
        printf("Updated assigned SU: %d\n", rec_ptr->su_id);
    } else if (add_action == 1) {
        printf("Moved assigned SU to waiting: %d\n", rec_ptr->su_id);
    } else if (add_action == 2) {
        printf("Updated waiting SU: %d\n", rec_ptr->su_id);
		  free(rec_ptr);
    } else {
        printf("Error with return value!\n");
    }
	 match = NULL;
    rec_ptr = NULL;
}

/* This function prints the records with the matching channel number in the
 * assigned sorted list.  
 */
void sas_lookup(ListPtr list_ptr, int channel_no)
{
	// NULL list will cause assert to fail in list functions used below
	if (list_ptr == NULL) {
		return;
	}
    su_info_t *rec_ptr = NULL;
    int i, num_in_list = list_size(list_ptr);  
	 IteratorPtr rover = NULL;

    if (num_in_list == 0) {
        printf("List is empty: no users on ch %d\n", channel_no);
    } else {
        printf("Assignment list has %d records.  Looking for SUs on ch %d\n", 
                num_in_list, channel_no);
        // print record of each user on channel
        rover = list_iter_front(list_ptr);
        for (i = 0; i < num_in_list; i++) {
				rec_ptr = list_access(list_ptr, rover);
				if (rec_ptr->channel == channel_no) {
					sas_record_print(rec_ptr);
				}
				rover = list_iter_next(rover);
        }
    }
	 rover = NULL;
    rec_ptr = NULL;
}

/* This function removes the record from either the sorted assigned list 
 * or the waiting queue.  There can only be one match.
 */
void sas_remove(ListPtr assn_list, ListPtr wait_q, int su_id)
{
    su_info_t *rec_ptr = NULL;
	 IteratorPtr match = NULL;
    int assigned_or_waiting = -1;

    // use list_elem_find and list_remove

	 // make sure list exists, NULL list will cause list validate to fail in list functions
	 if ((assn_list == NULL) || (wait_q == NULL)) {
		 return;
	 }

	 // create a template su_info_t struct to look for
    rec_ptr = (su_info_t*) malloc(sizeof(su_info_t));
	 rec_ptr->su_id = su_id;

	 match = list_elem_find(assn_list, rec_ptr);
	 if (match != NULL) { // found in assigned list
		 free(rec_ptr); // so template entry doesn't result in mem leak
		 rec_ptr = list_remove(assn_list, match);
		 assigned_or_waiting = 0;
	 }
	 else {
		 match = list_elem_find(wait_q, rec_ptr);
		 if (match != NULL) { // found in wait list
			 free(rec_ptr); // so template entry doesn't result in mem leak
			 rec_ptr = list_remove(wait_q, match);
			 assigned_or_waiting = 1;
		 }
		 else { // found nowhere
			 free(rec_ptr);
			 rec_ptr = NULL;
		 }
	 }
	 match = NULL;

    if (rec_ptr == NULL) {
        printf("Did not remove: %d\n", su_id);
    } else if (assigned_or_waiting == 0) {
        assert(rec_ptr->su_id == su_id);
        printf("Removed: %d from assigned list\n", su_id);
        sas_record_print(rec_ptr);
    } else if (assigned_or_waiting == 1) {
        assert(rec_ptr->su_id == su_id);
        printf("Removed: %d from waiting queue\n", su_id);
        sas_record_print(rec_ptr);
    }
	 free(rec_ptr);
    rec_ptr = NULL;
}

/* This function removes the records from the sorted assigned list 
 * that are assigned to 'channel' and appends them to the waiting queue.  
 * The search of the assigned list must be by increasing order of the 
 * secondary uers IDs.  A secondary user record removed from the assigned 
 * list is inserted into the tail of the waiting queue.
 */
void sas_move(ListPtr assn_list, int channel, ListPtr wait_q)
{
    int count_removed = 0;
	 int i = 0, listTop = 0;
	 IteratorPtr rover = NULL;
	 IteratorPtr previous = NULL;
	 su_info_t *removedData = NULL;

	 // make sure list exists
	 if ((assn_list == NULL) || (wait_q == NULL)) {
		  return;
	 }

	 rover = list_iter_front(assn_list);
	 listTop = list_size(assn_list);
	 // look through assigned list for matching channels
	 for (i = 0; i < listTop; i++) {
		 if (list_access(assn_list,rover)->channel == channel) {
		 	 // if match, remove from assigned list and add to wait list
			 previous = rover;
			 rover = list_iter_next(rover);
			 removedData = list_remove(assn_list, previous);
			 list_insert(wait_q, removedData, NULL);
			 count_removed++;
		 }
		 else {
			 rover = list_iter_next(rover);
		 }
	 }

    if (count_removed == 0) {
        printf("Did not find any users on channel %d\n", channel);
    } else {
        printf("Removed %d from channel %d\n", count_removed, channel);
    }

	 rover = NULL;
	 previous = NULL;
	 removedData = NULL;
}

/* Change the channel number of any secondary user in the assigned
 * list that is assigned to the old_channel to the new_channel
 */
void sas_change(ListPtr list_ptr, int old_channel, int new_channel)
{
	// NULL list will cause list validate to fail in list functions
	if (list_ptr == NULL) {
		return;
	}

    int count_moved = 0;
	 int i = 0;
	 IteratorPtr rover = NULL;

	 rover = list_iter_front(list_ptr);
	 for (i = 0; i < list_size(list_ptr); i++) {
		 if (list_access(list_ptr, rover)->channel == old_channel) {
			 list_access(list_ptr, rover)->channel = new_channel;
			 count_moved++;
		 }
		 rover = list_iter_next(rover);
	 }

    if (count_moved == 0) {
        printf("Did not find any users on channel %d\n", old_channel);
    } else {
        printf("Moved %d users from channel %d to %d\n", count_moved, 
                old_channel, new_channel);
    }
	 rover = NULL;
}

/* Assign a secondary user on the waiting list to a channel, if possible.
 *
 * First, if the waiting queue is empty, no move is possible
 *
 * Next, if the assigned list is full, no move is possible
 *
 * Finally, if neither case, then remove the secondary user at the head
 * of the waiting queue and insert into the assigned list, setting
 * the channel.
 */
void sas_assign(ListPtr assn_ptr, int size, ListPtr wait_q, int channel)
{
	// NULL list will cause assert to fail in list functions below
	if ((assn_ptr == NULL) || (wait_q == NULL)) {
		return;
	}

    su_info_t *rec_ptr = NULL;
    int assign_action = -2;

	 if (list_size(wait_q) == 0) { // wait queue empty
		 assign_action = 0;
	 }
	 else if (list_size(assn_ptr) == size) { // assign list full
		 assign_action = 1;
	 }
	 else {
		 rec_ptr = list_remove(wait_q, NULL);
		 list_insert_sorted(assn_ptr, rec_ptr);
		 rec_ptr->channel = channel;
		 assign_action = 2;
	 }
 
    if (assign_action == 0) {
        printf("No secondary users are waiting\n");
    } else if (assign_action == 1) {
        printf("User(s) waiting but the assigned list is full %d\n", size);
    } else if (assign_action == 2) {
        printf("Moved waiting SU %d to channel: %d\n", rec_ptr->su_id, channel);
    } else {
        printf("Error with assign action flag!\n");
    }
    rec_ptr = NULL;
}

/* print count of records in the assigned list and waiting queue
 */
void sas_stats(ListPtr sorted, int sorted_size, ListPtr unsorted)
{
	// NULL list will cause asserts to fail in list functions
	if ((sorted == NULL) || (unsorted == NULL)) {
		return;
	}

    // get the number in list and size of the list
    int num_in_list = list_size(sorted); 
    int num_in_queue = list_size(unsorted);
    printf("List records:  %d, Max list size: %d  ", 
            num_in_list, sorted_size);
    printf("Queue records: %d\n", num_in_queue);
}

/* this function frees the memory for either a sorted or unsorted list.
 */
void sas_cleanup(ListPtr list_ptr)
{
	// cleaning up NULL list will cause validate function to fail in list_destruct
	if (list_ptr == NULL) {
		return;
	}

	list_destruct(list_ptr);
}

/* Checks for a invalid channel number.  Channel numbers must be 1:10
 *
 * returns true if invalild channel
 *         false if valid channel
 */
int sas_invalid_ch(int channel_num)
{
    if (channel_num < 1 || channel_num > 10) return 1;
    return 0;
}

/* Sorts a given list using the specified sort type
 *
 * L: the list to be sorted.
 * sort_type: the type of sort to use. This value will be interpreted by
 *					the functions in list.c.
 */
void sas_sort(ListPtr L, int sort_type) {
	if (L == NULL) {
		return;
	}

	clock_t start, end;
	double elapse_time; /* time in milliseconds */

	int initialsize = list_size(L);
	start = clock();
	list_sort(L, sort_type);
	end = clock();
	elapse_time = 1000.0 * ((double) (end - start)) / CLOCKS_PER_SEC;
	
	// sas_print(L, "Waiting Queue"); // for debugging
	
	assert(list_size(L) == initialsize);
	printf("%d\t%f\t%d\n", initialsize, elapse_time, sort_type);
}

/* Adds a new entry to the tail of the given list. The data of
 * the new entry is not collected, only the su_id, which is passed
 * to the function as an argument.
 *
 * list_ptr: the list which a new tail is to be added to.
 * new_id: the su_id of the new entry to be added.
 */
void sas_addtail(ListPtr list_ptr, int new_id) {
	if (list_ptr == NULL) {
		return;
	}

    su_info_t *rec_ptr;
    rec_ptr = (su_info_t *) calloc(1, sizeof(su_info_t));
	 rec_ptr->su_id = new_id;
	 list_insert(list_ptr, rec_ptr, NULL);
}

/* Prompts user for secondary record input starting with the SU ID.
 * The input is not checked for errors but will default to an acceptable value
 * if the input is incorrect or missing.
 *
 * The input to the function assumes that the structure has already been
 * created.  The contents of the structure are filled in.
 *
 * There is no output.
 */
void sas_record_fill(su_info_t *new)
{
    char line[MAXLINE];
    char str[MAXLINE];
    assert(new != NULL);

    printf("secondary user ID number:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->su_id);
    printf("IP address:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->ip_address);
    printf("Access point IP address:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->access_point);

    printf("Authenticated (T/F):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%s", str);
    if (strcmp(str, "T")==0 || strcmp(str, "t")==0)
        new->authenticated = 1;
    else
        new->authenticated = 0;

    printf("Privacy (none|standard|strong|NSA):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%s", str);
    if (strcmp(str, "standard")==0)
	new->privacy = 1;
    else if (strcmp(str, "strong")==0)
	new->privacy = 2;
    else if (strcmp(str, "NSA")==0)
	new->privacy = 3;
    else
	new->privacy = 0;

    printf("Band (2.4|5.0):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%f", &new->band);

    printf("Channel:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->channel);
    if (sas_invalid_ch(new->channel))
        new->channel = 10;

    printf("Data rate:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%f", &new->data_rate);

    printf("Time received (int):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->time_received);
    printf("\n");
}

/* print the information for a particular secondary user record 
 *
 * Input is a pointer to a record, and no entries are changed.
 */
void sas_record_print(su_info_t *rec)
{
    const char *pri_str[] = {"none", "standard", "strong", "NSA"};
    assert(rec != NULL);
    printf("ID: %d, C: %d,", rec->su_id, rec->channel);
    printf(" MIP: %d, AID: %d,", rec->ip_address, 
            rec->access_point);
    printf(" Auth: %s,", rec->authenticated ? "T" : "F"); 
    printf(" Pri: %s, B: %g,", pri_str[rec->privacy], rec->band);
    printf(" R: %g", rec->data_rate);
    printf(" Time: %d\n", rec->time_received);
}

