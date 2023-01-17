// Testing and rating different battleship attack methods

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>



#define SIZE 10

#define man_input 'm'
#define ran_input 'r'

#define OVERLAP 1
#define CLEAR 0

#define EMPTY 0
#define HIT 1
#define MISS 2
#define SUNK 3

#define WIN 1
#define NO_WIN 0

#define SEEK 0
#define HUNT 1

#define MAX_TURNS 100
#define MAX_STRING 100

#define MISSES_BEFORE_SEEKING 4

enum square {
    empty,
    hit,
    miss
};

enum colour {
    black = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    purple = 35,
    cyan = 36,
    white = 37
};

struct coord {
    int x;
    int y;
};

struct arg_data {
    pthread_t id;
    int result;
};

char print_results = 'b';
char input_mode;
char seek_mode;
char hunt_mode;
int* num_turns;

int adjustment_factor;

int ship_sizes[5] = {2,3,3,4,5};
char ship_short[5] = {'P', 'C', 'S', 'D', 'A'};

//Prototypes
void get_ships(char ship_board[SIZE][SIZE]);
void place_ships(char ship_board[SIZE][SIZE]);
int check_ship_overlap(char board[SIZE][SIZE], struct coord start, int size, struct coord direction);
int game();
void man_game();
struct coord random_attack(char prev_hits[SIZE][SIZE]);
void print_board(char hits[SIZE][SIZE]);
int check_win(char hits[SIZE][SIZE], char ships[SIZE][SIZE]);
void print_ships(char ships[SIZE][SIZE]);
void print_int_board(char hits[SIZE][SIZE], int val[SIZE][SIZE]);
void print_colour(char text[MAX_STRING], enum colour colour);

void *simulator_thread(void *argument);
struct coord seek(char hits[SIZE][SIZE]);
struct coord hunt(char hits[SIZE][SIZE]);

struct coord checkerboard_attack(char prev_hits[SIZE][SIZE]);

struct coord probability_seek(char hits[SIZE][SIZE]);
struct coord combo_seek(char hits[SIZE][SIZE]);

struct coord four_dir_hunt(char hits[SIZE][SIZE]);
struct coord probability_hunt(char hits[SIZE][SIZE]);

int obstructed(char hits[SIZE][SIZE], struct coord start, struct coord end);
void add_probability(int prob[SIZE][SIZE], struct coord start, struct coord end);
void seek_for_ship(char hits[SIZE][SIZE], int prob[SIZE][SIZE], struct coord start, int size);
void hunt_for_ship(char hits[SIZE][SIZE], int prob[SIZE][SIZE], struct coord start, int size);
struct coord find_rand_highest(int prob[SIZE][SIZE]);
void calculate_probability_grid(char hits[SIZE][SIZE], int prob[SIZE][SIZE], int mode);
void calculate_checker_grid(int prob[SIZE][SIZE], int multiplier);
void try_confirmations(char hits[SIZE][SIZE]);
int hit_arrangements(char hits[SIZE][SIZE], int size, struct coord target);
int ship_alive(int prob[SIZE][SIZE], int size, struct coord target);

