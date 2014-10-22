// HunterView.c ... HunterView ADT implementation

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h" //... if you decide to use the Map ADT


typedef struct player {
    int health;
    LocationID location; 
     LocationID trail[TRAIL_SIZE];
} Player;
     
     
struct hunterView {
    GameView game; // yeah definitely should do this
    Round roundNumber;
    PlayerID currentPlayer;
     int score;
    
    Player players[NUM_PLAYERS]; 

    Map m;
    int ** mDistances;

    PlayerMessage * messages;
};

static LocationID * whereCanThisHunterGo (HunterView currentView, int *numLocations, 
                                          PlayerID hunter, int road, int rail, int sea);
static LocationID * whereCanDraculaGo (HunterView currentView, int *numLocations, int road, int sea);

int getTrailMove (HunterView h, int player, int moveNum) {
    return h->players[player].trail[moveNum];   
}

int mDistance (HunterView h, int i, int j) {
    if (i < 0 || j < 0) {return 1;};
    return h->mDistances[i][j];
}

char * getMessage (HunterView h, int player, int whichRound) {
    char * message = malloc (sizeof (char) * MESSAGE_SIZE);
    int messageLoc = whichRound * NUM_PLAYERS + player;
    strcpy (message, h->messages[messageLoc]);

    return message;
}

// What are the specified player's next possible moves if they were leaving from this location
int *whereCanTheyGoFrom (HunterView h, int *numLocations, int player,
                                int from, int round, int road, int rail, int sea)
{
    return connectedLocations (h->game, numLocations, from, player, round, road, rail, sea);
}
     

// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[]){
    HunterView hunterView = malloc(sizeof(struct hunterView));
    GameView game = newGameView(pastPlays, messages);
    
    //use the gameView to fill in the hunterView struct
    hunterView->roundNumber = getRound(game);
    hunterView->currentPlayer = getCurrentPlayer(game);
    hunterView->score = getScore(game);    
    
    int i; //filling in the players array in the hunterView struct
    for(i = 0; i < NUM_PLAYERS; i++){
      hunterView->players[i].health = getHealth(game, i);
      hunterView->players[i].location = getLocation(game, i);
      getHistory(game, i, hunterView->players[i].trail);
    }
    
    hunterView->game = game; //setting the gameView in the hunterView struct so can use later (not sure if you're allowed to do this)

    // init map
    hunterView->m = newMap();

    // init mDistances;
    hunterView->mDistances = malloc (sizeof (int *) * NUM_MAP_LOCATIONS);
    int trans[2] = {ROAD, BOAT};
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        hunterView->mDistances[i] = shortestPaths(hunterView->m, i, trans, 2);
    }
    
    printf ("HERE %d\n", hunterView->mDistances[70][4]);
    

    hunterView->messages = messages;

    // need the messages:
    return hunterView;
}
     
     
// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted){
    disposeMap (toBeDeleted->m);
    //int i;
    /* causes seg fault for some reason 
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        free (toBeDeleted->mDistances[70]);
    }
    free (toBeDeleted->mDistances); */
    disposeGameView(toBeDeleted->game);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView){
    assert(currentView != NULL);
    return currentView->roundNumber;
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView){
    assert(currentView != NULL);
    return currentView->currentPlayer;
}

// Get the current score
int giveMeTheScore(HunterView currentView){
    assert(currentView != NULL);
    return currentView->score;
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player){
    assert(currentView != NULL);
    return currentView->players[player].health;
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player){
    assert(currentView != NULL);
    return currentView->players[player].location;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player, LocationID trail[TRAIL_SIZE]){
    LocationID *array = currentView->players[player].trail;
    int i;
    for(i = 0; i < TRAIL_SIZE; i++) trail[i] = array[i];
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations, int road, int rail, int sea){
    return whereCanTheyGo (currentView, numLocations, currentView->currentPlayer, road, rail, sea);
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    // If player is dracula, call whereCanDraculaGo
    if (player == PLAYER_DRACULA) {
        return whereCanDraculaGo (currentView, numLocations, road, sea);
    } else {
        return whereCanThisHunterGo (currentView, numLocations, player, road, rail, sea);
    }
}


