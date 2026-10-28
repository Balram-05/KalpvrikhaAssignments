#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Helper function to swap values by manipulating their addresses (pointers)
void swap_via_pointers(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to print the matrix using only pointer arithmetic
void print_matrix(int *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // Equivalent to matrix[i][j]
            printf("%d ", *(matrix + i * n + j));
        }
        printf("\n");
    }
}

int main() {

    // 'n' represents the dimensions of the grid. 
    // Since the matrix is N x N, 'n' is both the total number of rows and columns.
    int n;
    printf("Enter matrix size (2-10): \n");
    if (scanf("%d", &n) != 1 || n < 2 || n > 10) {
        printf("Invalid input. Please enter a number between 2 and 10.\n");
        return 1;
    }

    // Seed the random number generator
    srand(time(NULL));

    // Allocate a contiguous 1D block of memory to represent the NxN matrix
    int *matrix = (int *)malloc(n * n * sizeof(int));
    if (matrix == NULL) return 1;

    // 1. Generate Matrix
    for (int i = 0; i < n * n; i++) {
        *(matrix + i) = rand() % 256; // Values between 0 and 255
    }

    printf("Original Randomly Generated Matrix:\n");
    print_matrix(matrix, n);
    printf("\n");

    // 2. Rotate Matrix 90° Clockwise (In-Place)
    // Step A: Transpose the matrix
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            // Pass the pointers (addresses) of the elements to swap them
            swap_via_pointers((matrix + i * n + j), (matrix + j * n + i));
        }
    }
    
    // Step B: Reverse each row horizontally
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n / 2; j++) {
            swap_via_pointers((matrix + i * n + j), (matrix + i * n + (n - 1 - j)));
        }
    }

    printf("Matrix after 90 degree Clockwise Rotation:\n");
    print_matrix(matrix, n);
    printf("\n");

    // 3. Apply 3x3 Smoothing Filter (In-Place)
    // Allocate exactly O(N) extra space (2 rows of size N) to hold original values
    int *prev_row = (int *)malloc(n * sizeof(int));
    int *curr_row = (int *)malloc(n * sizeof(int));
    
    if (prev_row == NULL || curr_row == NULL) return 1;

    // 'i' represents the current row index we are iterating through (0 to n-1)
    // 'j' represents the current column index we are iterating through (0 to n-1)

    for (int i = 0; i < n; i++) {
        // Backup the original current row before modifying it
        for (int j = 0; j < n; j++) {
            *(curr_row + j) = *(matrix + i * n + j);
        }

        // Compute smoothed values for the current row
        for (int j = 0; j < n; j++) {
            int sum = 0;
            int count = 0;

            // Iterate over the 3x3 window around (i, j)

            // 'r' (row) and 'c' (column) represent the specific coordinates of the neighbors 
            // inside the 3x3 window centered directly around the current cell (i, j).
            for (int r = i - 1; r <= i + 1; r++) {
                for (int c = j - 1; c <= j + 1; c++) {
                    // Check for valid bounds (edges and corners)
                    if (r >= 0 && r < n && c >= 0 && c < n) {
                        if (r == i - 1) {
                            // Read from the preserved original previous row
                            sum += *(prev_row + c);
                        } else if (r == i) {
                            // Read from the preserved original current row
                            sum += *(curr_row + c);
                        } else {
                            // Read directly from the matrix (next row is untouched yet)
                            sum += *(matrix + r * n + c);
                        }
                        count++;
                    }
                }
            }
            
            // Overwrite matrix via pointer arithmetic
            *(matrix + i * n + j) = sum / count;
        }

        // Copy original curr_row into prev_row for the next loop iteration
        for (int j = 0; j < n; j++) {
            *(prev_row + j) = *(curr_row + j);
        }
    }

    printf("Matrix after Applying 3x3 Smoothing Filter:\n");
    print_matrix(matrix, n);

    // Free dynamic memory
    free(prev_row);
    free(curr_row);
    free(matrix);

    return 0;
}