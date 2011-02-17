/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "castle.h"
#include "difficulty.h"
#include "mp2.h"
#include "speed.h"
#include "settings.h"
#include "luck.h"
#include "spell.h"
#include "race.h"
#include "morale.h"
#include "payment.h"
#include "monster.h"

#ifdef WITH_XML
#include "xmlccwrap.h"
#endif

struct monstats_t
{
    u8 attack;
    u8 defense;
    u8 damageMin;
    u8 damageMax;
    u16 hp;
    Speed::speed_t speed;
    u8 grown;
    u8 shots;
    const char* name;
    const char* multiname;
    cost_t cost;
};

namespace
{
    static monstats_t monsters[] = {
	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0,    "Unknown Monster", "Unknown Monsters" , { 0, 0, 0, 0, 0, 0, 0} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname             cost
	{     1,   1,   1,   1,   1,  Speed::VERYSLOW,  12,     0, _("Peasant")        , _("Peasants")       , { 20, 0, 0, 0, 0, 0, 0} },
	{     5,   3,   2,   3,  10,  Speed::VERYSLOW,   8,    12, _("Archer")         , _("Archers")        , { 150, 0, 0, 0, 0, 0, 0} },
	{     5,   3,   2,   3,  10,   Speed::AVERAGE,   8,    24, _("Ranger")         , _("Rangers")        , { 200, 0, 0, 0, 0, 0, 0} },
	{     5,   9,   3,   4,  15,   Speed::AVERAGE,   5,     0, _("Pikeman")        , _("Pikemen")        , { 200, 0, 0, 0, 0, 0, 0} },
	{     5,   9,   3,   4,  20,      Speed::FAST,   5,     0, _("Veteran Pikeman"), _("Veteran Pikemen"), { 250, 0, 0, 0, 0, 0, 0} },
	{     7,   9,   4,   6,  25,   Speed::AVERAGE,   4,     0, _("Swordsman")      , _("Swordsmen")      , { 250, 0, 0, 0, 0, 0, 0} },
	{     7,   9,   4,   6,  30,      Speed::FAST,   4,     0, _("Master Swordsman"), _("Master Swordsmen"), { 300, 0, 0, 0, 0, 0, 0} },
	{    10,   9,   5,  10,  30,  Speed::VERYFAST,   3,     0, _("Cavalry")        , _("Cavalries")      , { 300, 0, 0, 0, 0, 0, 0} },
	{    10,   9,   5,  10,  40, Speed::ULTRAFAST,   3,     0, _("Champion")       , _("Champions")      , { 375, 0, 0, 0, 0, 0, 0} },
	{    11,  12,  10,  20,  50,      Speed::FAST,   2,     0, _("Paladin")        , _("Paladins")       , { 600, 0, 0, 0, 0, 0, 0} },
	{    11,  12,  10,  20,  65,  Speed::VERYFAST,   2,     0, _("Crusader")       , _("Crusaders")      , { 1000, 0, 0, 0, 0, 0, 0} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname            cost
	{     3,   1,   1,   2,   3,   Speed::AVERAGE,  10,     0, _("Goblin")         , _("Goblins")       , { 40, 0, 0, 0, 0, 0, 0} },
	{     3,   4,   2,   3,  10,  Speed::VERYSLOW,   8,     8, _("Orc")            , _("Orcs")          , { 140, 0, 0, 0, 0, 0, 0} },
	{     3,   4,   3,   4,  15,      Speed::SLOW,   8,    16, _("Orc Chief")      , _("Orc Chiefs")    , { 175, 0, 0, 0, 0, 0, 0} },
	{     6,   2,   3,   5,  20,  Speed::VERYFAST,   5,     0, _("Wolf")           , _("Wolves")        , { 200, 0, 0, 0, 0, 0, 0} },
	{     9,   5,   4,   6,  40,  Speed::VERYSLOW,   4,     0, _("Ogre")           , _("Ogres")         , { 300, 0, 0, 0, 0, 0, 0} },
	{     9,   5,   5,   7,  60,   Speed::AVERAGE,   4,     0, _("Ogre Lord")      , _("Ogre Lords")    , { 500, 0, 0, 0, 0, 0, 0} },
	{    10,   5,   5,   7,  40,   Speed::AVERAGE,   3,     8, _("Troll")          , _("Trolls")        , { 600, 0, 0, 0, 0, 0, 0} },
	{    10,   5,   7,   9,  40,      Speed::FAST,   3,    16, _("War Troll")      , _("War Trolls")    , { 700, 0, 0, 0, 0, 0, 0} },
	{    12,   9,  12,  24,  80,      Speed::FAST,   2,     0, _("Cyclops")        , _("Cyclopes")      , { 750, 0, 0, 0, 0, 1, 0} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname            cost
	{     4,   2,   1,   2,   2,   Speed::AVERAGE,   8,     0, _("Sprite")         , _("Sprites")       , { 50, 0, 0, 0, 0, 0, 0} },
	{     6,   5,   2,   4,  20,  Speed::VERYSLOW,   6,     0, _("Dwarf")          , _("Dwarves")       , { 200, 0, 0, 0, 0, 0, 0} },
	{     6,   6,   2,   4,  20,   Speed::AVERAGE,   6,     0, _("Battle Dwarf")   , _("Battle Dwarves"), { 250, 0, 0, 0, 0, 0, 0} },
	{     4,   3,   2,   3,  15,   Speed::AVERAGE,   4,    24, _("Elf")            , _("Elves")         , { 250, 0, 0, 0, 0, 0, 0} },
	{     5,   5,   2,   3,  15,  Speed::VERYFAST,   4,    24, _("Grand Elf")      , _("Grand Elves")   , { 300, 0, 0, 0, 0, 0, 0} },
	{     7,   5,   5,   8,  25,      Speed::FAST,   3,     8, _("Druid")          , _("Druids")        , { 350, 0, 0, 0, 0, 0, 0} },
	{     7,   7,   5,   8,  25,  Speed::VERYFAST,   3,    16, _("Greater Druid")  , _("Greater Druids"), { 400, 0, 0, 0, 0, 0, 0} },
	{    10,   9,   7,  14,  40,      Speed::FAST,   2,     0, _("Unicorn")        , _("Unicorns")      , { 500, 0, 0, 0, 0, 0, 0} },
	{    12,  10,  20,  40, 100, Speed::ULTRAFAST,   1,     0, _("Phoenix")        , _("Phoenix")       , { 1500, 0, 1, 0, 0, 0, 0} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname            cost
	{     3,   1,   1,   2,   5,   Speed::AVERAGE,   8,     8, _("Centaur")        , _("Centaurs")      , { 60, 0, 0, 0, 0, 0, 0} },
	{     4,   7,   2,   3,  15,  Speed::VERYFAST,   6,     0, _("Gargoyle")       , _("Gargoyles")     , { 200, 0, 0, 0, 0, 0, 0} },
	{     6,   6,   3,   5,  25,   Speed::AVERAGE,   4,     0, _("Griffin")        , _("Griffins")      , { 300, 0, 0, 0, 0, 0, 0} },
	{     9,   8,   5,  10,  35,   Speed::AVERAGE,   3,     0, _("Minotaur")       , _("Minotaurs")     , { 400, 0, 0, 0, 0, 0, 0} },
	{     9,   8,   5,  10,  45,  Speed::VERYFAST,   3,     0, _("Minotaur King")  , _("Minotaur Kings"), { 500, 0, 0, 0, 0, 0, 0} },
	{     8,   9,   6,  12,  75,  Speed::VERYSLOW,   2,     0, _("Hydra")          , _("Hydras")        , { 800, 0, 0, 0, 0, 0, 0} },
	{    12,  12,  25,  50, 200,   Speed::AVERAGE,   1,     0, _("Green Dragon")   , _("Green Dragons") , { 3000, 0, 0, 0, 1, 0, 0} },
	{    13,  13,  25,  50, 250,      Speed::FAST,   1,     0, _("Red Dragon")     , _("Red Dragons")   , { 3500, 0, 0, 0, 1, 0, 0} }, 
	{    14,  14,  25,  50, 300,  Speed::VERYFAST,   1,     0, _("Black Dragon")   , _("Black Dragons") , { 4000, 0, 0, 0, 2, 0, 0} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname            cost
	{     2,   1,   1,   3,   3,      Speed::SLOW,   8,    12, _("Halfling")       , _("Halflings")     , { 50, 0, 0, 0, 0, 0, 0} },
	{     5,   4,   2,   3,  15,  Speed::VERYFAST,   6,     0, _("Boar")           , _("Boars")         , { 150, 0, 0, 0, 0, 0, 0} },
	{     5,  10,   4,   5,  30,  Speed::VERYSLOW,   4,     0, _("Iron Golem")     , _("Iron Golems")   , { 300, 0, 0, 0, 0, 0, 0} },
	{     7,  10,   4,   5,  35,      Speed::SLOW,   4,     0, _("Steel Golem")    , _("Steel Golems")  , { 350, 0, 0, 0, 0, 0, 0} },
	{     7,   7,   4,   8,  40,   Speed::AVERAGE,   3,     0, _("Roc")            , _("Rocs")          , { 400, 0, 0, 0, 0, 0, 0} },
	{    11,   7,   7,   9,  30,      Speed::FAST,   2,    12, _("Mage")           , _("Magi")          , { 600, 0, 0, 0, 0, 0, 0} },
	{    12,   8,   7,   9,  35,  Speed::VERYFAST,   2,    24, _("Archmage")       , _("Archmagi")      , { 700, 0, 0, 0, 0, 0, 0} },
	{    13,  10,  20,  30, 150,   Speed::AVERAGE,   1,     0, _("Giant")          , _("Giants")        , { 2000, 0, 0, 0, 0, 0, 1} },
	{    15,  15,  20,  30, 300,  Speed::VERYFAST,   1,    24, _("Titan")          , _("Titans")        , { 5000, 0, 0, 0, 0, 0, 2} },

	// atck dfnc  min  max   hp             speed grwn  shots  name                  multiname            cost
	{     4,   3,   2,   3,   4,   Speed::AVERAGE,   8,     0, _("Skeleton")       , _("Skeletons")     , { 75, 0, 0, 0, 0, 0, 0} },
	{     5,   2,   2,   3,  15,  Speed::VERYSLOW,   6,     0, _("Zombie")         , _("Zombies")       , { 150, 0, 0, 0, 0, 0, 0} },
	{     5,   2,   2,   3,  25,   Speed::AVERAGE,   6,     0, _("Mutant Zombie")  , _("Mutant Zombies"), { 200, 0, 0, 0, 0, 0, 0} },
	{     6,   6,   3,   4,  25,   Speed::AVERAGE,   4,     0, _("Mummy")          , _("Mummies")       , { 250, 0, 0, 0, 0, 0, 0} },
	{     6,   6,   3,   4,  30,      Speed::FAST,   4,     0, _("Royal Mummy")    , _("Royal Mummies") , { 300, 0, 0, 0, 0, 0, 0} },
	{     8,   6,   5,   7,  30,   Speed::AVERAGE,   3,     0, _("Vampire")        , _("Vampires")      , { 500, 0, 0, 0, 0, 0, 0} },
	{     8,   6,   5,   7,  40,      Speed::FAST,   3,     0, _("Vampire Lord")   , _("Vampire Lords") , { 650, 0, 0, 0, 0, 0, 0} },
	{     7,  12,   8,  10,  25,      Speed::FAST,   2,    12, _("Lich")           , _("Liches")        , { 750, 0, 0, 0, 0, 0, 0} },
	{     7,  13,   8,  10,  35,  Speed::VERYFAST,   2,    24, _("Power Lich")     , _("Power Liches")  , { 900, 0, 0, 0, 0, 0, 0} }, 
	{    11,   9,  25,  45, 150,   Speed::AVERAGE,   1,     0, _("Bone Dragon")    , _("Bone Dragons")  , { 1500, 0, 0, 0, 0, 0, 0} },

        // atck dfnc  min  max   hp             speed grwn  shots  name                  multiname                cost
	{     6,   1,   1,   2,   4,      Speed::FAST,   4,     0, _("Rogue")          , _("Rogues")            , { 50, 0, 0, 0, 0, 0, 0} },
	{     7,   6,   2,   5,  20,  Speed::VERYFAST,   4,     0, _("Nomad")          , _("Nomads")            , { 200, 0, 0, 0, 0, 0, 0} },
	{     8,   7,   4,   6,  20,      Speed::FAST,   4,     0, _("Ghost")          , _("Ghosts")            , { 1000, 0, 0, 0, 0, 0, 0} },
	{    10,   9,  20,  30,  50,  Speed::VERYFAST,   4,     0, _("Genie")          , _("Genies")            , { 650, 0, 0, 0, 0, 0, 1} },
	{     8,   9,   6,  10,  35,   Speed::AVERAGE,   4,     0, _("Medusa")         , _("Medusas")           , { 500, 0, 0, 0, 0, 0, 0} },
	{     8,   8,   4,   5,  50,      Speed::SLOW,   4,     0, _("Earth Elemental")  , _("Earth Elementals"), { 500, 0, 0, 0, 0, 0, 0} },
	{     7,   7,   2,   8,  35,  Speed::VERYFAST,   4,     0, _("Air Elemental")    , _("Air Elementals")  , { 500, 0, 0, 0, 0, 0, 0} },
	{     8,   6,   4,   6,  40,      Speed::FAST,   4,     0, _("Fire Elemental")   , _("Fire Elementals") , { 500, 0, 0, 0, 0, 0, 0} },
	{     6,   8,   3,   7,  45,   Speed::AVERAGE,   4,     0, _("Water Elemental")  , _("Water Elementals"), { 500, 0, 0, 0, 0, 0, 0} },

	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0, "Random Monster"  , "Random Monsters"  , { 0, 0, 0, 0, 0, 0, 0} },
	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0, "Random Monster 1", "Random Monsters 3", { 0, 0, 0, 0, 0, 0, 0} },
	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0, "Random Monster 2", "Random Monsters 2", { 0, 0, 0, 0, 0, 0, 0} },
	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0, "Random Monster 3", "Random Monsters 3", { 0, 0, 0, 0, 0, 0, 0} },
	{     0,   0,   0,   0,   0,  Speed::VERYSLOW,   0,     0, "Random Monster 4", "Random Monsters 4", { 0, 0, 0, 0, 0, 0, 0} },
    };

