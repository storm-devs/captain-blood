#ifndef ATTR_CREATOR_UTIL
#define ATTR_CREATOR_UTIL

#include "attributelist.h"

extern void DeleteAttribute (BaseAttribute* ptr);

/* Создать из моих атрибутов копию */
extern BaseAttribute* CreateAttributeCopy (BaseAttribute* source, BaseAttribute* dest = NULL);

/* Создать из миссионных аттрибутов */
extern BaseAttribute* CreateFromMissionParam (const IMOParams::Param* source);

/* Создать используя только тип */
extern BaseAttribute* CreateEmptyAttribute (IMOParams::Type type);

extern BaseAttribute* CreateEmptyAttribute2 (IMOParams::Type type);

#endif