// ./battleshiptester.c [m/r/f] [r/c/p/m] [n]
//                      input,  attack,   number of tests
int main (int argc, char* argv[]) {
    srand(time(NULL));
    if (argc < 5) {
        fprintf(stderr, "Usage is ./battleshiptester [input mode] [seek mode] [hunt mode] [number of games] [print mode]\n");
        fprintf(stderr, "\ninput_mode: 'r' for randomly generated, automatically run games | 'm' for manual control options\n");
        fprintf(stderr, "\nseek_mode: 'r' for random, 'c' for checkerboard, 'p' for probabilistc, 'b' for combination\n");
        fprintf(stderr, "\nhunt_mode: 'r' for random, '4' for 4-directional, 'p' for probabilistic\n");
        exit(1);
    }
    input_mode = argv[1][0];
    seek_mode = argv[2][0];
    hunt_mode = argv[3][0];
    // printf("input mode: %c\n", input_mode );
    // printf("seek mode: %c\n", seek_mode );
    // printf("hunt mode: %c\n", hunt_mode );
    // First get the type of input for the placed ships
        // Manual input [m]
        // Random generation [r]
        // From file [f] = filename.txt
        // TURNED INTO INTEGER REPRESENTING SHIP / NO-SHIP

        // Optional parameter for number of tests to run [n]
    

    // Then get the type of attack method
        // Random
        // Checkerboard
        // Probabilistic
        // Mixed
    
    // Continually run test
        // 1. Place ships into array
        // 2. Value array using attack method
        // 3. 'Attack' a non-targeted area
        // 4. Set hit/miss
        // Repeat stesp 2-4 until won
        // Return the number of turns
    
    

    int n = 1;
    if (argv[4] != NULL) {
        n = atoi(argv[4]);
    }

    if (argv[5] != NULL) {
        if (argv[5][0] == 'a') {
            print_results = 'a';
        } else {
        }
    }

    if (argv[6] != NULL) {
        adjustment_factor = atoi(argv[6]);
        if (print_results == 'a') {
            printf("ADJUSTMENT_FACTOR = %d\n", adjustment_factor);
        }
    }

    // assert(0 < n && n < 1000);

    num_turns = malloc(sizeof(int) * n);

    if (input_mode == 'm' && n > 1) {
        fprintf(stderr, "Can't perform manual input on more than one game at a time");
        exit(1);
    } else if (input_mode == 'm') {
        man_game();
        exit(1);
    }

    struct arg_data thread_data[n];

    for (int i = 0; i < n; i++) {
        //A thread for each game to simulate
        pthread_create(&thread_data[i].id, NULL, simulator_thread, &thread_data[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(thread_data[i].id, NULL);
        
    }

}

void *simulator_thread(void *argument) {
    struct arg_data *data = argument;

    data->result = game();
    printf("%d\n", data->result);
    return NULL;
}

void man_game() {
    print_results = 'a';
    //DONE manual ship input
    //DONE step number (['s'] [n])
    //DONE finish game ('ctrl-D')
    printf("Manual ship placement? (m) Default is random:");

    char ships[SIZE][SIZE];


    if (getchar() == 'm') {
        place_ships(ships);

    } else {
        get_ships(ships);
    }

    print_ships(ships);


    int turns = 0;
    int curr_mode = SEEK;
    int concurrent_misses = 0;

    char hits[SIZE][SIZE];

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            hits[x][y] = 0;
        }
    }

    int c = 's';
    int step_number = 1;
    while(1) {
        //Value array, pick target
        if (c == 's') {
            if (step_number == 0) {
                step_number = 0;
                c = 0;
            } else if (step_number > 1) {
                // printf("...%d\n", step_number);
                step_number--;
            
            } else {
                step_number = 0;
                c = 0;
            }
        }
        if (c == 0) {
            printf("awaiting input, press ctrl-D to continue to end of game: ");
            scan_char:
            c = getchar();
            // printf("input is: %c\n", c);
            if (c != EOF) {
                if (c == 's') {
                    printf("enter number of steps: ");
                    scanf("%d", &step_number);
                } else if (c == '\n') {
                    goto scan_char;
                } else if (c == 't') {
                    printf("testing confirmation prototype...\n");
                    //Try confirmations
                    try_confirmations(hits);
                    c = 0;
                }
            }
        }
        

        struct coord target;
        if (curr_mode == SEEK) {
            target = seek(hits);
            // printf("Targeted(%d,%d)\n", target.x, target.y);
        } else {
            // hunt mode
            target = hunt(hits);
        }
        
        //Increment turn counter
        turns++;

        //Change state, check win
        if (ships[target.x][target.y] != 0) {
            //Hit
            hits[target.x][target.y] = HIT;
            curr_mode = HUNT;
            concurrent_misses = 0;
        } else {
            hits[target.x][target.y] = MISS;
            concurrent_misses++;
            if (concurrent_misses > MISSES_BEFORE_SEEKING) {
                curr_mode = SEEK;
                concurrent_misses = 0;
            }
            
        }
        if (print_results == 'a') {
            print_board(hits);
        }

        if (check_win(hits, ships) == NO_WIN) {
            if (turns > MAX_TURNS) {
                fprintf(stderr, "Game taking too long. %d turns\n", turns);
                exit(1);
            }
            continue;
        }

        if (print_results == 'a') {
            printf("WIN after %d turns\n", turns);
            print_ships(ships);
        }
        break;

    }


    
}

