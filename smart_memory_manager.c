#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POOL_SIZE 1000000

/* 
 * STRUCTURE: MemoryBlock
 * Represents a single contiguous region of memory within our simulated pool.
 * It can be either an allocated region or a free region.
 */
struct MemoryBlock {
    int start_offset; // The starting byte index of this block within the memory pool
    int size_bytes;   // The size of this block in bytes
    int alloc_id;     // Unique identifier for this allocation (0 if the block is free)
    int is_allocated; // Status flag: 0 means Free, 1 means Allocated
};

// GLOBAL VARIABLES 

// Array storing the metadata for all blocks (both free and allocated)
struct MemoryBlock *block_metadata_array = NULL;

// The actual byte buffer simulating our physical memory
unsigned char *simulated_memory_pool = NULL;

// Tracks how many blocks (free + allocated) currently exist in the metadata array
int current_block_count = 1;

// Tracks the maximum number of blocks the metadata array can currently hold before needing a resize
int metadata_array_capacity = 2;

// The total size of the simulated memory pool in bytes (provided by user at startup)
int total_pool_size_bytes;

// An auto-incrementing counter to assign unique IDs to new successful allocations
int next_allocation_id = 1;

// Defines how the program searches for free blocks: 1 = First Fit, 2 = Best Fit, 3 = Worst Fit
int current_allocation_strategy = 1; 


// Expands the capacity of the block_metadata_array if we need to add more blocks
int ensureCapacity(int needed_blocks) {
    if (needed_blocks <= metadata_array_capacity)
        return 1; // We already have enough space

    int new_capacity = metadata_array_capacity * 2;
    while (new_capacity < needed_blocks)
        new_capacity *= 2;

    struct MemoryBlock *temp = realloc(block_metadata_array, new_capacity * sizeof(struct MemoryBlock));
    if (temp == NULL)
        return 0; // Reallocation failed

    block_metadata_array = temp;
    metadata_array_capacity = new_capacity;
    return 1;
}

// Scans the metadata array to find a suitable free block based on the current strategy
int findBlock(int requested_size) {
    int selected_index = -1;

    for (int i = 0; i < current_block_count; i++) {
        // Check if the block is free and large enough to hold the requested size
        if (block_metadata_array[i].is_allocated == 0 && block_metadata_array[i].size_bytes >= requested_size) {
            
            // First Fit: Pick the very first suitable block we find
            if (current_allocation_strategy == 1) { 
                return i;
            }

            if (selected_index == -1) {
                selected_index = i; // Initial match
            } 
            // Best Fit: Pick the smallest suitable free block
            else if (current_allocation_strategy == 2 && block_metadata_array[i].size_bytes < block_metadata_array[selected_index].size_bytes) { 
                selected_index = i;
            } 
            // Worst Fit: Pick the largest available free block
            else if (current_allocation_strategy == 3 && block_metadata_array[i].size_bytes > block_metadata_array[selected_index].size_bytes) { 
                selected_index = i;
            }
        }
    }
    return selected_index; // Returns -1 if no suitable block was found
}

// Scans for neighboring free blocks and merges them into a single larger free block
void mergeBlocks() {
    int i = 0;
    while (i < current_block_count - 1) {
        // If the current block and the next block are both free...
        if (block_metadata_array[i].is_allocated == 0 && block_metadata_array[i + 1].is_allocated == 0) {
            
            // Absorb the size of the next block into the current block
            block_metadata_array[i].size_bytes += block_metadata_array[i + 1].size_bytes;

            // Shift all subsequent blocks left by one position to remove the absorbed block
            for (int j = i + 1; j < current_block_count - 1; j++)
                block_metadata_array[j] = block_metadata_array[j + 1];

            current_block_count--;
            // We do NOT increment 'i' here because the newly merged block might 
            // be adjacent to another free block, so we check this same index again.
        } else {
            i++;
        }
    }
}

