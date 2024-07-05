#include "g_local.h"

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
                trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^<+%d ^JPoints ^$ ^yFLAG SLAYER ^$ ^,- ^7Killed enemy whilst holding flag.\n\"", pointsAwarded));
                trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Kill^7)\n\"", attacker->client->pers.netname, pointsAwarded));
                G_LocalSound(attacker->s.number, "sound/misc/events/elev_beep2.mp3");
                G_AddScore(attacker, pointsAwarded);
            }
        }
    }
}

// POINTS : FLAG CAPTURE EVENTS - Capture a flag with points.
void AwardFlagCapturePoints(gentity_t *other) {
    if (g_points_flagcapture.integer > 0 && level.gametypeData->teams) {
        int additionalPoints = g_points_flagcapture.integer;
        int totalPointsAwarded = 10 + additionalPoints; // Including the 10 points automatically added by the game

        // Check if the player captured the flag without taking damage
        if (!other->client->sess.tookDamageSinceFlagPickup && g_points_flagnodamage.integer > 0) {
            int noDamagePoints = g_points_flagnodamage.integer;
            totalPointsAwarded += noDamagePoints;
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^$^+ ^7UNTOUCHABLE FLAG ^+^$ ^<+%d ^JPoints\n\"", totalPointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$No Damage Flag Capture^7)\n\"", other->client->pers.netname, totalPointsAwarded));
        } else {
            trap_SendServerCommand(other - g_entities, va("chat -1 \"^$^+ ^7FLAG MASTER ^+^$ ^<+%d ^JPoints\n\"", totalPointsAwarded));
            trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Flag Capture^7)\n\"", other->client->pers.netname, totalPointsAwarded));
        }
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, additionalPoints);

        // Reset the flag
        other->client->sess.tookDamageSinceFlagPickup = qfalse;
    }

    // Additional Points for 5 Flag Captures - BuLLy
    if (other->client->sess.modData->flagcaps == 5) {
        int milestonePoints5 = g_points_flagcapture_5.integer;
        trap_SendServerCommand(other - g_entities, va("chat -1 \"^$^+ ^7FLAG SPECIALIST ^+^$ ^<+%d ^JPoints\n\"", milestonePoints5));
        trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$5 Flag Captures^7)\n\"", other->client->pers.netname, milestonePoints5));
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, milestonePoints5);
    }

    // Additional Points for 10 Flag Captures - BuLLy
    if (other->client->sess.modData->flagcaps == 10) {
        int milestonePoints10 = g_points_flagcapture_10.integer;
        trap_SendServerCommand(other - g_entities, va("chat -1 \"^$^+ ^7FLAG CRUSADER ^+^$ ^<+%d ^JPoints\n\"", milestonePoints10));
        trap_SendServerCommand(other - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$10 Flag Captures^7)\n\"", other->client->pers.netname, milestonePoints10));
        G_LocalSound(other->s.number, "sound/misc/events/elev_beep2.mp3");
        G_AddScore(other, milestonePoints10);
    }
}

void AwardFlagDefendPoints(gentity_t *ent) {
    if (g_points_flagdefend.integer > 0) {
        int pointsAwarded = g_points_flagdefend.integer;
        trap_SendServerCommand(ent - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Flag Defender)\n\"", ent->culprit->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(ent->culprit - g_entities, va("chat -1 \"^$^- ^7FLAG KEEPER ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(ent->culprit, pointsAwarded);
        G_LocalSound(ent->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardFlagDroppedPoints(gentity_t *ent) {
    if (g_points_flagdropped.integer > 0) {
        int pointsAwarded = g_points_flagdropped.integer;
        trap_SendServerCommand(ent - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Flag Attempt)\n\"", ent->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(ent - g_entities, va("chat -1 \"^$^- ^7FLAG RUNNER ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(ent, pointsAwarded); // Subtract points from the player's score
        G_LocalSound(ent->s.number, "sound/misc/events/track_switch.mp3");
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
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^7BLOOD THIRSTY ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardKillSpreePoints(gentity_t *attacker) {
    if (g_points_spree.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_spree.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Killing Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^75 KILLS ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardRampagePoints(gentity_t *attacker) {
    if (g_points_rampage.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_rampage.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Rampage)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^710 KILLS ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardDominatingPoints(gentity_t *attacker) {
    if (g_points_dominating.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_dominating.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Dominating)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^715 KILLS ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardUnstoppablePoints(gentity_t *attacker) {
    if (g_points_unstoppable.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_unstoppable.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Unstoppable Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^720 KILLS ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}

void AwardGodlikePoints(gentity_t *attacker) {
    if (g_points_godlike.integer > 0 && level.gametypeData->teams) {
        int pointsAwarded = g_points_godlike.integer;
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points ^5(Godlike Spree)\n\"", attacker->client->pers.netname, pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^$25 KILLS ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
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
        trap_SendServerCommand(attacker - g_entities, va("chat -1 \"^$^- ^7GOOD AIM ^+^$ ^<+%d ^JPoints\n\"", pointsAwarded));
        trap_SendServerCommand(attacker - g_entities, va("print \"%s ^7earned ^<+%d ^7Points (^$Headshot^7)\n\"", attacker->client->pers.netname, pointsAwarded));
        G_AddScore(attacker, pointsAwarded);
        G_LocalSound(attacker->s.number, "sound/misc/events/track_switch.mp3");
    }
}


