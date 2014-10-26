// dracula.c
// Implementation of your "Fury of Dracula" Dracula AI

#include <stdlib.h>
#include <stdio.h>
#include "Game.h"
#include "DracView.h"
#include "List.h"

#define POS_INF 999999
#define NEG_INF -POS_INF

#define HUNTER_WIN  NEG_INF

#define BLOOD_POINTS_WEIGHT         20   
#define DISTANCE_WEIGHT             12
#define IMM_VAMP_WEIGHT             20   
#define SCORE_WEIGHT                0
#define HUNTER_KNOWLEDGE_WEIGHT     2
#define FUTURE_DEST_WEIGHT          7
#define HIDE_WEIGHT 10
#define DN_WEIGHT   15


typedef struct bestMove {
    int move;
    int score;
} bestMove;

static bestMove *chooseBestMove (int isDracula, DracView state, int alpha, int beta, int depth);
static int evalState (DracView state);
static  char * moveToAbbrev (int move);
static int shortestDistanceToHunter (DracView state, int location);
static int * getPossibleMoves (int isDracula, DracView state, int * numMoves);

void decideDraculaMove(DracView gameState)
{
    int maxDepth = (giveMeTheRound (gameState) == 0) ? 1 : 4;
    bestMove * currBest;

    int depth = 1;
    while (depth <= maxDepth) {
        currBest = chooseBestMove (TRUE, gameState, NEG_INF, POS_INF, depth);

        registerBestPlay(moveToAbbrev (currBest->move), "Please don't find me :)");
        printf ("Depth: %d\tMove: %d\tScore: %d\n", depth, currBest->move, currBest->score);
        
        free (currBest);
        depth++;
    }
}

static bestMove *chooseBestMove (int isDracula, DracView state, int alpha, int beta, int depth) 
{
    // Allocate moves!
    bestMove * best = malloc (sizeof (bestMove));
    bestMove * reply;

    // If dracula is dead then the hunters have won in this instance,
    // and there's no moves to be made. Alternatively if the depth has
    // been reached (depth = 0) then just evaluate the state and return;
    if (depth == 0 || howHealthyIs (state, PLAYER_DRACULA) <= 0) {
        best->score = evalState (state);
        best->move = NOWHERE;
        return best;
    }

    // Dracula wants to maximise the score, and the hunters want to
    // minimise it.
    if (isDracula) {
        best->score = alpha;
    } else {
        best->score = beta;
    }


    // Generate all the moves for a given player
    int numMoves;
    int * possibleMoves = getPossibleMoves (isDracula, state, &numMoves);
    
   // printf ("... Depth: %d,\tnumMoves: %d\n", depth, numMoves);
    
    int i;
    for (i = 0; i < numMoves; i++) {
        if (possibleMoves[i] == -1) {continue;}
        // simulate the gamestate caused by current move we're trying
        state = simulateMove (state, possibleMoves[i]);
        
        // generate best response
        if (isDracula) {
            reply = chooseBestMove (FALSE, state, alpha, beta, depth - 1);
        } else {
            reply = chooseBestMove (giveMeTheTurn (state) == PLAYER_DRACULA, 
                                    state, alpha, beta, depth);
        }
        
        //printf ("Loc: %d\t Score: %d\n", possibleMoves[i], reply->score);
        // undo the gamestate changes
        state = undoMove (state);

        // chose the move that maximises our score if dracula
        // and minimises if hunter
        if (isDracula && reply->score >= best->score) {
            best->move = possibleMoves[i];
            alpha = best->score = reply->score;
        } else if (!isDracula && reply->score <= best->score) {
            best->move = possibleMoves[i];
            beta = best->score = reply->score;
        }

        // free the malloc'd reply
        free (reply);
        // alpha beta pruning
        if (beta <= alpha) {break; }
    }

    free (possibleMoves);
    
    //printf ("Player: %d, BestMove: %d\n", giveMeTheTurn(state), best->move);

    return best;
}

