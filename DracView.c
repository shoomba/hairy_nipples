// DracView.c ... DracView ADT implementation
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
#include "List.h"
#include "Map.h" //... if you decide to use the Map ADT

#define PLAY_LENGTH 8

typedef struct dracula {
    int health;
    
    List locations;
    List trail;

    int canHide;
    int canDoubleBack;
} dracula_t;

typedef struct hunter {
    int health;
    int location;
    List trail;
} hunter_t;

struct dracView { 
    dracula_t dracula;
    hunter_t hunters[NUM_PLAYERS - 1];

    int score;
    int turn;
    int round;

    int immVampLoc;
    int numTraps[NUM_MAP_LOCATIONS];

    // this is to help with game simulations
    struct dracView * prevState;

    Map m;
    int ** mDistances;
};

typedef struct dracPlay {
    char    identifier;
    char    location[2];
    char    encounters[2];
    char    action;
    char    buffer[2];
} dracPlay; 

typedef struct hunterPlay {
    char    identifier;
    char    location[2];
    char    encounters[4];
    char    buffer;
} hunterPlay; 

static LocationID getLocationFromPlay (dracPlay * play);
static void setDraculaMinionInfo (DracView dracView, char * pastPlays);
static void setDraculaTrailLocations (DracView dracView, char * pastPlays);
static void setupDracula (DracView dracView, GameView gameView, char * pastPlays);
static void setupHunters (DracView dracView, GameView gameView, char * pastPlays);

int mDistance (DracView d, int i, int j) {
    return d->mDistances[i][j];
}

int getTrailMove (DracView state, int player, int n) {
    if (player == PLAYER_DRACULA) {
        return ListGet (state->dracula.locations, n);
    } else {
        return ListGet (state->hunters[player].trail, n);
    }
}

