#define _POSIX_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "inventory.h"

#define PERMS S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

void cleanupFiles() {
	unlink("single.inv");
	unlink("multiple.inv");
	unlink("toomany.inv");
	unlink("multiple2.inv");
	unlink("newer.inv");
	unlink("duplicates.inv");
	unlink("inv1.inv");
	unlink("inv2.inv");
	unlink("merged.inv");
	unlink("combined.inv");
	unlink("combined2.inv");
	unlink("FAKEFILE");
	rmdir("inventories/subdir");
	unlink("inventories/inv1.inv");
	unlink("inventories/inv2.inv");
	unlink("inventories/inv3.inv");
	rmdir("inventories");
	rmdir("FAKEDIR");
}

int main(void) {
	struct record r0, r1, r2, r3, r4, r5, r6, rBad; 
	struct record *ptr1, *ptr2, *ptrc, *ptrd, *ptrm, *ptrBig;
	struct record records[NAMES_SIZE];
	struct record new;
	int count, i;
	
	r1.id = 16; r1.quantity = 99;
	r2.id = 12; r2.quantity = 2047;
	r3.id = 36; r3.quantity = 1;
	r4.id = 19; r4.quantity = 3;
	r5.id = 12; r5.quantity = 644;
	r6.id = 32; r6.quantity = 1;
	rBad.id = 80; rBad.quantity = 9;
	for(i = 0; i<NAMES_SIZE; i++) {
		r0.id = i;
		r0.quantity = 1;
		records[i] = r0;
	}

	ptr1 = (struct record*)malloc(3 * sizeof(struct record));
	ptr2 = (struct record*)malloc(3 * sizeof(struct record));
	ptrc = (struct record*)malloc(3 * sizeof(struct record));
	ptrd = (struct record*)malloc(3 * sizeof(struct record));
	ptrBig = (struct record*)malloc(1000 * sizeof(struct record));

	ptr1[0] = r1;
	ptr1[1] = r2;
	ptr1[2] = r3;

	ptr2[0] = r4;
	ptr2[1] = r5;
	ptr2[2] = r6;

	ptrd[0] = r2;
	ptrd[1] = r3;
	ptrd[2] = r5;

	cleanupFiles();

	printf("\nTesting printRecords on a single record\n");
	printRecords(&records[0], 1);

	printf("\nTesting printRecords on multiple records\n");
	printRecords(ptr1, 3);

	printf("\nTesting printRecords with invalid ID\n");
	printRecords(&rBad, 1);

	printf("\nWriting single record to single.inv\n");
	writeInventory(&records[5], "single.inv", 1);

	printf("\nWriting multiple records to multiple.inv\n");
	writeInventory(&records[0], "multiple.inv", 6);

	printf("\nWriting too many records to toomany.inv\n");
	writeInventory(&records[0], "toomany.inv", 100);

	printf("\nReading single record from single.inv\n");
	count = readInventory(ptrc, "single.inv");
	printRecords(ptrc, count);

	printf("\nReading multiple records from multiple.inv\n");
	count = readInventory(ptrBig, "multiple.inv");
	printRecords(ptrBig, count);

	printf("\nReading from non-existant file\n");
	readInventory(ptrBig, "FAKEFILE");

	printf("\nPrinting single record from single.inv\n");
	printInventory("single.inv");

	printf("\nPrinting multiple records from multiple.inv\n");
	printInventory("multiple.inv");

	printf("\nCloning multiple.inv to multiple2.inv\n");
	cloneInventory("multiple.inv", "multiple2.inv");
	printf("Original:\n");
	printInventory("multiple.inv");
	printf("Clone:\n");
	printInventory("multiple2.inv");

	printf("\nWaiting a bit\n");
	sleep(1);

	writeInventory(&records[16], "newer.inv", 4);
	printf("\nTesting updating newer.inv with older file\n");
	if(updateInventory("newer.inv", "single.inv"))
		printf("Updated:\n");
	else
		printf("Not updated:\n");
	printInventory("newer.inv");
	printf("\nTesting updating single.inv with newer file\n");
	if(updateInventory("single.inv", "newer.inv"))
		printf("Updated:\n");
	else
		printf("Not updated:\n");
	printInventory("single.inv");
	printf("\nTesting updating single.inv with non-existant file\n");
	if(updateInventory("single.inv", "FAKEFILE"))
		printf("Updated:\n");
	else
		printf("Not updated:\n");
	printInventory("single.inv");
	printf("\nTesting updating non-existant file\n");
	if(updateInventory("FAKEFILE", "single.inv"))
		printf("Updated:\n");
	else
		printf("Not updated:\n");
	
	printf("\nTesting combineDuplicates on duplicate file\n");
	writeInventory(ptrd, "duplicates.inv", 3);
	printf("Before combining:\n");
	printInventory("duplicates.inv");
	combineDuplicates("duplicates.inv");
	printf("After combining:\n");
	printInventory("duplicates.inv");

	printf("\nTesting combineDuplicates without duplicates\n");
	writeInventory(ptr2, "inv2.inv", 3);
	printf("Before combining:\n");
	printInventory("inv2.inv");
	combineDuplicates("inv2.inv");
	printf("After combining:\n");
	printInventory("inv2.inv");

	printf("\nTesting combineDuplicates on non-existant file\n");
	combineDuplicates("FAKEFILE");

	printf("\nTesting merging two files\n");
	writeInventory(ptr1, "inv1.inv", 3);
	printf("inv1:\n");
	printInventory("inv1.inv");
	printf("inv2:\n");
	printInventory("inv2.inv");
	mergeInventories("inv1.inv", "inv2.inv", "merged.inv");
	printf("merged:\n");
	printInventory("merged.inv");

	printf("\nTesting merging the result with itself\n");
	mergeInventories("merged.inv", "merged.inv", "merged.inv");
	printInventory("merged.inv");

	printf("\nVerifying that the source files were removed\n");
	readInventory(ptrBig, "inv1.inv");
	readInventory(ptrBig, "inv2.inv");
	
	printf("\nConstructing directory\n");
	if(mkdir("inventories", 0) == -1)
		perror("Make directory failed: ");
	chmod("inventories", PERMS);
	writeInventory(ptr1, "inventories/inv1.inv", 3);
	writeInventory(ptr2, "inventories/inv2.inv", 3);
	writeInventory(&records[10], "inventories/inv3.inv", 4);

	printf("\nReading directory\n");
	count = readDirectory("inventories", &ptrm);
	printRecords(ptrm, count);

	printf("\nReading non-existant directory\n");
	/*ptrm should have been allocated by readDirectory */
	free(ptrm);
	readDirectory("FAKEDIR", &ptrm);
	/*this is a memory leak unless readDirectory does not allocate
	on a failure */

	printf("\nMerging directory to combined.inv\n");
	mergeDirectory("inventories", "combined.inv");
	printInventory("combined.inv");

	printf("\nVerifying that the source files were removed\n");
	readInventory(ptrBig, "inventories/inv1.inv");
	readInventory(ptrBig, "inventories/inv2.inv");
	readInventory(ptrBig, "inventories/inv3.inv");

	printf("\nMerging directory with a subdirectory\n");
	printf("The merge should succeed, but the deletion can fail\n");
	if(mkdir("inventories", 0) == -1)
		perror("Make directory failed: ");
	chmod("inventories", PERMS);
	if(mkdir("inventories/subdir", 0) == -1)
		perror("Make directory failed: ");
	chmod("inventories/subdir", PERMS);
	writeInventory(ptr1, "inventories/inv1.inv", 3);
	writeInventory(ptr2, "inventories/inv2.inv", 3);
	writeInventory(&records[10], "inventories/inv3.inv", 4);
	mergeDirectory("inventories", "combined2.inv");
	printInventory("combined2.inv");

	free(ptr1);
	free(ptr2);
	free(ptrc);
	free(ptrd);
	free(ptrBig);
	
	
  return 0;
}
