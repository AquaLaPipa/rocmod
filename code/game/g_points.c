#include "g_local.h"

// Function prototypes
void UpdatePlayerPoints(gentity_t *player, int pointsAwarded);
void AwardFlagKillPoints(gentity_t *attacker, gentity_t *self);
void AwardFlagCapturePoints(gentity_t *other);
void AwardFlagDefendPoints(gentity_t *ent);
void AwardFlagDroppedPoints(gentity_t *ent);

void AwardFirstBloodPoints(gentity_t *attacker);
void AwardKillSpreePoints(gentity_t *attacker);
void AwardRampagePoints(gentity_t *attacker);
void AwardDominatingPoints(gentity_t *attacker);
void AwardUnstoppablePoints(gentity_t *attacker);
void AwardGodlikePoints(gentity_t *attacker);
void AwardSpreeStopperPoints(gentity_t *attacker);

void AwardHeadshotPoints(gentity_t *attacker);
void UpdateVoteDisplayString(gentity_t *ent);

// Rank system
const char* ranks[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
    "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
    "31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
    "41", "42", "43", "44", "45", "46", "47", "48", "49", "50",
    "51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
    "61", "62", "63", "64", "65", "66", "67", "68", "69", "70",
    "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
    "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
    "91", "92", "93", "94", "95", "96", "97", "98", "99", "100"
};

// Points required system
const int pointsRequired[] = {
    0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
    100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195,
    200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260, 265, 270, 275, 280, 285, 290, 295,
    300, 305, 310, 315, 320, 325, 330, 335, 340, 345, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395,
    400, 405, 410, 415, 420, 425, 430, 435, 440, 445, 450, 455, 460, 465, 470, 475, 480, 485, 490, 495, 500
};

#define NUM_RANKS (sizeof(ranks) / sizeof(ranks[0]))

void UpdatePlayerPoints(gentity_t *player, int pointsAwarded) {
    int newRankIndex = player->client->sess.currentRankIndex;
    player->client->sess.totalPoints += pointsAwarded;

    while (newRankIndex < NUM_RANKS - 1 && player->client->sess.totalPoints >= pointsRequired[newRankIndex + 1]) {
        newRankIndex++;
    }

    if (newRankIndex != player->client->sess.currentRankIndex) {
        player->client->sess.currentRankIndex = newRankIndex;
        G_LogPrintf("Player %s promoted to %s\n", player->client->pers.netname, ranks[newRankIndex]);

        // Send the promotion message to the player
        trap_SendServerCommand(player - g_entities, va("cp \"^<LEVEL UP\n^7%s!\"", ranks[newRankIndex]));

        // Announce the promotion to all players
        trap_SendServerCommand(-1, va("chat \"^7%s ^yhas been promoted to ^1%s!\n\"", player->client->pers.netname, ranks[newRankIndex]));
        G_LocalSound(player->s.number, "sound/misc/events/rifle_shot.mp3");
        ClientUserinfoChanged(player->client->ps.clientNum);
    }
}

/*
===============
FLAG EVENTS
===============
*/

// POINTS : FLAG SLAYER - Kill someone while carrying a flag.
void AwardFlagKillPoints(gentity_t *attacker, gentity_t *self) {
    if (attacker && attacker->client && self && self->client) {
        if (attacker->s.number == level.redFlagCarrier || attacker->s.number == level.blueFlagCarrier) {
            if (g_points_flagkill.integer > 0) {
                int pointsAwarded = g_points_flagkill.integer;
                trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^<+%d ^JPoints  ^+ ^1FLAG SLAYER ^+  ^,- ^7Killed enemy whilst holding flag.\n\"", pointsAwarded));
                trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Kill^7)\n\"", attacker->client->pers.netname, pointsAwarded));
                G_LocalSound(attacker->s.number, "sound/misc/events/elev_beep2.mp3");
                G_AddScore(attacker, pointsAwarded);
				UpdatePlayerPoints(attacker, pointsAwarded); // Call UpdatePlayerPoints with pointsAwarded
            }
        }
    }
}

