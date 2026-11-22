/* Renaming the header's Player type so we can define our own Player struct. */
#define Player HeaderPlayer
#include "Players_data.h"
#undef Player

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 50
#define TEAM_COUNT 10
#define MAX_PLAYERS_PER_TEAM 50
#define NO_LIMIT 1000000000
#define LINE "-------------------------------------------------------------------------------------------------------------\n"

typedef enum { BATSMAN = 1, BOWLER = 2, ALL_ROUNDER = 3 } Role;

typedef struct {
    int playerId;
    char name[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    Role role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} Player;

typedef struct PlayerNode {
    Player data;
    struct PlayerNode *next;
} PlayerNode;

/* Each sorted node points to a player in the main team linked list. */
typedef struct SortedNode {
    PlayerNode *player;
    struct SortedNode *next;
} SortedNode;

typedef struct {
    int teamId;
    char name[MAX_NAME_LENGTH + 1];
    int totalPlayers;
    float averageBattingStrikeRate;

    PlayerNode *players;
    PlayerNode *playersTail;

    /* Index 1 = Batsmen, 2 = Bowlers, 3 = All-rounders. */
    SortedNode *roleHeads[4];
} Team;

/* Temporary nodes used while merging role lists across all teams. */
typedef struct OutputNode {
    Player *player;
    struct OutputNode *next;
} OutputNode;

static Team teamList[TEAM_COUNT];

/* ---------- Basic helper functions ---------- */

const char *roleName(Role role) {
    switch (role) {
        case BATSMAN: return "Batsman";
        case BOWLER: return "Bowler";
        case ALL_ROUNDER: return "All-rounder";
        default: return "Unknown";
    }
}

void clearInputLine(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* Discard remaining characters. */
    }
}

/* Reads an integer between min and max. Returns 1 on success, 0 otherwise. */
int readInt(const char *prompt, int *value, int min, int max) {
    printf("%s", prompt);

    if (scanf("%d", value) != 1) {
        clearInputLine();
        printf("Invalid input. Please enter a number.\n");
        return 0;
    }

    if (*value < min || *value > max) {
        printf("Invalid value.\n");
        return 0;
    }
    return 1;
}

/* Reads a non-negative decimal number. Returns 1 on success, 0 otherwise. */
int readFloat(const char *prompt, float *value) {
    printf("%s", prompt);

    if (scanf("%f", value) != 1 || *value < 0) {
        clearInputLine();
        printf("Invalid input. Please enter a non-negative number.\n");
        return 0;
    }
    return 1;
}

/* Keeps asking until the user enters 1, 2 or 3. */
Role readRole(void) {
    int choice;

    while (!readInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", &choice, BATSMAN, ALL_ROUNDER)) {
        /* Ask again. */
    }
    return (Role)choice;
}

/* Reads a full line (including spaces) and removes the newline. */
void readLine(char *destination, int size) {
    if (fgets(destination, size, stdin) != NULL) {
        size_t length = strlen(destination);

        if (length > 0 && destination[length - 1] == '\n') {
            destination[length - 1] = '\0';
        } else {
            clearInputLine();
        }
    } else {
        destination[0] = '\0';
    }
}

float calculatePerformanceIndex(const Player *player) {
    float battingScore = (player->battingAverage * player->strikeRate) / 100.0f;

    if (player->role == BATSMAN) {
        return battingScore;
    }

    if (player->role == BOWLER) {
        return (player->wickets * 2.0f) + (100.0f - player->economyRate);
    }
    return battingScore + (player->wickets * 2.0f);
}

/* The average includes only Batsmen and All-rounders. Bowlers are excluded. */
void updateTeamStatistics(Team *team) {
    PlayerNode *current = team->players;
    float strikeRateTotal = 0.0f;
    int battingPlayerCount = 0;
    int total = 0;

    while (current != NULL) {
        total++;

        if (current->data.role == BATSMAN || current->data.role == ALL_ROUNDER) {
            strikeRateTotal += current->data.strikeRate;
            battingPlayerCount++;
        }
        current = current->next;
    }

    team->totalPlayers = total;

    if (battingPlayerCount == 0) {
        team->averageBattingStrikeRate = 0.0f;
    } else {
        team->averageBattingStrikeRate = strikeRateTotal / battingPlayerCount;
    }
}

/* ---------- Searching ---------- */