    double upgrade_ratio = 0;
}

void Monster::SetUpgradeRatio(double rate)
{
    upgrade_ratio = rate;
}

void Monster::UpdateStats(const std::string & spec)
{
#ifdef WITH_XML
    // parse monsters.xml
    TiXmlDocument doc;
    const TiXmlElement* xml_monsters = NULL;
    monstats_t* ptr = &monsters[0];

    if(doc.LoadFile(spec.c_str()) &&
        NULL != (xml_monsters = doc.FirstChildElement("monsters")))
    {
        const TiXmlElement* xml_monster = xml_monsters->FirstChildElement("monster");
        for(; xml_monster; xml_monster = xml_monster->NextSiblingElement("monster"))
        {
	    cost_t & cost = ptr->cost;
            int value;

    	    xml_monster->Attribute("skip", &value);
	    if(0 == value)
	    {
    		xml_monster->Attribute("attack", &value); if(value) ptr->attack = value;
    		xml_monster->Attribute("defense", &value); if(value) ptr->defense = value;
    		xml_monster->Attribute("damage_min", &value); if(value) ptr->damageMin = value;
    		xml_monster->Attribute("damage_max", &value); if(value) ptr->damageMax = value;
    		xml_monster->Attribute("hp", &value); if(value) ptr->hp = value;
    		xml_monster->Attribute("speed", &value); ptr->speed = Speed::INSTANT < value ? Speed::INSTANT : (Speed::CRAWLING > value ? Speed::CRAWLING : static_cast<Speed::speed_t>(value));
    		xml_monster->Attribute("grown", &value); ptr->grown = value;
    		xml_monster->Attribute("shots", &value); ptr->shots = value;
    		xml_monster->Attribute("gold", &value); cost.gold = value;
    		xml_monster->Attribute("wood", &value); cost.wood = value;
    		xml_monster->Attribute("mercury", &value); cost.mercury = value;
    		xml_monster->Attribute("ore", &value); cost.ore = value;
    		xml_monster->Attribute("sulfur", &value); cost.sulfur = value;
    		xml_monster->Attribute("crystal", &value); cost.crystal = value;
    		xml_monster->Attribute("gems", &value); cost.gems = value;
	    }

	    ++ptr;

	    // out of range
	    if((ptr - &monsters[0]) > WATER_ELEMENT) break;
        }
    }
    else
    VERBOSE(spec << ": " << doc.ErrorDesc());
#endif
}