// Handles user requests to allocate memory
void allocateMemory() {
    int requested_size;
    printf("Enter requested size: ");
    if (scanf("%d", &requested_size) != 1) {
        printf("Invalid input.\n");
        return;
    }

    if (requested_size <= 0 || requested_size > total_pool_size_bytes) {
        printf("Invalid allocation size.\n");
        return;
    }

    int target_index = findBlock(requested_size);

    // If no block could satisfy the request, check for external fragmentation
    if (target_index == -1) {
        int total_free_memory = 0;
        printf("Allocation failed.\n");

        for (int i = 0; i < current_block_count; i++) {
            if (block_metadata_array[i].is_allocated == 0)
                total_free_memory += block_metadata_array[i].size_bytes;
        }

        // External fragmentation happens when total free space is enough, but not contiguous
        if (total_free_memory >= requested_size) {
            printf("External fragmentation detected.\n");
            printf("Available free blocks: ");
            for (int i = 0; i < current_block_count; i++) {
                if (block_metadata_array[i].is_allocated == 0)
                    printf("%d ", block_metadata_array[i].size_bytes);
            }
            printf("\n");
        } else {
            printf("Insufficient free memory.\n");
        }
        return;
    }

    // Calculate how much space is left over in the chosen block after allocation
    int leftover_space = block_metadata_array[target_index].size_bytes - requested_size;

    // If we need to split the block, ensure the metadata array has room for a new entry
    if (leftover_space > 0 && !ensureCapacity(current_block_count + 1)) {
        printf("Metadata allocation failed.\n");
        return;
    }

    int block_start_offset = block_metadata_array[target_index].start_offset;

    // Split the block: Create a new free block representing the leftover space
    if (leftover_space > 0) {
        // Shift blocks right to make room for the new leftover free block
        for (int i = current_block_count; i > target_index + 1; i--)
            block_metadata_array[i] = block_metadata_array[i - 1];

        block_metadata_array[target_index + 1].start_offset = block_start_offset + requested_size;
        block_metadata_array[target_index + 1].size_bytes = leftover_space;
        block_metadata_array[target_index + 1].alloc_id = 0;
        block_metadata_array[target_index + 1].is_allocated = 0;

        current_block_count++;
    }

    // Update the chosen block's metadata to reflect the allocation
    block_metadata_array[target_index].size_bytes = requested_size;
    block_metadata_array[target_index].alloc_id = next_allocation_id;
    block_metadata_array[target_index].is_allocated = 1;

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n", next_allocation_id, block_start_offset, requested_size);
    next_allocation_id++;
}

// Handles user requests to free previously allocated memory
void releaseMemory() {
    int target_id, found_index = -1;
    printf("Enter allocation ID: ");
    if (scanf("%d", &target_id) != 1) {
        printf("Invalid input.\n");
        return;
    }

    // Search for the block with the matching allocation ID
    for (int i = 0; i < current_block_count; i++) {
        if (block_metadata_array[i].is_allocated == 1 && block_metadata_array[i].alloc_id == target_id) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        printf("Invalid allocation ID.\n");
        return;
    }

    // Mark the block as free and remove its ID
    block_metadata_array[found_index].is_allocated = 0;
    block_metadata_array[found_index].alloc_id = 0;
    
    // Combining this newly freed block with any neighbors
    mergeBlocks();
    
    printf("Allocation released successfully.\n");
}