DracView simulateMove (DracView prevState, int move) {
    DracView newState = malloc (sizeof (struct dracView));
     //printf ("YOYOM: %d\n", move);
    // IMPORTANT: all trails, which are implemented as lists, are copied
    // by reference. The idea is that we won't ever remove moves from the trail
    // only add them. after this function is called, the new additions will be 
    // removed from the list (effectively resetting them). Everything else (except
    // map details, but we never touch it anyhoo) is copied over, so changes we make
    // wont affect previous states.
    *newState = *prevState;
    newState->prevState = prevState;

    
    if (newState->turn == PLAYER_DRACULA) {
        // add the latest move to draculas trail.
        ListInsertFront (newState->dracula.trail, move);

        // if the move is hide, then dracula's location is the same as his previous one.
        // if the move is double back, then dracula's location is the same as his location n turns ago.
        // if the move is teleport, dracula's location is castle_dracula
        int newLocation;
        if (move == HIDE) {
            newLocation = ListGet (newState->dracula.locations, 0);
            newState->dracula.canHide = FALSE;
        } else if (move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5) {
            int n = move - DOUBLE_BACK_1;
            newLocation = ListGet (newState->dracula.locations, n);
            newState->dracula.canDoubleBack = FALSE;
        } else if (move == TELEPORT) {
            newLocation = CASTLE_DRACULA;
        } else {
            newLocation = move;
        }
        

        ListInsertFront (newState->dracula.locations, newLocation);
       // printf ("YOYO: %d\n", newLocation);
        // if dracula's location is a sea location, he loses health.
        int locationType = idToType (newLocation);
        if (locationType == SEA) {
            newState->dracula.health -= LIFE_LOSS_SEA;
        } 

        // if dracula's location is castle dracula, he gains health.
        if (newLocation == CASTLE_DRACULA) {
            newState->dracula.health += LIFE_GAIN_CASTLE_DRACULA;  
        }

        // if dracula is in a city then he'll lay a trap or imm. vampire
        if (locationType == LAND) {
            // if the turn is div. by 13, and draculas in a city then place an imm vamp in dracs loc.
            // otherwise, place a trap there if there are < 3.
            if (newState->round % 13 == 0) {
                newState->immVampLoc = newLocation;
            } else if (newState->numTraps[newLocation] < 3) {
                newState->numTraps[newLocation]++;
            }
        }

        // score decreases by 1 each time dracula finishes a turn
        newState->score--;

        // also update the round
        newState->round++;

    } else {
        hunter_t * hunter = &newState->hunters[newState->turn];

        // add the latest move to the hunters trail.
        ListInsertFront (hunter->trail, move);

        // if there are traps in the hunters location, then while the hunters health is
        // > 0, encounter each trap in turn.
        while (newState->numTraps[move] > 0 && hunter->health > 0) {
            hunter->health -= LIFE_LOSS_TRAP_ENCOUNTER;
            newState->numTraps[move]--;
        }

        // if the hunter is still alive, and there's an immature vampire there, then vanquish him!
        if (newState->immVampLoc == move && hunter->health > 0) {
            newState->immVampLoc = NOWHERE;
        }

        // if the hunter is still alive and in the same loc' as dracula and not in the sea
        // then do the things
        int dracLocation = ListGet (newState->dracula.locations, 0);
        if (hunter->health > 0 && idToType(move) == LAND && dracLocation == move) {
            newState->dracula.health -= LIFE_LOSS_HUNTER_ENCOUNTER;
            hunter->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
        }

        // if the hunter is dead, then set their location to the hospital 
        // otherwise, if they've rested, they gain 3 hp (max 9)
        // otherwise set their location to the move
        if (hunter->health <= 0) {
            hunter->location = ST_JOSEPH_AND_ST_MARYS;
            hunter->health = 0;
            newState->score -= SCORE_LOSS_HUNTER_HOSPITAL;
        } else if (hunter->location == move) {
            if (hunter->health + LIFE_GAIN_REST >= GAME_START_HUNTER_LIFE_POINTS) {
                hunter->health = GAME_START_HUNTER_LIFE_POINTS;
            } else {
                hunter->health += LIFE_GAIN_REST;
            }
        } else {
            hunter->location = move;
        }
    }

    // now update the turn
    newState->turn = (newState->turn + 1) % NUM_PLAYERS;

    // if it's now a hunters turn who was dead, restore their life points
    if (newState->turn != PLAYER_DRACULA && newState->hunters[newState->turn].health == 0) {
        newState->hunters[newState->turn].health = GAME_START_HUNTER_LIFE_POINTS;
    }

    // if its now draculas turn, then before he makes a move the oldest move is popped from the trail
    // NOTE that the reason this is located here is so that dracula knows he can double back.
    if (newState->turn == PLAYER_DRACULA) {
        // if 6 rounds ago dracula laid an immature vampire and it still aint dead, then
        // it matures. otherwise, 6 rounds ago dracula laid a trap
        if (newState->immVampLoc != NOWHERE && (newState->turn - 6) % 13 == 0) {
            newState->immVampLoc = NOWHERE;
            newState->score -= SCORE_LOSS_VAMPIRE_MATURES;
        } else {
            // how to deal with traps that fall off trail? for now just assume that
            // if theres still a trap where we were 6 turns ago, its out.
            int oldTrapLoc = ListGet (newState->dracula.locations, 5);
            if (oldTrapLoc != NOWHERE && newState->numTraps[oldTrapLoc] > 0) {
                newState->numTraps[oldTrapLoc]--;
            }
        }

        // if the move that fell off the trail was a HIDE / DN, then set canHide /
        // canDoubleBack to TRUE
        int oldMove = ListGet (newState->dracula.trail, 5);
        if (!newState->dracula.canHide && oldMove == HIDE) {
            newState->dracula.canHide = TRUE;
        } else if (!newState->dracula.canDoubleBack && (oldMove >= DOUBLE_BACK_1 || oldMove <= DOUBLE_BACK_5)) {
            newState->dracula.canDoubleBack = TRUE;
        }
    }

    return newState;
}

