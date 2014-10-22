// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include <stdio.h>
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "List.h"
#include "string.h"

typedef struct player {
    int health;
    int location; // this is the actual location: ie a place on a map, or S? or C? or UNKOWN.
                  // If we see a double back or hide, that move will be the last in the trail
                  // but the location will always be updated (to work out score/health etc.)
    List trail;
    List locations;
} player;

struct gameView {
    int score;  
    int turn; 
    Round round;
    player players[NUM_PLAYERS];

    Map gameMap;
};

typedef struct dracPlay {
    char    identifier;
    char    location[2];
    char    encounters[2];
    char    action;
} dracPlay; 

typedef struct hunterPlay {
    char    identifier;
    char    location[2];
    char    encounters[4];
} hunterPlay;   

static void execDraculaPlay (GameView gameView, dracPlay * play) {
    
    // ------------ Move Phase ------------
    // some of dracula's moves aren't actually places, (like double back, hide,
    // and teleport). 
    int move;
    LocationID location;
    PlaceType type;

    // if the location isn't on the map, then pick from the remaining possible moves.
    
    if (play->location[0] == 'C' && play->location[1] == '?') {
        // if he's in an unknown city
        move = CITY_UNKNOWN;
        location = CITY_UNKNOWN;
        type = LAND;
    } else if (play->location[0] == 'S' && play->location[1] == '?') {
        // if dracula's in an unknown sea
        move = SEA_UNKNOWN;
        location = SEA_UNKNOWN;
        type = SEA;    
    } else if (play->location[0] == 'H' && play->location[1] == 'I') {
        // if dracula hides, the location is the same as the last
        // and also dracula must be on land.
        move = HIDE;
        location = gameView->players[PLAYER_DRACULA].location;
        type = LAND; 
    } else if (play->location[0] == 'T' && play->location[1] == 'P') {
        move = TELEPORT;
        location = CASTLE_DRACULA;
        type = LAND;
    } else if (play->location[0] == 'D' && 
               play->location[1] >= '1' && play->location[1] <= '5') {

        // if he's doubling back, need to know how much by.
        int n = play->location[1] - '1';
        move = DOUBLE_BACK_1 + n;

        // get the move in the trail
        location = ListGet(gameView->players[PLAYER_DRACULA].trail, n);

        if (location == CITY_UNKNOWN) {
            type = LAND;
        } else if (location == SEA_UNKNOWN) {
            type = SEA;
        } else if (validPlace (location)) {
            type = idToType (location);
        } else {
            type = LAND;
        }
        
        /*printf ("Prevmove: %d\n", prevMove);

        // if that move was a hide move, get the move before that
        if (prevMove == HIDE) {
            printf ("It was!");
            prevMove = ListGet(gameView->players[PLAYER_DRACULA].trail, n + 1);
            printf ("It was!");
        }

        // now if the move was a teleport then we're back at castle dracula
        if (prevMove == TELEPORT) {
            location = CASTLE_DRACULA;
            type = LAND;
        } else if (prevMove == CITY_UNKNOWN) {
            // if we're in an unknown city
            location = CITY_UNKNOWN;
            type = LAND;
        } else if (prevMove == SEA_UNKNOWN) {
            // if we're in the sea
            location = SEA_UNKNOWN;
            type = SEA;
        } else {
            // otherwise, we're in an actual location. In this case
            // we need to find out the type of place it is.
            location = prevMove;
            printf ("MOVE1: %d\n", move);
            type = idToType(prevMove);
        }           
        // and we're done (with double back moves, that is)*/
    } else {
        // if it's none of these things, then dracula is in an actual location
        // so we can call abbrevToID to find out!
        move = location = abbrevToID (play->location);
        printf ("MOVE2: %d\n", move);
        type = idToType (location);
    }

    // if dracula ended up at castle dracula he gains 10 blood points
    if (location == CASTLE_DRACULA) {
        gameView->players[PLAYER_DRACULA].health += LIFE_GAIN_CASTLE_DRACULA;
    }

    // if dracula's on the sea then he loses two points
    if (type == SEA) {
        gameView->players[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;   
    }
    
    // remove the last move in draculas trail.
    ListRemoveEnd (gameView->players[PLAYER_DRACULA].trail);
    ListRemoveEnd (gameView->players[PLAYER_DRACULA].locations);
    // add the latest move to the trail, and set draculas last known location
    ListInsertFront (gameView->players[PLAYER_DRACULA].trail, move);
    ListInsertFront (gameView->players[PLAYER_DRACULA].locations, location);
    gameView->players[PLAYER_DRACULA].location = location;  
      

    // ------------ Encounters Phase ------------
    // (i don't think we need to do anything here ??)

    // ------------ Action Phase ------------
    // if  a trap leaves the trail we don't care. But we do if a vampire matures,
    // in which case the score decreases
    if (play->action == 'V') {
        gameView->score -= SCORE_LOSS_VAMPIRE_MATURES;
    }
}

static void execHunterPlay (GameView gameView, hunterPlay * play) {
    PlayerID pID = gameView->turn;

    // ------------ Move Phase ------------
    // check which location the move corresponds to
    LocationID location = abbrevToID (play->location);

    // remove the last move in the player's trail.
    ListRemoveEnd (gameView->players[pID].trail);
    // add the latest move to the trail
    ListInsertFront (gameView->players[pID].trail, location);

    // ------------ Encounters Phase ------------
    // iterate through the encounters string and complete approp. action for each.
    int i = 0;
    while (play->encounters[i] != '.' && i < 4) {
        switch (play->encounters[i]) {
            case 'T':
                // lose health if hunter encounters trap
                gameView->players[pID].health -= LIFE_LOSS_TRAP_ENCOUNTER;
                break;
            case 'V':
                // nothing really happens but maybe it would be nice to know?
                break;
            case 'D':
                // if the hunter encounters dracco then he loses 10 and we lose 4.
                gameView->players[pID].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
                gameView->players[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
                break;
        }       
        i++;
    }

    // if the hunter has died during this time, then they're sent back to ol' st mary
    // and the score is decreased by 6.
    if (gameView->players[pID].health < 1) {
        gameView->score -= 6;
        gameView->players[pID].health = 0; // just in case it was something weird.

        // so I'm not sure whether to set this as the place they moved to OR
        // the hospital at this point.
        location = ST_JOSEPH_AND_ST_MARYS;
    }


    // ------------ End Phase ------------
    // if the hunter is in the last sea/city as they started they gain 
    // 3 life points (max = 9). This doesn't apply if they've died.
    if (location == gameView->players[pID].location && gameView->players[pID].health != 0) {
        gameView->players[pID].health += LIFE_GAIN_REST;

        if (gameView->players[pID].health > GAME_START_HUNTER_LIFE_POINTS) {
            gameView->players[pID].health = GAME_START_HUNTER_LIFE_POINTS;
        }
    }

    // now we can change the player location, and that's all that happens (i think!)
    gameView->players[pID].location = location;
}

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    // Allocate memory for game view.
    GameView gameView = malloc(sizeof(struct gameView));

    // Initialise gameView instance variables.
    gameView->score = GAME_START_SCORE;
    gameView->round = 0;
    gameView->turn  = PLAYER_LORD_GODALMING;
    gameView->gameMap = newMap();

    // Initialise player instances.
    int i;
    for (i = 0; i < NUM_PLAYERS; i++) {
        // set intitial player health.
        if (i == PLAYER_DRACULA) {
            gameView->players[i].health = GAME_START_BLOOD_POINTS;
            gameView->players[i].locations = newList();
        } else { 
            gameView->players[i].health = GAME_START_HUNTER_LIFE_POINTS;
        }

        // each player begins in an unknown location
        gameView->players[i].location = UNKNOWN_LOCATION;

        // create and initialise the trail lists
        gameView->players[i].trail = newList();
        int j;
        for (j = 0; j < TRAIL_SIZE; j++) {
            if (i == PLAYER_DRACULA) {
                ListInsertFront (gameView->players[i].locations, UNKNOWN_LOCATION);
            }
            ListInsertFront (gameView->players[i].trail, UNKNOWN_LOCATION);
        }
    } 

    // Traverse the pastPlays string. Each play consists of 7 chars, and
    // they're all separated by a ' ' (except the last), so the number of
    // plays so far is (strlen(pastPlays) + 1)/8. 
    int numPlays = (strlen(pastPlays) + 1) / 8;
    for (i = 0; i < numPlays; i++) {
        // determine which player it is using turn (don't need to read that
        // pesky char)
        if (gameView->turn == PLAYER_DRACULA) {
            // current move begins at (8*i)-th character
            dracPlay * play = (dracPlay *) &pastPlays[i * 8];

            // we'll just wrap everything in a neat little function
            execDraculaPlay (gameView, play);

            // after dracula's turn the round increases by 1 (shiit)
            // and the score decreases by 1
            gameView->score -= SCORE_LOSS_DRACULA_TURN;
            gameView->round++;
        } else {
            // current move begins at (8*i)-th character
            hunterPlay * play = (hunterPlay *) &pastPlays[i * 8];

            // we'll just wrap everything in a neat little function
            execHunterPlay (gameView, play);
        }

        // increase turn at the end of each move.
        gameView->turn = (gameView->turn + 1) % NUM_PLAYERS;

        // if it's a hunter's turn and they're dead, restore their life points
        // and set their location to the hospital
        if (gameView->turn != PLAYER_DRACULA && gameView->players[gameView->turn].health == 0) {
            gameView->players[gameView->turn].health = GAME_START_HUNTER_LIFE_POINTS;
            gameView->players[gameView->turn].location = ST_JOSEPH_AND_ST_MARYS;
        }
    }

    return gameView;
}

// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    // free the trails for each player
    int i;
    for (i = 0; i < NUM_PLAYERS; i++) {
        dropList (toBeDeleted->players[i].trail);
    }

    // free the map
    disposeMap (toBeDeleted->gameMap);

    // now free the gameView
    free (toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    assert (currentView != NULL);
    return currentView->round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    assert (currentView != NULL);
    return currentView->turn;
}

// Get the current score
int getScore(GameView currentView)
{
    assert (currentView != NULL);
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    
    assert (currentView != NULL);
    return currentView->players[player].health;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    assert (currentView != NULL);

    // if it's dracula then we return the last move in the trail
    if (player == PLAYER_DRACULA) {
        return ListGet(currentView->players[player].locations, 0);
    } else {
        // if it's a hunter, just return the location of that
        // hunter.
        return currentView->players[player].location;
    }
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    int * t = ListToArray(currentView->players[player].trail);

    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = t[i];
    }

    free(t);
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

// connectedLocations() returns an array of LocationID that represent
//   all locations that are connected to the given LocationID.
// road, rail and sea are connections should only be considered
//   if the road, rail, sea parameters are TRUE.
// The size of the array is stored in the variable pointed to by numLocations
// The array can be in any order but must contain unique entries
// Your function must take into account the round and player id for rail travel
// Your function must take into account that Dracula can't move to
//   the hospital or travel by rail but need not take into account Dracula's trail
// The destination 'from' should be included in the array

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    // then initialise an array of zeros which, for each location, is = 1
    // if we can connect to that location this round.
    LocationID * adjArray = calloc (numV(currentView->gameMap), sizeof (LocationID));

    *numLocations = 1;
    adjArray[from] = 1;

    // now if we're looking for all road connections, let's just grab that
    if (road) {
        int numRoadConnections;
        LocationID * roadConnections = getConnections (currentView->gameMap, from, 
                                            ROAD, 1, &numRoadConnections);
        int i;
        for (i = 0; i < numRoadConnections; i++) {
            if (roadConnections[i] == ST_JOSEPH_AND_ST_MARYS && player == PLAYER_DRACULA) {
                continue;
            }
            if (!adjArray[roadConnections[i]]) { 
                (*numLocations)++; 
                adjArray[roadConnections[i]] = 1; 
            } 
        }
        free (roadConnections);
    }

    // now for sea!
    if (sea) {
        int numSeaConnections;
        LocationID * seaConnections = getConnections (currentView->gameMap, from, 
                                            BOAT, 1, &numSeaConnections);
        
        int i;
        for (i = 0; i < numSeaConnections; i++) {
            if (!adjArray[seaConnections[i]]) {
                (*numLocations)++; 
                adjArray[seaConnections[i]] = 1; 
            } 
        }
        free (seaConnections);
    }

    // and finally, rail!
    if (rail && player != PLAYER_DRACULA) {
        // first we need to see how long we get to travel for
        int maxDistance = (currentView->round + player) % 4;

        int numRailConnections;
        LocationID * railConnections = getConnections (currentView->gameMap, from, 
                                            RAIL, maxDistance, &numRailConnections);

        int i;
        for (i = 0; i < numRailConnections; i++) {
            if (!adjArray[railConnections[i]]) {

                (*numLocations)++; 
                adjArray[railConnections[i]] = 1; 
            } 
        }

        free (railConnections);
    }

    // Finally we can allocate the array we're returning, since we know how 
    // large it is
    LocationID * connections = malloc (*numLocations * sizeof(LocationID));
    int i, j = 0;
    for (i = 0; i < numV(currentView->gameMap); i++) {
        if (adjArray[i]) {
            connections[j] = i;
            j++;
        }
    }

    free (adjArray);
    return connections;
}