static int * getPossibleMoves (int isDracula, DracView state, int * numMoves) {
    int * moves;

    if (isDracula) {
        moves = whereCanIgo (state, numMoves);

        // In later additions, we should impose some ordering HERE 
        // to speed up the tree seach
    } else {
        moves = whereCanTheyGo (state, numMoves, giveMeTheTurn (state));

        // cull every move that takes the hunter further from dracula
        int baseDistance = mDistance (state, whereIs(state, giveMeTheTurn(state)), whereIs (state, PLAYER_DRACULA));
        int i, n = *numMoves;
        for (i = 0; i < n; i++) {
            int futureDistance = mDistance (state, moves[i], whereIs (state, PLAYER_DRACULA));
            //printf ("f-b: %d\n", futureDistance - baseDistance);
            if (futureDistance > baseDistance) {
                moves[i] = -1;
            }
        }
    }

    return moves;
}

static int evalState (DracView state) 
{
    int dracHealthScore = howHealthyIs (state, PLAYER_DRACULA);
    int shortestDistance = shortestDistanceToHunter(state, whereIs (state, PLAYER_DRACULA));
    int isVampAlive = (getVampState (state) != NOWHERE);
    int currScore = GAME_START_SCORE - giveMeTheScore (state);

    int numFutureDestinations;
    int * futureDestinations = whereCanIgo(state, &numFutureDestinations);
    free (futureDestinations);

    int hideBonus = 0;
    int DBbonus = 0;
    int i;
    for (i = 0; i < numFutureDestinations; i++) {
        if (futureDestinations[i] == HIDE) {
            hideBonus = 1;
        } else if (futureDestinations[i] >= DOUBLE_BACK_1 &&
                    futureDestinations[i] <= DOUBLE_BACK_5) {
            DBbonus = 1;
        }
    }


    int whatHuntersKnow = 0;
    int j, k;
    for (i = 0; i < TRAIL_SIZE; i++) {
        int isLocationKnown = 0;
        int dracMove = getTrailMove (state, PLAYER_DRACULA, i);
        if (dracMove == UNKNOWN_LOCATION) break;

        for (j = i; j >= 0; j--) {
            for (k = 0; k < NUM_PLAYERS - 1; k++) {
                int hunterMove = getTrailMove (state, k, j);
                if (hunterMove == dracMove) {
                    isLocationKnown = 1;
                    whatHuntersKnow++;
                    break;
                }
            }

            if (isLocationKnown) { break; }
        }
    }

    if (dracHealthScore > 40) {
        dracHealthScore = 40;
    }

    int score = BLOOD_POINTS_WEIGHT * dracHealthScore +
                DISTANCE_WEIGHT * shortestDistance +
                IMM_VAMP_WEIGHT * isVampAlive +
                SCORE_WEIGHT * currScore +
                (FUTURE_DEST_WEIGHT * numFutureDestinations) -
                HUNTER_KNOWLEDGE_WEIGHT * whatHuntersKnow +
                HIDE_WEIGHT * hideBonus +
                DN_WEIGHT * DBbonus;


    return score;
}

static int shortestDistanceToHunter (DracView state, int location) {
    int shortestDistance = POS_INF;
    int i;
    for (i = 0; i < NUM_PLAYERS - 1; i++) {
        int hunterLoc = whereIs (state, i);
        int distanceToHunter = mDistance (state, location, hunterLoc);
        if (distanceToHunter < shortestDistance) {
            shortestDistance = distanceToHunter;
        }
    }

    return shortestDistance;
}

static  char * moveToAbbrev (int move) 
{  
    if (validPlace (move)) {
        return idToAbbrev(move);
    }
    
    switch (move) {
        case HIDE:
            return "HI";
            break;
        case DOUBLE_BACK_1:
            return "D1";
            break;
        case DOUBLE_BACK_2:
            return "D2";
            break;
        case DOUBLE_BACK_3:
            return "D3";
            break;
        case DOUBLE_BACK_4:
            return "D4";
            break;
        case DOUBLE_BACK_5:
            return "D5";
            break;
        case TELEPORT:
            return "TP";
            break;
        default:
            return "TP";
    }
}