static LocationID * whereCanThisHunterGo (HunterView currentView, int *numLocations, 
                                          PlayerID hunter, int road, int rail, int sea) 
{
    // first check that we know where the hunter is (ie, their location isnt UNKNOWN) 
    LocationID hunterLocation = whereIs (currentView, hunter);
    if (hunterLocation == UNKNOWN_LOCATION) {
        *numLocations = 0;
        return NULL;
    }

    // then use the connectedLocations function from gameView to get all adjacent locations that they
    // could visit
    return connectedLocations (currentView->game, numLocations, 
                               hunterLocation, hunter, currentView->roundNumber,
                               road, rail, sea);
}

static LocationID * whereCanDraculaGo (HunterView currentView, int *numLocations, int road, int sea) 
{
    // first check if we know where dracula is.
    LocationID dracLocation = whereIs (currentView, PLAYER_DRACULA);
    if (dracLocation == CITY_UNKNOWN || dracLocation == SEA_UNKNOWN || dracLocation == UNKNOWN_LOCATION) {
        *numLocations = 0;
        return NULL;
    }

    // so if we do know where dracula is, then let's get all the road and sea connections using the
    // connectedLocations function in gameView
    int numConnectedLocations;
    LocationID * possibleLocations = connectedLocations (currentView->game, &numConnectedLocations, 
                                                         dracLocation, PLAYER_DRACULA, currentView->roundNumber,
                                                         road, 0, sea);

    *numLocations = numConnectedLocations;

    // Allocate and init of all locations, entries are set to 0 if the location can't be visited and 1 if it can
    LocationID * canGoTo = calloc (NUM_MAP_LOCATIONS, sizeof (LocationID));
    int i;
    for (i = 0; i < numConnectedLocations; i++) {
        canGoTo[possibleLocations[i]] = 1;
    }
    //
    // now lets check draculas trail to check if he can hide or double back (to our knowledge)
    //int canHide, 
    int canDoubleBack, trailMove;
    //canHide = 
    canDoubleBack = 1;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trailMove = currentView->players[PLAYER_DRACULA].trail[i];
        if (trailMove == HIDE) {
            //canHide = 0;
        } else if (trailMove >= DOUBLE_BACK_1 && trailMove <= DOUBLE_BACK_5) {
            canDoubleBack = 0;
        }
    }
    
    // if dracula can double back (to our knowledge), then all adjacent locations 
    // in the trail can be visited again (using double back). Otherwise, they can't be
    if (!canDoubleBack) {
        // iterate through the trail. For each known location that corresponds to an
        // adjacent location, remove it from connectedLocations;
        for (i = 0; i < TRAIL_SIZE; i++) {
            trailMove = currentView->players[PLAYER_DRACULA].trail[i];
            // The following are not known locations
            if (trailMove != CITY_UNKNOWN && 
                trailMove != SEA_UNKNOWN &&
                trailMove != UNKNOWN_LOCATION &&
                trailMove != HIDE &&
                !(trailMove >= DOUBLE_BACK_1 && trailMove <= DOUBLE_BACK_5)) {

                // if the move is teleport, then we know dracula was in castle black
                // otherwise it's an actual location
                if (trailMove == TELEPORT) {
                    trailMove = CASTLE_DRACULA;
                }

                // if the location is adjacent to draculas current position, then remove it
                // from the canGoTo array and decrease numLocations by 1
                if (canGoTo[trailMove]) {
                    canGoTo[trailMove] = 0;
                    (*numLocations)--;
                }
            }
        }
    }

    // so according to the header spec, we 'ALWAYS' have to return draculas current 
    // location (which means it doesnt even matter if dracula can hide or not...)
    if (canGoTo[dracLocation] == 0) {
        canGoTo[dracLocation] = 1;
        (*numLocations)++;
    }

    // finally, malloc a new array to return
    LocationID * moves = malloc ((*numLocations) * sizeof(LocationID));
    int j = 0;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if (canGoTo[i]) {
            moves[j] = i;
            j++;
        }
    }

    // free what we've malloc'd
    free (canGoTo);
    free (possibleLocations);

    return moves;
}