int game() {
    int turns = 0;
    int curr_mode = SEEK;
    int concurrent_misses = 0;

    char hits[SIZE][SIZE];
    char ships[SIZE][SIZE];
    get_ships(ships);

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            hits[x][y] = 0;
        }
    }

    while(1) {
        //Value array, pick target
        struct coord target;
        if (curr_mode == SEEK) {
            target = seek(hits);
            // printf("Targeted(%d,%d)\n", target.x, target.y);
        } else {
            // hunt mode
            target = hunt(hits);
        }
        
        //Increment turn counter
        turns++;

        

        //Change state, check win
        if (ships[target.x][target.y] != 0) {
            //Hit
            hits[target.x][target.y] = HIT;
            curr_mode = HUNT;
            concurrent_misses = 0;
        } else {
            hits[target.x][target.y] = MISS;
            concurrent_misses++;
            if (concurrent_misses > MISSES_BEFORE_SEEKING) {
                curr_mode = SEEK;
                concurrent_misses = 0;
            }
            
        }
        if (print_results == 'a') {
            print_board(hits);
        }

        if (check_win(hits, ships) == NO_WIN) {
            if (turns > MAX_TURNS) {
                fprintf(stderr, "Game taking too long. %d turns\n", turns);
                exit(1);
            }
            continue;
        }

        if (print_results == 'a') {
            printf("WIN after %d turns\n", turns);
            print_ships(ships);
        }
        break;

    }

    return turns;
}

void get_ships(char ship_board[SIZE][SIZE]) {
    
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            ship_board[x][y] = 0;
        }
    }
    
    // if (input_mode == 'r') {
        //Randomly generate ships
        char ships[5] = {1,1,1,1,1};

        while(1) {

            int ship_picked = rand() % 5; //Random int between 0 and 5

            if (ships[ship_picked] == 0) {
                continue;
            }
            int size = ship_sizes[ship_picked];
            int rand_dir = rand() % 2;
            struct coord rand_coord;
            struct coord direction;

            if (rand_dir == 1) {
                //Going right
                rand_coord.x = rand() % (SIZE - size);
                rand_coord.y = rand() % SIZE;

                direction.x = 1;
                direction.y = 0;
            } else {
                //Going down
                rand_coord.x = rand() % SIZE;
                rand_coord.y = rand() % (SIZE - size);

                direction.x = 0;
                direction.y = 1;
            }

            if (check_ship_overlap(ship_board, rand_coord, size, direction) == CLEAR) {
                ships[ship_picked] = 0;

                int i = 0;
                for (i = 0; i < size; i++) {
                    struct coord test_coord;
                    test_coord.x = rand_coord.x + i*direction.x;
                    test_coord.y = rand_coord.y + i*direction.y;

                    ship_board[test_coord.x][test_coord.y] = ship_short[ship_picked];
                }
            }

            int all_placed = 1;
            for(int i = 0; i < 5; i++) {
                if (ships[i] != 0) {
                    all_placed = 0;
                }
            }
            if (all_placed) {
                break;
            }

        }



    // }

}

