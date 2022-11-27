
#include "TipsManager.h"


TipsManager::TipsManager():tips_types(_FL_),tips(_FL_)
{
	isEnable = false;
}

TipsManager::~TipsManager()
{
	for (int i=0;i<tips;i++)
	{
		if (tips[i])
		{
			delete tips[i];
		}
	}

	tips.Empty();
}

//Инициализировать объект
bool TipsManager::Create(MOPReader & reader)
{
	if (GetManager(&Mission()))
	{
		return false;
	}

	Registry(TipsManagerGroup());

	InitParams(reader);
		
	if (!EditMode_IsOn())	
	{
	//	SetUpdate(&TipsManager::Draw,ML_ALPHA5);
		Enable(true);
		//SetUpdate(&TipsManager::Draw,ML_GUI1);
	}
	else
	{
		Enable(false);
		//DelUpdate(&TipsManager::Draw);
	}
	
	return true;
}

//Обновить параметры
bool TipsManager::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Нарисовать модельку
void _cdecl TipsManager::Draw(float dltTime, long level)
{
	MissionObject* player = Mission().Player();

	if (!player) return;

	Matrix mat(true);
	player->GetMatrix(mat);

	for (int i = 0; i<(int)tips.Size(); i++)
	if (tips[i]->IsActive())
	{		
		TipDescr* tip = &tips_types[tips[i]->GetTipRef()];
		
		tips[i]->Update(dltTime);

		if (tips[i]->GetAlpha() < 0.01f) continue;

		float dist = (mat.pos - tips[i]->GetPos()).GetLength();
		
		if ( dist < 5.0f)
		{			
			float alpha = 1 - Clampf((dist-2), 0.0f, 3.0f)/3.0f;			

			Matrix mVP(Render().GetView(), Render().GetProjection());
			Vector4 v = mVP.Projection(tips[i]->GetPos() + Vector (0,1,0), Render().GetViewport().Width * 0.5f, Render().GetViewport().Height * 0.5f);				
			if (v.w <= 0.0f) continue;
			
			if (!tip->widget.Validate())
			{
				if(tip->widgetName.NotEmpty())
				{
					static const ConstString typeId("GUIWidget");
					tip->widget.FindObject(&Mission(),tip->widgetName,typeId);
				}

				if(!tip->widget.Ptr())
				{
					tip->widgetName.Empty();
				}
			}

			if (tip->widget.Ptr())
			{
				if (tip->is3D)
				{
					float w,h;
					tip->widget.Ptr()->GetSize(w,h);

					float asp = tip->widget.Ptr()->GetAspect();				
	
					tip->widget.Ptr()->SetAlign(IGUIElement::OnLeft);
					tip->widget.Ptr()->SetPosition((v.x/Render().GetViewport().Width )/asp - w*0.5f,
											       (v.y/Render().GetViewport().Height)		- h*0.5f);
				}				

				tip->widget.Ptr()->SetAlpha(alpha * tips[i]->GetAlpha());				
				tip->widget.Ptr()->Draw();				
			}		

			tips[i]->SetState(ITip::inactive);
		}
	}

	for( int i = 0 ; i < tips_types ; i++ )
	{
		TipDescr &d = tips_types[i];

		if( d.widget.Ptr() && d.update )
		{
			d.widget.Ptr()->Update(dltTime);
		}
	}
}

void TipsManager::InitParams(MOPReader & reader)
{
	int count = reader.Array();

	tips_types.AddElements(count);
	for (int i=0;i<count;i++)
	{
		TipDescr* tip = &tips_types[i];
		tip->id = reader.String();
		tip->widget.Reset();
		tip->widgetName = reader.String();
		tip->is3D = reader.Bool();

		//// переделать потом по уму ////

		tip->update = true;

		for( int j = 0 ; j < i ; j++ )
		{
			if( tips_types[i].widgetName == tip->widgetName )
			{
				tip->update = false;
				break;
			}
		}

		/////////////////////////////////
	}
}

ITip* TipsManager::CreateTip(const ConstString & id, Vector pos,void* ptr)
{
	for (int i=0;i<tips_types;i++)
	{
		if (tips_types[i].id == id)
		{
			Tip* tip = NEW Tip(this,i,ptr);

			tip->SetPos(pos);
			tips.Add(tip);

			return tip;
		}
	}

	return null;
}

void TipsManager::DeleteTip(ITip* tip)
{
	for (int i=0;i<tips;i++)
	{
		if (tips[i] == tip)
		{
			tips.DelIndex(i);
			
			return;
		}
	}
}

void TipsManager::Enable(bool enable)
{
	if (isEnable == enable)
		return;

	isEnable = enable;

	if (isEnable)	
	{
		SetUpdate(&TipsManager::Draw,ML_GUI1);
	}
	else
	{
		DelUpdate(&TipsManager::Draw);
	}
}

void TipsManager::SetTipState(ITip::TState _state, void* ptr)
{
	for (int i=0;i<tips;i++)
	{
		if (tips[i]->GetObjPtr() == ptr)
		{
			tips[i]->SetState(_state);
		}
	}
}

bool TipsManager::IsAnyActiveTip()
{
	for (int i=0;i<tips;i++)
	if (tips[i]->IsActive())
	{
		if (tips[i]->GetState() == ITip::active)
		{
			return true;
		}
	}

	return false;
}

MOP_BEGINLISTCG(TipsManager, "TipsManager", '1.00', 0, "TipsManager", "Interface")

	MOP_ARRAYBEGC("Tips", 0, 1000, "Description of tips")
		MOP_STRINGC("ID", "", "ID of tip")				
		MOP_STRING("Widget Name", "")
		MOP_BOOL("Is 3D", true)
	MOP_ARRAYEND
	
MOP_ENDLIST(TipsManager)


