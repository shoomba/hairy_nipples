
#include <stdlib.h>
#include <stdio.h>
#include "Game.h"
#include "HunterView.h"

#define LAST_KNOWN_LOCATION     0
#define ROUNDS_SINCE_LKL        1
#define DRACULA_IN_SEA          2
#define LOCATIONS_START         3
#define MEDOIDS_START           75
#define HIT_LIST_ZERO           '!'

#define POS_INF 999999


// see http://www.cs.umb.edu/cs738/pam1.pdf
static int * medoids (HunterView h, int * data, int ndata, int k) {
    // Maintain a set, S of medoids, and U that are datapoints that arent medoids
    int * S = malloc (sizeof (int) * k);

    printf ("Step 5\n");

    // for each data point, maintain two numbers:
    //  Dp = Distance from pth data point to closest medoid
    //  Ep = Distance from pth data point to SECOND closest medoid
    int * D = malloc (ndata * sizeof(int));
    int * E = malloc (ndata * sizeof(int));


    // BUILD PHASE
    // inititalise S by adding to it an object for which the sum of
    // distances to all other objects is minimal.
    int minCost = POS_INF;
    int i, j;
    for (i = 0; i < ndata; i++) {
        int cost = 0;
        for (j = 0; j < ndata; j++) {
            cost += mDistance (h, data[i], data[j]);
        }

        if (cost < minCost) {
            S[0] = data[i];
            minCost = cost;
        }
    }

    printf ("Step 6\n");

    // build the rest of the starting medoids.
    int numMedoids = 1;
    // while |S| < k, find a good medoid to add.
    while (numMedoids < k) {
        
        // update D
        for (i = 0; i < ndata; i++) {
            D[i] = mDistance(h, data[i], S[0]);
            for (j = 1; j < numMedoids; j++) {
                int d = mDistance(h, data[i], S[j]);
                if (d < D[i]) {D[i] = d;}
            }
        }

        int bestCandidate;
        int bestCandidateScore = -1;

        // For each i in U
        for (i = 0; i < ndata; i++) {
            // if i is already a medoid (distance = 0) then skip
            if (D[i] == 0) {
                continue;
            }

            int score = 0;

            // For each j != i in U
            for (j = 0; j < ndata; j++) {
                if (D[j] == 0 || j == i) {
                    continue;
                }

                // Find Dj = distance to closest point in S
                // find dij = distance from j to candidate i
                // Let Cij = max (Dj - d(j,i), 0). Add it to the score.
                int diff = D[j] - mDistance(h, data[i], data[j]);
                score += (diff > 0) ? diff : 0;
            }

            if (score > bestCandidateScore) {
                bestCandidate = data[i];
                bestCandidateScore = score;
            }
        }

        // Choose the i which maximises the score!
        S[numMedoids] = bestCandidate;
        numMedoids++;
    }

    printf ("Step 7\n");

    // SWAP PHASE
    int done = FALSE;
    while (!done) {
        // Update D and E
        for (i = 0; i < ndata; i++) {
            D[i] = POS_INF;
            E[i] = POS_INF;
            for (j = 0; j < k; j++) {
                int d = mDistance(h, data[i], S[j]);     
                if (d < D[i]) {
                    E[i] = D[i];
                    D[i] = d;
                } else if (d < E[i]) {
                    E[i] = d;
                }
            }
        }
        
        int swapI, swapL, bestK = POS_INF;
        // for each medoid i in S
        for (i = 0; i < k; i++) {
            // for each data point l in U. Compute the diff.
            // in cost when replacing i with l.
            int l;
            for (l = 0; l < ndata; l++) {
                if (D[l] == 0) { continue; }

                int K = 0;
                // for each data point j in U - {l}
                for (j = 0; j < ndata; j++) {
                    if (D[j] == 0 || j == l) { continue; }

                    int dji = mDistance (h, data[j], S[i]);
                    int djl = mDistance (h, data[j], data[l]);

                    // if (dji > Dj)   (ie, j is not in the cluster around i)
                    //      Let K += min(djl - Dj, 0)
                    // else if (dji == Dj)  (ie, j is in the cluster around i)
                    //      Let K += min(djl,Ej) - Dj.
                    if (dji > D[j]) {
                        int diff = djl - D[j];
                        K += (diff < 0) ? diff : 0;
                    } else {
                        int d = (djl < E[j]) ? djl : E[j];
                        K += d - D[j];
                    }
                }
                if (K < bestK) { swapI = i; swapL = l; bestK = K; }
            }
        }

        printf ("bestKhmm: %d\n", bestK);

        // if bestK < 0, then we should swap, otherwise theres
        // no reason to swap and we're done
        if (bestK < 0) {
            S[swapI] = data[swapL];
        } else {
            done = TRUE;
        }
    }
    printf ("Step 8");

    free (D);
    free (E);
    return S;
}