DracView undoMove (DracView currState) {
    // Load the last state
    DracView prevState = currState->prevState;

    // the only thing we have to care about are the lists which we've altered
    // in our simuation. All the other info is retained in prevState
    if (prevState->turn == PLAYER_DRACULA) {
        ListRemoveFront (prevState->dracula.trail);
        ListRemoveFront (prevState->dracula.locations);
    } else {
        ListRemoveFront (prevState->hunters[prevState->turn].trail);
    }

    // now free the previous state (but dont dispose of it since we need to keep
    // the lists!)
    free (currState);

    return prevState;
}

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    // allocate memory for dracView
    DracView dracView = malloc(sizeof(struct dracView));

    // create a gameView to summarise almost all game content.
    // in our dracView. This gameView will never be used again!
    GameView gameView = newGameView (pastPlays, messages);

    // Grab info about the score, round and turn (turn should be dracula but eh)
    dracView->score = getScore (gameView);
    dracView->round = getRound (gameView);
    dracView->turn = getCurrentPlayer (gameView);

    // setup dracula info
    setupDracula (dracView, gameView, pastPlays);

    // setup hunters
    setupHunters (dracView, gameView, pastPlays);

    // Initialise numTraps
    int i;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dracView->numTraps[i] = 0;
    }

    // Initialise imm. vampire location
    dracView->immVampLoc = NOWHERE;

    // determine minion/trap info
    setDraculaMinionInfo (dracView, pastPlays);

    dracView->prevState = NULL;

    // setup map and mDistances
        // init map
    dracView->m = newMap();

    // init mDistances;
    dracView->mDistances = malloc (sizeof (int *) * NUM_MAP_LOCATIONS);
    int trans[3] = {ROAD, BOAT, RAIL};
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dracView->mDistances[i] = shortestPaths(dracView->m, i, trans, 3);
    }

    disposeGameView (gameView);

    return dracView;
}
     
// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    // Dispose of all the lists
    dropList (toBeDeleted->dracula.trail);
    dropList (toBeDeleted->dracula.locations);

    int i;
    for (i = 0; i < NUM_PLAYERS - 1; i++) {
        dropList (toBeDeleted->hunters[i].trail);
    }

    // dispose of map things
    disposeMap (toBeDeleted->m);
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        free (toBeDeleted->mDistances[i]);
    }
    free (toBeDeleted->mDistances);

    free( toBeDeleted );
}