// Adjusts the size of an existing allocation
void resizeMemory() {
    int target_id, new_requested_size, target_index = -1;

    printf("Enter allocation ID: ");
    if (scanf("%d", &target_id) != 1) {
        printf("Invalid input.\n");
        return;
    }

    printf("Enter new size: ");
    if (scanf("%d", &new_requested_size) != 1) {
        printf("Invalid input.\n");
        return;
    }

    if (new_requested_size <= 0 || new_requested_size > total_pool_size_bytes) {
        printf("Invalid size.\n");
        return;
    }

    // Locate the block
    for (int i = 0; i < current_block_count; i++) {
        if (block_metadata_array[i].is_allocated == 1 && block_metadata_array[i].alloc_id == target_id) {
            target_index = i;
            break;
        }
    }

    if (target_index == -1) {
        printf("Invalid allocation ID.\n");
        return;
    }

    int current_size = block_metadata_array[target_index].size_bytes;

    if (new_requested_size == current_size) {
        printf("Size is unchanged.\n");
        return;
    }

    // CASE 1: Shrinking the block in place
    if (new_requested_size < current_size) {
        int freed_space = current_size - new_requested_size;

        if (!ensureCapacity(current_block_count + 1)) {
            printf("Metadata allocation failed.\n");
            return;
        }

        // Shift blocks right to insert the newly freed space as its own block
        for (int i = current_block_count; i > target_index + 1; i--)
            block_metadata_array[i] = block_metadata_array[i - 1];

        // Shrink original block
        block_metadata_array[target_index].size_bytes = new_requested_size;

        // Initialize the new free block
        block_metadata_array[target_index + 1].start_offset = block_metadata_array[target_index].start_offset + new_requested_size;
        block_metadata_array[target_index + 1].size_bytes = freed_space;
        block_metadata_array[target_index + 1].alloc_id = 0;
        block_metadata_array[target_index + 1].is_allocated = 0;

        current_block_count++;
        mergeBlocks();
        printf("Allocation resized successfully.\n");
        return;
    }

    // CASE 2: Expanding into an adjacent free block
    int needed_extra_space = new_requested_size - current_size;
    
    // Check if the block immediately after is free and large enough
    if (target_index + 1 < current_block_count && 
        block_metadata_array[target_index + 1].is_allocated == 0 && 
        block_metadata_array[target_index + 1].size_bytes >= needed_extra_space) {
        
        block_metadata_array[target_index].size_bytes = new_requested_size;
        block_metadata_array[target_index + 1].start_offset += needed_extra_space;
        block_metadata_array[target_index + 1].size_bytes -= needed_extra_space;

        // If we consumed the entire free block perfectly, remove it from metadata
        if (block_metadata_array[target_index + 1].size_bytes == 0) {
            for (int i = target_index + 1; i < current_block_count - 1; i++)
                block_metadata_array[i] = block_metadata_array[i + 1];
            current_block_count--;
        }

        printf("Allocation expanded in place.\n");
        return;
    }

    // CASE 3: Move allocation to another entirely new free block
    int old_start_offset = block_metadata_array[target_index].start_offset;
    int new_block_index = findBlock(new_requested_size);

    if (new_block_index == -1 || new_block_index == target_index) {
        printf("Resize failed. No suitable free block to relocate data to.\n");
        return;
    }

    int new_start_offset = block_metadata_array[new_block_index].start_offset;
    int leftover_space_in_new_block = block_metadata_array[new_block_index].size_bytes - new_requested_size;

    if (leftover_space_in_new_block > 0 && !ensureCapacity(current_block_count + 1)) {
        printf("Metadata allocation failed.\n");
        return;
    }

    // Move the simulated physical data safely
    memmove(simulated_memory_pool + new_start_offset, simulated_memory_pool + old_start_offset, current_size);

    // Split the new location if it was larger than we needed
    if (leftover_space_in_new_block > 0) {
        for (int i = current_block_count; i > new_block_index + 1; i--)
            block_metadata_array[i] = block_metadata_array[i - 1];

        block_metadata_array[new_block_index + 1].start_offset = new_start_offset + new_requested_size;
        block_metadata_array[new_block_index + 1].size_bytes = leftover_space_in_new_block;
        block_metadata_array[new_block_index + 1].alloc_id = 0;
        block_metadata_array[new_block_index + 1].is_allocated = 0;
        current_block_count++;
    }

    // Setup the new relocated block
    block_metadata_array[new_block_index].size_bytes = new_requested_size;
    block_metadata_array[new_block_index].alloc_id = target_id;
    block_metadata_array[new_block_index].is_allocated = 1;

    // Release the old region where the data used to be
    for (int i = 0; i < current_block_count; i++) {
        if (block_metadata_array[i].is_allocated == 1 && block_metadata_array[i].alloc_id == target_id && i != new_block_index) {
            block_metadata_array[i].is_allocated = 0;
            block_metadata_array[i].alloc_id = 0;
            break;
        }
    }

    mergeBlocks();
    printf("Allocation moved and resized successfully.\n");
}

// Iterates through the metadata array and prints the current status of the pool
void displayMap() {
    printf("\nMemory Map (%d bytes):\n", total_pool_size_bytes);
    for (int i = 0; i < current_block_count; i++) {
        if (block_metadata_array[i].is_allocated == 1) {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", 
                   block_metadata_array[i].alloc_id, 
                   block_metadata_array[i].size_bytes, 
                   block_metadata_array[i].start_offset);
        } else {
            printf("[Free: %d bytes, Start: %d]\n", 
                   block_metadata_array[i].size_bytes, 
                   block_metadata_array[i].start_offset);
        }
    }
}