static void msgToHitList (char * message) {
    int i;
    for (i = 0; i < MESSAGE_SIZE - 1; i++) {
        message[i] -= HIT_LIST_ZERO;
    }
}
static void hitListToMsg (char * hitList) {
    int i;
    printf ("Step 16\n");
    for (i = 0; i < MESSAGE_SIZE - 1; i++) {
        hitList[i] += HIT_LIST_ZERO;
    }
    printf ("Step 17\n");
    hitList[MESSAGE_SIZE-1] = '\0';
    printf ("Step 18\n");
}

static char * initHitList () {
    char * hitList = calloc (MESSAGE_SIZE, sizeof (char));

    hitList[LAST_KNOWN_LOCATION] = NOWHERE;
    hitList[ROUNDS_SINCE_LKL] = 0;
    hitList[DRACULA_IN_SEA] = FALSE;
    hitList[MESSAGE_SIZE - 1] = 0;

    int i;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if (idToType (i) == LAND) {
            hitList[LOCATIONS_START + i] = 1;
        }
    }

    return hitList;   
}

static char * buildHitList (int rootLocation) {
    char * hitList = calloc (MESSAGE_SIZE, sizeof (char));

    hitList[LAST_KNOWN_LOCATION] = rootLocation;
    hitList[ROUNDS_SINCE_LKL] = 0;
    hitList[DRACULA_IN_SEA] = (idToType (rootLocation) == SEA) ? TRUE : FALSE;
    hitList[LOCATIONS_START + rootLocation] = 1;
    hitList[MESSAGE_SIZE - 1] = 0;

    return hitList;
}

static void destroyHitList (char * hitList) {
    free (hitList);
}

