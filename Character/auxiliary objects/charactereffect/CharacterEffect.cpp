
#include "CharacterEffect.h"
#include "..\..\Character\Components\CharacterAnimation.h"
#include "..\..\Character\Components\CharacterPhysics.h"
#include "..\..\Character\Components\CharacterItems.h"



Color CharacterEffect::ReadColorFromGraph(array<TMdlPoint> & points, float time)
{
	if (time < points[0].time)
	{
		return points[0].c;
	}

	if (time>points[points.Last()].time)
	{
		return points[points.Last()].c;
	}

	for (int i = 0;i < (int)points.Size()-1; i++)
	{
		if (time < points[i+1].time)
		{
			float k = (time-points[i].time) / (points[i+1].time-points[i].time);
			Color res;
			res.Lerp(points[i].c, points[i+1].c, k);
			return res;
		}
	}

	return Color(0.0f);
}


float CharacterEffect::ReadValueFromGraph(array<TMdlPoint> & points, float time)
{
	if (time < points[0].time)
	{
		return points[0].value;
	}

	if (time>points[points.Last()].time)
	{
		return points[points.Last()].value;
	}

	for (int i = 0;i < (int)points.Size()-1; i++)
	{
		if (time < points[i+1].time)
		{
			float k = (time-points[i].time) / (points[i+1].time-points[i].time);
			return Lerp(points[i].value, points[i+1].value, k);
			//return points[i].value + (points[i+1].value-points[i].value) * k;
		}
	}

	return 0.0f;
}



CharacterEffect::CharacterEffect():effects(_FL_, 1),instances(_FL_,16),htEffects(_FL_)
{
	level = ML_ALPHA1;
}

CharacterEffect::~CharacterEffect()
{
	Release();
}

void CharacterEffect::Release()
{
	for (int i=0;i<(int)effects.Size();i++)
	{
		RELEASE(effects[i].md_anim);
		RELEASE(effects[i].md_model);

		effects[i].cl_graph.Release();
		//delete effects[i].cl_graph;
		
		effects[i].l_graph.Release();
		//delete effects[i].l_graph;

		//effects[i].md_graph->Release();
		//delete effects[i].md_graph;

		effects[i].wg_graph.Release();
		//delete effects[i].wg_graph;
	}

	effects.DelAll();
}

//Инициализировать объект
bool CharacterEffect::Create(MOPReader & reader)
{
	InitParams(reader);
		
	///if (EditMode_IsOn())	
	//{
		SetUpdate(&CharacterEffect::Draw, level);	
	//}
	//else
	//{		
	//	DelUpdate(&CharacterEffect::Draw);
	//}
	
	return true;
}

void CharacterEffect::PostCreate()
{
	for (int i=0;i<(int)effects.Size();i++)
	{
		TEffect* effect = &effects[i];

		MOSafePointer mo;
		
		FindObject( effect->wg_name, mo);
		effect->wg_wiget = mo.Ptr();

		FindObject( effect->l_light_name, mo);
		effect->l_light = mo.Ptr();
	}
}

void CharacterEffect::InitParams(MOPReader &reader)
{
	Release();

	int count = reader.Array();

	effects.AddElements(count);
	for (int i=0;i<count;i++)
	{
		TEffect* effect = &effects[i];

		effect->timelife = 0.0f;

		effect->id = reader.String();
		effect->attach_to_node = reader.Bool();
		effect->blend_time = reader.Float();

		effect->cl_wp_id = reader.String();
		effect->cl_color = reader.Colors();
		//effect->cl_graph = NEW Graphic();
		effect->cl_graph.ReadParams(reader);
		effect->timelife = coremax(effect->timelife,effect->cl_graph.GraphicMaxTime());
		

		effect->l_wp_id = reader.String();
		effect->l_locator = reader.String().c_str();
		effect->l_light_name = reader.String();
		effect->l_light = null;
		//effect->l_graph = NEW Graphic();
		effect->l_graph.ReadParams(reader);
		effect->timelife = coremax(effect->timelife,effect->l_graph.GraphicMaxTime());
		
		effect->md_model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());
		effect->md_anim = Animation().Create(reader.String().c_str(), _FL_);

		if (effect->md_model)
		{
			effect->md_model->SetAnimation(effect->md_anim);
			
			if (effect->md_anim) effect->md_anim->Pause(true);
		}
		
		effect->md_node = reader.String().c_str();
		effect->md_bindedToChar = reader.Bool();
		
		if (reader.Bool() && effect->md_model)
		{
			effect->md_model->SetFloatAlphaReference(0.003921f);
		}

		level = reader.Long();
		level = (ML_PARTICLES5 - ML_ALPHA3)*level/100 + ML_ALPHA3;

		//effect->md_graph = NEW Graphic();
		//effect->md_graph->ReadParams(reader);

		
		effect->md_points.Empty();
		int count = reader.Array();
		effect->md_points.AddElements(count);
		for (int j=0; j<count; j++)
		{
			TMdlPoint* point = &effect->md_points[j];
			point->time = reader.Float();
			point->value = reader.Float();
			point->c = reader.Colors();
		}

		float graphicMaxTime = 0.0f;
		if (effect->md_points.Size() > 0)
		{
			graphicMaxTime = effect->md_points[effect->md_points.Last()].time;
		}
		



		effect->timelife = coremax(effect->timelife, graphicMaxTime/*effect->md_graph->GraphicMaxTime()*/);

		effect->wg_name = reader.String();
		effect->wg_wiget = null;
		//effect->wg_graph = NEW Graphic();
		effect->wg_graph.ReadParams(reader);
		effect->timelife = coremax(effect->timelife,effect->wg_graph.GraphicMaxTime());

		// VANO: добавляем в хеш таблицу
		*htEffects.Add(effect->id) = int(i);
	}
}

