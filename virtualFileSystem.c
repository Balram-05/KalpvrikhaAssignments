
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME 50
#define MAX_INPUT 4096

/* Virtual disk */
char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];

/* Free block structure */
typedef struct FreeBlock
{
    int index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

/* File and directory structure */
typedef struct FileNode
{
    char name[MAX_NAME + 1];
    int isDirectory;

    struct FileNode *parent;
    struct FileNode *children;

    struct FileNode *next;
    struct FileNode *prev;

    int *blockPointers;
    int blockCount;
    int contentSize;

} FileNode;

/* Global pointers */
FreeBlock *freeHead = NULL;
FreeBlock *freeTail = NULL;

FileNode *root = NULL;
FileNode *cwd = NULL;

int freeCount = 0;
void printPath(FileNode *directory);

/* FREE BLOCK MANAGEMENT */

/* Insert a free block at the tail */
void addFreeBlock(int index)
{
    FreeBlock *newNode;

    newNode = (FreeBlock *)malloc(sizeof(FreeBlock));

    if (newNode == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    newNode->index = index;
    newNode->next = NULL;
    newNode->prev = freeTail;

    if (freeHead == NULL)
    {
        freeHead = newNode;
        freeTail = newNode;
    }
    else
    {
        freeTail->next = newNode;
        freeTail = newNode;
    }

    freeCount++;
}

/* Remove a block from the head */
int getFreeBlock()
{
    FreeBlock *temp;
    int index;

    if (freeHead == NULL)
        return -1;

    temp = freeHead;
    index = temp->index;

    freeHead = freeHead->next;

    if (freeHead != NULL)
        freeHead->prev = NULL;
    else
        freeTail = NULL;

    free(temp);
    freeCount--;

    return index;
}

/* Initialize all disk blocks as free */
void initializeDisk()
{
    int i;

    freeHead = NULL;
    freeTail = NULL;
    freeCount = 0;

    for (i = 0; i < NUM_BLOCKS; i++)
    {
        addFreeBlock(i);
    }
}

/* Return all blocks of a file to free list */
void releaseFileBlocks(FileNode *file)
{
    int i;

    for (i = 0; i < file->blockCount; i++)
    {
        addFreeBlock(file->blockPointers[i]);
    }

    free(file->blockPointers);

    file->blockPointers = NULL;
    file->blockCount = 0;
    file->contentSize = 0;
}


/* DIRECTORY AND FILE MANAGEMENT */

/* Create a new file or directory node */
FileNode *createNode(char *name, int isDirectory, FileNode *parent)
{
    FileNode *newNode;

    newNode = (FileNode *)malloc(sizeof(FileNode));

    if (newNode == NULL)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    strcpy(newNode->name, name);

    newNode->isDirectory = isDirectory;
    newNode->parent = parent;

    newNode->children = NULL;

    newNode->next = NULL;
    newNode->prev = NULL;

    newNode->blockPointers = NULL;
    newNode->blockCount = 0;
    newNode->contentSize = 0;

    return newNode;
}

/* Search for an entry in a directory */
FileNode *findEntry(FileNode *directory, char *name)
{
    FileNode *temp;

    if (directory == NULL || directory->children == NULL)
        return NULL;

    temp = directory->children;

    do
    {
        if (strcmp(temp->name, name) == 0)
            return temp;

        temp = temp->next;

    } while (temp != directory->children);

    return NULL;
}

/* Insert a node into circular doubly linked list */
void insertEntry(FileNode *directory, FileNode *newNode)
{
    FileNode *last;

    if (directory->children == NULL)
    {
        newNode->next = newNode;
        newNode->prev = newNode;

        directory->children = newNode;
    }
    else
    {
        last = directory->children->prev;

        newNode->next = directory->children;
        newNode->prev = last;

        last->next = newNode;
        directory->children->prev = newNode;
    }
}

/* Remove a node from circular doubly linked list */
void removeEntry(FileNode *directory, FileNode *node)
{
    if (node->next == node)
    {
        directory->children = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;

        if (directory->children == node)
            directory->children = node->next;
    }

    node->next = NULL;
    node->prev = NULL;
}


/* DIRECTORY OPERATIONS */

/* Create a directory */
void makeDirectory(char *name)
{
    FileNode *newNode;

    if (findEntry(cwd, name) != NULL)
    {
        printf("An entry with this name already exists.\n");
        return;
    }

    newNode = createNode(name, 1, cwd);

    if (newNode == NULL)
        return;

    insertEntry(cwd, newNode);

    printf("Directory '%s' created successfully.\n", name);
}

/* Create an empty file */
void createFile(char *name)
{
    FileNode *newNode;

    if (findEntry(cwd, name) != NULL)
    {
        printf("An entry with this name already exists.\n");
        return;
    }

    newNode = createNode(name, 0, cwd);

    if (newNode == NULL)
        return;

    insertEntry(cwd, newNode);

    printf("File '%s' created successfully.\n", name);
}

/* Change current working directory */

void changeDirectory(char *name)
{
    FileNode *target;

    if (strcmp(name, "..") == 0)
    {
        if (cwd->parent != NULL)
            cwd = cwd->parent;
    }
    else if (strcmp(name, "/") == 0)
    {
        cwd = root;
    }
    else
    {
        target = findEntry(cwd, name);

        if (target == NULL || target->isDirectory == 0)
        {
            printf("Directory not found.\n");
            return;
        }

        cwd = target;
    }

    printf("Moved to ");
    printPath(cwd);
    printf("\n");
}

/* Display current working directory path */
void printPath(FileNode *directory)
{
    if (directory == root)
    {
        printf("/");
        return;
    }

    printPath(directory->parent);

    printf("%s/", directory->name);
}

/* List directory contents */
void listDirectory()
{
    FileNode *temp;

    if (cwd->children == NULL)
    {
        printf("(empty)\n");
        return;
    }

    temp = cwd->children;

    do
    {
        if (temp->isDirectory)
            printf("%s/\n", temp->name);
        else
            printf("%s\n", temp->name);

        temp = temp->next;

    } while (temp != cwd->children);
}

/* Remove an empty directory */
void removeDirectory(char *name)
{
    FileNode *target;

    target = findEntry(cwd, name);

    if (target == NULL)
    {
        printf("Directory not found.\n");
        return;
    }

    if (target->isDirectory == 0)
    {
        printf("This is a file, not a directory.\n");
        return;
    }

    if (target->children != NULL)
    {
        printf("Directory is not empty.\n");
        return;
    }

    removeEntry(cwd, target);

    free(target);

    printf("Directory removed successfully.\n");
}


/* FILE OPERATIONS */


/* Write content into a file */
void writeFile(char *name, char *content)
{
    FileNode *file;
    int size;
    int required;
    int i;
    int copied = 0;
    int *newBlockPointers = NULL;

    file = findEntry(cwd, name);

    if (file == NULL)
    {
        printf("File not found.\n");
        return;
    }

    if (file->isDirectory)
    {
        printf("Cannot write to a directory.\n");
        return;
    }

    size = strlen(content);

    required = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    /* Check whether enough blocks will be available */
    if (required > freeCount + file->blockCount)
    {
        printf("Disk is full. Not enough free blocks.\n");
        return;
    }

    /* Allocate memory before releasing existing blocks */
    if (required > 0)
    {
        newBlockPointers = (int *)malloc(required * sizeof(int));

        if (newBlockPointers == NULL)
        {
            printf("Memory allocation failed.\n");
            return;
        }
    }

    /* Release previously allocated blocks */
    releaseFileBlocks(file);

    file->blockPointers = newBlockPointers;
    file->blockCount = required;
    file->contentSize = size;

    /* Allocate blocks and copy content */
    for (i = 0; i < required; i++)
    {
        int block = getFreeBlock();
        int remaining = size - copied;
        int amount;

        file->blockPointers[i] = block;

        if (remaining > BLOCK_SIZE)
            amount = BLOCK_SIZE;
        else
            amount = remaining;

        memcpy(virtualDisk[block], content + copied, amount);

        copied += amount;
    }

    printf("Data written successfully (size=%d bytes).\n", size);
}


/* Read file content from virtual disk */
void readFile(char *name)
{
    FileNode *file;

    int i;
    int remaining;

    file = findEntry(cwd, name);

    if (file == NULL)
    {
        printf("File not found.\n");
        return;
    }

    if (file->isDirectory)
    {
        printf("Cannot read a directory.\n");
        return;
    }

    if (file->contentSize == 0)
    {
        printf("(empty file)\n");
        return;
    }

    remaining = file->contentSize;

    for (i = 0; i < file->blockCount; i++)
    {
        int amount;

        if (remaining > BLOCK_SIZE)
            amount = BLOCK_SIZE;
        else
            amount = remaining;

        fwrite(virtualDisk[file->blockPointers[i]],
               1, amount, stdout);

        remaining -= amount;
    }

    printf("\n");
}

/* Delete a file */
void deleteFile(char *name)
{
    FileNode *file;

    file = findEntry(cwd, name);

    if (file == NULL)
    {
        printf("File not found.\n");
        return;
    }

    if (file->isDirectory)
    {
        printf("Use rmdir to remove directories.\n");
        return;
    }

    releaseFileBlocks(file);

    removeEntry(cwd, file);

    free(file);

    printf("File deleted successfully.\n");
}


/* DISK USAGE */

/* Display disk information */
void diskUsage()
{
    int used;

    used = NUM_BLOCKS - freeCount;

    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeCount);

    printf("Disk Usage: %.2f%%\n",
           (used * 100.0) / NUM_BLOCKS);
}


/* MEMORY CLEANUP */

/* Free all files and directories recursively */
void freeDirectory(FileNode *directory)
{
    FileNode *first;
    FileNode *temp;
    FileNode *nextNode;

    if (directory == NULL || directory->children == NULL)
        return;

    first = directory->children;
    temp = first;

    do
    {
        nextNode = temp->next;

        if (temp->isDirectory)
        {
            freeDirectory(temp);
        }
        else
        {
            free(temp->blockPointers);
        }

        free(temp);

        temp = nextNode;

    } while (temp != first);

    directory->children = NULL;
}

/* Free remaining free block nodes */
void cleanup()
{
    FreeBlock *temp;

    freeDirectory(root);

    free(root);

    root = NULL;
    cwd = NULL;

    while (freeHead != NULL)
    {
        temp = freeHead;
        freeHead = freeHead->next;

        free(temp);
    }

    freeTail = NULL;
    freeCount = 0;
}

int main()
{
    char input[MAX_INPUT];

    char command[20];
    char name[MAX_NAME + 1];
    char content[MAX_INPUT];

    char *start;
    char *end;

    /* Initialize virtual disk */
    initializeDisk();

    /* Initialize root directory */
    root = createNode("/", 1, NULL);

    if (root == NULL)
    {
        printf("Unable to initialize root directory.\n");
        cleanup();
        return 1;
    }

    cwd = root;

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (1)
    {
        printPath(cwd);
        printf(" > ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        /* Remove newline */
        input[strcspn(input, "\n")] = '\0';

        if (sscanf(input, "%19s", command) != 1)
            continue;

        if (strcmp(command, "exit") == 0)
        {
            cleanup();

            printf("Memory released. Exiting program...\n");
            break;
        }

        else if (strcmp(command, "mkdir") == 0)
        {
            if (sscanf(input, "%19s %50s", command, name) == 2)
            {
                makeDirectory(name);
            }
            else
            {
                printf("Usage: mkdir directory_name\n");
            }
        }

        else if (strcmp(command, "create") == 0)
        {
            if (sscanf(input, "%19s %50s", command, name) == 2)
            {
                createFile(name);
            }
            else
            {
                printf("Usage: create file_name\n");
            }
        }

        else if (strcmp(command, "write") == 0)
        {
            start = strchr(input, ' ');

            if (start != NULL)
            {
                while (*start == ' ')
                    start++;

                end = strchr(start, ' ');

                if (end != NULL)
                {
                    int nameLength = end - start;

                    if (nameLength > 0 && nameLength <= MAX_NAME)
                    {
                        strncpy(name, start, nameLength);
                        name[nameLength] = '\0';

                        end++;

                        while (*end == ' ')
                            end++;

                        if (*end == '"')
                        {
                            end++;

                            char *closingQuote = strrchr(end, '"');

                            if (closingQuote != NULL)
                            {
                                *closingQuote = '\0';
                                writeFile(name, end);
                            }
                            else
                            {
                                printf("Content must end with a quote.\n");
                            }
                        }
                        else
                        {
                            printf("Use double quotes around content.\n");
                        }
                    }
                    else
                    {
                        printf("Invalid filename.\n");
                    }
                }
                else
                {
                    printf("Usage: write filename \"content\"\n");
                }
            }
            else
            {
                printf("Usage: write filename \"content\"\n");
            }
        }

        else if (strcmp(command, "read") == 0)
        {
            if (sscanf(input, "%19s %50s", command, name) == 2)
                readFile(name);
            else
                printf("Usage: read filename\n");
        }

        else if (strcmp(command, "delete") == 0)
        {
            if (sscanf(input, "%19s %50s", command, name) == 2)
                deleteFile(name);
            else
                printf("Usage: delete filename\n");
        }

        else if (strcmp(command, "rmdir") == 0)
        {
            if (sscanf(input, "%19s %50s", command, name) == 2)
                removeDirectory(name);
            else
                printf("Usage: rmdir directory_name\n");
        }

        else if (strcmp(command, "ls") == 0)
        {
            listDirectory();
        }

        else if (strcmp(command, "cd") == 0)
        {
        if (sscanf(input, "%19s %50s", command, name) == 2)
            changeDirectory(name);
        else
            printf("Usage: cd directory_name\n");
        }

        else if (strcmp(command, "pwd") == 0)
        {
            printPath(cwd);
            printf("\n");
        }

        else if (strcmp(command, "df") == 0)
        {
            diskUsage();
        }

        else
        {
            printf("Invalid command.\n");
        }
    }

    return 0;
}