// Where can dracula go. Returns:
//      NULL, if draculas location is unknown (hasnt made a move)
//      NULL, if dracula can't move anywhere.
//      ARRAY OF LOCATIONS, if dracula cannot hide OR double back.
//      ARRAY OF LOCATIONS / HIDE / DOUBLE_BACK's 
//                          if dracula can hide / double back.
//                          if we can hide AND double back, then double_back_1 will not
//                          be returned (its preferrable to hide)
LocationID *whereCanIgo(DracView currState, int *numLocations) {
    int dracLocation = ListGet (currState->dracula.locations, 0);
    *numLocations = 0;

    if (dracLocation == NOWHERE) {
        return NULL;
    }

    // first get everywhere that dracula can travel to by road
    int numRoadLocs;
    LocationID * roadLocations = getConnections (currState->m, dracLocation,
                                                   ROAD, 1, &numRoadLocs);

    // then get everywhere we can travel by sea
    int numSeaLocs;
    LocationID * seaLocations = getConnections (currState->m, dracLocation,
                                                   BOAT, 1, &numSeaLocs); 

    // init a boolean array of size NUM_LOCATIONS. TRUE indicates that we can visit
    // that location next move.
    int * isPossibleMove = calloc (NUM_MAP_LOCATIONS, sizeof (int));
    int i;
    for (i = 0; i < numRoadLocs; i++) {
        if (roadLocations[i] != ST_JOSEPH_AND_ST_MARYS && !isPossibleMove[roadLocations[i]]) {
            (*numLocations)++;
            isPossibleMove[roadLocations[i]] = TRUE;
        }
    }
       

    
    for (i = 0; i < numSeaLocs; i++) {
        if (seaLocations[i] != ST_JOSEPH_AND_ST_MARYS && !isPossibleMove[seaLocations[i]]) {
            (*numLocations)++;
            isPossibleMove[seaLocations[i]] = TRUE;
        }
    }

    free (roadLocations);
    free (seaLocations);


    // now time to check if we can hide/doubleback
    int numExtraMoves = 0;
    int extraMoves[5];

    if (currState->dracula.canHide) {
        extraMoves[0] = HIDE;
        numExtraMoves++;
    } else if (currState->dracula.canDoubleBack) {
        extraMoves[0] = DOUBLE_BACK_1;
        numExtraMoves++;
    }

    // if we can double back, then iterate through the last 2..5 locations
    // and if they're adjacent add them
    if (currState->dracula.canDoubleBack) {
        for (i = 1; i < TRAIL_SIZE - 1; i++) {
            int loc = ListGet (currState->dracula.locations, i);
            if (loc != NOWHERE && isPossibleMove[loc]) {
                extraMoves[numExtraMoves] = DOUBLE_BACK_1 + i;
                numExtraMoves++;
            }
        }
    }

    // for every move in the trail EXCEPT THE LAST ONE, set isPossible to false. 
    // the last one is technically removed from draculas trail on his turn.
    for (i = 0; i < TRAIL_SIZE - 1; i++) {
        int loc = ListGet (currState->dracula.locations, i);
        if (loc != NOWHERE && isPossibleMove[loc]) {
            (*numLocations)--;
            isPossibleMove[loc] = FALSE;
        }
    }

    // if there are STILL no moves, then dracula has to teleport
    if ((*numLocations) + numExtraMoves == 0) {
        free (isPossibleMove);
        
        int * possibleMoves = malloc (1 * sizeof(int));
        possibleMoves[0] = TELEPORT;
        (*numLocations) = 1;

        return possibleMoves;
    }

    // otherwise allocate and init the array with the locations that dracula can go..
    int * possibleMoves = malloc (sizeof(int) * ((*numLocations) + numExtraMoves));
    int j = 0;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if (isPossibleMove[i]) {
            possibleMoves[j] = i;
            j++;
        }
    }

    // add the extra moves
    for (i = 0; i < numExtraMoves; i++) {
        possibleMoves[j] = extraMoves[i];
        j++;
    }

    (*numLocations) = (*numLocations) + numExtraMoves;
    

    free (isPossibleMove);

    // fuckin finally done
    return possibleMoves;
}


// Returns all the locations that a given player can move to.
LocationID *whereCanTheyGo(DracView currState, int *numLocations, PlayerID player)
{
    // if it's dracula then just call whereCanIGo
    if (player == PLAYER_DRACULA) {
        return whereCanIgo (currState, numLocations);
    }

    // otherwise we got a hunter on our hands!

    // get the hunter's current location
    int hunterLoc = currState->hunters[player].location;
    *numLocations = 0;

    if (hunterLoc == UNKNOWN_LOCATION) {
        return NULL;
    }

    // then initialise an array of zeros which, for each location, is = 1
    // if we can connect to that location this round.
    LocationID * isPossibleMove = calloc (NUM_MAP_LOCATIONS, sizeof (int));

    // get the road connections
    int numRoadConnections;
    LocationID * roadConnections = getConnections (currState->m, hunterLoc, 
                                                   ROAD, 1, &numRoadConnections);

    // get the sea connections
    int numSeaConnections;
    LocationID * seaConnections = getConnections (currState->m, hunterLoc, 
                                                   BOAT, 1, &numSeaConnections);

    // get the rail connections
    int maxDistance = (currState->round + player) % 4;
    int numRailConnections;
    LocationID * railConnections = getConnections (currState->m, hunterLoc, 
                                                   RAIL, maxDistance, &numRailConnections);

    int i;
    for (i = 0; i < numRoadConnections; i++) {
        if (!isPossibleMove[roadConnections[i]]) { 
            (*numLocations)++; 
            isPossibleMove[roadConnections[i]] = TRUE; 
        } 
    }

    for (i = 0; i < numSeaConnections; i++) {
        if (!isPossibleMove[seaConnections[i]]) { 
            (*numLocations)++; 
            isPossibleMove[seaConnections[i]] = TRUE; 
        } 
    }

    for (i = 0; i < numRailConnections; i++) {
        if (!isPossibleMove[railConnections[i]]) { 
            (*numLocations)++; 
            isPossibleMove[railConnections[i]] = TRUE; 
        } 
    }
    
    free (roadConnections);
    free (seaConnections);
    free (railConnections);

    // Finally we can allocate the array we're returning, since we know how 
    // large it is
    int * possibleMoves = malloc ((*numLocations) * sizeof(int));
    int j = 0;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if (isPossibleMove[i]) {
            possibleMoves[j] = i;
            j++;
        }
    }

    free (isPossibleMove);
    
    return possibleMoves;
}