Monster::Monster(u8 m) : id(UNKNOWN)
{
    if(m <= WATER_ELEMENT)
	id = m;
    else
    if(MONSTER_RND1 == m)
	id = Rand(LEVEL1).GetID();
    else
    if(MONSTER_RND2 == m)
	id = Rand(LEVEL2).GetID();
    else
    if(MONSTER_RND3 == m)
	id = Rand(LEVEL3).GetID();
    else
    if(MONSTER_RND4 == m)
	id = Rand(LEVEL4).GetID();
    else
    if(MONSTER_RND == m)
	id = Rand(LEVEL0).GetID();
}

Monster::Monster(const Spell & sp) : id(UNKNOWN)
{
    switch(sp())
    {
	case Spell::SETEGUARDIAN:
	case Spell::SUMMONEELEMENT: id = EARTH_ELEMENT; break;

	case Spell::SETAGUARDIAN:
        case Spell::SUMMONAELEMENT: id = AIR_ELEMENT; break;

	case Spell::SETFGUARDIAN:
        case Spell::SUMMONFELEMENT: id = FIRE_ELEMENT; break;

	case Spell::SETWGUARDIAN:
        case Spell::SUMMONWELEMENT: id = WATER_ELEMENT; break;
        default: break;
    }
}

Monster::Monster(u8 race, u32 dw) : id(UNKNOWN)
{
    id = FromDwelling(race, dw).id;
}

