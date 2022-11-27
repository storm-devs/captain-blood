
#include "CharacterPattern.h"
#include "CharacterController.h"

CharacterControllerDeclarant * CharacterControllerDeclarant::first = null;

TWeaponType GetWeaponType(const char* type)
{
	if (!type) return wp_undropable;

	if (type[0]=='U' || type[0]=='u')
	{
		return wp_undropable;
	}
	else
	if (type[0]=='S' || type[0]=='s')
	{
		if (type[5]=='2') return wp_sword2;
		if (type[5]=='3') return wp_sword3;

		return wp_sword1;
	}
	else
	if ((type[0]=='A' || type[0]=='a') && type[1]=='x')
	{
		if (type[3]=='2') return wp_axe2;
		if (type[3]=='3') return wp_axe3;

		return wp_axe1;
	}
	else
	if ( (type[0]=='T' || type[0]=='t') && type[1]=='w')
	{
		if (type[9]=='2') return wp_twohanded2;
		if (type[9]=='3') return wp_twohanded3;

		return wp_twohanded1;	
	}
	else	
	if (type[0]=='G' || type[0]=='g')
	{		
		return wp_gun;
	}
	else	
	if ((type[0]=='A' || type[0]=='a') && type[1]=='r')
	{		
		return wp_armor;
	}
	else
	if ((type[0]=='B' || type[0]=='b') &&
		(type[1]=='O' || type[1]=='o') &&
		(type[2]=='X' || type[2]=='x'))
	{
		return wp_box;
	}
	else
	if ((type[0]=='B' || type[0]=='b') &&
		(type[1]=='O' || type[1]=='o') &&
		(type[2]=='M' || type[2]=='m'))
	{
		if (strlen(type)==4)
		{
			return wp_bomb;
		}

		return wp_bombardier_body;
	}
	else
	if ((type[0]=='R' || type[0]=='r') &&
		(type[1]=='I' || type[1]=='i') &&
		(type[2]=='F' || type[2]=='f'))
	{
		return wp_rifle;
	}
	else
	if ((type[0]=='R' || type[0]=='r') &&
		(type[1]=='A' || type[1]=='a') &&
		(type[2]=='P' || type[2]=='p'))
	{
		return wp_rapire;
	}	

	return wp_undropable;
}

CharacterPattern::CharacterPattern(): logic_loactors(_FL_), item_descr(_FL_, 1), body_parts(_FL_, 1), armorsets(_FL_, 1)
{
	fBloodDropDamage = 18.0f;
	pattern_was_updated = false;

	rdData = null;
}

CharacterPattern::~CharacterPattern()
{
	RELEASE(rdData);
}