// POINTS : FLAG CAPTURE EVENTS - Capture a flag with points.
void AwardFlagCapturePoints(gentity_t *other) {
    if (g_points_flagcapture.integer > 0 && level.gametypeData->teams) {
        int additionalPoints = g_points_flagcapture.integer;
        int totalPointsAwarded = 10 + additionalPoints; // Including the 10 points automatically added by the game
        int pointsAwarded = totalPointsAwarded; // Declare and initialize pointsAwarded

        // Check if the player captured the flag without taking damage
        if (!other->client->sess.tookDamageSinceFlagPickup && g_points_flagnodamage.integer > 0) {
            int noDamagePoints = g_points_flagnodamage.integer;
            pointsAwarded += noDamagePoints;
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1UNTOUCHABLE FLAG ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$No Damage Flag Capture^7)\n\"", other->client->pers.netname, pointsAwarded));
        } else {
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1FLAG MASTER ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Capture^7)\n\"", other->client->pers.netname, pointsAwarded));
        }
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, totalPointsAwarded);
        UpdatePlayerPoints(other, pointsAwarded); // Call UpdatePlayerPoints with pointsAwarded

        // Reset the flag
        other->client->sess.tookDamageSinceFlagPickup = qfalse;
    }

    // Additional Points for 5 Flag Captures - BuLLy
    if (other->client->sess.modData->flagcaps == 5) {
        int milestonePoints5 = g_points_flagcapture_5.integer;
        trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1FLAG SPECIALIST ^+ ^<+%d ^JPoints\n\"", milestonePoints5));
        trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$5 Flag Captures^7)\n\"", other->client->pers.netname, milestonePoints5));
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, milestonePoints5);
        UpdatePlayerPoints(other, milestonePoints5);
    }

    // Additional Points for 10 Flag Captures - BuLLy
    if (other->client->sess.modData->flagcaps == 10) {
        int milestonePoints10 = g_points_flagcapture_10.integer;
        trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1FLAG CRUSADER ^+ ^<+%d ^JPoints\n\"", milestonePoints10));
        trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$10 Flag Captures^7)\n\"", other->client->pers.netname, milestonePoints10));
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, milestonePoints10);
        UpdatePlayerPoints(other, milestonePoints10);
    }

    // Points for Last Second Flag Capture or First 10 Seconds Capture - BuLLy
    if ((g_points_flagstart.integer > 0 || g_points_flagend.integer > 0) && level.gametypeData->teams) {
        int remainingTime = (g_timelimit.integer * 60000) - (level.time - level.startTime);
        int lastSeconds = 10000; // 10 seconds
        int firstTenSeconds = 10000; // First 10 seconds

        if (remainingTime <= lastSeconds) {
            int pointsAwarded = g_points_flagend.integer;
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1FLAG HERO ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Capture in last 10 seconds^7)\n\"", other->client->pers.netname, pointsAwarded));
            G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
            G_AddScore(other, pointsAwarded);
            UpdatePlayerPoints(other, pointsAwarded);
        } else if (level.time - level.startTime <= firstTenSeconds) {
            int pointsAwarded = g_points_flagstart.integer; // Use the same CVAR for simplicity
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1FLAG HUNGRY ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Capture in first 10 seconds^7)\n\"", other->client->pers.netname, pointsAwarded));
            G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
            G_AddScore(other, pointsAwarded);
            UpdatePlayerPoints(other, pointsAwarded);
        }
    }

    // Check for Quick Capture - BuLLy
    if (g_points_flagquick.integer > 0) {
        int timeSinceFlagPickup = level.time - other->client->sess.flagPickupTime;
        int speedyFlagThreshold = 10000; // 10 seconds for example

        if (timeSinceFlagPickup <= speedyFlagThreshold) {
            int speedyFlagPoints = g_points_flagquick.integer;
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1SPEEDY FLAG ^+ ^<+%d ^JPoints\n\"", speedyFlagPoints));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Quick Capture^7)\n\"", other->client->pers.netname, speedyFlagPoints));
            G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
            G_AddScore(other, speedyFlagPoints);
            UpdatePlayerPoints(other, speedyFlagPoints);
        }
    }

    // Check for Critical Capture - BuLLy
    if (g_points_flagequalizer.integer > 0) {
        int blueScore = level.teamScores[TEAM_BLUE];
        int redScore = level.teamScores[TEAM_RED];

        if ((other->client->sess.team == TEAM_BLUE && blueScore == redScore + 1) || 
            (other->client->sess.team == TEAM_RED && redScore == blueScore + 1)) {
            int equalizerPoints = g_points_flagequalizer.integer;
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^+ ^1THE EQUALIZER ^+ ^<+%d ^JPoints\n\"", equalizerPoints));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Equalizer^7)\n\"", other->client->pers.netname, equalizerPoints));
            G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
            G_AddScore(other, equalizerPoints);
            UpdatePlayerPoints(other, equalizerPoints);
        }
    }
}
void AwardFlagDefendPoints(gentity_t *ent) {
    if (g_points_flagdefend.integer > 0) {
        int pointsAwarded = g_points_flagdefend.integer;
        trap_SendServerCommand(ent - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Flag Defender)\n\"", ent->culprit->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(ent->culprit - g_entities, va("chat -1 \"^+ ^1FLAG KEEPER ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(ent->culprit, pointsAwarded);
        G_LocalSound(ent->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(ent->culprit, pointsAwarded);
    }
}

void AwardFlagDroppedPoints(gentity_t *ent) {
    if (g_points_flagdropped.integer > 0) {
        int pointsAwarded = g_points_flagdropped.integer;
        trap_SendServerCommand(ent - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Flag Attempt)\n\"", ent->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(ent - g_entities, va("chat -1 \"^+ ^1FLAG RUNNER ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(ent, pointsAwarded); // Subtract points from the player's score
        G_LocalSound(ent->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(ent, pointsAwarded);
    }
}

/*
===============
KILLING SPREES
===============
*/
void AwardFirstBloodPoints(gentity_t *attacker) {
    if (g_points_firstblood.integer > 0) {
        int pointsAwarded = g_points_firstblood.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(First Blood)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yBLOOD THIRSTY ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardKillSpreePoints(gentity_t *attacker) {
    if (g_points_spree.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_spree.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Killing Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yKILLING SPREE ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardRampagePoints(gentity_t *attacker) {
    if (g_points_rampage.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_rampage.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Rampage)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yRAMPAGE ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardDominatingPoints(gentity_t *attacker) {
    if (g_points_dominating.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_dominating.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Dominating)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yDOMINATING ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardUnstoppablePoints(gentity_t *attacker) {
    if (g_points_unstoppable.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_unstoppable.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Unstoppable Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yUNSTOPPABLE ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardGodlikePoints(gentity_t *attacker) {
    if (g_points_godlike.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_godlike.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Godlike Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^yGODLIKE ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}

void AwardSpreeStopperPoints(gentity_t *attacker) {
    if (g_points_spreestopper.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_spreestopper.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Spree Stopper)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^7TAXMAN ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);

        attacker->client->sess.modData->spreeStops++;
        
        // Additional Points for stopping 5 Sprees
        if (attacker->client->sess.modData->spreeStops == 5) {
            int spreeStopper5Points = g_points_spreestopper_5.integer;
            trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^7SPREE BREAKER ^+ %s ^JPoints\n\"", spreeStopper5Points));
            trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned %d ^7Points (^$5 Sprees Stopped^7)\n\"", attacker->client->pers.netname, spreeStopper5Points));
            G_AddScore(attacker, spreeStopper5Points);
			UpdatePlayerPoints(attacker, spreeStopper5Points);
        }

        // Additional Points for stopping 10 Sprees
        if (attacker->client->sess.modData->spreeStops == 10) {
            int spreeStopper10Points = g_points_spreestopper_10.integer;
            trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^7SPREE TERMINATOR ^+ %s ^JPoints\n\"", spreeStopper10Points));
            trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned %d ^7Points (^$10 Sprees Stopped^7)\n\"", attacker->client->pers.netname, spreeStopper10Points));
            G_AddScore(attacker, spreeStopper10Points);
			UpdatePlayerPoints(attacker, spreeStopper10Points);
        }
    }
}

/*
===============
GAMEPLAY EVENTS
===============
*/

void AwardHeadshotPoints(gentity_t *attacker) {
    if (g_points_headshot.integer > 0) {
        int pointsAwarded = g_points_headshot.integer;
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^+ ^3GOOD AIM ^+ ^<+%d ^JPoints\n\"", pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Headshot^7)\n\"", attacker->client->pers.netname, pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
		UpdatePlayerPoints(attacker, pointsAwarded);
    }
}