bool Monster::isValid(void) const
{
    return id != UNKNOWN;
}

bool Monster::operator== (const Monster & m) const
{
    return id == m.id;
}

bool Monster::operator!= (const Monster & m) const
{
    return id != m.id;
}

u8 Monster::operator() (void) const
{
    return id;
}

u8 Monster::GetID(void) const
{
    return id;
}

void Monster::Upgrade(void)
{
    id = GetUpgrade().id;
}

u8 Monster::GetAttack(void) const
{
    return monsters[id].attack;
}

u8 Monster::GetDefense(void) const
{
    return monsters[id].defense;
}

u8 Monster::GetRace(void) const
{
    if(UNKNOWN == id)	return Race::BOMG;
    else
    if(GOBLIN > id)	return Race::KNGT;
    else
    if(SPRITE > id)	return Race::BARB;
    else
    if(CENTAUR > id)	return Race::SORC;
    else
    if(HALFLING > id)	return Race::WRLK;
    else
    if(SKELETON > id)	return Race::WZRD;
    else
    if(ROGUE > id)	return Race::NECR;

    return Race::BOMG;
}

u8  Monster::GetDamageMin(void) const
{
    return monsters[id].damageMin;
}

u8  Monster::GetDamageMax(void) const
{
    return monsters[id].damageMax;
}

u8  Monster::GetShots(void) const
{
    return monsters[id].shots;
}

u16 Monster::GetHitPoints(void) const
{
    return monsters[id].hp;
}

u8  Monster::GetSpeed(void) const
{
    return monsters[id].speed;
}

u8  Monster::GetGrown(void) const
{
    return monsters[id].grown;
}

u16 Monster::GetRNDSize(bool skip_factor) const
{
    const u32 hps = (GetGrown() ? GetGrown() : 1) * GetHitPoints();
    u32 res = Rand::Get(hps, hps + hps / 2);

    if(!skip_factor)
    {
	u16 factor = 100;

	switch(Settings::Get().GameDifficulty()) 	 
	{
	    case Difficulty::EASY:      factor = 80; break;
	    case Difficulty::NORMAL:    factor = 100; break;
	    case Difficulty::HARD:      factor = 130; break;
	    case Difficulty::EXPERT:    factor = 160; break;
	    case Difficulty::IMPOSSIBLE:factor = 190; break;
	    default: break;
	}

	res = (res * factor / 100);
	// force minimal
	if(res == 0) res = 1;
    }

    return GetCountFromHitPoints(id, res);
}

bool Monster::isUndead(void) const
{
    switch(id)
    {
        case SKELETON:
        case ZOMBIE:
        case MUTANT_ZOMBIE:
        case MUMMY:
        case ROYAL_MUMMY:
        case VAMPIRE:
        case VAMPIRE_LORD:
        case LICH:
        case POWER_LICH:
        case BONE_DRAGON:
	case GHOST: return true;

	default: break;
    }

    return false;
}

bool Monster::isElemental(void) const
{
    switch(id)
    {
        case EARTH_ELEMENT:
        case AIR_ELEMENT:
        case FIRE_ELEMENT:
        case WATER_ELEMENT: return true;

	default: break;
    }

    return false;
}

bool Monster::isDragons(void) const
{
    switch(id)
    {
       case GREEN_DRAGON:
       case RED_DRAGON:
       case BLACK_DRAGON:
       case BONE_DRAGON: return true;

       default: break;
    }

    return false;
}

bool Monster::isFly(void) const
{
    switch(id)
    {
	case SPRITE:
	case PHOENIX:
	case GARGOYLE:
	case GRIFFIN:
	case GREEN_DRAGON:
	case RED_DRAGON:
	case BLACK_DRAGON:
	case ROC:
	case VAMPIRE:
	case VAMPIRE_LORD:
	case BONE_DRAGON:
	case GHOST:
	case GENIE:	return true;

	default: break;
    }

    return false;
}