void place_ships(char ship_board[SIZE][SIZE]) {
    
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            ship_board[x][y] = 0;
        }
    }

    char ships[5] = {1,1,1,1,1};

    while(1) {
        print_ships(ship_board);

        int ship_picked = -1;
        printf("Ships to place...\n");
        for (int i = 0; i < 5; i++) {
            if (ships[i] != 0) {
                printf("%c - %d\n", ship_short[i], ship_sizes[i]);
            }
        }
        
        //Choose ship
        while (1) {
            printf("Pick ship to place: ");
            char char_picked;
            if (scanf("%c", &char_picked) == EOF) {
                fprintf(stderr, "half-manual, half-automatic placing isnt finished yet!\n");
                exit(1);
            }
            if (char_picked == '\n') {
                scanf("%c", &char_picked);
            }

            for (int i = 0; i < 5; i++) {
                if (ship_short[i] == char_picked) {
                    if (ships[i] == 0) {
                        printf("%c already placed, pick another ship...", ship_short[i]);
                        break;
                    } else {
                        ship_picked = i;
                        break;
                    }
                }
            }
            if (ship_picked != -1) {
                break;
            }
        }
        
        //Place ship
        int size = ship_sizes[ship_picked];

        

        //Choose location
        struct coord location;
        while (1) {
            printf("Pick location [col] [row]: ");

            scanf("%d %d", &location.x, &location.y);

            struct coord direction;
            printf("Pick direction [U/D/R/L]: ");
            while (1) {
                char dir_picked;
                scanf("%c", &dir_picked);
                if (dir_picked == '\n') {
                    scanf("%c", &dir_picked);
                }
                if (dir_picked == 'U') {
                    direction.x = 0;
                    direction.y = -1;
                    break;
                } else if (dir_picked == 'D') {
                    direction.x = 0;
                    direction.y = 1;
                    break;
                } else if (dir_picked == 'R') {
                    direction.x = 1;
                    direction.y = 0;
                    break;
                } else if (dir_picked == 'L') {
                    direction.x = -1;
                    direction.y = 0;
                    break;
                } else {
                    printf("Invalid direction. Choose again\n");
                }
            }

            if (check_ship_overlap(ship_board, location, size, direction) == CLEAR) {
                ships[ship_picked] = 0;

                int i = 0;
                for (i = 0; i < size; i++) {
                    struct coord test_coord;
                    test_coord.x = location.x + i*direction.x;
                    test_coord.y = location.y + i*direction.y;

                    ship_board[test_coord.x][test_coord.y] = ship_short[ship_picked];
                }
                break;
            } else {
                printf("Invalid ship placement. Choose again\n");
            }
        } 
        

        int all_placed = 1;
        for(int i = 0; i < 5; i++) {
            if (ships[i] != 0) {
                all_placed = 0;
            }
        }
        if (all_placed) {
            break;
        }

    }

}

//UTILITY FUNCTIONS - perform a basic calculation based on given data (does not change it)

//Simple check to see if ship's places are already taken
// No need to check bounds, that is done when generating possible ships
int check_ship_overlap(char board[SIZE][SIZE], struct coord start, int size, struct coord direction) {
    int i = 0;
    for (i = 0; i < size; i++) {
        struct coord test_coord;
        test_coord.x = start.x + i*direction.x;
        test_coord.y = start.y + i*direction.y;

        if (board[test_coord.x][test_coord.y] != 0) {
            //Ship already there
            return OVERLAP;
        }
    }
    return CLEAR;
}

//Checks for any non-hit squares that contains ships
int check_win(char hits[SIZE][SIZE], char ships[SIZE][SIZE]) {
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {

        if (ships[x][y] != 0) {
            if (hits[x][y] == HIT) {
                continue;
            } else {
                return NO_WIN;
            }
        }
        }
    }
    return WIN;
}

//Given a grid of probability weights, 
//it either selects one of the first 10 maximum locations, or randomly searches for a max value
struct coord find_rand_highest(int prob[SIZE][SIZE]) {
    //Find maximum value
    int max = 0;
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (prob[x][y] > max) {
                max = prob[x][y];
            }
        }
    }

    //Find up to 4 alternatives
    struct coord max_alt[4];
    int num_alt = 0;

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (prob[x][y] == max) {
                struct coord new_alt;
                new_alt.x = x;
                new_alt.y = y;

                if (num_alt < 4) {
                    max_alt[num_alt] = new_alt;
                    num_alt++;
                } else {
                    //Overrite random alt
                    max_alt[rand() % 4] = new_alt;
                    num_alt++;
                }

                if (num_alt > 10) {
                    //Pick randomly until you get max value
                    while(1) {
                        new_alt.x = rand() % SIZE;
                        new_alt.y = rand() % SIZE;

                        if (prob[new_alt.x][new_alt.y] == max) {
                            return new_alt;
                        }
                    }
                }

            }
        }
    }

    //Pick random from alternatives
    if (num_alt > 4) {
        return max_alt[rand() % 4];
    } else {
        return max_alt[rand() % num_alt];
    }
}

//TWO MAIN STATES

struct coord seek(char hits[SIZE][SIZE]) {
    
    if (seek_mode == 'r') {
        //Random method
        return random_attack(hits);
    } else if (seek_mode == 'c') {
        //Checkerboard method
        return checkerboard_attack(hits);
    } else if (seek_mode == 'p') {
        return probability_seek(hits);
    } else if (seek_mode =='b') {
        return combo_seek(hits);
    }
    return checkerboard_attack(hits);
}

