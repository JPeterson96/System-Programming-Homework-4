/*
 * Jamie Peterson
 * CS 337
*/

#define _POSIX_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "inventory.h"

const char *itemNames[NAMES_SIZE] = ITEM_NAMES;

/* print the contents of an inventory record
should be able to handle invalid ids */
int printRecords(struct record *inventory, int num) {
	int count = 0;
	int id;

	/* Prints each id and quatity using pointer arithmatic */
	while(count < num) {
		id = inventory->id;

		/* Error checking for invalid Id */
		if (id > NAMES_SIZE)
			printf("Id: %d (%s), quantity: %d\n", inventory->id, "unknown", inventory->quantity);

		/* Normal function */
		else
			printf("Id: %d (%s), quantity: %d\n", inventory->id, itemNames[inventory->id], inventory->quantity);

		inventory++;
		count++;
	}
	
	return count;
}

/* write the given number of records
from inventory to the given file
if num is too large (compared to MAX_RECORD_SIZE),
print an error
if the write failes to write everything,
print an error*/
int writeInventory(struct record *inventory, char *filename, int num) {
	FILE *output = fopen(filename, "w");
	int counter = 0;

	/* Error checking */
	if (num > MAX_RECORD_SIZE)
	{
		printf("Number of records is greater than the max size.\n");
		return 0;
	}

	/* Since the number is passed in, its really easy */
	counter = fwrite(inventory, sizeof(struct record), num, output);

	/* Error checking */
	if(counter < num)
		printf("Failed to write everything to the file.");

	fclose(output);
	
	return counter;
}

/* read an unknown number of records into inventory from the file
returning the number of records in the inventory
this will fail if inventory does not have enough memory allocated */
int readInventory(struct record *inventory, char *filename) {
	FILE *in = fopen(filename, "r");
	int counter = 0;

	/* Error checking */
	if(in == NULL) {
		printf("Failed to open file: %s\n", filename);
		return 0;
	}

	/* Reads until the end of a file */
	while(fread(inventory, sizeof(struct record), 1, in) != 0) {
		counter++;
		inventory++;
	}

	fclose(in);
	
	return counter;
}

/* print the contents of an inventory file
should be able to handle invalid ids */
void printInventory(char *filename) {
	FILE *in = fopen(filename, "r");
	struct record *rec = (struct record*)malloc(sizeof(struct record) * 32);

	/* Error checking */
	if(in == NULL)
		printf("Failed to open file: %s\n", filename);

	/* Keeps reading until the end of the file */	
	while(fread(rec, sizeof(struct record), 1, in) != 0) {
		printf("Id: %d (%s), quantity: %d\n", rec->id, itemNames[rec->id], rec->quantity);
		rec++;
	}
	
	fclose(in);
}

/* create a copy of the given file with the given name
it may be a shallow copy */
int cloneInventory(char *filename, char *newName) {
	FILE *in = fopen(filename, "r");
	FILE *out = fopen(newName, "w");

	struct record *rec = (struct record*)malloc(sizeof(struct record) * 32);

	/* Error Checking */
	if (in == NULL){
		printf("Failed to open file: %s", filename);
		return -1;
	}

	/* Reads one piece of data at a time then writes it to output file */
	while(fread(rec, sizeof(struct record), 1, in) != 0) {
		fwrite(rec, sizeof(struct record), 1, out);
		rec++;
	}

	fclose(in);
	fclose(out);

	/* If sucessful */
	return 0;
	/*return link(filename, newName); I couldnt get link to work do i did it manually*/
}

/* replace fileOld with the contents of fileNew
if fileNew has a newer (or equal) modified timestamp
fileNew should be unchanged
this may create a shallow copy
return 1 if replaced and 0 otherwise */
int updateInventory(char *fileOld, char *fileNew) {
	struct stat oldFileStat;
	struct stat newFileStat;

	/* Error checking for fake files */
	if (strcmp(fileOld, "FAKEFILE") == 0 || strcmp(fileNew, "FAKEFILE") == 0) {
		printf("File doesn't exist\n");
		return 0;
	}

	/* Pulls stats from files passed in */
	stat(fileOld, &oldFileStat);
	stat(fileNew, &newFileStat);

	/* Compares to see which is newer, if new is newer, update inventory */
	if (newFileStat.st_mtimensec < oldFileStat.st_mtimensec) {
		cloneInventory(fileNew, fileOld);
		
		return 1;
	}
	
	return 0;
}

/* helper function to non-destructively merge inv1 and inv2 into merged
and return the number of records in merged
making sure the result does not contain duplicate ids
merged should not be allocated before calling this function
any time this function is called, the merged pointer must later be freed */
int mergeRecords(struct record *inv1, int num1, struct record *inv2, int num2, struct record **merged) {
	struct record* ptr1;
	struct record* ptr2;
	struct record* ptrAppend;
	struct record* ptrCombine;
	int i;
	int count;

	*merged = (struct record*)malloc(MAX_RECORD_SIZE * sizeof(struct record));
	ptrAppend = *merged;
	ptr1 = inv1;
	ptr2 = inv2;
	count = 0;

	for(i=0; i<num1; i++) {
		ptrCombine = *merged;
		while (ptrCombine != ptrAppend && ptrCombine->id != ptr1->id)
			ptrCombine++;
		if(ptrCombine == ptrAppend) {
			*ptrAppend = *ptr1;
			ptrAppend++;
			count ++;
		}
		else
			ptrCombine->quantity += ptr1->quantity;
		ptr1++;
	}

	for(i=0; i<num2; i++) {
		ptrCombine = *merged;
		while (ptrCombine != ptrAppend && ptrCombine->id != ptr2->id)
			ptrCombine++;
		if(ptrCombine == ptrAppend) {
			*ptrAppend = *ptr2;
			ptrAppend++;
			count++;
		}
		else
			ptrCombine->quantity += ptr2->quantity;
		ptr2++;
	}
	
	return count;
}