bool Monster::isWide(void) const
{
    switch(id)
    {
	case CAVALRY:
	case CHAMPION:
	case WOLF:
	case UNICORN:
	case PHOENIX:
	case CENTAUR:
	case GRIFFIN:
	case HYDRA:
	case GREEN_DRAGON:
	case RED_DRAGON:
	case BLACK_DRAGON:
	case BOAR:
	case ROC:
	case BONE_DRAGON:
	case NOMAD:
	case MEDUSA:	return true;

	default: break;
    }

    return false;
}

bool Monster::isArchers(void) const
{
    return GetShots();
}

bool Monster::isAllowUpgrade(void) const
{
    return id != GetUpgrade().id;
}

bool Monster::isHideAttack(void) const
{
    switch(id)
    {
        case Monster::ROGUE:
        case Monster::SPRITE:
        case Monster::VAMPIRE:
        case Monster::VAMPIRE_LORD:
        case Monster::HYDRA: return true;

        default: break;
    }

    return false;
}

bool Monster::isTwiceAttack(void) const
{
    switch(id)
    {
        case WOLF:
        case PALADIN:
        case CRUSADER:
        case ELF:
        case GRAND_ELF:
        case RANGER: return true;

        default: break;
    }

    return false;
}

Monster Monster::GetUpgrade(void) const
{
    switch(id)
    {
        case ARCHER:		return Monster(RANGER);
        case PIKEMAN:		return Monster(VETERAN_PIKEMAN);
        case SWORDSMAN:		return Monster(MASTER_SWORDSMAN);
        case CAVALRY:		return Monster(CHAMPION);
        case PALADIN:		return Monster(CRUSADER);
        case ORC:		return Monster(ORC_CHIEF);
        case OGRE:		return Monster(OGRE_LORD);
        case TROLL:		return Monster(WAR_TROLL);
        case DWARF:		return Monster(BATTLE_DWARF);
        case ELF:		return Monster(GRAND_ELF);
        case DRUID:		return Monster(GREATER_DRUID);
        case ZOMBIE:		return Monster(MUTANT_ZOMBIE);
        case MUMMY:		return Monster(ROYAL_MUMMY);
        case VAMPIRE:		return Monster(VAMPIRE_LORD);
        case LICH:		return Monster(POWER_LICH);
        case MINOTAUR:		return Monster(MINOTAUR_KING);
        case GREEN_DRAGON:	return Monster(RED_DRAGON);
        case RED_DRAGON:	return Monster(BLACK_DRAGON);
        case IRON_GOLEM:	return Monster(STEEL_GOLEM);
        case MAGE:		return Monster(ARCHMAGE);
        case GIANT:		return Monster(TITAN);

	default: break;
    }

    return Monster(id);
}

Monster Monster::FromDwelling(u8 race, u32 dwelling)
{
    switch(dwelling)
    {
        case DWELLING_MONSTER1:
        switch(race)
        {
	case Race::KNGT: return Monster(PEASANT);
	case Race::BARB: return Monster(GOBLIN);
	case Race::SORC: return Monster(SPRITE);
	case Race::WRLK: return Monster(CENTAUR);
	case Race::WZRD: return Monster(HALFLING);
	case Race::NECR: return Monster(SKELETON);
	default: break;
        }
        break;

        case DWELLING_MONSTER2:
        switch(race)
        {
	case Race::KNGT: return Monster(ARCHER);
	case Race::BARB: return Monster(ORC);
	case Race::SORC: return Monster(DWARF);
	case Race::WRLK: return Monster(GARGOYLE);
	case Race::WZRD: return Monster(BOAR);
	case Race::NECR: return Monster(ZOMBIE);
	default: break;
        }
        break;

        case DWELLING_UPGRADE2:
        switch(race)
        {
	case Race::KNGT: return Monster(RANGER);
	case Race::BARB: return Monster(ORC_CHIEF);
	case Race::SORC: return Monster(BATTLE_DWARF);
	case Race::WRLK: return Monster(GARGOYLE);
	case Race::WZRD: return Monster(BOAR);
	case Race::NECR: return Monster(MUTANT_ZOMBIE);
	default: break;
        }
        break;

        case DWELLING_MONSTER3:
        switch(race)
        {
	case Race::KNGT: return Monster(PIKEMAN);
	case Race::BARB: return Monster(WOLF);
	case Race::SORC: return Monster(ELF);
	case Race::WRLK: return Monster(GRIFFIN);
	case Race::WZRD: return Monster(IRON_GOLEM);
	case Race::NECR: return Monster(MUMMY);
	default: break;
        }
        break;

        case DWELLING_UPGRADE3:
        switch(race)
        {
	case Race::KNGT: return Monster(VETERAN_PIKEMAN);
	case Race::BARB: return Monster(WOLF);
	case Race::SORC: return Monster(GRAND_ELF);
	case Race::WRLK: return Monster(GRIFFIN);
	case Race::WZRD: return Monster(STEEL_GOLEM);
	case Race::NECR: return Monster(ROYAL_MUMMY);
	default: break;
        }
        break;

        case DWELLING_MONSTER4:
        switch(race)
        {
	case Race::KNGT: return Monster(SWORDSMAN);
	case Race::BARB: return Monster(OGRE);
	case Race::SORC: return Monster(DRUID);
	case Race::WRLK: return Monster(MINOTAUR);
	case Race::WZRD: return Monster(ROC);
	case Race::NECR: return Monster(VAMPIRE);
	default: break;
        }
        break;

        case DWELLING_UPGRADE4:
        switch(race)
        {
	case Race::KNGT: return Monster(MASTER_SWORDSMAN);
	case Race::BARB: return Monster(OGRE_LORD);
	case Race::SORC: return Monster(GREATER_DRUID);
	case Race::WRLK: return Monster(MINOTAUR_KING);
	case Race::WZRD: return Monster(ROC);
	case Race::NECR: return Monster(VAMPIRE_LORD);
	default: break;
        }
        break;

        case DWELLING_MONSTER5:
        switch(race)
        {
	case Race::KNGT: return Monster(CAVALRY);
	case Race::BARB: return Monster(TROLL);
	case Race::SORC: return Monster(UNICORN);
	case Race::WRLK: return Monster(HYDRA);
	case Race::WZRD: return Monster(MAGE);
	case Race::NECR: return Monster(LICH);
	default: break;
        }
        break;

        case DWELLING_UPGRADE5:
        switch(race)
        {
	case Race::KNGT: return Monster(CHAMPION);
	case Race::BARB: return Monster(WAR_TROLL);
	case Race::SORC: return Monster(UNICORN);
	case Race::WRLK: return Monster(HYDRA);
	case Race::WZRD: return Monster(ARCHMAGE);
	case Race::NECR: return Monster(POWER_LICH);
	default: break;
        }
        break;

	case DWELLING_MONSTER6:
        switch(race)
        {
	case Race::KNGT: return Monster(PALADIN);
	case Race::BARB: return Monster(CYCLOPS);
	case Race::SORC: return Monster(PHOENIX);
	case Race::WRLK: return Monster(GREEN_DRAGON);
	case Race::WZRD: return Monster(GIANT);
	case Race::NECR: return Monster(BONE_DRAGON);
	default: break;
        }
        break;

        case DWELLING_UPGRADE6:
        switch(race)
        {
	case Race::KNGT: return Monster(CRUSADER);
	case Race::BARB: return Monster(CYCLOPS);
	case Race::SORC: return Monster(PHOENIX);
	case Race::WRLK: return Monster(RED_DRAGON);
	case Race::WZRD: return Monster(TITAN);
	case Race::NECR: return Monster(BONE_DRAGON);
	default: break;
        }
        break;

        case DWELLING_UPGRADE7:
        switch(race)
        {
	case Race::KNGT: return Monster(CRUSADER);
	case Race::BARB: return Monster(CYCLOPS);
	case Race::SORC: return Monster(PHOENIX);
	case Race::WRLK: return Monster(BLACK_DRAGON);
	case Race::WZRD: return Monster(TITAN);
	case Race::NECR: return Monster(BONE_DRAGON);
	default: break;
        }
        break;

        default: break;
    }

    return Monster(UNKNOWN);
}