struct coord hunt(char hits[SIZE][SIZE]) {
    if (hunt_mode == 'r') {
        return random_attack(hits);
    } else if (hunt_mode == '4') {
        return four_dir_hunt(hits);
    } else if (hunt_mode == 'p') {
        return probability_hunt(hits);
    } else {
        return random_attack(hits);
    }
}

//SEEKING MODES
//----
struct coord random_attack(char prev_hits[SIZE][SIZE]) {
    
    struct coord rand_coord;

    while(1) {
        rand_coord.x = rand() % SIZE;
        rand_coord.y = rand() % SIZE;

        if (prev_hits[rand_coord.x][rand_coord.y] != 0) {
            continue;
        } else {
            break;
        }
    }
    
    return rand_coord;
}

struct coord checkerboard_attack(char prev_hits[SIZE][SIZE]) {
    struct coord rand_coord;
    int tries = 0;
    while(1) {
        //Generate y coord first.
        rand_coord.y = rand() % SIZE;
        rand_coord.x = rand() % (SIZE / 2);
        //If even or zero, then offset x by 1
        if (rand_coord.y % 2 == 0) {
            rand_coord.x *= 2;
            rand_coord.x += 1;
        } else {
            rand_coord.x *= 2;
        }

        // printf("rand_cord is (%d,%d)\n", rand_coord.x, rand_coord.y);
        if (tries > SIZE * SIZE) {
            //Not possible, return random target instead
            return random_attack(prev_hits);
        }
        if (prev_hits[rand_coord.x][rand_coord.y] != EMPTY) {
            tries++;
            continue;
        } else {
            break;
        }
    }
    
    return rand_coord;
}

struct coord probability_seek(char hits[SIZE][SIZE]) {
    //For every unobstructed square (no miss):
        //Iterate through each ship size
        //Check for an obstruction in each direction
        //If no obstruction add probability in that direction for ship size tested


    int prob[SIZE][SIZE];

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            prob[x][y] = 0;
        }
    }
    
    calculate_probability_grid(hits, prob, SEEK);

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] != EMPTY) {
                prob[x][y] = -1;
            }
        }
    }



    if (print_results == 'a') {
        print_int_board(hits, prob);
    }

    return find_rand_highest(prob);

}

struct coord combo_seek(char hits[SIZE][SIZE]) {
    //For every unobstructed square (no miss):
        //Iterate through each ship size
        //Check for an obstruction in each direction
        //If no obstruction add probability in that direction for ship size tested


    int prob[SIZE][SIZE];

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            prob[x][y] = 0;
        }
    }
    
    calculate_probability_grid(hits, prob, SEEK);
    calculate_checker_grid(prob, adjustment_factor);

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] != EMPTY) {
                prob[x][y] = -1;
            }
        }
    }

    if (print_results == 'a') {
        print_int_board(hits, prob);
    }

    return find_rand_highest(prob);

}


// HUNTING MODES

struct coord four_dir_hunt(char hits[SIZE][SIZE]) {
    //Generate weighting around every prev hit
    int weight[SIZE][SIZE];

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            weight[x][y] = 0;
        }
    }

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] == HIT) {
                if (x > 1) {
                //Weight left
                weight[x - 1][y] += 1;
                }
                if (y > 1) {
                //Weight up
                weight[x][y - 1] += 1;
                }
                if (x < SIZE - 1) {
                //Weight right
                weight[x + 1][y] += 1;
                }
                if (y < SIZE - 1) {
                //Weight down
                weight[x][y + 1] += 1;
                }
            }
        }
    }

    //Find 3 max points
    int max = 0;
    int num_max = 0;
    struct coord max1;
    struct coord max2;
    struct coord max3;

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] == EMPTY) {
                if (weight[x][y] > max) {
                    max = weight[x][y];
                }
            }
        }
    }

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] == EMPTY) {
                if (weight[x][y] == max) {
                    if (num_max == 0) {
                        max1.x = x;
                        max1.y = y;
                        num_max = 1;
                    } else if (num_max == 1) {
                        max2.x = x;
                        max2.y = y;
                        num_max = 2;
                    } else if (num_max == 2) {
                        max3.x = x;
                        max3.y = y;
                        num_max = 3;
                    }
                }
            }
        }
    }

    //Pick a random max target out of up to 3
    if (num_max == 0) {
        //No possible target, seek instead
        return seek(hits);
    }
    int chosen_coord = rand() % num_max;
    if (chosen_coord == 0) {
        return max1;
    } else if (chosen_coord == 1) {
        return max2;
    } else if (chosen_coord == 2) {
        return max3;
    } else {
        return max1;
    }
}