static void updateHitList (HunterView h, int move, char * hitList, int currRound) {
    hitList[ROUNDS_SINCE_LKL]++;

    if (validPlace(move)) {
        // if the move is a new location, then out with the old hitList and in
        // with the new!
        char * newHitList = buildHitList (move);
        
        int p;
        for (p = 0; p < MESSAGE_SIZE; p++) {
            hitList[p] = newHitList[p];
        }

        destroyHitList (newHitList);

    } else if (move == HIDE) {
        // if the move is hide, then nothing happens to the hitlist, since
        // dracul doesn't move anywhere.
        hitList[ROUNDS_SINCE_LKL]--;

    } else if (move == TELEPORT) {
        // build a new hitlist, with the start currently at
        // CASTLE_DRACULA
        char * newHitList = buildHitList (CASTLE_DRACULA);
        
        int p;
        for (p = 0; p < MESSAGE_SIZE; p++) {
            hitList[p] = newHitList[p];
        }

        destroyHitList (newHitList);

    } else if (move == CITY_UNKNOWN || move == SEA_UNKNOWN) {
        // init an array, where entry = 1 means that dracula could have moved
        // here this turn.
        int * possibleMoves = calloc (NUM_MAP_LOCATIONS, sizeof(int));

        // For every place in the hitlist, generate all locations dracula could
        // have just moved to.
        int byRoad, bySea;
        byRoad = bySea = TRUE; int moveType = LAND;
        if (move == SEA_UNKNOWN) { byRoad = FALSE; moveType = SEA; }

        int i, j;
        for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
            // if the ith location is in the hitlist, then find all locations connected
            // to that location (not by rail).
            if (hitList[LOCATIONS_START + i]) {
                int numMoves;
                int * movesFromLocation = whereCanTheyGoFrom(h, &numMoves, PLAYER_DRACULA, i, giveMeTheRound(h), 
                                                             byRoad, 0, bySea);
                
                // For every location that dracula could get to from location i, 
                // if it's of the right type (city or sea), then its a possible
                // location of dracula now.
                for (j = 0; j < numMoves; j++) {
                    int newMove = movesFromLocation[j];
                    
                    if (idToType(newMove) == moveType) {
                        possibleMoves[newMove] = 1; 
                    }
                }

                free (movesFromLocation);
            }
        }

        // Update the hitlist
        for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
            if (possibleMoves[i]) {
                hitList[LOCATIONS_START + i]++;
            }
            
            // If draculas moving to a city, then remove all locations from the
            // hitlist which are sea locations, vice versa.
            if (idToType(i) != moveType) {
                hitList[LOCATIONS_START + i] = 0;
            }
        }

        free (possibleMoves);

        // finally, if draculas in the sea, set approp. flag
        hitList[DRACULA_IN_SEA] = (move == CITY_UNKNOWN) ? FALSE : TRUE;

    } else if (move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5) {
        // So the question is, when was the last time we knew where 
        // dracula was. If he double backs to before when we knew, then we 
        // akwardly have to build new hitlist using the moves in reverse order.

        // say the trail is D5 X X X X X
        //                         ^LKL
        // prevKnownRound = 3
        // then n = 5
        // m = n - prevKnownRound = 2
        // so we build a new hitlist starting from newLoc
        // and add all the moves after (trail[4], trail[5])

        // say the trail is D5 X X X X X
        //                             ^LKL
        // prevKnownRound = 5
        // then n = 5
        // then we just grab the hitList from 5 turns ago.

        // say the trail is D1 X X X X X
        //                     ^LKL
        // prevKnownRound = 1
        // then n = 1
        // then we just grab the hitList from 1 turns ago.

        // say the trail is D1 X X X X X
        //                       ^LKL
        // prevKnownRound = 2
        // then n = 1
        // then we just grab the hitList from 1 turns ago.


        int prevKnownRound = hitList[ROUNDS_SINCE_LKL];
        int n = move - DOUBLE_BACK_1 + 1;
        char * newHitList;

        if (n > prevKnownRound) {
            // Dracula's double backed to a location from before his last known one.
            // So the idea is to rebuild the hit list, starting from the last known
            // location, using the moves in reverse order.
            int m = n - prevKnownRound;
            newHitList = buildHitList (hitList[LAST_KNOWN_LOCATION]);

            int i = 1;
            while (i <= m) {
                updateHitList (h, getTrailMove(h, PLAYER_DRACULA, giveMeTheRound(h) - currRound + i), 
                                newHitList, currRound - prevKnownRound - i);
                i++;
            }
        } else {
            // otherwise we can just grab the hitlist from that many rounds
            // ago. We'll grab it from ol' Mina Harker because she had the latest
            // hitList from then.!
            newHitList = getMessage (h, PLAYER_MINA_HARKER, currRound - n);
            msgToHitList (newHitList);
        }

        // But wherever dracula DOUBLE_BACKED to MUST be adjacent to his current location.
        // so we can compare the new hitlist with the old one.
        int i, j;
        for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
            if (!newHitList[LOCATIONS_START + i]) {
                continue;
            }

            int isValid = FALSE;

            for (j = 0; j < NUM_MAP_LOCATIONS; j++) {
                if (newHitList[LOCATIONS_START + j] && mDistance(h, i, j) <= 1) {
                    isValid = TRUE;
                    break;
                }
            }

            if (!isValid) {
                newHitList[LOCATIONS_START + i] = 0;
            }
        }

        // copy the new hitlist into the old
        int p;
        for (p = 0; p < MESSAGE_SIZE; p++) {
            hitList[p] = newHitList[p];
        }
        destroyHitList (newHitList);
    } 
}

