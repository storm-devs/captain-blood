////////////////////////////////////////////////////////////////////
//
// UnderJollyRoger.spa.h
//
// Auto-generated on Thursday, 01 April 2010 at 15:34:26
// XLAST project version 1.0.93.0
// SPA Compiler version 2.0.9328.0
//
////////////////////////////////////////////////////////////////////

#ifndef __X_STRINGID_TITLENAME_SPA_H__
#define __X_STRINGID_TITLENAME_SPA_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Title info
//

#define TITLEID_X_STRINGID_TITLENAME                0x4F4307D1

//
// Context ids
//
// These values are passed as the dwContextId to XUserSetContext.
//


//
// Context values
//
// These values are passed as the dwContextValue to XUserSetContext.
//

// Values for X_CONTEXT_PRESENCE

#define CONTEXT_PRESENCE_PRESENCE_MENU              0
#define CONTEXT_PRESENCE_PRESENCE_MISSION1          1
#define CONTEXT_PRESENCE_PRESENCE_MISSION3          2
#define CONTEXT_PRESENCE_PRESENCE_MISSION2          3
#define CONTEXT_PRESENCE_PRESENCE_MISSION7          4
#define CONTEXT_PRESENCE_PRESENCE_MISSION5          5
#define CONTEXT_PRESENCE_PRESENCE_MISSION6          6
#define CONTEXT_PRESENCE_PRESENCE_MISSION15         7
#define CONTEXT_PRESENCE_PRESENCE_CUTSCENE          8
#define CONTEXT_PRESENCE_PRESENCE_MULTIPLAYER       9
#define CONTEXT_PRESENCE_PRESENCE_IDLE              10

// Values for X_CONTEXT_GAME_MODE

#define CONTEXT_GAME_MODE_ADVENTURE_MODE            0
#define CONTEXT_GAME_MODE_MULTIPLAYERBATTLE         1

//
// Property ids
//
// These values are passed as the dwPropertyId value to XUserSetProperty
// and as the dwPropertyId value in the XUSER_PROPERTY structure.
//


//
// Achievement ids
//
// These values are used in the dwAchievementId member of the
// XUSER_ACHIEVEMENT structure that is used with
// XUserWriteAchievements and XUserCreateAchievementEnumerator.
//

#define ACHIEVEMENT_FIRST_BLOOD                     1
#define ACHIEVEMENT_SLAYER                          2
#define ACHIEVEMENT_EXECUTIONER                     3
#define ACHIEVEMENT_RIPPER                          4
#define ACHIEVEMENT_SAILOR                          5
#define ACHIEVEMENT_MUSKETEER                       6
#define ACHIEVEMENT_DUELIST                         7
#define ACHIEVEMENT_BUTCHER                         8
#define ACHIEVEMENT_SHARPSHOOTER                    9
#define ACHIEVEMENT_KNIGHT                          10
#define ACHIEVEMENT_SNIPER                          11
#define ACHIEVEMENT_DEMOLITION                      12
#define ACHIEVEMENT_SHREDDER                        13
#define ACHIEVEMENT_HEADCUTTER                      14
#define ACHIEVEMENT_MURDERER                        15
#define ACHIEVEMENT_RAGE                            16
#define ACHIEVEMENT_WEAPON_MASTER                   17
#define ACHIEVEMENT_RACKETEER                       18
#define ACHIEVEMENT_MERCILESS                       19
#define ACHIEVEMENT_BERSERKER                       20
#define ACHIEVEMENT_BOATSWAIN                       21
#define ACHIEVEMENT_CAPTAIN                         22
#define ACHIEVEMENT_CAPER                           23
#define ACHIEVEMENT_JOLLY_ROGER                     24
#define ACHIEVEMENT_PIRATE_KING                     25
#define ACHIEVEMENT_AVENGER                         26
#define ACHIEVEMENT_DEFENDER_OF_THE_WEAK            27
#define ACHIEVEMENT_MASTER_OF_REVENGE               28
#define ACHIEVEMENT_PETER_THE_CONQUEROR             29
#define ACHIEVEMENT_PETER_THE_VICTORIOUS            30
#define ACHIEVEMENT_CAPTAIN_BLOOD                   31
#define ACHIEVEMENT_FISH_FOOD                       32
#define ACHIEVEMENT_TRICKSTER                       33
#define ACHIEVEMENT_BURGLAR                         34
#define ACHIEVEMENT_UNSTOPPABLE                     35
#define ACHIEVEMENT_LOOTER                          36
#define ACHIEVEMENT_THE_MONEY_BAG                   37
#define ACHIEVEMENT_TREASURE_HUNTER                 38
#define ACHIEVEMENT_MASTER                          39
#define ACHIEVEMENT_GUNMASTER                       40
#define ACHIEVEMENT_CATAPULT                        41
#define ACHIEVEMENT_NOVICE                          42
#define ACHIEVEMENT_SURVIVOR                        43
#define ACHIEVEMENT_MASSACRE                        44
#define ACHIEVEMENT_MANSLAUGHTER                    45
#define ACHIEVEMENT_VETERAN                         46
#define ACHIEVEMENT_SEA_WOLF                        49
#define ACHIEVEMENT_TERROR_OF_THE_SEA               50

//
// AvatarAssetAward ids
//


//
// Stats view ids
//
// These are used in the dwViewId member of the XUSER_STATS_SPEC structure
// passed to the XUserReadStats* and XUserCreateStatsEnumerator* functions.
//

// Skill leaderboards for ranked game modes

#define STATS_VIEW_SKILL_RANKED_ADVENTURE_MODE      0xFFFF0000
#define STATS_VIEW_SKILL_RANKED_MULTIPLAYERBATTLE   0xFFFF0001

// Skill leaderboards for unranked (standard) game modes

#define STATS_VIEW_SKILL_STANDARD_ADVENTURE_MODE    0xFFFE0000
#define STATS_VIEW_SKILL_STANDARD_MULTIPLAYERBATTLE 0xFFFE0001

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



#ifdef __cplusplus
}
#endif

#endif // __X_STRINGID_TITLENAME_SPA_H__


