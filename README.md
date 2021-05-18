# Linked-List-and-Sort-Algorithm

This was a school project for one of my C programming courses.

In this program, we implement an application for storing information of various users in lists.
The lists are implemented using two way linked lists, and there is also an implementation of a few
sorting algrorithms to sort the lists. Each user is identified with a user id, is added to the waiting
list when initially created. They can then be assigned to different 'channel' numbers when moved from 
the waiting list to the assigned list. The other information stored in each user entry is not used 
by the code, and is there for information only. There are different commands described below to
manage the different channels and all users on them, to simulate managing a communication network.

These are the commands available to the user.

    CREATE x: Creates list of size 'x'.
    ADDSU: Adds user to the waiting list, after prompting to input user data. Simply adds new user 
        to the end of the waiting list, does not sort.
    DELSU x: Deletes user with ID 'x' from the list.
    LISTCH x: Lists all users on channel 'x'.
    CLEARCH x: Move users on channel 'x' from assigned to waiting list
    CHANGECH old new: Change users in assigned list on channel 'old' to channel 'new'
    ASSIGNSU x: Assign first user in waiting list to channel 'x' on assigned list. Users are added to
        assigned list in correct position, list is created already sorted.
    PRINTASSIGNED: Print info of all users currently on assigned list
    PRINTWAITING: Print info of all users currently on waiting list
    STATS: Print number of users on each list, and the max size of the list
    SORT x: Sorts waiting list using given sort type 'x' (by order of user id, as defined in comparison 
        function). Returns the size of the list, the runtime in milliseconds, and the sort type.
        1: Insertion sort
        2: Recursive selection sort
        3: Iterative selection sort
        4: Merge sort
    ADDTAIL su_id: Creates and adds user of id 'su_id' to the waiting list. All other data except 
        ID is set to 0. This allows you to quickly add users to the list for easier testing of 
        the program.
    QUIT: Exit the program and return all memory

Below is a brief description of each file in the application. Further information can be found
in the header comment of each file.

  lab3.c: The main function for the application, provides commands to the user and takes input.  
  sas_support.c: Provides functions for interaction with the records, using functions from list.c.  
  list.c: Provides functions for interacting with a two way linked list.  
  datatypes.h: defines the data type for a user entry in the list.  
  sas_support.h: prototype function definitions for sas_support.c.  
  list.h: prototype function and struct definitions for list.c.  
  mp3test.sh: script to test program for performance analysis.  
  makefile: compiles application.  