void decideHunterMove(HunterView gameState) {
    // get the currPlayer and the round
    int currRound = giveMeTheRound (gameState);
    int whosTurn = whoAmI (gameState);

    char * hitList;
    int hitListChanged = FALSE;

    printf ("Here1 %d\n", mDistance(gameState, 70, 3));

    // The first thing we want is a hitlist, which is all the locations
    // that dracula could be right now. (plus some other information)
    if (currRound == 0) {
        if (whosTurn == PLAYER_LORD_GODALMING) { hitList = initHitList (); hitListChanged = TRUE; }
        else { hitList = getMessage (gameState, PLAYER_LORD_GODALMING, 0);  msgToHitList(hitList);}
    } else {
        // So the question is, have we learned any information
        int newLocation = FALSE;

        // grab the latest hitlist
        if (whosTurn == PLAYER_LORD_GODALMING) {
            hitList = getMessage (gameState, PLAYER_MINA_HARKER, currRound - 1);   
        } else {
            hitList = getMessage (gameState, whosTurn - 1, currRound); 
        }

        msgToHitList (hitList);

        // how long since we've known draculas location?
        int timeSinceLKL = hitList[ROUNDS_SINCE_LKL];

        // if the current player is Lord G, then timeSinceLKL has increased.
        if (whosTurn == PLAYER_LORD_GODALMING) {
            timeSinceLKL++;
        }

        // if timeSinceLKL = 0, then we already just found out
        // draculas location. Otherwise see if we've learned something 
        // new
        if (timeSinceLKL > 0) {
            // do we know dracula's location RIGHT NOW?
            // if so, build a new hitlist (unless time since  = 0, since
            // then a previous hunter already discovered them)
            int dracLocation = whereIs (gameState, PLAYER_DRACULA);
            if (validPlace (dracLocation)) {
                hitListChanged = TRUE;
                newLocation = TRUE;
                
                destroyHitList (hitList);
                hitList = buildHitList (dracLocation);
            } 

            // otherwise, maybe a hunter has moved to a location in
            // draculas trail and we've learned something.
            if (!newLocation) {
                // Load draculas trail
                int dracTrail[TRAIL_SIZE];
                giveMeTheTrail (gameState, PLAYER_DRACULA, dracTrail);

                // for each move in the trail made after the LKL, 
                // check if we've learned where dracula was.
                int i = 0;
                while (i < timeSinceLKL && i < TRAIL_SIZE) {
                    if (validPlace (dracTrail[i])) {
                        newLocation = TRUE;
                        // if we've found a new location, then create a
                        // new hitlist starting from that location.
                        destroyHitList (hitList);
                        hitList = buildHitList (dracTrail[i]);

                        // for every move made afterwards, update the hitlist.
                        int j = i - 1;
                        while (j >= 0) {
                            updateHitList (gameState, dracTrail[j], hitList, currRound - (j+1));
                            j--;
                        }
                    }

                    // if we've found a new known location, we're out.
                    if (newLocation) { hitListChanged = TRUE; break; }
                    i++;
                }
                
                // if we still havent found a new location and we're lord GOLDAMING
                // then update the latest move.
                if (!newLocation && whosTurn == PLAYER_LORD_GODALMING) {
                    hitListChanged = TRUE; 
                    updateHitList (gameState, getTrailMove (gameState, PLAYER_DRACULA, 0), 
                                    hitList, currRound);
                }

                // if we learnt something or not, the fact that we
                // don't know where dracula is RIGHT NOW means that
                // dracula isnt in any location where the previous 
                // hunters moved to this round, nor is he in any location
                // where the yet-to-move hunters are.
                for (i = 0; i < NUM_PLAYERS - 1; i++) {
                    int hunterLoc = (i < whosTurn) ? getTrailMove (gameState, i, 0) : whereIs (gameState, i);
                    if(idToType(hunterLoc) == LAND && hitList[LOCATIONS_START + hunterLoc]) {
                        hitList[LOCATIONS_START + hunterLoc] = 0;
                        hitListChanged = TRUE; 
                    } 
                }
            }
        }
    }

    printf ("Here2 %d\n", mDistance(gameState, 70, 3));

    int p; 
    printf ("WHATS GOING ON: ");
    for (p = 0; p < MESSAGE_SIZE; p++){
        printf ("%d", hitList[p]);
    }
    printf ("\n");

    int desiredLocation;

    // if the hitlist has changed, we need to work out new places to go
    if (hitListChanged) {
        // So we have a new hitlist! Now it's time to choose where each hunter will
        // go. Dracula is either on sea or land. If he's at the sea, we'll try
        // and aim for the hunters to go for good choices of port cities. Otherwise
        // aim for the hunters to choose to go to cities in the hitlist such that
        // theyre well distributed.

        printf ("Here3 %d\n", mDistance(gameState, 70, 3));
        int numDests = 0;
        int * possibleDests;
        if (hitList[DRACULA_IN_SEA]) {
            // for each location in the hitlist, find the port cities next to it.
            int neighbourPortCities[NUM_MAP_LOCATIONS] = {0};
            printf ("Here4SEA %d\n", mDistance(gameState, 70, 3));
            int i;
            for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
                if (hitList[LOCATIONS_START + i]) {
                    int numNearbyCities;
                    int *nearbyCities = whereCanTheyGoFrom (gameState, &numNearbyCities, PLAYER_DRACULA, i, 
                                                            giveMeTheRound(gameState), 0, 0, 1);

                    int j;
                    for (j = 0; j < numNearbyCities; j++) {
                        int c = nearbyCities[j];
                        if (idToType (c) == SEA) {continue;}
                        if (!neighbourPortCities[c]) {
                            numDests++;
                            neighbourPortCities[c] = 1;
                        }
                    }

                    free (nearbyCities);
                }
            }

            possibleDests = malloc (sizeof (int) * numDests);
            int j = 0;
            for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
                if (neighbourPortCities[i]) {
                    possibleDests[j] = i;
                    j++;
                }
            }
        } else {
            // Count how many locations in the hitlist.
            printf ("Here4LAND %d\n", mDistance(gameState, 70, 3));
            int i;
            for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
                if (hitList[LOCATIONS_START + i]) { numDests++; }
            }
            printf ("Here4LAND2 %d\n", mDistance(gameState, 70, 3));

            // allocate enough memory
            possibleDests = malloc (sizeof (int) * numDests);

            // fill the array
            int j = 0;
            for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
                if (hitList[LOCATIONS_START + i]) {
                    possibleDests[j] = i;
                    j++;
                }
            }  
            printf ("Here4LAND3 %d\n", mDistance(gameState, 70, 3));
        }


        // If the number of destinations is greater than 4 (usually is), then
        // find the medoids!
        int * dests;
        if (numDests > 4) {
            dests = medoids (gameState, possibleDests, numDests, 4);
            numDests = 4;
        } else {
            dests = possibleDests;
        }

        printf ("Here4 %d\n", mDistance(gameState, 70, 3));

        // So we have between 1 and 4 choices of desinations. Want each hunter
        // to choose to move to a destination such that
        //          - Each destination has at least one hunter moving to it.
        //          - Total distance between each desitination and the hunter(s)
        //            visiting is minimised.

        // note: this is CRUDE.
        
        int * visited = calloc (numDests, sizeof(int));
        int bestChoices[NUM_PLAYERS - 1];
        int bestCost = POS_INF;
        int i,j,k,l;
        for (i = 0; i < numDests; i++) {
            visited[i]++;
            for (j = 0; j < numDests; j++) {
                visited[j]++;
                for (k = 0; k < numDests; k++) {
                    visited[k]++;
                    for (l = 0; l < numDests; l++) {
                        visited[l]++;
                        // HERE: if the number of clashes is less then permitted,
                        // work out the cost of this config.
                        int v, isValidConfig = TRUE;
                        for (v = 0; v < numDests; v++) {
                            if (!visited[v]) {isValidConfig = FALSE;}
                        }
            
                        if (isValidConfig) { 
                            int cost = mDistance(gameState, whereIs(gameState, PLAYER_LORD_GODALMING), dests[i]) +
                                       mDistance(gameState, whereIs(gameState, PLAYER_DR_SEWARD), dests[j]) +
                                       mDistance(gameState, whereIs(gameState, PLAYER_VAN_HELSING), dests[k]) +
                                       mDistance(gameState, whereIs(gameState, PLAYER_MINA_HARKER), dests[l]);

                            if (cost < bestCost) {
                                bestCost = cost;
                                bestChoices[PLAYER_LORD_GODALMING] = dests[i];
                                bestChoices[PLAYER_DR_SEWARD] = dests[j];
                                bestChoices[PLAYER_VAN_HELSING] = dests[k];
                                bestChoices[PLAYER_MINA_HARKER] = dests[l];
                            }
                        }
                        visited[l]--;
                    }
                    visited[k]--;
                }
                visited[j]--;
            }
            visited[i]--;
        }
        
        printf ("Step 9\n");

        free (visited);
        free (dests);
        if (numDests > 4) {
            free (possibleDests);
        }

        // Now that we have the best choices, update the mediods section of the hitList.
        for (i = 0; i < NUM_PLAYERS - 1; i++) {
            hitList[MEDOIDS_START + i] = bestChoices[i];
        }
        
        printf ("Step 9\n");

        // Also the desired location is the one corresponding to this player.
        desiredLocation = bestChoices[whosTurn];

    } else {
        // if the hitlist hasnt changed, then just set our desired location to be
        // as specified in the previous one. 
        desiredLocation = hitList[MEDOIDS_START + whosTurn];
    }


    // if its the first ever round then just go there. Otherwise need to find the
    // best way to get to our desired location.
    
    printf ("Step 11\n");
    if (currRound != 0) {
        // so how we gunna get there?
        // generate list of all places i can go
        // choose the one which takes me closest to where I want to be
        // caveat: I also would prefer to visit places in the hitlist
        // so theres a certain trade off between how close we get to the desired location
        // and visiting places in the hitlist
        // so for the moment lets say that the score of each move is
        //      (how close I am - how close I can be) * (score of the location + 3)
        int numMoves;
        int * myMoves = whereCanIgo (gameState, &numMoves, 1, 1, 1);
        
        printf ("Step 12\n");

        int bestMove;
        int bestScore = -POS_INF;

        int baseDistance = mDistance (gameState, whereIs (gameState, whosTurn), desiredLocation);
        int i;
        for (i = 0; i < numMoves; i++) {
            int score = (baseDistance - mDistance (gameState, myMoves[i], desiredLocation)) * (hitList[LOCATIONS_START + myMoves[i]] + 3);
            if (score > bestScore) {
                bestScore = score;
                bestMove = myMoves[i];
            }
        } 
        
        printf ("Step 14\n");

        free (myMoves);

        desiredLocation = bestMove;
    }

    //int p; 
    printf ("WHATS GOING ON: ");
    for (p = 0; p < MESSAGE_SIZE; p++){
        printf ("%d", hitList[p]);
    }
    printf ("\n");

    hitListToMsg (hitList);
    printf ("Here1 %d\n", mDistance(gameState, 70, 3));
   // printf ("\nMessage2: %s\n", hitList);
    registerBestPlay(idToAbbrev (desiredLocation), hitList);
}


// 