struct coord probability_hunt(char hits[SIZE][SIZE]) {
    int prob[SIZE][SIZE];

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            prob[x][y] = 0;
        }
    }
    
    calculate_probability_grid(hits, prob, HUNT);

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] != EMPTY) {
                prob[x][y] = -1;
            }
        }
    }



    if (print_results == 'a') {
        print_int_board(hits, prob);
    }

    return find_rand_highest(prob);

}


//ASSSISTING ALGORITHMS - used for calculating weights and arrangements
//----

void calculate_probability_grid(char hits[SIZE][SIZE], int prob[SIZE][SIZE], int mode) {
    //Iterate through each ship size
    int ships[5] = {2,3,3,4,5};
    for (int j = 0; j < 5; j++) {
        int size = ships[j];

        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                if (mode == SEEK) {
                    if (hits[x][y] != MISS) {
                        struct coord start;
                        start.x = x;
                        start.y = y;
                        //Check and calculate probability
                        seek_for_ship(hits, prob, start, size);
                    }
                } else {
                    //HUNT
                    if (hits[x][y] == HIT) {
                        struct coord start;
                        start.x = x;
                        start.y = y;
                        //Check and calculate probability
                        hunt_for_ship(hits, prob, start, size);
                    }
                }
                
            }
        }
    }
}

void calculate_checker_grid(int prob[SIZE][SIZE], int multiplier) {
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (prob[x][y] != 0) {
                if ((y % 2 == 1) && (x % 2 == 0)) {
                    prob[x][y] += multiplier;
                } else if ((x % 2 == 1) && (y % 2 == 0)) {
                    prob[x][y] += multiplier;
                }
            }
            
        }
    }
}

void seek_for_ship(char hits[SIZE][SIZE], int prob[SIZE][SIZE], struct coord start, int size) {
    struct coord end;
    //Check up
    end.x = start.x;
    end.y = start.y - (size - 1);
    if (obstructed(hits, start, end) == CLEAR) {
        //Add probability
        add_probability(prob, start, end);
    }

    //Check right
    end.x = start.x + (size - 1);
    end.y = start.y;
    if (obstructed(hits, start, end) == CLEAR) {
        //Add probability
        add_probability(prob, start, end);
    }

    //Check down
    end.x = start.x;
    end.y = start.y + (size - 1);
    if (obstructed(hits, start, end) == CLEAR) {
        //Add probability
        add_probability(prob, start, end);
    }

    //Check left
    end.x = start.x - (size - 1);
    end.y = start.y;
    if (obstructed(hits, start, end) == CLEAR) {
        //Add probability
        add_probability(prob, start, end);
    }
}

void hunt_for_ship(char hits[SIZE][SIZE], int prob[SIZE][SIZE], struct coord start, int size) {
    struct coord end;
    struct coord begin;
    
    //Check vertically, from end.y = start.y - size to end.y = start.y + size
    end.x = start.x;
    begin.x = start.x;
    for (int i = -(size - 1); i <= 0; i++) {
        begin.y = start.y + i;
        end.y = begin.y + (size - 1);

        if (obstructed(hits, begin, end) == CLEAR) {
            //Add probability
            add_probability(prob, begin, end);
        }
    }
    
    //Check horizontally
    end.y = start.y;
    begin.y = start.y;
    for (int i = -(size - 1); i <= 0; i++) {
        begin.x = start.x + i;
        end.x = begin.x + (size - 1);

        if (obstructed(hits, begin, end) == CLEAR) {
            //Add probability
            add_probability(prob, begin, end);
        }
    }
    
}