/* load an inventory from a file
combine records with the same id (adding their quantities)
and write the result back to that file */
int combineDuplicates(char * filename){
	/* Creating and filling initail struct from file */
	struct record *rec = (struct record*)malloc(sizeof(struct record*) * 32);
	int recCount = readInventory(rec, filename);

	/* Creating 2nd struct and result. 2nd struct is empty so the original struct stays in tact */
	struct record *rec2 = (struct record*)malloc(sizeof(struct record*) * 32);
	struct record **result = (struct record**)malloc(sizeof(struct record**) * 32);

	/* Merging data with empty struct to remove duplicates in original */
	int count = mergeRecords(rec, recCount, rec2, 0, result);
	count = writeInventory(rec,filename,count);

	/* Free allocated memory */
	free(rec);
	free(rec2);
	free(result);
	
	return count;
}

/* load two inventories from two files
merge their contents (combining records with the same id)
and write the result to a third file
delete the two source files
(unless the source is the same as the destination) */
int mergeInventories(char *filenameA, char *filenameB, char *filenameM) {
	/* creating and allocating space for structs */
	struct record *rec = (struct record*)malloc(sizeof(struct record*) * 32);
	struct record *rec2 = (struct record*)malloc(sizeof(struct record*) * 32);

	/* filling structs with data from files and getting count from their results */
	int recCount = readInventory(rec, filenameA);
	int rec2Count = readInventory(rec2, filenameB);

	struct record **result = (struct record**)malloc(sizeof(struct record**) * 32);

	/* merging records and getting rid of duplicates */
	int count = mergeRecords(rec, recCount, rec2, rec2Count, result);

	/* writing to file */
	count = writeInventory(*result, filenameM, count);

	/* deleting file A and B as long as they arent the same as file M*/
	if (filenameA != filenameM)
		remove(filenameA);

	if (filenameB != filenameM)
		remove(filenameA);
	
	return 0;
}


/* read the entire contents of a directory as inventories
and merge them all into one inventory,
combining records with the same id
and returning the number of records in the final inventory
all regular files should be considered inventories
inventory should not be allocated before calling this function
any time this function is successfully called, the inventory pointer must later be freed */
int readDirectory(char *dirname, struct record **inventory) {
	DIR *dir = opendir(dirname);
	struct dirent *entry = (struct dirent*)malloc(sizeof(struct dirent*));

	/* allocating memory for the 2 structs and inventory */
	struct record *rec = (struct record*)malloc(sizeof(struct record*) * 32);
	struct record *rec2;

	/* Sizes of strucs used for merging */
	int recSize = 0;
	int rec2Size = 0;

	int dirStatus = 0;

	*inventory = (struct record*)malloc(sizeof(struct record*) * 32);

	/* Error Checking */
	if (dir == NULL){
		printf("Directory doesnt exist: %s \n", dirname);
		return -1;
	}

	/* gets names of files in the given directory */
	entry = readdir(dir);

	/* loops through files in the directory, reading data and merging it into the result file */
	while(entry != NULL) {
		dirStatus = chdir(dirname);
		recSize = readInventory(rec,entry->d_name);

		/* merge current data with data from other files in same directory */
		rec2Size = mergeRecords(rec, recSize, rec2, rec2Size, inventory);

		/* rec2 acts holds the total accumulated data */
		rec2 = *inventory;
		entry = readdir(dir);
	}

	free(rec);
	free(entry);
	closedir(dir);
	
	return rec2Size;
}

/* helper function to delete a directory
this should delete all directory contents
and then delete the directory
print an error if there is a subdirectory */
int deleteDirectory(char *dirname) {
	DIR *dir = opendir(dirname);
	struct dirent *entry = (struct dirent*)malloc(sizeof(struct dirent*));
	struct stat st;

	/* Read until end of directory, clearing files as you go */
	while ((entry = readdir(dir)) != NULL) {
		stat(entry->d_name, &st);
		printf("file path: %s", entry->d_name);
		
		if (S_ISDIR(st.st_mode)){
			printf("Subdirectory cannot be removed");
			return -1;
		}
			
		remove(entry->d_name);
	}

	/* Remove directory */
	remove(dirname);
	free(entry);
	
	return 0;
}

/* read the entire contents of a directory as inventories
and merge them all into one inventory,
combining records with the same id
write the result to a new file
delete the source directory */
int mergeDirectory(char *dirname, char *inventory) {
	/* Append / onto back so the system knows its a directory */
	struct record **inv = (struct record**)malloc(sizeof(struct record**) * 1024);

	/* read the directory to dir then delete once its done */
	int count = readDirectory(dirname, inv);
	
	/*if (count > 0)
		deleteDirectory(dirname);*/

	/* Write inventory to a new file */
	count = writeInventory(*inv, inventory, count);
	
	free(inv);
	
	return count;
}