void _cdecl CharacterEffect::Draw(float dltTime, long level)
{
	for (int i=0;i<(int)instances.Size();i++)
	{		
		TEffectInstance& effect = instances[i];
		TEffect& effect_ref = effects[effect.ref];

		if (effect_ref.attach_to_node && effect.onNode && effect.time<effect_ref.timelife-effect_ref.blend_time)
		{						
			if (!string::IsEqual(effect.node,effect.owner->animation->CurrentNode()))
			{
				if (effect_ref.blend_time<0.05f)
				{
					effect.time = effect_ref.timelife + 0.01f;
				}
				else
				{
					effect.onNode = false;
					effect.timescale = (effect_ref.timelife - effect.time)/effect_ref.blend_time;
				}
			}
		}		

		effect.time += dltTime * effect.timescale;

		if (effect.time>effect_ref.timelife)
		{
			if (effect.cl_wp_index!=-1 && effect.cl_wp_index <(int)effect.owner->items->items.Size())
			{
				effect.owner->items->items[effect.cl_wp_index].color = 0.0f;//effect_ref.cl_color * effect_ref.cl_graph.ReadGraphic(effect_ref.timelife);
			}

			if (effect_ref.l_light) effect_ref.l_light->Activate(false);

			if (effect_ref.md_anim)
			{
				effect_ref.md_anim->Start();
				//effect_ref.md_anim->Pause(true);
			}

			if (effect_ref.wg_wiget) effect_ref.wg_wiget->Show(false);

			instances.DelIndex(i);
			i--;

			continue;
		}

		//Effect1
		if (effect.cl_wp_index!=-1 && effect.cl_wp_index<(int)effect.owner->items->items.Size())
		{
			effect.owner->items->items[effect.cl_wp_index].color = effect_ref.cl_color * effect_ref.cl_graph.ReadGraphic(effect.time);
		}

		//Effect2
		if (effect.l_wp_index!=-1 && effect_ref.l_light)
		{
			Matrix mtx;
			if (effect.owner->items->FindLocatorinItem(effect.l_wp_index,effect_ref.l_locator,mtx))
			{			
				char params[3][32];

				crt_snprintf(params[0], ARRSIZE(params[0]),"%f",mtx.pos.x);
				crt_snprintf(params[1], ARRSIZE(params[1]),"%f",mtx.pos.y);
				crt_snprintf(params[2], ARRSIZE(params[2]),"%f",mtx.pos.z);

				const char * prms[3];
				prms[0] = params[0];
				prms[1] = params[1];
				prms[2] = params[2];
			
				effect_ref.l_light->Command("moveto", 3, prms);
			
				crt_snprintf(params[0], ARRSIZE(params[0]),"%f", effect_ref.l_graph.ReadGraphic(effect.time));
				effect_ref.l_light->Command("SetMultiply", 1, prms);				
			}
		}

		//Effect3
		if (effect_ref.md_model)
		{
			if (effect_ref.md_anim && !effect.traveled_to_node)
			{
				if (string::IsEqual(effect_ref.md_anim->CurrentNode(),effect_ref.md_node))
				{
					effect.traveled_to_node = true;
				}
			}
			
			if (effect.traveled_to_node)
			{
				Color color;
				//color.a = effect_ref.md_graph->ReadGraphic(effect.time);

				color = ReadColorFromGraph(effect_ref.md_points, effect.time);
				color.a = ReadValueFromGraph(effect_ref.md_points, effect.time);
				
			
				effect_ref.md_model->SetUserColor(color);			

				Matrix mtx;
				effect.owner->physics->GetMatrixWithBoneOffset(mtx);
			
				Matrix mtx2;
				effect.owner->physics->GetModelMatrix(mtx2);

				mtx2.pos = mtx.pos;

				if (effect_ref.md_bindedToChar)
				{
					effect_ref.md_model->SetTransform(mtx2);
				} else
				{
					effect_ref.md_model->SetTransform(effect.md_bornedMtx);
				}
				effect_ref.md_model->Draw();
			}
		}

		//Effect4
		if (effect_ref.wg_wiget)
		{
			const char* params[2];

			static char str[32];			
			crt_snprintf(str, sizeof(str),"%4.3f",Clampf(effect_ref.wg_graph.ReadGraphic(effect.time)));	
			params[0] = str;			

			effect_ref.wg_wiget->Command("setfxa",1,params);
		}
	}
}