void add_probability(int prob[SIZE][SIZE], struct coord start, struct coord end) {
    //Iterate from start to end assuming either horizontal or vertical
    if (start.x == end.x) {
        //Iterate vertically
        for (int i = start.y; i != end.y;) {
            prob[start.x][i] += 1;

            if (start.y > end.y) {
                i--;
            } else {
                i++;
            }
        }

    } else if (start.y == end.y){
        //Iterate horizontally
        for (int i = start.x; i != end.x;) {
            prob[i][start.y] += 1;

            if (start.x > end.x) {
                i--;
            } else {
                i++;
            }
        }

    }
    prob[end.x][end.y] += 1;

}

//Searches to find any very-restricted ships using the given information
//i.e. if a ship can only fit one way using hit and miss information, then we know where it is
void try_confirmations(char hits[SIZE][SIZE]) {

    //For each hit calculate the number of ways ships can fit through it and add them to that coordinate
    //If one arrangmenet has all the values equal to 1, then fire there
    //If not, target one arrangement that hass all values equal to 2 etc.
    struct coord target;
    int size;

    int arrangements[5][SIZE][SIZE];
    for (int j = 0; j < 5; j++) {
        for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            arrangements[j][x][y] = 0;
            
        }
        }
    }
    
    // Get arrangements per ship
    for (int j = 0; j < 5; j++) {
        size = ship_sizes[j];

        for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] == HIT) {
                target.x = x;
                target.y = y;
                arrangements[j][x][y] = hit_arrangements(hits, size, target);
            }
            
        }
        }
    }
    int ships_alive[5] = {1,1,1,1,1};
    // Confirm each ship, starting with smallest
    for (int j = 0; j < 5; j++) {
        size = ship_sizes[j];

        //Check the smallest smaller ship alive
        if (j > 0) {
            for (int j2 = 0; j2 < j; j2++) {
                if (ships_alive[j2] == 1) {
                    //Factor the smaller ship's possible arrangements
                    for (int x = 0; x < SIZE; x++) {
                        for (int y = 0; y < SIZE; y++) {
                        //If arrangement_smaller_ship > 2 then consider arrangement_this_ship = 0
                            if (arrangements[j2][x][y] > 2) {
                                arrangements[j][x][y] = 0;
                            }
                        }
                    }
                    break;
                }
            }
        }


        for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (hits[x][y] == HIT) {
                target.x = x;
                target.y = y;
                if (ship_alive(arrangements[j], size, target) == 0) {
                    ships_alive[j] = 0;
                    goto ship_state_checked;
                }
            }
        }
        }

        ship_state_checked:
        print_int_board(hits, arrangements[j]);
    }
    
    

    //If an arrangement that is all 1s has been found and been completely hit, update 

}

int hit_arrangements(char hits[SIZE][SIZE], int size, struct coord target) {
    struct coord begin;
    struct coord end;
    
    int arrangements = 0;
    
    //Check vertically, from end.y = start.y - size to end.y = start.y + size
    end.x = target.x;
    begin.x = target.x;
    for (int i = -(size - 1); i <= 0; i++) {
        begin.y = target.y + i;
        end.y = begin.y + (size - 1);

        if (obstructed(hits, begin, end) == CLEAR) {
            arrangements++;
        }
    }
    
    //Check horizontally
    end.y = target.y;
    begin.y = target.y;
    for (int i = -(size - 1); i <= 0; i++) {
        begin.x = target.x + i;
        end.x = begin.x + (size - 1);

        if (obstructed(hits, begin, end) == CLEAR) {
            arrangements++;
        }
    }

    return arrangements;
}