// Get the current round
Round giveMeTheRound(DracView currentView)
{
    return currentView->round;
}

// Get the current turn
int giveMeTheTurn (DracView currentView) {
    return currentView->turn;
}

// Get the current score
int giveMeTheScore(DracView currentView)
{
    return currentView->score;
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player)
{
    if (player == PLAYER_DRACULA) {
        return currentView->dracula.health;
    } else {
        return currentView->hunters[player].health;
    }
}

LocationID whereIs (DracView currentView, int player) {
    if (player == PLAYER_DRACULA) {
        return ListGet (currentView->dracula.locations, 0);
    } else {
        return currentView->hunters[player].location;
    }
}


//          *** Static Functionc *** 
static LocationID getLocationFromPlay (dracPlay * play) {
    // For each possible type of move (Dn, HI, TP, LOCATION)
    if (play->location[0] == 'H' && play->location[1] == 'I') {
        // If it's a hide move, then call getLocationFromPlay again
        // setting the dracPlay pointer to the previous dracPlay location in the
        // pastPlays string
        return getLocationFromPlay (play - (1 * NUM_PLAYERS));

    } else if (play->location[0] == 'D' &&
               play->location[1] >= '1' && play->location[1] <= '5') {
        // If it's a double back move, then first work out how much by,
        // then call getLocationFromPlay again setting the dracPlay pointer
        // to the appropriate location in the pastPlays string
        int n = play->location[1] - '0';
        return getLocationFromPlay (play - (n * NUM_PLAYERS));

    } else if (play->location[0] == 'T' && play->location[1] == 'P') {
        // If it's a teleport move, then dracula is definitely in CASTLE
        // dracula
        return CASTLE_DRACULA;
    } else {
        // Otherwise, use the abbrevToID function from places.h to get the 
        // locationID.
        return abbrevToID (play->location);
    }
}

static void setDraculaMinionInfo (DracView dracView, char * pastPlays) {
    // Traverse the pastPlays string to keep track of immature vampires
    // and traps. 
    int i, numPlays = (strlen(pastPlays) + 1) / PLAY_LENGTH;
    for (i = 0; i < numPlays; i++) {
        // first character gives us which player it is
        if (pastPlays[i * PLAY_LENGTH] == 'D') {
            // current move begins at (8*i)-th character
            dracPlay * play = (dracPlay *) &pastPlays[i * PLAY_LENGTH];

            // get dracula's current location
            LocationID currLoc = getLocationFromPlay (play);

            // If dracula lays a trap, then update numTraps
            if (play->encounters[0] == 'T') {
                dracView->numTraps[currLoc]++;
            } 

            // If dracula plants an immature vamp, then update the location
            if (play->encounters[1] == 'V') {
                dracView->immVampLoc = currLoc;
            }

            // If a trap leaves the trail, find out where that trap was placed
            // and update numTraps
            if (play->action == 'M') {
                LocationID trapLoc = getLocationFromPlay (play - (NUM_PLAYERS * TRAIL_SIZE));
                dracView->numTraps[trapLoc]--;
            } else if (play->action == 'V') {
                // otherwise if an immature vampire has matured, then set immatureVampLocation
                // to unknown
                 dracView->immVampLoc = NOWHERE;
            }
        } else {
            // So with hunters, we're only interested in their encounters.

            // current move begins at (8*i)-th character
            hunterPlay * play = (hunterPlay *) &pastPlays[i * PLAY_LENGTH];

            // get the hunters new location
            LocationID currLoc = abbrevToID (play->location);

            int j = 0;
            while (j < 4 && play->encounters[j] != '.') {
                // if the hunter encounters a trap, then remove that trap
                // from numTraps
                if (play->encounters[j] == 'M') {
                    dracView->numTraps[currLoc]--;
                } else if (play->encounters[j] == 'V') {
                    // if the hunter encounters an immature vampire
                    // then that badboy is gone
                    dracView->immVampLoc = NOWHERE;
                }
                j++;
            }
        }
    }
}

