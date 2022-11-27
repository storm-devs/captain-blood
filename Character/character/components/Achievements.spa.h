////////////////////////////////////////////////////////////////////
//
// .\Achievements.spa.h
//
// Auto-generated on Friday, 17 October 2003 at 13:39:18
// XLAST project version 1.0.27.0
// SPA Compiler version 2.0.7776.0
//
////////////////////////////////////////////////////////////////////

#ifndef __ACHIEVEMENTS_SAMPLE_SPA_H__
#define __ACHIEVEMENTS_SAMPLE_SPA_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Title info
//

#define TITLEID_ACHIEVEMENTS_SAMPLE                 0xFFFF0079

//
// Context ids
//
// These values are passed as the dwContextId to XUserSetContext.
//

#define CONTEXT_PLAYMODE                            0
#define CONTEXT_PADDLESIZE                          1
#define CONTEXT_BALLSPEED                           20
#define CONTEXT_POINTSTOEND                         30

//
// Context values
//
// These values are passed as the dwContextValue to XUserSetContext.
//

// Values for CONTEXT_PLAYMODE

#define CONTEXT_PLAYMODE_SINGLEPLAYER               0
#define CONTEXT_PLAYMODE_MULTIPLAYER                1

// Values for CONTEXT_PADDLESIZE

#define CONTEXT_PADDLESIZE_SMALL                    0
#define CONTEXT_PADDLESIZE_MEDIUM                   1
#define CONTEXT_PADDLESIZE_LARGE                    2

// Values for CONTEXT_BALLSPEED

#define CONTEXT_BALLSPEED_SLOW                      0
#define CONTEXT_BALLSPEED_FAST                      1

// Values for CONTEXT_POINTSTOEND

#define CONTEXT_POINTSTOEND_FIFTEEN                 0
#define CONTEXT_POINTSTOEND_TWENTYONE               1

// Values for X_CONTEXT_PRESENCE

#define CONTEXT_PRESENCE_SINGLEPLAYER               0
#define CONTEXT_PRESENCE_MULTIPLAYER                1
#define CONTEXT_PRESENCE_LOBBY                      2

// Values for X_CONTEXT_GAME_MODE

#define CONTEXT_GAME_MODE_SINGLEPLAYER              0
#define CONTEXT_GAME_MODE_MULTIPLAYER               1

//
// Property ids
//
// These values are passed as the dwPropertyId value to XUserSetProperty
// and as the dwPropertyId value in the XUSER_PROPERTY structure.
//

#define PROPERTY_GAMES                              0x10000011
#define PROPERTY_WINS                               0x10000022
#define PROPERTY_LOSSES                             0x10000033
#define PROPERTY_POINTS                             0x10000044
#define PROPERTY_GAMETIME                           0x10000055

//
// Achievement ids
//
// These values are used in the dwAchievementId member of the
// XUSER_ACHIEVEMENT structure that is used with
// XUserWriteAchievements and XUserCreateAchievementEnumerator.
//

#define ACHIEVEMENT_PLAYED1GAME                     100
#define ACHIEVEMENT_PLAYED10GAMES                   101
#define ACHIEVEMENT_PLAYED100GAMES                  102
#define ACHIEVEMENT_WON3INAROW                      200
#define ACHIEVEMENT_WON10INAROW                     201
#define ACHIEVEMENT_PONG_A_THON_I                   202

//
// Stats view ids
//
// These are used in the dwViewId member of the XUSER_STATS_SPEC structure
// passed to the XUserReadStats* and XUserCreateStatsEnumerator* functions.
//

// Skill leaderboards for ranked game modes

#define STATS_VIEW_SKILL_RANKED_SINGLEPLAYER        0xFFFF0000
#define STATS_VIEW_SKILL_RANKED_MULTIPLAYER         0xFFFF0001

// Skill leaderboards for unranked (standard) game modes

#define STATS_VIEW_SKILL_STANDARD_SINGLEPLAYER      0xFFFE0000
#define STATS_VIEW_SKILL_STANDARD_MULTIPLAYER       0xFFFE0001

// Title defined leaderboards


//
// Stats view column ids
//
// These ids are used to read columns of stats views.  They are specified in
// the rgwColumnIds array of the XUSER_STATS_SPEC structure.  Rank, rating
// and gamertag are not retrieved as custom columns and so are not included
// in the following definitions.  They can be retrieved from each row's
// header (e.g., pStatsResults->pViews[x].pRows[y].dwRank, etc.).
//

//
// Matchmaking queries
//
// These values are passed as the dwProcedureIndex parameter to
// XSessionSearch to indicate which matchmaking query to run.
//


//
// Gamer pictures
//
// These ids are passed as the dwPictureId parameter to XUserAwardGamerTile.
//

#define GAMER_PICTURE_IMAGE_WON10INAROW             11
#define GAMER_PICTURE_IMAGE_PONGATHON1              12


#ifdef __cplusplus
}
#endif

#endif // __ACHIEVEMENTS_SAMPLE_SPA_H__