Monster Monster::FromObject(u8 obj)
{
    switch(obj)
    {
        case MP2::OBJ_WATCHTOWER:	return Monster(ORC);
        case MP2::OBJ_EXCAVATION:	return Monster(SKELETON);
        case MP2::OBJ_CAVE:		return Monster(CENTAUR);
        case MP2::OBJ_TREEHOUSE:	return Monster(SPRITE);
        case MP2::OBJ_ARCHERHOUSE:	return Monster(ARCHER);
        case MP2::OBJ_GOBLINHUT:	return Monster(GOBLIN);
        case MP2::OBJ_DWARFCOTT:	return Monster(DWARF);
        case MP2::OBJ_HALFLINGHOLE:	return Monster(HALFLING);
        case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT: 	return Monster(PEASANT);

	case MP2::OBJ_RUINS:		return Monster(MEDUSA);
        case MP2::OBJ_TREECITY:		return Monster(SPRITE);
        case MP2::OBJ_WAGONCAMP:	return Monster(ROGUE);
        case MP2::OBJ_DESERTTENT:	return Monster(NOMAD);

        case MP2::OBJ_TROLLBRIDGE:	return Monster(TROLL);
        case MP2::OBJ_DRAGONCITY:	return Monster(RED_DRAGON);
        case MP2::OBJ_CITYDEAD:		return Monster(POWER_LICH);

        case MP2::OBJ_ANCIENTLAMP:	return Monster(GENIE);

        // loyalty version
	case MP2::OBJ_WATERALTAR:	return Monster(WATER_ELEMENT);
        case MP2::OBJ_AIRALTAR:		return Monster(AIR_ELEMENT);
        case MP2::OBJ_FIREALTAR:	return Monster(FIRE_ELEMENT);
        case MP2::OBJ_EARTHALTAR:	return Monster(EARTH_ELEMENT);
	case MP2::OBJ_BARROWMOUNDS:	return Monster(GHOST);

        default: break;
    }

    return Monster(UNKNOWN);
}

Monster Monster::Rand(level_t level)
{
    switch(level)
    {
	default: return Monster(Rand::Get(PEASANT, WATER_ELEMENT));

	case LEVEL1:
	case LEVEL2:
	case LEVEL3:
	case LEVEL4:
		break;
    }

    std::vector<Monster> monsters;
    monsters.reserve(30);

    for(u8 ii = PEASANT; ii <= WATER_ELEMENT; ++ii)
    {
	Monster mons(ii);
	if(mons.GetLevel() == level) monsters.push_back(mons);
    }

    return monsters.size() ? *Rand::Get(monsters) : UNKNOWN;
}