static void setDraculaTrailLocations (DracView dracView, char * pastPlays) {
    // initialise all locations to UNKNOWN_LOCATION
    /*int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        ListInsertFront (dracView->dracula.locations, UNKNOWN_LOCATION);
    }*/

    // start with -1 -1 -1 -1 -1 -1
    // then we get draculas first move

    // since dracula is always last to have a turn, his last turn was in the
    // last round.
    int turnRound = dracView->round - 1;

    // For draculas last TRAIL_SIZE moves, get the location using
    // getLocationFromPlay. 
    dracPlay * play;    
    int i = 0;
    while (turnRound >= 0 && i < TRAIL_SIZE) {
        // cast a pointer to draculas ith last move in pastPlays
        int playLocation = PLAY_LENGTH * (NUM_PLAYERS * turnRound + PLAYER_DRACULA);
        play = (dracPlay *) &pastPlays[playLocation];
        //ListRemoveEnd (dracView->dracula.locations);
        int move = getLocationFromPlay (play); printf ("MOVE!!: %d\n", move);
        ListInsertEnd (dracView->dracula.locations, move);

        turnRound--;
        i++;
    }
    
    // now in case we havent filled up enough space:
    while (i < TRAIL_SIZE) {
        ListInsertEnd (dracView->dracula.locations, UNKNOWN_LOCATION);
        i++;
    }
}

static void setupDracula (DracView dracView, GameView gameView, char * pastPlays) {
    // get health
    dracView->dracula.health = getHealth (gameView, PLAYER_DRACULA);

    // initialise canX
    dracView->dracula.canHide = TRUE;
    dracView->dracula.canDoubleBack = TRUE;

    // initialise lists for trail and locations
    dracView->dracula.trail = newList ();
    dracView->dracula.locations = newList ();

    // get the trail from the game view, and add them to our lists
    // (add to end of the list each time since we add more recent
    // moves first.)
    int dracTrail[TRAIL_SIZE];
    getHistory (gameView, PLAYER_DRACULA, dracTrail);
    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        // for every move EXCEPT the last one, if it's a DN or HIDE then set canHide 
        // and canDoubleBack appropriately. We don't care about the last move since
        // when it's dracula's turn itll be removed from the trail anyway.
        if (i  < TRAIL_SIZE - 1) {
            if (dracTrail[i] == HIDE) {
                dracView->dracula.canHide = FALSE;
            } else if (dracTrail[i] >= DOUBLE_BACK_1 && dracTrail[i] <= DOUBLE_BACK_5) {
                dracView->dracula.canDoubleBack = FALSE;
            } 
        }

        ListInsertEnd (dracView->dracula.trail, dracTrail[i]);
    }

    // set draculas trail locations
    setDraculaTrailLocations (dracView, pastPlays);
}

static void setupHunters (DracView dracView, GameView gameView, char * pastPlays) {
    // for each hunter
    int i, j;
    int hunterTrail[TRAIL_SIZE];
    for (i = 0; i < NUM_PLAYERS - 1; i++) {
        // get the health of the player
        dracView->hunters[i].health = getHealth (gameView, i);

        // initialise list for trail
        dracView->hunters[i].trail = newList ();

        // get the trail from the game view and add to list
        // (add to end of the list each time since we add newest first.)
        getHistory (gameView, i, hunterTrail);
        for (j = 0; j < TRAIL_SIZE; j++) {
            ListInsertEnd (dracView->hunters[i].trail, hunterTrail[i]);
        }

        // get their current location
        dracView->hunters[i].location = getLocation (gameView, i);
    }
}
