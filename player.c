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
   char *plays = "GMU.... SCA.... HBE.... MCF.... DMN.V.. GST.... SAO.... HSO.... MMR.... DHIT... GMU.... SCA.... HBE.... MTO.... DEDT... GFR.... SAO.... HSO.... MSR.... DD2T... GNU.... SEC.... HBC.... MLS.... DLVT... GLI.... SLO.... HBE.... MAO.... DIR.... GHA.... SEC.... HSJ.... MBB.... DDUT.V. GNS.... SAO.... HVA.... MBO.... DHIT.M. GEC.... SMS.... HSO.... MMA.... DGWT.M. GLO.... SMR.... HBC.... MCA.... DD1T.M. GLVT... SMI.... HBE.... MGR.... DAO.... GIR.... SMR.... HSJ.... MCA.... DEC.... GAO.... SMS.... HVA.... MAO.... DLET.M. GNS.... STS.... HSO.... MEC.... DBU.VM. GAM.... SGO.... HVA.... MLO.... DPAT.M. GBUV... SMR.... HSJ.... MEC.... DNAT.M. GCO.... SPAT... HZA.... MLET... DBB.... GAM.... SBO.... HMU.... MEC.... DAO.... GNS.... SBO.... HST.... MAO.... DEC.... GEC.... SBB.... HGE.... MAO.... DNS.... GAO.... SAO.... HGO.... MMS.... DEDT... GMS.... SIR.... HRO.... MBA.... DMNT.M. GMR.... SLV.... HNP.... MTO.... DLOT... GCF.... SSW.... HBI.... MSR.... DD2T... GBO.... SLOT... HNP.... MLE.... DLVT... GBB.... SMNTT.. HTS.... MEC.... DIR.... GAO.... SLVT... HMS.... MAO.... DDU.VM. GGW.... SSW.... HAO.... MGW....";
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
