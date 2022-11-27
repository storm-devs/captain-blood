#ifndef QuickEvent_H
#define QuickEvent_H

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\common_h\AnimationStdEvents.h"
#include "..\..\..\Common_h\ITips.h"

class Character;

#define MG_QE GroupId('Q','E','v','t')

#define MOP_QE_POSITION		MOP_POSITION("Position", Vector(0.0f))
#define MOP_QE_ANGELS		MOP_ANGLES("Angels", Vector(0.0f))
#define MOP_QE_RADIUS		MOP_FLOAT("Radius", 3.0f)
#define MOP_QE_STARTNODE	MOP_STRING("Player Start Node", "")
#define MOP_QE_BTNAME       MOP_STRING("Button Name", "ChrA")	

#define MOP_QE_COL_LOCPOS	MOP_POSITION("Colider locPos", Vector(0.0f))
#define MOP_QE_COL_LOCANG	MOP_ANGLES("Colider locAng", Vector(0.0f))
#define MOP_QE_COL_WIDTH	MOP_FLOAT("Colider Width", 0.5f)
#define MOP_QE_COL_HEIGHT	MOP_FLOAT("Colider Height", 0.5f)
#define MOP_QE_COL_LENGHT	MOP_FLOAT("Colider Lenght", 0.5f)

#define MOP_QE_PRT_NAME	    MOP_STRING("Particle Name", "")
#define MOP_QE_PRT_LOCPOS   MOP_POSITION("Particle locPos", Vector(0.0f))
#define MOP_QE_PRT_LOCANG	MOP_ANGLES("Particle locAng", Vector(0.0f))
#define MOP_QE_PRT_RADIUS	MOP_FLOAT("Particle show radius", 5.0f)

#define MOP_QE_BUTTON_WGT	MOP_STRING("Button Tip", "")


#define MOP_QE_SHDCAST	    MOP_BOOLC("Shadow Cast", true, "")
#define MOP_QE_SHDRCV	    MOP_BOOLC("Shadow Reseive", true, "")

#define MOP_QE_ACTIVATE	    MOP_BOOLC("Active", true, "Active QuickEvent in start mission time")
#define MOP_QE_VISIBLE	    MOP_BOOLC("Visible", true, "Show QuickEvent in start mission time")
#define MOP_QE_TR_ACCEPT    MOP_MISSIONTRIGGERG("Accept trigger", "Accept")	
#define MOP_QE_TR_ENTER     MOP_MISSIONTRIGGERG("Enter trigger", "Enter")
#define MOP_QE_TR_EXIT      MOP_MISSIONTRIGGERG("Exit trigger", "Exit")
#define MOP_QE_TR_INTERRUPT MOP_MISSIONTRIGGERG("Interrupt trigger", "Interrupt")
#define MOP_QE_TR_SUCCESS   MOP_MISSIONTRIGGERG("Success trigger", "Success")

#define MOP_QE_STD MOP_QE_POSITION MOP_QE_ANGELS MOP_QE_RADIUS MOP_QE_STARTNODE MOP_QE_BTNAME MOP_QE_COL_LOCPOS MOP_QE_COL_LOCANG MOP_QE_COL_WIDTH MOP_QE_COL_HEIGHT MOP_QE_COL_LENGHT MOP_QE_PRT_NAME MOP_QE_PRT_LOCPOS MOP_QE_PRT_LOCANG MOP_QE_PRT_RADIUS MOP_QE_BUTTON_WGT MOP_QE_SHDCAST MOP_QE_SHDRCV MOP_QE_ACTIVATE MOP_QE_VISIBLE MOP_QE_TR_ACCEPT MOP_QE_TR_ENTER MOP_QE_TR_EXIT MOP_QE_TR_INTERRUPT MOP_QE_TR_SUCCESS
//#define MOP_QE_STD MOP_QE_POSITION MOP_QE_ANGELS MOP_QE_RADIUS MOP_QE_STARTNODE MOP_QE_BTNAME MOP_QE_PRT_NAME MOP_QE_PRT_LOCPOS MOP_QE_PRT_LOCANG MOP_QE_PRT_RADIUS MOP_QE_BUTTON_WGT MOP_QE_SHDCAST MOP_QE_SHDRCV MOP_QE_ACTIVATE MOP_QE_VISIBLE MOP_QE_TR_ACCEPT MOP_QE_TR_ENTER MOP_QE_TR_EXIT MOP_QE_TR_INTERRUPT MOP_QE_TR_SUCCESS

class QuickEvent : public AcceptorObject
{
public:
	class AnimListener : public IAnimationListener
	{
	public:
		QuickEvent* pQuickEvent;

		AnimListener()
		{
			pQuickEvent = NULL;
		};

		void Init(QuickEvent* quickEvent)
		{
			pQuickEvent = quickEvent;
		};

