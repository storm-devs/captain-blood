
#include "ButtonMG.h"
#include "CharacterAnimation.h"

#include "..\..\..\common_h\AnimationNativeAccess.h"

array<CharacterButtonMiniGame::TNodeButton> CharacterButtonMiniGame::nodeButtons(_FL_);

CharacterButtonMiniGame::CharacterButtonMiniGame(Character * character): buttonsStatus(_FL_)
{
	chr = character;

	end_node = null;
	played_node = null;	
	nodeChanged = false;
	last_show_wg = -1;
	m_time = null;
	bGameBroken = false;
	auto_win_game = false;
}

void CharacterButtonMiniGame::Init()
{
	buttonsStatus.Empty();

	AddButton("MngA");
	AddButton("MngB");
	AddButton("MngC");
	AddButton("MngD");

	AddButton("MngLeft");
	AddButton("MngRight");
	AddButton("MngUp");
	AddButton("MngDown");

	mode = mg_disabled;

	iMG_Button = -1;

	button_times = 5;
	button_duration = 0.3f;

	win_triger = NULL;
	lose_triger = NULL;
	button_triger = NULL;

	last_show_wg = -1;

	debug_count = 0;
}

void CharacterButtonMiniGame::AddButton(const char* BtnName)
{
	PressedButton PrsdBtn;

	crt_strcpy(PrsdBtn.ButtonName, sizeof(PrsdBtn.ButtonName), BtnName);
	PrsdBtn.bBeenPressed = false;
	PrsdBtn.ButtonCode = chr->Controls().FindControlByName(BtnName);

	buttonsStatus.Add(PrsdBtn);
}

void CharacterButtonMiniGame::ChoseRandomButton(bool remember_button, float border,int offset)
{
	char nodeName[48];
	sprintf_s(nodeName,sizeof(nodeName),"%s%s",chr->GetObjectID().c_str(),chr->animation->CurrentNode());

	unsigned long hash = string::Hash(nodeName);

	bool need_add = false;

	if (remember_button)
	{
		if (string::IsEqual(chr->arbiter->GetDifficulty().c_str(), "Easy") ||
			string::IsEqual(chr->arbiter->GetDifficulty().c_str(), "Normal"))
		{		
			for (int i=0;i<(int)nodeButtons.Size();i++)
			{
				if (nodeButtons[i].hash == hash)
				{
					iMG_Button = nodeButtons[i].button;
					return;
				}
			}

			need_add = true;
		}
	}

	int iMG_newbutton = iMG_Button;

	while (iMG_Button == iMG_newbutton)
	{
		iMG_newbutton = int(Rnd(border))+offset;
	}

	iMG_Button = iMG_newbutton;

	if (nodeButtons.Size()<1024 && need_add)
	{
		TNodeButton& node = nodeButtons[nodeButtons.Add()];		
		node.hash = hash;
		node.button = iMG_Button;
	}
}

void CharacterButtonMiniGame::SetMissionTime(MissionObject* tm)
{
	m_time = tm;
}

bool CharacterButtonMiniGame::IfMissionTime()
{
	return (m_time!=0);
}