bool CharacterPattern::EditMode_Update(MOPReader & reader)
{		
	//Модельки
	modelName = reader.String().c_str();
	armorName = reader.String().c_str();
	ragdollName = reader.String().c_str();
	shadowName = reader.String().c_str();
		
	//Анимацмя
	animationName = reader.String().c_str();
					
	//Физические парметры	
	fRad = reader.Float();
	fHeight = reader.Float();
	
	//парметры AI		
	aiType = reader.Enum();
	side = reader.Enum();

	hp = (float)reader.Long();

	hpBar = reader.String();	

	fBloodDropDamage = reader.Float();

	fHitDmgMultiplayer = reader.Float();
	fBombDmgMultiplayer = reader.Float();
	fShootDmgMultiplayer = reader.Float();
	fRageDmgMultiplayer = reader.Float();

	attackSpeedModifier = reader.Float();

	block_cooldown = reader.Float();
	fBlockProbabilityPerHit = reader.Float() * 0.01f;
	fBlockProbabilityPerShoot = reader.Float() * 0.01f;

	fTotalDmgMultiplier = reader.Float();

	max_armor_damage_absorb = reader.Float() * 0.01f;

	fCointAltetude = reader.Float();

	bShowCoinED = reader.Bool();

	fHeadRotateLimit = reader.Float();
	fHeadRotateOffset = reader.Float();

	bonusTable = reader.String();

	cHairParams = reader.String();

	step_effectTable = reader.String();
	shoot_effectTable = reader.String();

	sword_effectTable = reader.String();

	bombExplosionPattern = reader.String();

	bShotImmuneWhenRoll = reader.Bool();

	int count = reader.Array();	
	logic_loactors.Empty();
	//Логические локаторы	
	for(long i = 0; i < count; i++)
	{
		TLogicLoactor* logic_locator = &logic_loactors[logic_loactors.Add()];

		logic_locator->id = reader.String();
		logic_locator->locator = reader.String();
	}	

	//Предметы
	count = reader.Array();	
	
	item_descr.Empty();

	item_descr.AddElements(count);
	for(long i = 0; i < count; i++)
	{		
		TItemDescr* descr = &item_descr[i];//item_descr.Add()];

		descr->id = reader.String();
		descr->weapon_type = GetWeaponType(reader.Enum().c_str());
		descr->fProbality = reader.Float();
		descr->time_lie = reader.Float();
		descr->itemLife = -1;

		descr->armor_damage_absorb = reader.Float() * 0.01f;
		descr->drop_priority = reader.Long();

		descr->effectTable = reader.String();
		descr->armor_mtl = reader.Bool();
		
		descr->tip_id = reader.String();

		descr->modelName = reader.String().c_str();
		descr->boneName = reader.String().c_str();

		descr->broken_part1 = reader.String().c_str();
		descr->broken_part2 = reader.String().c_str();

		descr->locatorName = reader.String();
		descr->useTrail = reader.Bool();
		
		descr->uniqTexture = reader.String().c_str();
		if (!descr->uniqTexture[0]) descr->uniqTexture = null;				
		descr->trailTechnique = reader.Enum().c_str();
		descr->trailPower = reader.Float();

		descr->showFlare = reader.Bool();		
		descr->attachedobject = reader.String();
		descr->locatortoattache = reader.String().c_str();
		descr->objectoffset = reader.Position();		
	}

	armor_hp_drop = reader.Long();
	armor_drop_prop = reader.Long();

	int num_sets = reader.Array();	
	armorsets.Empty();
	armorsets.AddElements(num_sets);
	for(long i = 0; i < num_sets; i++)
	{
		//armorsets.Add();

		int num_armors = reader.Array();
		
		for(long j = 0; j < num_armors; j++)
		{
			armorsets[i].id_armors.Add(reader.String());
		}		
	}
	
	//Параметры рендера
	fLODDist = reader.Float();
	fLODDist *= fLODDist;
	dynamicLighting = reader.Bool();
	
	shadowCast = reader.Bool();
	shadowReceive = reader.Bool();
	fDistShadow = reader.Float();
	
	seaReflection = reader.Bool();
	fDistReflect = reader.Float();	
	
	seaRefraction = reader.Bool();
	fDistRefract = reader.Float();

	FatalityParams = reader.String();
	AttackParams = reader.String();
	

	count = reader.Array();

	body_parts.Empty();
	body_parts.AddElements(count);

	for(long i = 0; i < count; i++)
	{
		TPartOfMe* part = &body_parts[i];//body_parts.Add()];

		part->id = reader.String();
		part->modelName = reader.String().c_str();
		part->locator = reader.String();
		part->isBox = reader.Bool();
	}	
	
	BasePattern::EditMode_Update(reader);

	return true;	
}

void CharacterPattern::PostCreate()
{
	if (FindObject(bombExplosionPattern, bombExplosionMO))
	{
		static const ConstString bombExplosionPatternId = ConstString("BombExplosionPattern");
		if (!bombExplosionMO.Ptr()->Is(bombExplosionPatternId))
			bombExplosionMO.Reset();
	}
}

// Загрузить данные регдолла, если они еще не загружены
void CharacterPattern::LoadRagdoll()
{
	// если уже загружено или нет регдолла то выходим
	if (rdData || string::IsEmpty(ragdollName))
		return;

	char ragdollPath[128];
	crt_strcpy(ragdollPath, sizeof(ragdollPath), "Resource\\Models\\");
	crt_strcat(ragdollPath, sizeof(ragdollPath), ragdollName);	

	rdData = Files().LoadData(ragdollPath, _FL_);
	if (!rdData)
	{
		api->Error("Ragdoll \"%s\" for character pattern \"%s\" not loaded...", ragdollPath, GetObjectID().c_str());
		return;
	}
}

const char * CharacterPattern::comment = "Character Pattern";