// Calculates and prints memory usage statistics
void displayStatistics() {
    int total_allocated_bytes = 0;
    int total_free_bytes = 0;
    int allocated_block_count = 0;
    int free_block_count = 0;
    int largest_free_block_size = 0;

    for (int i = 0; i < current_block_count; i++) {
        if (block_metadata_array[i].is_allocated == 1) {
            total_allocated_bytes += block_metadata_array[i].size_bytes;
            allocated_block_count++;
        } else {
            total_free_bytes += block_metadata_array[i].size_bytes;
            free_block_count++;
            if (block_metadata_array[i].size_bytes > largest_free_block_size)
                largest_free_block_size = block_metadata_array[i].size_bytes;
        }
    }

    printf("\nTotal memory: %d bytes\n", total_pool_size_bytes);
    printf("Allocated: %d bytes\n", total_allocated_bytes);
    printf("Free: %d bytes\n", total_free_bytes);
    printf("Allocated blocks: %d\n", allocated_block_count);
    printf("Free blocks: %d\n", free_block_count);
    printf("Largest free block: %d bytes\n", largest_free_block_size);
}

// Allows the user to switch between First Fit, Best Fit, and Worst Fit algorithms
void changeStrategy() {
    int choice;
    printf("\n1. First Fit\n");
    printf("2. Best Fit\n");
    printf("3. Worst Fit\n");
    printf("Choose strategy: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        return;
    }

    if (choice >= 1 && choice <= 3) {
        current_allocation_strategy = choice;
        printf("Allocation strategy changed successfully.\n");
    } else {
        printf("Invalid strategy.\n");
    }
}

int main() {
    printf("Enter memory pool size: ");

    if (scanf("%d", &total_pool_size_bytes) != 1 || total_pool_size_bytes < 1 || total_pool_size_bytes > MAX_POOL_SIZE) {
        printf("Invalid pool size.\n");
        return 1;
    }

    // Allocate the simulated physical memory
    simulated_memory_pool = malloc(total_pool_size_bytes);
    if (simulated_memory_pool == NULL) {
        printf("Memory pool allocation failed.\n");
        return 1;
    }

    // Allocate the metadata tracking array
    block_metadata_array = malloc(metadata_array_capacity * sizeof(struct MemoryBlock));
    if (block_metadata_array == NULL) {
        printf("Metadata allocation failed.\n");
        free(simulated_memory_pool);
        return 1;
    }

    // Initialize the pool as one single massive free block
    block_metadata_array[0].start_offset = 0;
    block_metadata_array[0].size_bytes = total_pool_size_bytes;
    block_metadata_array[0].alloc_id = 0;
    block_metadata_array[0].is_allocated = 0;

    printf("\nSelect allocation strategy:\n");
    printf("1. First Fit\n");
    printf("2. Best Fit\n");
    printf("3. Worst Fit\n");
    printf("Choice: ");

    if (scanf("%d", &current_allocation_strategy) != 1 || current_allocation_strategy < 1 || current_allocation_strategy > 3) {
        printf("Invalid strategy. Defaulting to First Fit.\n");
        current_allocation_strategy = 1; 
    }

    int user_menu_choice;
    do {
        printf("\n========== SMART MEMORY MANAGER ==========\n");
        printf("1. Allocate memory\n");
        printf("2. Release memory\n");
        printf("3. Resize allocation\n");
        printf("4. Display memory map\n");
        printf("5. Display memory statistics\n");
        printf("6. Change allocation strategy\n");
        printf("7. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &user_menu_choice) != 1) {
            printf("Invalid input.\n");
            break;
        }

        switch (user_menu_choice) {
            case 1: allocateMemory(); break;
            case 2: releaseMemory(); break;
            case 3: resizeMemory(); break;
            case 4: displayMap(); break;
            case 5: displayStatistics(); break;
            case 6: changeStrategy(); break;
            case 7: printf("Exiting Memory Manager.\n"); break;
            default: printf("Invalid menu choice.\n");
        }

    } while (user_menu_choice != 7);

    // Free all dynamically allocated memory before exiting to prevent memory leaks
    free(simulated_memory_pool);
    free(block_metadata_array);
    return 0;
}