void CharacterButtonMiniGame::Start(MiniGame_Desc& desc, bool auto_win)
{
	if (mode != mg_disabled) return;

	auto_win_game = auto_win;

	autoSkip = false;

	if (!FilterDifficulty(desc.difficulty))
	{
		if (!desc.autowin_difficulty || string::IsEmpty(desc.autowin_difficulty)) return;
		if (!FilterDifficulty(desc.autowin_difficulty)) return;
		autoSkip = true;
	}	
	
	bGameBroken = false;
	debug_count = 0;

	end_node = desc.end_node;
	played_node = null;
	nodeChanged = false;
	
	button_duration=desc.fDuration;
	button_times=desc.iTimes;

	id = desc.id;

	MOSafePointer mo;
	
	chr->FindObject(desc.wintriger,mo);
	win_triger = mo.Ptr();

	chr->FindObject(desc.losetriger,mo);
	lose_triger = mo.Ptr();

	chr->FindObject(desc.buttontriger,mo);
	button_triger = mo.Ptr();

	for (dword i=0;i<buttonsStatus.Size();i++)	
	{
		buttonsStatus[i].bBeenPressed = false;
	}

	mode = desc.type;
	
	if (mode == mg_button_cycled)
	{
		num_pressed = (int)((float)button_times * 0.5f);
		button_time = button_duration;
	}
	else
	{
		num_pressed = 0;
	}

	right_pressed = false;
	lose_triggered = false;

	wait_end_mg = desc.wait_end_mg;

	if (string::IsEqual(desc.button,"MngRND"))
	{
		ChoseRandomButton(NeedToRememeber(desc), 7.99f,0);		
	}
	else
	if (string::IsEqual(desc.button,"MngRND_Btn"))
	{
		ChoseRandomButton(NeedToRememeber(desc),3.99f,0);		
	}
	else
	if (string::IsEqual(desc.button,"MngRND_DPad"))
	{
		ChoseRandomButton(NeedToRememeber(desc),3.99f,4);
	}
	else
	{
		for (dword i=0;i<buttonsStatus.Size();i++)	
		{
			if (string::IsEqual(buttonsStatus[i].ButtonName,desc.button))
			{
				iMG_Button = i;
				break;
			}
		}
	}

	if (iMG_Button == -1)
	{
		mode = mg_disabled;

		return;
	}

	for (int i=0;i<16;i++)
	{
		widgets[i] = desc.widgets[i];
	}

	if (!autoSkip) ShowWidget(true);	
}

void CharacterButtonMiniGame::ShowWidgetImpl(bool show,int bt_index, bool is_anim)
{
	MOSafePointer mo;

	if (is_anim)
	{
		chr->FindObject(widgets[bt_index+8],mo);
	}
	else
	{
		chr->FindObject(widgets[bt_index],mo);
	}

	if (mo.Ptr())
	{
		mo.Ptr()->Show(show);
	}	
}

void CharacterButtonMiniGame::ShowWidget(bool show)
{
	if (iMG_Button==-1) return;	

	if (show && last_show_wg != -1)
	{
		ShowWidgetImpl(false,last_show_wg,true);
		ShowWidgetImpl(false,last_show_wg,false);		
	}
	else
	{
		last_show_wg = -1;
	}

		
	if (button_times > 1 || mode == mg_button_cycled)
	{
		ShowWidgetImpl(show,iMG_Button,true);
	}
	else
	{
		ShowWidgetImpl(show,iMG_Button,false);
	}

	last_show_wg = iMG_Button;
}

bool CharacterButtonMiniGame::IsButtonPressed()
{
	if (iMG_Button==-1) return false;	

	if (auto_win_game) return true;

	bool bNeededButtonPressed = false;
	int  iAnotherButtonNum = 0;

	for (dword i=0;i<buttonsStatus.Size();i++)	
	{
		if (i==iMG_Button)
		{
			if (buttonsStatus[i].bBeenPressed)
			{		
				bNeededButtonPressed = true;
			}
		}
		else
		{
			if (buttonsStatus[i].bBeenPressed)
			{		
				iAnotherButtonNum++;
			}
		}
	}

	return bNeededButtonPressed && iAnotherButtonNum<2;
}

