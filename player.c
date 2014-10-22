/**
 * Runs a player's game turn ...
 *
 * Can produce either a Hunter player or a Dracula player
 * depending on the setting of the I_AM_DRACULA #define
 *
 * This is a dummy version of the real player.c used when
 * you submit your AIs. It is provided so that you can
 * test whether your code is likely to compile ...
 *
 * Note that it used to drive both Hunter and Dracula AIs.
 * It first creates an appropriate view, and then invokes the
 * relevant decide-my-move function, which should use the 
 * registerBestPlay() function to send the move back.
 *
 * The real player.c applies a timeout, so that it will halt
 * decide-my-move after a fixed amount of time, if it doesn't
 * finish first. The last move registered will be the one
 * used by the game engine. This version of player.c won't stop
 * your decide-my-move function if it goes into an infinite
 * loop. Sort that out before you submit.
 *
 * Based on the program by David Collien, written in 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
#include "DracView.h"
#include "dracula.h"
#else
#include "HunterView.h"
#include "hunter.h"
#endif

// moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

int main(int argc, char *argv[])
{
#ifdef I_AM_DRACULA
   DracView gameState;
   char *plays = "GMU.... SMA.... HPA.... MCD.... DAT.V.. GZA.... SSR.... HLE.... MCD.... DVAT... GSZ.... SBO.... HPA.... MCD.... DSJT... GBC.... SBB.... HLE.... MCD.... DZAT... GKL.... SNA.... HPA.... MCD.... DMUT... GCD.... SPA.... HLE.... MCD.... DVET... GCD.... SPA.... HLE.... MCD.... DAS..V. GKL.... SST.... HPA.... MKL.... DBIT.M. GCD.... SFR.... HLE.... MCD.... DNPT.M. GCD.... SST.... HPA.... MCD.... DROT.M. GCD.... SFR.... HST.... MCD.... DHIT.M. GCD.... SST.... HFR.... MCD.... DD1T.M. GCD.... SST.... HFR.... MCD.... DTS.... GKL.... SFR.... HST.... MKL.... DCG.VM. GCD.... SST.... HNU.... MCD.... DMS..M. GCD.... SNU.... HMU.... MCD.... DALT.M. GCD.... SMU.... HZA.... MCD.... DGRT.M. GCD.... SZA.... HSZ.... MCD.... DCAT.M. GCD.... SZA.... HSZ.... MCD.... DLST... GKL.... SVI.... HBD.... MKL.... DMAT.V. GCD.... SBD.... HSZ.... MCD.... DSNT... GCD.... SSZ.... HBC.... MCD.... DSRT.M. GCD.... SBC.... HKL.... MCD.... DHIT.M. GCD.... SKL.... HCD.... MCD.... DALT.M. GCD.... SKL.... HCD.... MCD.... DGRT.M. GKL.... SSZ.... HKL.... MKL.... DCAT.M. GCD.... SBC.... HCD.... MCD.... DLS.VM. GCD.... SKL.... HCD.... MCD.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DSNT.M. GCD.... SCD.... HCD.... MCD.... DSRT.M. GKL.... SKL.... HKL.... MKL.... DALT.M. GCD.... SCD.... HCD.... MCD.... DD2T.M. GCD.... SCD.... HCD.... MCD.... DHIT.V. GKL.... SKL.... HKL.... MKL.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GCD.... SCD.... HCD.... MCD.... DSNT.M. GKL.... SKL.... HKL.... MKL.... DBB..M. GCD.... SCD.... HCD.... MCD.... DD2T.M. GCD.... SCD.... HCD.... MCD.... DSRT.M. GKL.... SKL.... HKL.... MKL.... DAL.VM. GCD.... SCD.... HCD.... MCD.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GKL.... SKL.... HKL.... MKL.... DCAT... GCD.... SCD.... HCD.... MCD.... DD2T.M. GCD.... SCD.... HCD.... MCD.... DSNT.M. GKL.... SKL.... HKL.... MKL.... DSRT.V. GCD.... SCD.... HCD.... MCD.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DHIT.M. GKL.... SKL.... HKL.... MKL.... DALT.M. GCD.... SCD.... HCD.... MCD.... DD2T.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GKL.... SKL.... HKL.... MKL.... DCAT.M. GCD.... SCD.... HCD.... MCD.... DGR.VM. GKL.... SKL.... HKL.... MKL.... DHIT.M. GKL.... SKL.... HKL.... MKL.... DALT.M. GCD.... SCD.... HCD.... MCD.... DMAT.M. GKL.... SKL.... HKL.... MKL.... DLST.M. GCD.... SCD.... HCD.... MCD.... DSNT.M. GKL.... SKL.... HKL.... MKL.... DSRT.V. GCD.... SCD.... HCD.... MCD.... DHIT.M. GCD.... SCD.... HCD.... MCD.... DD1T.M. GKL.... SKL.... HKL.... MKL.... DALT.M. GCD.... SCD.... HCD.... MCD.... DMAT.M. GKL.... SKL.... HKL.... MKL.... DCAT.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GKL.... SKL.... HKL.... MKL.... DSN.VM. GKL.... SKL.... HKL.... MKL.... DSRT.M. GSZ.... SSZ.... HSZ.... MSZ.... DALT.M. GKL.... SBC.... HBC.... MBC.... DGRT.M. GCD.... SGA.... HGA.... MKL.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GCD.... SCD.... HCD.... MCD.... DSNT.V. GKL.... SKL.... HKL.... MKL.... DSRT.M. GCD.... SCD.... HCD.... MCD.... DHIT.M. GCD.... SCD.... HCD.... MCD.... DD1T.M. GKL.... SKL.... HKL.... MKL.... DALT.M. GCD.... SCD.... HCD.... MCD.... DMAT.M. GCD.... SCD.... HCD.... MCD.... DLST.M. GKL.... SKL.... HKL.... MKL.... DCA.VM. GCD.... SCD.... HCD.... MCD.... DGRT.M. GCD.... SGA.... HGA.... MGA.... DD4T.M. GGA.... SCD.... HCD.... MCD.... DSNT.M. GCD.... SGA.... HGA.... MGA.... DSRT.M. GGA.... SCD.... HCD.... MCD.... DHIT.M. GGA.... SCD.... HCD.... MCD.... DALT.V. GBC.... SKL.... HKL.... MKL.... DGRT.M. GGA.... SCD.... HCD.... MCD.... DMAT.M. GCD.... SGA.... HGA.... MGA.... DLST.M. GGA.... SCD.... HCD.... MCD.... DSNT.M. GCD.... SGA.... HGA.... MGA.... DSRT.M. GCD.... SGA.... HGA.... MGA.... DHIT.M. GKL.... SCN.... HBC.... MBC.... DD1.VM. GCD.... SBC.... HGA.... MGA.... DMAT.M. GGA.... SGA.... HCD.... MCD.... DLST.M. GCD.... SCD.... HGA.... MGA.... DSNT.M. GGA.... SGA.... HCD.... MCD.... DBB..M. GGA.... SGA.... HCD.... MCD....";
   PlayerMessage msgs[3] = {"","",""};
   gameState = newDracView(plays,msgs);
   decideDraculaMove(gameState);
   disposeDracView(gameState);
#else
   HunterView gameState;
      char *plays = "GMU.... SCA.... HBE.... MCF.... DC?.V.. GST.... SAO.... HSO.... MMR.... DC?T... GMU.... SCA.... HBE.... MTO.... DC?T... GFR.... SAO.... HSO.... MSR.... DD1T... GST.... SEC.... HBC.... MMR.... DTPT... GMU.... SAO.... HGA.... MMI.... DGAT... GVI.... SMS.... HGATD.. MMU.... DC?T.V.";
   PlayerMessage msgs[35] = {" !!!\"\"\"!\"\"!\"\"!\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"\"\"!!\"\"\"\"\"\"\"\"\"!\"\"\"\"!\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"!N0)3!!!!!!!!!!!!!!!!!!!!",
    " !!!\"\"\"!\"\"!\"\"!\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"\"\"!!\"\"\"\"\"\"\"\"\"!\"\"\"\"!\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"!N0)3!!!!!!!!!!!!!!!!!!!!",
    " !!!\"\"\"!\"\"!\"\"!\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"\"\"!!\"\"\"\"\"\"\"\"\"!\"\"\"\"!\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"!N0)3!!!!!!!!!!!!!!!!!!!!",
    " !!!\"\"\"!\"\"!\"\"!\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"\"\"!!\"\"\"\"\"\"\"\"\"!\"\"\"\"!\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"!\"\"\"\"\"\"!N0)3!!!!!!!!!!!!!!!!!!!!",
    "And so it begins...",
    " \"!!###!##!!#!####!##!####!########!!#########!#!##!###############!######!4H[K!!!!!!!!!!!!!!!!!!!!",
    " \"!!###!##!!#!####!##!####!########!!#########!#!##!##########!####!######!EH[K!!!!!!!!!!!!!!!!!!!!",
    " \"!!###!##!!#!####!##!####!########!!#########!#!##!##########!####!######!EH[K!!!!!!!!!!!!!!!!!!!!",
    " \"!!###!##!!#!####!##!####!########!!#########!#!##!#########!!####!######!EH.K!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)",
    " #!!$$$!$$!\"$!$$$$\"$$\"$$$$!$$$$$$$$!!$$$$$$$$!!$\"$$!$$$$$$$$$!!$$$$!$$$$$$!N0)S!!!!!!!!!!!!!!!!!!!!",
    " #!!$$$!$$!\"$!$$$$\"$$\"$$$$!$$$$$$$$!!$$$$$$$$!!$!$$!$$$$$$$$$!!$$$$!$$$$$$!40)>!!!!!!!!!!!!!!!!!!!!",
    " #!!$$$!$$!\"$!$$$$!$$\"$$$$!$$$$$$$$!!$$$$$$$$!!$!$$!$$$$$$$$$!!$$$$!$$$$$$!@Y)>!!!!!!!!!!!!!!!!!!!!",
    " #!!$$$!$$!!$!$$$$!$$\"$$$$!$$$$$$$$!!$$$$$$$$!!$!$$!$$$$$$$$$!!$$$$!$$$$$$!d@.Y!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%%%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%%%\"\"%%%!!%%%%%%!4H[K!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%%%\"\"%%%!!%%%%%%!\\H[\"!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%%%\"\"%%%!!%%%%%%!\\H[\"!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%%%!\"%%%!!%%%%%%!\\H.\"!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%!%!\"%%%!!%%%%%%!\\H.\"!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%!%!!%%%!!%%%%%%!EH.K!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%%%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%!%!!%%%!!%%%%%%!EH.K!!!!!!!!!!!!!!!!!!!!",
    " $!!%%%!%%!!%!%%!%!%%#%%%%!%!%%%%%%!!%%%%%%%%\"!%!%%!%%%%%%%!%!!%%%!!%%%%%%!EHZK!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)",
    "2!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2222!!!!!!!!!!!!!!!!!!!!",
    "2!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2222!!!!!!!!!!!!!!!!!!!!",
    "2!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2222!!!!!!!!!!!!!!!!!!!!",
    "2!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2222!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)",
    ";!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!;;;;!!!!!!!!!!!!!!!!!!!!",
    ";!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!;;;;!!!!!!!!!!!!!!!!!!!!",
    ";!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!;;;;!!!!!!!!!!!!!!!!!!!!",
    ";!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!;;;;!!!!!!!!!!!!!!!!!!!!",
    "Please don't find me :)"};
   gameState = newHunterView(plays,msgs);
   decideHunterMove(gameState);
   disposeHunterView(gameState);
#endif 
   printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
   return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay (char *play, PlayerMessage message) {
   strncpy(latestPlay, play, MOVE_SIZE-1);
   latestPlay[MOVE_SIZE-1] = '\0';
     
   strncpy(latestMessage, message, MESSAGE_SIZE);
   latestMessage[MESSAGE_SIZE-1] = '\0';
}