/* Team IDs are 1 to 10 in the same order as the teams[] array, so binary search works on team IDs. O(log n). */
Team *searchTeamById(int teamId) {
    int low = 0, high = TEAM_COUNT - 1;

    while (low <= high) {
        int middle = low + (high - low) / 2;

        if (teamList[middle].teamId == teamId) {
            return &teamList[middle];
        }

        if (teamList[middle].teamId < teamId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return NULL;
}

/* Asks for a team ID and returns the team, or NULL if invalid. */
Team *readTeam(void) {
    int teamId;
    Team *team;

    if (!readInt("Enter Team ID (1-10): ", &teamId, 1, TEAM_COUNT)) {
    return NULL;
    }

    team = searchTeamById(teamId);

    if (team == NULL) {
        printf("Team not found.\n");
    }
    return team;
}

int playerIdExists(int playerId) {
    int i;

    for (i = 0; i < TEAM_COUNT; i++) {
        PlayerNode *current = teamList[i].players;

        while (current != NULL) {
            if (current->data.playerId == playerId) {
                return 1;
            }
            current = current->next;
        }
    }
    return 0;
}

/* ---------- Linked-list insertion ---------- */

int insertPlayer(Team *team, const Player *player) {
    PlayerNode *newPlayer;
    SortedNode *newSortedNode;
    SortedNode **roleHead;
    SortedNode *current;

    /* Allocate both nodes first, so nothing changes if allocation fails. */
    newPlayer = (PlayerNode *)malloc(sizeof(PlayerNode));
    newSortedNode = (SortedNode *)malloc(sizeof(SortedNode));

    if (newPlayer == NULL || newSortedNode == NULL) {
        free(newPlayer);
        free(newSortedNode);
        printf("Memory allocation failed. Player was not added.\n");
        return 0;
    }

    newPlayer->data = *player;
    newPlayer->next = NULL;

    newSortedNode->player = newPlayer;
    newSortedNode->next = NULL;

    /* 1. Append to the main player list. */
    if (team->players == NULL) {
        team->players = newPlayer;
    } else {
        team->playersTail->next = newPlayer;
    }
    team->playersTail = newPlayer;

    /* 2. Insert into the role's performance-index list (descending order). */
    roleHead = &team->roleHeads[player->role];
    current = *roleHead;

    if (current == NULL ||
        current->player->data.performanceIndex < player->performanceIndex) {
        newSortedNode->next = *roleHead;
        *roleHead = newSortedNode;
    } else {
        while (current->next != NULL &&
               current->next->player->data.performanceIndex >= player->performanceIndex) {
            current = current->next;
        }
        newSortedNode->next = current->next;
        current->next = newSortedNode;
    }
    updateTeamStatistics(team);
    return 1;
}

/* ---------- Initialization from Players_data.h ---------- */

void initializeTeams(void) {
    int i;

    /* teamList is static, so counts, lists and heads already start at zero/NULL. */
    for (i = 0; i < TEAM_COUNT; i++) {
        teamList[i].teamId = i + 1;
        strcpy(teamList[i].name, teams[i]);
    }

    for (i = 0; i < playerCount; i++) {
        Player newPlayer = {0};
        int j;

        newPlayer.playerId = players[i].id;
        strncpy(newPlayer.name, players[i].name, MAX_NAME_LENGTH);
        strncpy(newPlayer.teamName, players[i].team, MAX_NAME_LENGTH);
        newPlayer.totalRuns = players[i].totalRuns;
        newPlayer.battingAverage = players[i].battingAverage;
        newPlayer.strikeRate = players[i].strikeRate;
        newPlayer.wickets = players[i].wickets;
        newPlayer.economyRate = players[i].economyRate;

        if (strcmp(players[i].role, "Batsman") == 0) {
            newPlayer.role = BATSMAN;
        } else if (strcmp(players[i].role, "Bowler") == 0) {
            newPlayer.role = BOWLER;
        } else {
            newPlayer.role = ALL_ROUNDER;
        }

        newPlayer.performanceIndex = calculatePerformanceIndex(&newPlayer);

        for (j = 0; j < TEAM_COUNT; j++) {
            if (strcmp(teamList[j].name, players[i].team) == 0) {
                break;
            }
        }

        if (j == TEAM_COUNT) {
            continue;
        }

        if (teamList[j].totalPlayers >= MAX_PLAYERS_PER_TEAM) {
            printf("Team %s is full. Could not load player: %s\n", teamList[j].name, newPlayer.name);
        } else if (!insertPlayer(&teamList[j], &newPlayer)) {
            printf("Could not load player: %s\n", newPlayer.name);
        }
    }
}

/* ---------- Display functions ---------- */

void printPlayerHeader(int includeTeam) {
    printf(LINE);

    if (includeTeam) {
        printf("%-6s %-25s %-16s %-13s %-7s %-7s %-7s %-7s %-7s %-10s\n",
               "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Perf.Index");
    } else {
        printf("%-6s %-25s %-13s %-7s %-7s %-7s %-7s %-7s %-10s\n",
               "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Perf.Index");
    }

    printf(LINE);
}

void printPlayer(const Player *player, int includeTeam) {
    printf("%-6d %-25.25s ", player->playerId, player->name);

    if (includeTeam) {
        printf("%-16.16s ", player->teamName);
    }

    printf("%-13s %-7d %-7.1f %-7.1f %-7d %-7.1f %-10.2f\n",
           roleName(player->role), player->totalRuns, player->battingAverage, player->strikeRate, player->wickets, player->economyRate, player->performanceIndex);
}

/* Requirement 2: display all players of a team. */
void displayTeamPlayers(void) {
    Team *team = readTeam();
    PlayerNode *current;

    if (team == NULL) {
        return;
    }

    printf("\nPlayers of Team %s:\n", team->name);
    printPlayerHeader(0);

    current = team->players;

    if (current == NULL) {
        printf("No players found.\n");
    }

    while (current != NULL) {
        printPlayer(&current->data, 0);
        current = current->next;
    }

    printf(LINE);
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

/* Requirement 3: sort teams by average batting strike rate (descending). */
void displayTeamsByStrikeRate(void) {
    Team *sortedTeams[TEAM_COUNT];
    int i, j;

    for (i = 0; i < TEAM_COUNT; i++) {
        sortedTeams[i] = &teamList[i];
    }

    /* Selection sort in descending order. */
    for (i = 0; i < TEAM_COUNT - 1; i++) {
        int bestIndex = i;

        for (j = i + 1; j < TEAM_COUNT; j++) {
            if (sortedTeams[j]->averageBattingStrikeRate >
                sortedTeams[bestIndex]->averageBattingStrikeRate) {
                bestIndex = j;
            }
        }

        if (bestIndex != i) {
            Team *temporary = sortedTeams[i];
            sortedTeams[i] = sortedTeams[bestIndex];
            sortedTeams[bestIndex] = temporary;
        }
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("---------------------------------------------------------------\n");
    printf("%-8s %-20s %-15s %-15s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("---------------------------------------------------------------\n");

    for (i = 0; i < TEAM_COUNT; i++) {
        printf("%-8d %-20s %-15.2f %-15d\n", sortedTeams[i]->teamId, sortedTeams[i]->name, sortedTeams[i]->averageBattingStrikeRate, sortedTeams[i]->totalPlayers);
    }
    printf("---------------------------------------------------------------\n");
}

/* Requirement 4: top K players of a team by role. */
void displayTopKPlayers(void) {
    Team *team = readTeam();
    Role role;
    int k;
    int displayed = 0;
    SortedNode *current;

    if (team == NULL) {
        return;
    }

    role = readRole();

    if (!readInt("Enter number of players (K): ", &k, 1, NO_LIMIT)) {
        return;
    }

    printf("\nTop %d %s(s) of Team %s:\n", k, roleName(role), team->name);
    printPlayerHeader(0);

    /* The role list is already sorted, so only K nodes are visited: O(K). */
    current = team->roleHeads[role];

    while (current != NULL && displayed < k) {
        printPlayer(&current->player->data, 0);
        current = current->next;
        displayed++;
    }

    if (displayed == 0) {
        printf("No players of this role found in the team.\n");
    }

    printf(LINE);
    printf("Players displayed: %d\n", displayed);
}

/* ---------- Requirement 5: merge sorted role lists ---------- */

OutputNode *mergeTwoLists(OutputNode *first, OutputNode *second) {
    OutputNode dummy;
    OutputNode *tail = &dummy;

    dummy.next = NULL;

    while (first != NULL && second != NULL) {
        if (first->player->performanceIndex >= second->player->performanceIndex) {
            tail->next = first;
            first = first->next;
        } else {
            tail->next = second;
            second = second->next;
        }
        tail = tail->next;
    }

    tail->next = (first != NULL) ? first : second;
    return dummy.next;
}

/* Divide and conquer: merge pairs of lists, then merge the results. This takes O(N log t), where t is the number of teams. */
OutputNode *mergeTeamLists(OutputNode *listArray[], int left, int right) {
    int middle;
    OutputNode *first;
    OutputNode *second;

    if (left == right) {
        return listArray[left];
    }

    middle = left + (right - left) / 2;
    first = mergeTeamLists(listArray, left, middle);
    second = mergeTeamLists(listArray, middle + 1, right);

    return mergeTwoLists(first, second);
}

void freeOutputList(OutputNode *head) {
    while (head != NULL) {
        OutputNode *temporary = head;
        head = head->next;
        free(temporary);
    }
}

/* Copies one team's sorted role list into temporary output nodes, so the original team lists stay unchanged. */
OutputNode *copyRoleList(SortedNode *source) {
    OutputNode *head = NULL;
    OutputNode *tail = NULL;

    while (source != NULL) {
        OutputNode *newNode = (OutputNode *)malloc(sizeof(OutputNode));

        if (newNode == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }

        newNode->player = &source->player->data;
        newNode->next = NULL;

        if (head == NULL) {
            head = newNode;
        } else {
            tail->next = newNode;
        }
        tail = newNode;
        source = source->next;
    }
    return head;
}

void displayAllPlayersOfRole(void) {
    Role role = readRole();
    OutputNode *teamLists[TEAM_COUNT];
    OutputNode *mergedList;
    OutputNode *current;
    int i;
    int total = 0;

    for (i = 0; i < TEAM_COUNT; i++) {
        teamLists[i] = copyRoleList(teamList[i].roleHeads[role]);
    }

    mergedList = mergeTeamLists(teamLists, 0, TEAM_COUNT - 1);

    printf("\nAll %s Players Across All Teams\n", roleName(role));
    printPlayerHeader(1);

    current = mergedList;

    while (current != NULL) {
        printPlayer(current->player, 1);
        current = current->next;
        total++;
    }

    printf(LINE);
    printf("Total %s players: %d\n", roleName(role), total);
    freeOutputList(mergedList);
}

/* ---------- Requirement 1: add a player ---------- */

void addPlayer(void) {
    Team *team = readTeam();
    Player newPlayer;

    if (team == NULL) {
        return;
    }

    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("This team has reached the maximum limit of 50 players.\n");
        return;
    }

    printf("\nEnter Player Details:\n");

    if (!readInt("Player ID (1-1000): ", &newPlayer.playerId, 1, 1000)) {
        return;
    }

    if (playerIdExists(newPlayer.playerId)) {
        printf("A player with this ID already exists.\n");
        return;
    }

    clearInputLine();

    printf("Name: ");
    readLine(newPlayer.name, sizeof(newPlayer.name));

    if (strlen(newPlayer.name) == 0) {
        printf("Name cannot be empty.\n");
        return;
    }

    strcpy(newPlayer.teamName, team->name);
    newPlayer.role = readRole();

    /* Each read stops at the first invalid value. */
    if (!readInt("Total Runs: ", &newPlayer.totalRuns, 0, NO_LIMIT) ||
        !readFloat("Batting Average: ", &newPlayer.battingAverage) ||
        !readFloat("Strike Rate: ", &newPlayer.strikeRate) ||
        !readInt("Wickets: ", &newPlayer.wickets, 0, NO_LIMIT) ||
        !readFloat("Economy Rate: ", &newPlayer.economyRate)) {
        printf("Player was not added.\n");
        return;
    }

    newPlayer.performanceIndex = calculatePerformanceIndex(&newPlayer);

    if (insertPlayer(team, &newPlayer)) {
        printf("\nPlayer added successfully to Team %s!\n", team->name);
        printf("Calculated Performance Index: %.2f\n", newPlayer.performanceIndex);
    }
}

/* ---------- Memory cleanup ---------- */

void freeAllMemory(void) {
    int i, role;

    for (i = 0; i < TEAM_COUNT; i++) {
        PlayerNode *playerCurrent = teamList[i].players;

        while (playerCurrent != NULL) {
            PlayerNode *temporary = playerCurrent;
            playerCurrent = playerCurrent->next;
            free(temporary);
        }

        for (role = BATSMAN; role <= ALL_ROUNDER; role++) {
            SortedNode *sortedCurrent = teamList[i].roleHeads[role];

            while (sortedCurrent != NULL) {
                SortedNode *temporary = sortedCurrent;
                sortedCurrent = sortedCurrent->next;
                free(temporary);
            }
        }
    }
}

/* ---------- Main menu ---------- */

void displayMenu(void) {
    printf("\n============================================================\n"
           "             ICC ODI PLAYER PERFORMANCE ANALYZER\n"
           "============================================================\n"
           "1. Add Player to Team\n"
           "2. Display Players of a Specific Team\n"
           "3. Display Teams by Average Batting Strike Rate\n"
           "4. Display Top K Players of a Specific Team by Role\n"
           "5. Display All Players of a Specific Role Across All Teams\n"
           "6. Exit\n"
           "============================================================\n");
}

int main(void) {
    int choice = 0;

    (void)teamCount;
    initializeTeams();

    printf("ICC ODI Player Performance Analyzer initialized successfully.\n");
    printf("Loaded %d predefined players across %d teams.\n", playerCount, TEAM_COUNT);

    do {
        displayMenu();

        if (!readInt("Enter your choice: ", &choice, 1, 6)) {
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1: addPlayer(); break;
            case 2: displayTeamPlayers(); break;
            case 3: displayTeamsByStrikeRate(); break;
            case 4: displayTopKPlayers(); break;
            case 5: displayAllPlayersOfRole(); break;
            case 6:
                freeAllMemory();
                printf("Exiting ICC ODI Player Performance Analyzer.\n");
                break;
        }
    } while (choice != 6);

    return 0;
}