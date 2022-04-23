#define _POSIX_SOURCE 1

#define MAX_RECORD_SIZE 50
#define NAMES_SIZE 41
#define ITEM_NAMES {"fragment", "note", "book", "bell", "stone", "flower", "fruit", "meat", "bone", "feather", "butterfly", "firefly", "rune", "moss", "resin", "twig", "arrow", "bolt", "pot", "flask", "seed", "tear", "key", "medallion", "talisman", "staff", "seal", "hat", "shirt", "pants", "gloves", "sword", "hammer", "spear", "axe", "dagger", "bow", "crossbow", "shield", "torch", "ash"}

struct record {
	unsigned int id;
	unsigned int quantity;
};

int printRecords(struct record *inventory, int num);
int writeInventory(struct record *inventory, char *filename, int num);
int readInventory(struct record *inventory, char *filename);
void printInventory(char *filename);
int cloneInventory(char *filename, char *newName);
int updateInventory(char *fileOld, char *fileNew);
int combineDuplicates(char * filename);
int mergeInventories(char *filenameA, char *filenameB, char *filenameM);
int readDirectory(char *dirname, struct record **inventory);
int mergeDirectory(char *dirname, char *inventory);