u8 Monster::GetLevel(void) const
{
    switch(id)
    {
	case PEASANT:
	case ARCHER:
	case GOBLIN:
	case ORC:
	case SPRITE:
	case CENTAUR:
	case HALFLING:
	case SKELETON:
	case ZOMBIE:
	case ROGUE:
	case MONSTER_RND1:	return LEVEL1;

	case RANGER:
	case PIKEMAN:
	case VETERAN_PIKEMAN:
	case ORC_CHIEF:
	case WOLF:
	case DWARF:
	case BATTLE_DWARF:
	case ELF:
	case GRAND_ELF:
	case GARGOYLE:
	case BOAR:
	case IRON_GOLEM:
	case MUTANT_ZOMBIE:
	case MUMMY:
	case NOMAD:
	case MONSTER_RND2:	return LEVEL2;

	case SWORDSMAN:
	case MASTER_SWORDSMAN:
	case CAVALRY:
	case CHAMPION:
	case OGRE:
	case OGRE_LORD:
	case TROLL:
	case WAR_TROLL:
	case DRUID:
	case GREATER_DRUID:
	case GRIFFIN:
	case MINOTAUR:
	case MINOTAUR_KING:
	case STEEL_GOLEM:
	case ROC:
	case MAGE:
	case ARCHMAGE:
	case ROYAL_MUMMY:
	case VAMPIRE:
	case VAMPIRE_LORD:
	case LICH:
	case GHOST:
	case MEDUSA:
	case EARTH_ELEMENT:
	case AIR_ELEMENT:
	case FIRE_ELEMENT:
	case WATER_ELEMENT:
	case MONSTER_RND3:	return LEVEL3;

	case PALADIN:
	case CRUSADER:
	case CYCLOPS:
	case UNICORN:
	case PHOENIX:
	case HYDRA:
	case GREEN_DRAGON:
	case RED_DRAGON:
	case BLACK_DRAGON:
	case GIANT:
	case TITAN:
	case POWER_LICH:
	case BONE_DRAGON:
        case GENIE:
	case MONSTER_RND4:	return LEVEL4;
    
	case MONSTER_RND:
    	    switch(Rand::Get(0, 3))
    	    {
		default:	return LEVEL1;
		case 1:		return LEVEL2;
		case 2:		return LEVEL3;
		case 3:		return LEVEL4;
    	    }
	    break;

	default: break;
    }

    return LEVEL0;
}

u32 Monster::GetDwelling(void) const
{
    switch(id)
    {
	case PEASANT:
	case GOBLIN:
	case SPRITE:
	case CENTAUR:
	case HALFLING:
	case SKELETON:		return DWELLING_MONSTER1;

	case ARCHER:
	case ORC:
	case ZOMBIE:
	case DWARF:
	case GARGOYLE:
	case BOAR:		return DWELLING_MONSTER2;

	case RANGER:
	case ORC_CHIEF:
	case BATTLE_DWARF:
	case MUTANT_ZOMBIE:	return DWELLING_UPGRADE2;

	case PIKEMAN:
	case WOLF:
	case ELF:
	case IRON_GOLEM:
	case MUMMY:
	case GRIFFIN:		return DWELLING_MONSTER3;

	case VETERAN_PIKEMAN:
	case GRAND_ELF:
	case STEEL_GOLEM:
	case ROYAL_MUMMY:	return DWELLING_UPGRADE3;

	case SWORDSMAN:
	case OGRE:
	case DRUID:
	case MINOTAUR:
	case ROC:
	case VAMPIRE:		return DWELLING_MONSTER4;

	case MASTER_SWORDSMAN:
	case OGRE_LORD:
	case GREATER_DRUID:
	case MINOTAUR_KING:
	case VAMPIRE_LORD:	return DWELLING_UPGRADE4;

	case CAVALRY:
	case TROLL:
	case MAGE:
	case LICH:
	case UNICORN:
	case HYDRA:		return DWELLING_MONSTER5;

	case CHAMPION:
	case WAR_TROLL:
	case ARCHMAGE:
	case POWER_LICH:	return DWELLING_UPGRADE5;

	case PALADIN:
	case CYCLOPS:
	case PHOENIX:
	case GREEN_DRAGON:
	case GIANT:
	case BONE_DRAGON:	return DWELLING_MONSTER6;

	case CRUSADER:
	case RED_DRAGON:
	case TITAN:		return DWELLING_UPGRADE6;

	case BLACK_DRAGON:	return DWELLING_UPGRADE7;
    
	default: break;
    }

    return 0;
}

const char* Monster::GetName(void) const
{
    return _(monsters[id].name);
}

const char* Monster::GetMultiName(void) const
{
    return _(monsters[id].multiname);
}