void CharacterEffect::MakeEffect(Character* owner, const ConstString & id)
{
	if (!owner) return;

	StopEffect(id);

	if (int * index = htEffects.Find(id))//for (int i=0;i<(int)effects.Size();i++)
	{
		int i = *index;
		//if (string::IsEqual(effects[i].id,id))
		{
			TEffectInstance* effect = &instances[instances.Add()];

			effect->owner = owner;
			effect->ref = i;
			effect->time = 0.0f;
			effect->timescale = 1.0f;

			effect->md_bindedToChar = effects[i].md_bindedToChar;
			if (effect->md_bindedToChar == false)
			{
				Matrix mtx(false);
				owner->physics->GetMatrixWithBoneOffset(mtx);

				owner->physics->GetModelMatrix(effect->md_bornedMtx);
				effect->md_bornedMtx.pos = mtx.pos;

				//api->Trace("pos: %f, %f, %f\n", effect->md_bornedMtx.pos.x, effect->md_bornedMtx.pos.y, effect->md_bornedMtx.pos.z);
			}

			crt_strcpy(effect->node, sizeof(effect->node),owner->animation->CurrentNode());			
			effect->onNode = true;
			effect->traveled_to_node = true;

			effect->cl_wp_index = owner->items->FindItemIndex(effects[i].cl_wp_id);
			effect->l_wp_index  = owner->items->FindItemIndex(effects[i].l_wp_id);
			
			if (effects[i].l_light) effects[i].l_light->Activate(true);

			if (effects[i].md_anim)
			{				
				effects[i].md_anim->Goto(effects[i].md_node,0.0f);
				effects[i].md_anim->Pause(false);
				effect->traveled_to_node = false;
			}			

			if (effects[i].wg_wiget)
			{
				effects[i].wg_wiget->Show(true);
			}

			return;
		}
	}

	/*for (int i=0;i<(int)effects.Size();i++)
	{
		if (string::IsEqual(effects[i].id,id))
		{
			int a = 1;
		}
	}*/

	LogicDebug("Can't activate effect: %s for %s",id.c_str(), owner->GetObjectID().c_str());
}

void CharacterEffect::StopEffect(const ConstString & id)
{
	for (int i=0;i<(int)instances.Size();i++)
	{		
		TEffectInstance& effect = instances[i];
		TEffect& effect_ref = effects[effect.ref];

		if (effect_ref.id == id)
		{
			instances.DelIndex(i);
			break;
		}
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(CharacterEffect, "Character effects", '1.00', 50, "Advanced character's effects", "Character objects")


	MOP_ARRAYBEG("Effects", 0, 10000)

		MOP_STRINGC("ID", "","ID of Effect")
		MOP_BOOLC("Attached to Node", true,"Auto disable if Node was changed")
		MOP_FLOATEX("Blend time", 0.2f,0.0f,10.0f)
				
		//Effect1
		MOP_STRINGC("Color Wp_ID", "", "ID of Weapon that needed to change color")
		MOP_COLORC("Color",0xffffffff,"Color of Weapon")
		MOP_GRAPHIC("Color Intecity","Color Intecity")			

		//Effect2
		MOP_STRINGC("Light Wp_ID", "", "ID of Weapon on that attached weapon")
		MOP_STRINGC("Light Wp_Locator", "","Name of Weapon Locator")
		MOP_STRINGC("Light Name", "", "Name of Mission Point Light")
		MOP_GRAPHIC("Light Intecity", "Light Intecity")					

		//Effect3		
		MOP_STRINGC("Model Name", "", "Name of Model used in Effect")
		MOP_STRINGC("Model Animation", "", "Name of Animation used in Effect")
		MOP_STRINGC("Model Node Name", "", "Name of Animation Node that used in Effect")
		MOP_BOOLC("Model binded to character", true, "Bind effect model to character")
		MOP_BOOLC("Smooth alpha", true, "Use geom smooth alpha")
		MOP_LONGEXC("Draw level", 0, 0, 100, "от ML_ALPHA1 до ML_PARTICLES5 меняется в процентах");
		
		//MOP_GRAPHIC("Model Tracperancy","Model Tracperancy")		

		MOP_ARRAYBEGC("Model Tracperancy", 2, 1000000, "Model Tracperancy")
			MOP_FLOATEX("time", 0.0f, 0.0f, 1024.0f)
			MOP_FLOATEX("value", 0.0f, 0.0f, 1.0f)
			MOP_COLOR("color", Color(0.0f))
		MOP_ARRAYEND

		//Effect4
		MOP_STRINGC("Widget Name", "", "Name of Widget used in Effect")		
		MOP_GRAPHIC("Widget Tracperancy","Widget Tracperancy")		

	MOP_ARRAYEND

MOP_ENDLIST(CharacterEffect)


