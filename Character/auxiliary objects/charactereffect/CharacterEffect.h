
#ifndef _CharacterEffect_h_
#define _CharacterEffect_h_

#include "..\..\..\Common_h\Mission.h"
#include "..\..\Character\Character.h"
#include "..\..\..\Common_h\Graphic\Graphic.h"

class CharacterEffect : public MissionObject
{	
public:
	struct TMdlPoint
	{
		float time;
		float value;
		Color c;
	};

	static float ReadValueFromGraph(array<TMdlPoint> & points, float time);
	static Color ReadColorFromGraph(array<TMdlPoint> & points, float time);

private:

	struct TEffect
	{
		ConstString id;
		bool attach_to_node;

		float timelife;
		float blend_time;

		//Effect1
		ConstString cl_wp_id;		
		Color		cl_color;
		Graphic	    cl_graph;

		//Effect2
		ConstString    l_wp_id;
		const char*    l_locator;
		ConstString    l_light_name;		
		Graphic		   l_graph;
		MissionObject* l_light;
		
		//Effect3
		IGMXScene*  md_model;
		IAnimation* md_anim;
		const char* md_node;
		bool		md_bindedToChar;
		//Graphic*	md_graph;
		array<TMdlPoint> md_points;

		//Effect4		
		ConstString    wg_name;
		Graphic		   wg_graph;		
		MissionObject* wg_wiget;

		TEffect() : md_points (_FL_, 1)
		{

		}
	};

	array<TEffect> effects;
	HashTable<int, 128, 4>	htEffects;

	struct TEffectInstance
	{
		char node[64];
		int ref;
		Character* owner;
		float time;
		float timescale;

		int cl_wp_index;
		int l_wp_index;		

		bool md_bindedToChar;
		Matrix md_bornedMtx;

		bool traveled_to_node;
		bool onNode;

		TEffectInstance() : md_bornedMtx (true)
		{

		}
	};

	array<TEffectInstance> instances;

	int level;

public:

	CharacterEffect();
	virtual ~CharacterEffect();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();		

	void InitParams(MOPReader &reader);

	void _cdecl Draw(float dltTime, long level);

	void Release();

	void MakeEffect(Character* owner,const ConstString & id);
	void StopEffect(const ConstString & id);

	MO_IS_FUNCTION(CharacterEffect, MissionObject);

};

#endif