const char* Monster::GetPluralName(u32 count) const
{
    switch(id)
    {
	case PEASANT:		return ngettext("Peasant", "Peasants", count);
	case ARCHER:		return ngettext("Archer", "Archers", count);
	case RANGER:		return ngettext("Ranger", "Rangers", count);
	case PIKEMAN:		return ngettext("Pikeman", "Pikemen", count);
	case VETERAN_PIKEMAN:	return ngettext("Veteran Pikeman", "Veteran Pikemen", count);
	case SWORDSMAN:		return ngettext("Swordsman", "Swordsmen", count);
	case MASTER_SWORDSMAN:	return ngettext("Master Swordsman", "Master Swordsmen", count);
	case CAVALRY:		return ngettext("Cavalry", "Cavalries", count);
	case CHAMPION:		return ngettext("Champion", "Champions", count);
	case PALADIN:		return ngettext("Paladin", "Paladins", count);
	case CRUSADER:		return ngettext("Crusader", "Crusaders", count);

	case GOBLIN:		return ngettext("Goblin", "Goblins", count);
	case ORC:		return ngettext("Orc", "Orcs", count);
	case ORC_CHIEF:		return ngettext("Orc Chief", "Orc Chiefs", count);
	case WOLF:		return ngettext("Wolf", "Wolves", count);
	case OGRE:		return ngettext("Ogre", "Ogres", count);
	case OGRE_LORD:		return ngettext("Ogre Lord", "Ogre Lords", count);
	case TROLL:		return ngettext("Troll", "Trolls", count);
	case WAR_TROLL:		return ngettext("War Troll", "War Trolls", count);
	case CYCLOPS:		return ngettext("Cyclops", "Cyclopes", count);

	case SPRITE:		return ngettext("Sprite", "Sprites", count);
	case DWARF:		return ngettext("Dwarf", "Dwarves", count);
	case BATTLE_DWARF:	return ngettext("Battle Dwarf", "Battle Dwarves", count);
	case ELF:		return ngettext("Elf", "Elves", count);
	case GRAND_ELF:		return ngettext("Grand Elf", "Grand Elves", count);
	case DRUID:		return ngettext("Druid", "Druids", count);
	case GREATER_DRUID:	return ngettext("Greater Druid", "Greater Druids", count);
	case UNICORN:		return ngettext("Unicorn", "Unicorns", count);
	case PHOENIX:		return ngettext("Phoenix", "Phoenix", count);

	case CENTAUR:		return ngettext("Centaur", "Centaurs", count);
	case GARGOYLE:		return ngettext("Gargoyle", "Gargoyles", count);
	case GRIFFIN:		return ngettext("Griffin", "Griffins", count);
	case MINOTAUR:		return ngettext("Minotaur", "Minotaurs", count);
	case MINOTAUR_KING:	return ngettext("Minotaur King", "Minotaur Kings", count);
	case HYDRA:		return ngettext("Hydra", "Hydras", count);
	case GREEN_DRAGON:	return ngettext("Green Dragon", "Green Dragons", count);
	case RED_DRAGON:	return ngettext("Red Dragon", "Red Dragons", count);
	case BLACK_DRAGON:	return ngettext("Black Dragon", "Black Dragons", count);

	case HALFLING:		return ngettext("Halfling", "Halflings", count);
	case BOAR:		return ngettext("Boar", "Boars", count);
	case IRON_GOLEM:	return ngettext("Iron Golem", "Iron Golems", count);
	case STEEL_GOLEM:	return ngettext("Steel Golem", "Steel Golems", count);
	case ROC:		return ngettext("Roc", "Rocs", count);
	case MAGE:		return ngettext("Mage", "Magi", count);
	case ARCHMAGE:		return ngettext("Archmage", "Archmagi", count);
	case GIANT:		return ngettext("Giant", "Giants", count);
	case TITAN:		return ngettext("Titan", "Titans", count);

	case SKELETON:		return ngettext("Skeleton", "Skeletons", count);
	case ZOMBIE:		return ngettext("Zombie", "Zombies", count);
	case MUTANT_ZOMBIE:	return ngettext("Mutant Zombie", "Mutant Zombies", count);
	case MUMMY:		return ngettext("Mummy", "Mummies", count);
	case ROYAL_MUMMY:	return ngettext("Royal Mummy", "Royal Mummies", count);
	case VAMPIRE:		return ngettext("Vampire", "Vampires", count);
	case VAMPIRE_LORD:	return ngettext("Vampire Lord", "Vampire Lords", count);
	case LICH:		return ngettext("Lich", "Liches", count);
	case POWER_LICH:	return ngettext("Power Lich", "Power Liches", count);
	case BONE_DRAGON:	return ngettext("Bone Dragon", "Bone Dragons", count);

	case ROGUE:		return ngettext("Rogue", "Rogues", count);
	case NOMAD:		return ngettext("Nomad", "Nomads", count);
	case GHOST:		return ngettext("Ghost", "Ghosts", count);
	case GENIE:		return ngettext("Genie", "Genies", count);
	case MEDUSA:		return ngettext("Medusa", "Medusas", count);
	case EARTH_ELEMENT:	return ngettext("Earth Elemental", "Earth Elementals", count);
	case AIR_ELEMENT:	return ngettext("Air Elemental", "Air Elementals", count);
	case FIRE_ELEMENT:	return ngettext("Fire Elemental", "Fire Elementals", count);
	case WATER_ELEMENT:	return ngettext("Water Elemental", "Water Elementals", count);

	default: break;
    }

    return 1 == count ? GetName() : GetMultiName();
}

u8 Monster::GetSpriteIndex(void) const
{
    return UNKNOWN < id ? id - 1 : 0;
}

ICN::icn_t Monster::ICNMonh(void) const
{

    return id >= PEASANT && id <= WATER_ELEMENT ? static_cast<ICN::icn_t>(ICN::MONH0000 + id - PEASANT) : ICN::UNKNOWN;
}

payment_t Monster::GetCost(void) const
{
    return payment_t(monsters[id].cost);
}

payment_t Monster::GetUpgradeCost(void) const
{
    Monster upgr = GetUpgrade();
    payment_t pay = id != upgr.id ? upgr.GetCost() - GetCost() : GetCost();

    pay.wood *= upgrade_ratio;
    pay.mercury *= upgrade_ratio;
    pay.ore *= upgrade_ratio;
    pay.sulfur *= upgrade_ratio;
    pay.crystal *= upgrade_ratio;
    pay.gems *= upgrade_ratio;
    pay.gold *= upgrade_ratio;

    return pay;
}

u32 Monster::GetCountFromHitPoints(const Monster & mons, u32 hp)
{
    if(hp)
    {
	const u16 hp1 = mons.GetHitPoints();
	const u32 count = hp / hp1;
	return (count * hp1) < hp ? count + 1 : count;
    }

    return 0;
}