		void _cdecl AnimEvent(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			Assert(pQuickEvent);

			if (numParams<1) return;

			pQuickEvent->AnimEvent(params[0]);			
		};
	};			

	AnimListener * quick_anim_events;
	AnimationStdEvents events;

	bool inZone;
	bool inParticleZone;
	bool bQuickEventEnabled;
	bool isActiveInitial;
	
	Vector position;
	float  radius;
	Vector player_point;
	
	ConstString startNode;
	char   buttonName[64];
	long buttonCode;
	
	long cheatButton;

	Vector init_pos;
	Vector init_angels;

	bool   wasCompleted;

	bool   pl_has_sec_wp;

	struct TColider
	{
		Vector cld_loc_pos;
		Vector cld_loc_angels;
		Vector cld_size;
		IPhysBox* colider;
		IPhysBox* colider2;
		Matrix colider_matrix;

		IMissionQTObject* aiColider;

		MissionObject* mo;

		TColider()
		{
			cld_loc_pos = 0.0f;
			cld_loc_angels = 0.0f;
			cld_size = 0.2f;
			colider = null;
			colider2 = null;
			mo = null;
			aiColider = null;
		};

		~TColider()
		{
			RELEASE(colider);
			RELEASE(colider2);
			RELEASE(aiColider);			
		}

		void Activate(bool isAct)
		{
			if (colider) colider->Activate(isAct);
			if (colider2) colider2->Activate(isAct);
			
			if (aiColider) aiColider->Activate(isAct);
		};

		void Init(IPhysicsScene* scene,MissionObject* _mo)
		{
			mo = _mo;

			colider = scene->CreateBox(_FL_, cld_size * 2.0f,colider_matrix,false);
			Assert(colider);				
			colider->SetGroup(phys_character);

			colider2 = scene->CreateBox(_FL_, cld_size * 2.0f,colider_matrix,false);
			Assert(colider2);				
			colider2->SetGroup(phys_world);

			if(!aiColider)
			{
				aiColider = mo->QTCreateObject(MG_AI_COLLISION, _FL_);
			}

			//aiColider.SetBox( -cld_size, cld_size );			
			aiColider->SetBoxCenter(0.0f);			
			aiColider->SetBoxSize(cld_size * 2.0f);	

			aiColider->SetMatrix(colider_matrix);
			
			if (aiColider) aiColider->Activate(true);
		};

		void DeleteColider()
		{
			RELEASE(colider);
			RELEASE(colider2);
		};

	};

	TColider colider;
	

	ConstString particle_name;
	IParticleSystem* particle;
	Vector particle_loc_pos;
	Vector particle_loc_angels;
	float particle_radius;
	Matrix particle_matrix;

	ConstString tip_name;
	ITip* button_tip;

	Matrix transform;

	Character* player;

	IGMXScene*  model;
	IAnimation* anim;

	bool shd_cast;
	bool shd_rcv;

	bool show_model;

	float  dist_to_move;
	Vector move_dir;

	bool  need_act;

	MissionTrigger triger_Accept;
	MissionTrigger triger_Enter;
	MissionTrigger triger_Exit;
	MissionTrigger triger_Interrupt;
	MissionTrigger triger_Success;

	QuickEvent();
	virtual ~QuickEvent();

	virtual void Activate(bool isActive);
	virtual void Show(bool isShow);

	virtual bool Create(MOPReader & reader);
	virtual void InitData();
	virtual bool ReadStdParams(MOPReader & reader);	
	virtual void Restart();
	
	virtual void Release();	

	virtual bool Accept(MissionObject * obj);
	virtual void BeginQuickEvent();
	virtual void QuickEventUpdate(float dltTime);	
	virtual void QuickEventCheck() {};
	virtual bool AllowInterruptByButton();
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);	
	virtual void HideHUD();

	virtual void _cdecl Draw(float dltTime, long level);
	virtual void _cdecl EditMode_Draw(float dltTime, long level);
	virtual void _cdecl Work(float dltTime, long level);

	virtual void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	virtual void _cdecl ShadowDraw(const char * group, MissionObject * sender);
	
	virtual void GetBox(Vector & min, Vector & max);	
	virtual Matrix & GetMatrix(Matrix & mtx);	

	virtual bool InteruptOnHit() { return false; };

	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader) = null;
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	// Util Functions
	inline virtual void  ActivateLink(const char* link) { if (anim) anim->ActivateLink(link,true); };
	inline virtual void  GotoNode(const char* link) { if (anim) anim->Goto(link,0.0f); };
	inline virtual void ShowObject(const ConstString & object_name,bool show);
	virtual void SetPlayerAnimEvents();
	virtual void RemovePlayerAnimEvents();

	virtual void ActivateColision(bool isAct);

	void InitHintParticle();
	void PlayHintParticle(bool start);

	bool MovePlayer(float dltTime);

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual const char* GetName() { return "QuckEvent";};

	MO_IS_FUNCTION(QuckEvent, MissionObject);
};

#endif