int ship_alive(int prob[SIZE][SIZE], int size, struct coord target) {
    struct coord begin;
    struct coord end;
    
    //Return 1 if not able to confirm sink, 0 if ship is definitely sunk
    
    //Check vertically, from end.y = start.y - size to end.y = start.y + size
    end.x = target.x;
    begin.x = target.x;
    int i,j;
    for (i = -(size - 1); i <= 0; i++) {
        begin.y = target.y + i;
        end.y = begin.y + (size - 1);

        if (0 > end.x || end.x >= SIZE) {
            return 1;
        } else if (0 > begin.x || begin.x >= SIZE) {
            return 1;
        }
        if (0 > end.y || end.y >= SIZE) {
            return 1;
        } else if (0 > begin.y || begin.y >= SIZE) {
            return 1;
        }

        
        //Iterate vertically
        for (j = begin.y; j != end.y;) {
            
            if (prob[begin.x][j] != 1) {
                return 1;
            }

            if (begin.y > end.y) {
                j--;
            } else {
                j++;
            }
        }
        
    }
    
    //Check horizontally
    end.y = target.y;
    begin.y = target.y;
    for (i = -(size - 1); i <= 0; i++) {
        begin.x = target.x + i;
        end.x = begin.x + (size - 1);

        if (0 > end.x || end.x >= SIZE) {
            return 1;
        } else if (0 > begin.x || begin.x >= SIZE) {
            return 1;
        }
        if (0 > end.y || end.y >= SIZE) {
            return 1;
        } else if (0 > begin.y || begin.y >= SIZE) {
            return 1;
        }

        //Iterate horizontally
        for (j = begin.x; j != end.x;) {
            
            if (prob[j][begin.y] != 1) {
                return 1;
            }

            if (begin.x > end.x) {
                j--;
            } else {
                j++;
            }
        }
    }

    //Able to confirm ship is sunk
    return 0;
}

int obstructed(char hits[SIZE][SIZE], struct coord start, struct coord end) {
    //Iterate through from start coord to end coord
    //Check for bounds of grid first
    if (0 > end.x || end.x >= SIZE) {
        return OVERLAP;
    } else if (0 > start.x || start.x >= SIZE) {
        return OVERLAP;
    }
    if (0 > end.y || end.y >= SIZE) {
        return OVERLAP;
    } else if (0 > start.y || start.y >= SIZE) {
        return OVERLAP;
    }

    //Iterate to find any obstructions
    if (start.x == end.x) {
        //Iterate vertically
        for (int i = start.y; i != end.y;) {
            
            if (hits[start.x][i] == MISS) {
                return OVERLAP;
            }

            if (start.y > end.y) {
                i--;
            } else {
                i++;
            }
        }
    } else if (start.y == end.y){
        //Iterate horizontally
        for (int i = start.x; i != end.x;) {
            
            if (hits[i][start.y] == MISS) {
                return OVERLAP;
            }

            if (start.x > end.x) {
                i--;
            } else {
                i++;
            }
        }
    }
    if (hits[end.x][end.y] == MISS) {
        return OVERLAP;
    }

    //Otherwise unobstructed
    return CLEAR;
}





//PRINTING FUNCTIONS
//---
void print_colour(char text[MAX_STRING], enum colour colour) {
    printf("\033[1;%dm", colour);
    fputs(text, stdout);
    printf("\033[0m");
}

void print_board(char hits[SIZE][SIZE]) {
    printf("\\ 00 01 02 03 04 05 06 07 08 09\n");
    for (int y = 0; y < SIZE; y++) {
        printf("%c ", (y + 'A'));
        for (int x = 0; x < SIZE; x++) {
            if (hits[x][y] == EMPTY) {
                print_colour("-- ", cyan);
            } else if (hits[x][y] == MISS) {
                print_colour("OO ", blue);
            } else if (hits[x][y] == HIT) {
                print_colour("XX ", red);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_ships(char ships[SIZE][SIZE]) {
    printf("\\ 00 01 02 03 04 05 06 07 08 09\n");
    for (int y = 0; y < SIZE; y++) {
        printf("%c ", (y + 'A'));
        for (int x = 0; x < SIZE; x++) {
            if (ships[x][y] != 0) {
                printf("-%c ", ships[x][y]);
            } else {
                printf("-- ");
            }
        }
        printf("%d\n", y);
    }
    printf("\n");
}

void print_int_board(char hits[SIZE][SIZE], int val[SIZE][SIZE]) {
    printf("\\ 00 01 02 03 04 05 06 07 08 09\n");
    for (int y = 0; y < SIZE; y++) {
        printf("%c ", (y + 'A'));
        for (int x = 0; x < SIZE; x++) {
            if (hits[x][y] == EMPTY || val[x][y] > 0) {
                printf("%2d ", val[x][y]);
            } else {
                if (hits[x][y] == MISS) {
                    print_colour("OO ", blue);
                } else if (hits[x][y] == HIT) {
                    print_colour("XX ", red);
                } else {
                    print_colour("-- ", cyan);
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}