void CharacterButtonMiniGame::Update(float fDeltaTime,IAnimation** anim, int num_anim)
{			
	if (num_anim<=0) return;
	
	if (m_time && chr->logic->IsActor())
	{		
		AGNA_GetCurrentFrame frame;
		float cur_frame = -1.0f;

		for (int i=0;i<num_anim;i++)
		{
			if (anim[i])
			{					
				anim[i]->GetNativeGraphInfo(frame);

				dword color = 0xffffff00;
				if (cur_frame<0.0f)
				{
					cur_frame = frame.currentFrame;
				}
				else
				{
					if (fabs(cur_frame - frame.currentFrame) >0.001f)
					{
						color = 0xffff00ff;
					}
				}				

				chr->Render().Print(10,(float)150 + i*12.0f,color,"%s : %s, frame %4.3f, timestep %4.3f", anim[i]->GetName(), anim[i]->CurrentNode(),frame.currentFrame, anim[i]->GetTimeEventStep());				
			}
		}	

		if (!bGameBroken)
		{			
			cur_frame = -1.0f;

			bool achtung = false;

			for (int i=0;i<num_anim;i++)
			{
				if (anim[i])
				{
					anim[i]->GetNativeGraphInfo(frame);					

					if (cur_frame<0.0f)
					{
						cur_frame = frame.currentFrame;
					}
					else
					{
						if (fabs(cur_frame - frame.currentFrame) >0.001f)
						{
							achtung = true;
								
							break;								
						}
					}
				}
			}

			if (achtung)
			{
				debug_count++;

				if (debug_count>2)
				{
					bGameBroken = true;
					m_time->Activate(true);
				}
			}
			else
			{
				debug_count = 0;
			}								
		}
		
		if (m_time)
		{
			if (bGameBroken && m_time->IsActive())
			{
				return;
			}
		}		
	}

	if (mode == mg_disabled) return;

	/*api->Trace("Frame");
	api->Trace("before:");

	for (int i=0;i<num_anim;i++)
	{
		if (anim[i]) api->Trace(anim[i]->CurrentNode());
	}*/

	if (!played_node)
	{
		played_node = anim[0]->CurrentNode();
	}
	
	/*else
	{
		if (right_pressed && nodeChanged)
		{				
			WinNormalMG(anim, num_anim);
		}			
	}*/

	if (!string::IsEqual(played_node, anim[0]->CurrentNode()))
	{
		if (string::IsEqual(played_node, end_node))
		{			
			Finish();
			return;
		}
		else
		{
			//nodeChanged = false;
			played_node = anim[0]->CurrentNode();
		}
	}

	if (!string::IsEqual(played_node, end_node))
	{				
		{
			//nodeChanged = true;
			if (mode == mg_button_normal)
			{				
				ActivateLink("Proceed",anim, num_anim);					
				//nodeChanged = false;
			}
			/*else
				if (mode == mg_button_cycled)
				{
					ActivateLink("win",anim, num_anim);	
				}*/
		}
	}

	if (mode != mg_disabled && button_triger)
	{
		if (chr->Controls().GetControlStateType(buttonsStatus[iMG_Button].ButtonName) == CST_ACTIVATED)
		{
			chr->LogicDebug("Button Triger in '%s' ButtonMG Activated:", id);
			button_triger->Activate(true);
		}
	}
		
	if (autoSkip)
	{
		if (mode == mg_button_normal)
		{
			WinNormalMG(anim, num_anim);

			return;
		}
		else
		if (mode == mg_button_cycled)
		{
			WinCycledMG(anim, num_anim);

			return;
		}
	}

	switch (mode)
	{
		case mg_button_normal:		
		{			
			if (!right_pressed)
			{
				if (button_times>1)
				{
					if (chr->Controls().GetControlStateType(buttonsStatus[iMG_Button].ButtonName) == CST_ACTIVATED)
					{				
						num_pressed++;
	
						if (num_pressed>button_times)
						{
							right_pressed = true;
						}
					}
				}
				else
				{
					for (dword i=0;i<buttonsStatus.Size();i++)	
					{
						if (chr->Controls().GetControlStateFloat(buttonsStatus[i].ButtonName)>0.7f)
						{
							buttonsStatus[i].bBeenPressed = true;
						}
					}					
			
					if (IsButtonPressed())
					{					
						right_pressed = true;
					}
				}

				if (right_pressed && !wait_end_mg)
				{
					ShowWidget(false);
				}
			}

			// Vano: изменил этот кусок, чтобы срабатывал lose trigger
			if (string::IsEqual(played_node, end_node))
			{
				if (right_pressed)
					WinNormalMG(anim, num_anim);
				/*else
				{
					if (lose_triger)
					{
						lose_triggered = true;
						chr->LogicDebug("Lose Triger in '%s' ButtonMG Activated:", id);
						lose_triger->Activate(true);
					}
					mode = mg_disabled;	
				}*/
			}
			/*if (right_pressed && string::IsEqual(played_node, end_node))
			{				
				WinNormalMG(anim, num_anim);
			}*/			
		}
		break;
		case mg_button_cycled:
		{
			button_time -= fDeltaTime;

			if (button_time<0)
			{
				button_time=button_duration;

				num_pressed--;

				if (num_pressed<0)
				{					
					if (lose_triger)
					{
						lose_triggered = true;
						chr->LogicDebug("Lose Triger in '%s' ButtonMG Activated:", id);
						lose_triger->Activate(true);
					}

					ShowWidget(false);

					mode = mg_disabled;

					ActivateLink("lose",anim, num_anim);
				}
			}

			if (chr->Controls().GetControlStateType(buttonsStatus[iMG_Button].ButtonName) == CST_ACTIVATED)
			{				
				num_pressed++;

				if (num_pressed>button_times)
				{
					WinCycledMG(anim, num_anim);
				}
			}
		}
		break;		
	}	


	/*api->Trace("after:");

	for (int i=0;i<num_anim;i++)
	{
		if (anim[i]) api->Trace(anim[i]->CurrentNode());
	}

	api->Trace(" ");*/
}