MOP_BEGINLISTCG(CharacterPattern, "CharacterPattern", '1.00', 15, CharacterPattern::comment, "Character")

	MOP_ENUMBEG("CharacterCtrl")
		
		for(CharacterControllerDeclarant * decl = CharacterControllerDeclarant::GetFirst(); decl; decl = decl->GetNext())
		{
			MOP_ENUMELEMENT(decl->Name().c_str())
		}		

	MOP_ENUMEND
	MOP_ENUMBEG("CharacterSide")
		MOP_ENUMELEMENT("Enemy")
		MOP_ENUMELEMENT("Ally")
		MOP_ENUMELEMENT("Ally FrFire")
		MOP_ENUMELEMENT("Boss")
		MOP_ENUMELEMENT("NPC")
	MOP_ENUMEND
	MOP_ENUMBEG("WeaponType")
		MOP_ENUMELEMENT("Undropable")
		MOP_ENUMELEMENT("Sword")
		MOP_ENUMELEMENT("Sword2")
		MOP_ENUMELEMENT("Sword3")
		MOP_ENUMELEMENT("Axe")
		MOP_ENUMELEMENT("Axe2")
		MOP_ENUMELEMENT("Axe3")
		MOP_ENUMELEMENT("Twohanded")
		MOP_ENUMELEMENT("Twohanded2")
		MOP_ENUMELEMENT("Twohanded3")
		MOP_ENUMELEMENT("Rifle")
		MOP_ENUMELEMENT("Gun")
		MOP_ENUMELEMENT("Rapire")
		MOP_ENUMELEMENT("Armor")
		MOP_ENUMELEMENT("Bomb")
	MOP_ENUMEND

	MOP_ENUMBEG("WeaponTypeBroken")
		MOP_ENUMELEMENT("Broken1")
		MOP_ENUMELEMENT("Broken2")
		MOP_ENUMELEMENT("Broken3")
		MOP_ENUMELEMENT("Broken4")
		MOP_ENUMELEMENT("Broken5")
		MOP_ENUMELEMENT("Broken6")
		MOP_ENUMELEMENT("Broken7")
		MOP_ENUMELEMENT("Broken8")
		MOP_ENUMELEMENT("Broken9")		
	MOP_ENUMEND

	MOP_ENUMBEG("TrailTechnique")
		MOP_ENUMELEMENT("Blend")
		MOP_ENUMELEMENT("Add")
		MOP_ENUMELEMENT("Multiply")
	MOP_ENUMEND

	//Физические параметры	
	MOP_STRING("Model name", "")
	MOP_STRING("Armor Model name", "")
	MOP_STRING("Ragdoll name", "")
	MOP_STRING("Shadow model name", "")
	MOP_STRING("Animation", "")	
	MOP_FLOATEX("Raduis", 0.4f, 0.01f, 10.0f);
	MOP_FLOATEX("Height", 1.8f, 0.2f, 10.0f);
	//Игровые параметры
	MOP_ENUM("CharacterCtrl", "AI type")	
	MOP_ENUM("CharacterSide", "Side")
	MOP_LONGEX("HP", 100, 1, 1000000000)
	MOP_STRING("HP Bar", "")

	MOP_FLOATEX("BloodDropDamage", 25.0f, 0.0f, 2000.0f)

	MOP_FLOATEX("HitDmgMultiplayer", 1.0f, 0.0f, 10.0f)
	MOP_FLOATEX("BombDmgMultiplayer", 1.0f, 0.0f, 10.0f)
	MOP_FLOATEX("ShootDmgMultiplayer", 1.0f, 0.0f, 10.0f)
	MOP_FLOATEX("RageDmgMultiplayer", 1.0f, 0.0f, 10.0f)

	MOP_FLOATEX("AttackSpeedModifier", 1.0f, 0.0f, 20.0f)

	MOP_FLOATEXC("Block cooldown", 5.0f, 0.0f, 1000.0f, "Block cooldown")
	MOP_FLOATEXC("Block probability hit", 15.0f, 0.0f, 100.0f, "Block probability increasing each hit")
	MOP_FLOATEXC("Block probability shoot", 15.0f, 0.0f, 100.0f, "Block probability increasing each shoot hit")

	MOP_FLOATC("Damage multiplier", 1.0f, "Outcome damage multiplier")
	
	MOP_FLOATEX("Max Armor damage absorb", 99.0f, 0.0f, 99.0f)

	MOP_FLOATEX("CointAltetude", 2.0f, 0.25f, 10.0f);	
	MOP_BOOL("Show Coin", false)

	MOP_FLOATEX("HeadRot Limit", 40.0f, 0.0f, 90.0f);
	MOP_FLOATEX("HeadRot Offset", 0.0f, -180.0f, 180.0f);

	MOP_STRING("Bonus Table", "")

	MOP_STRING("Hair Params", "")

	MOP_STRING("FootStep Effect Table", "")
	MOP_STRING("Shoot Effect Table", "")

	MOP_STRING("Character Effect Table", "")

	MOP_STRING("Bomb explosion pattern", "StdBombExplosionPattern")

	MOP_BOOL("Immune to shot when roll", true)

	//Логические локаторы
	MOP_ARRAYBEGC("Logic locators", 0, 1000, "Geometry locators aliases")
		MOP_STRINGC("Name", "", "Logic locator identifier")
		MOP_STRINGC("Locator", "", "Geometry locator name, is empty, item can't draw")
	MOP_ARRAYEND
	
	//Предметы
	MOP_ARRAYBEGC("Items", 0, 1000000, "Items list")
		MOP_STRINGC("Id", "", "Identifier of this item (can be not unique)")
		MOP_ENUM("WeaponType","Type")
		MOP_FLOATEX("Drop Probality", 80.0f, 0.0f, 100.0f)
	    MOP_FLOATEX("Weapon Lie Time", 10.0f, 1.0f, 100000.0f);
		MOP_FLOATEX("Armor damage absorb", 10.0f, 0.0f, 99.0f)
		MOP_LONGEX("Armor drop priority", 0, 0, 15)
			
		MOP_STRING("Effect Table", "")
		MOP_BOOL("Armor material", false)

		MOP_STRINGC("TipID", "Weapon", "Identifier of tip")		
		MOP_STRINGC("Model", "", "Model name for this item")
		MOP_STRINGC("Bone Name", "", "Name of Bone")
		MOP_STRINGC("Broken Part1", "", "Model name for first broken part")
		MOP_STRINGC("Broken Part2", "", "Model name for second broken part")				

		MOP_STRINGC("Locator", "", "Logic or geometry locator name")
		MOP_BOOLC("Trail", false, "Can show draw trail for this item")
		MOP_STRINGC("Trail texture", "", "You can set unique texture for this trail")
		MOP_ENUM("TrailTechnique", "Trail technique")
		MOP_FLOATC("Trail power", 1.0f, "")
		MOP_BOOLC("Flare", false, "Show Flare for this item")
		MOP_STRINGC("Attached object", "", "Name of mission object ")
		MOP_STRINGC("Locator to attache", "", "You can set name ")
		MOP_POSITION("Object Offset", Vector(0.0f))
	MOP_ARRAYEND
	
	MOP_LONGEX("damage when drop armor", 30, 0, 1000)
	MOP_LONGEX("armor drop probality", 50, 0, 100)

	MOP_ARRAYBEGC("Armor Sets", 0, 25, "Armor Sets")
		MOP_ARRAYBEGC("Set", 0, 25, "Armor Set")
			MOP_STRINGC("ID", "", "Armor ID")
		MOP_ARRAYEND
	MOP_ARRAYEND	
		
	MOP_GROUPBEG("Render params")
		MOP_FLOATEX("Distance of LOD", 15.0f, 5.0f, 1000.0f);
		MOP_BOOL("Dynamic lighting", false)
		MOP_BOOLC("Shadow cast", false, "Character can is shadow cast by some objects")
		MOP_BOOLC("Shadow receive", false, "Character can is shadow receive from casting objects")
		MOP_FLOATEX("Distance to drop shadow", 20.0f, 5.0f, 1000.0f);
		MOP_BOOLC("Sea reflection", false, "Character can reflect in sea")
		MOP_FLOATEX("Distance to reflect", 20.0f, 5.0f, 1000.0f);
		MOP_BOOLC("Sea refraction", false, "Character can refract in sea")
		MOP_FLOATEX("Distance to refract", 20.0f, 5.0f, 1000.0f);
	MOP_GROUPEND()	

	MOP_STRING("Fatality Params", "")
	MOP_STRING("Attack Params", "")

	MOP_ARRAYBEGC("Body Parts", 0, 1000, "Body Parts List")
		MOP_STRING("ID", "")
		MOP_STRING("ModelName", "")
		MOP_STRING("Locator", "")
		MOP_BOOL("Is Box", true)
	MOP_ARRAYEND

MOP_ENDLIST(CharacterPattern)