void CharacterButtonMiniGame::ChoseLink(IAnimation** anim, int num_anim)
{
	if (mode == mg_button_normal)
	{
		if (right_pressed || autoSkip)
		{	
			ActivateLink("Proceed",anim, num_anim);
		}
	}	

	if (!wait_end_mg) ShowWidget(false);	
}

void CharacterButtonMiniGame::Finish()
{
	if (!wait_end_mg) ShowWidget(false);	

	//if (wait_end_mg)
	//ActivateTriger();
	if (lose_triger)
	{
		lose_triggered = true;
		chr->LogicDebug("Lose Triger in '%s' ButtonMG Activated:", id);
		lose_triger->Activate(true);
	}
	
	mode = mg_disabled;
}

void CharacterButtonMiniGame::ActivateTriger()
{
	if (mode == mg_button_normal)
	{
		if (right_pressed)
		{	
			if (win_triger)
			{
				chr->LogicDebug("Win Triger in '%s' ButtonMG Activated:", id);
				win_triger->Activate(true);
			}
			
			return;
		}
	}
	else
	if (mode == mg_button_cycled)
	{
		if (num_pressed>button_times)
		{
			chr->LogicDebug("Win Triger in '%s' ButtonMG Activated:", id);
			if (win_triger) win_triger->Activate(true);

			return;
		}
	}
}

void CharacterButtonMiniGame::ActivateLink(const char* link,IAnimation** anim, int num_anim)
{	
	for (int i=0;i<num_anim;i++)
	{
		if (anim[i])
		{
			//api->Trace("proceed %s",anim[i]->CurrentNode());
			anim[i]->ActivateLink(link,true);
		}
	}

}

bool CharacterButtonMiniGame::FilterDifficulty(const char* difficulty)
{
	if (!difficulty) return true;
	if (string::IsEmpty(difficulty)) return true;

	char diff[32];
	
	diff[0] = 0;
	int index=0;

	bool doCmp = false;

	for (int i=0;i<(int)strlen(difficulty);i++)
	{		
		doCmp = false;

		if ((difficulty[i] >= 65 && difficulty[i] <= 90) ||
			(difficulty[i] >= 97 && difficulty[i] <= 122))
		{
			diff[index] = difficulty[i];

			index++;
			diff[index] = 0;

			 if (i==strlen(difficulty)-1)
			 {
				 doCmp = true;
			 }			
		}
		else
		{
			if (diff[0]!=0)
			{
				doCmp = true;
			}
		}

		if (doCmp)
		{
			if (string::IsEqual(chr->arbiter->GetDifficulty().c_str(),diff))			
			{
				return true;
			}

			diff[0] = 0;
			index=0;
		}		
	}

	return false;
}

void CharacterButtonMiniGame::WinNormalMG(IAnimation** anim, int num_anim)
{
	ChoseLink(anim, num_anim);

	if (string::IsEqual(played_node, end_node))
	{
		ActivateTriger();
		mode = mg_disabled;	
	}	
}

void CharacterButtonMiniGame::WinCycledMG(IAnimation** anim, int num_anim)
{
	num_pressed = 0;					

	ActivateTriger();
	ShowWidget(false);

	mode = mg_disabled;

	ActivateLink("win",anim, num_anim);	
}

bool CharacterButtonMiniGame::NeedToRememeber(MiniGame_Desc& desc)
{
	for (int i=0;i<(int)desc.memory_params.Size();i++)
	{
		if (string::IsEqual(chr->arbiter->GetDifficulty().c_str(),desc.memory_params[i].difficulty))
		{
			return desc.memory_params[i].remember_button;
		}
	}

	return true;
}