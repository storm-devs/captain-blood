#include "JoystickManager.h"

#include "..\..\common_h\controls.h"

JoystickManager *joystickManager = null;

JoystickManager::JoystickManager() :
	m_devices (_FL_),
	m_Controls(_FL_),
//	m_primaryIndex(-1),
//	m_secondaryIndex(-1),
	m_isPaused(false)
{
	InitControls();

	m_index_1 = -1;
	m_index_2 = -1;
	m_index_3 = -1;
	m_index_4 = -1;

	joystickManager = this;
}

JoystickManager::~JoystickManager()
{
	for( int i = 0 ; i < m_devices; i++ )
	{
		DELETE(m_devices[i])
	}

	m_devices.DelAll();
}

void JoystickManager::AddJoystick(IJoystick *joy)
{
	Assert(joy);
	m_devices.Add(joy);
}

bool JoystickManager::SetJoystickValues(IJoystick::ValueType type, const void *values, long nBytes, long joystickIndex)
{
	dword index = -1;

	switch( joystickIndex )
	{
		case 0:
		//	index = m_primaryIndex;
			index = m_index_1;
			break;

		case 1:
		//	index = m_secondaryIndex;
			index = m_index_2;
			break;

		case 2:
			index = m_index_3;
			break;
		case 3:
			index = m_index_4;
			break;

		default:
			return false;
	}

	if( index >= m_devices.Size())
		return false;

	return m_devices[index]->SetValues(type,values,nBytes);
}

void JoystickManager::InitControls()
{
	m_Controls.DelAll();

	Control ctrl;

	ctrl.realDeviceIndex  = INVALID_CODE;
	ctrl.realControlIndex = INVALID_CODE;
	
	ctrl.name = "gp_X1"; m_Controls.Add(ctrl);
	ctrl.name = "gp_Y1"; m_Controls.Add(ctrl);
	ctrl.name = "gp_X2"; m_Controls.Add(ctrl);
	ctrl.name = "gp_Y2"; m_Controls.Add(ctrl);
	ctrl.name = "gp_X3"; m_Controls.Add(ctrl);
	ctrl.name = "gp_Y3"; m_Controls.Add(ctrl);
	ctrl.name = "gp_X4"; m_Controls.Add(ctrl);
	ctrl.name = "gp_Y4"; m_Controls.Add(ctrl);

	ctrl.name = "gp_B1"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B2"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B3"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B4"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B5"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B6"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B7"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B8"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B9"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B10"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B11"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B12"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B13"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B14"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B15"; m_Controls.Add(ctrl);
	ctrl.name = "gp_B16"; m_Controls.Add(ctrl);

	ctrl.name = "gp_PovH"; m_Controls.Add(ctrl);
	ctrl.name = "gp_PovV"; m_Controls.Add(ctrl);

	for( dword i = 1; i < MaxJoysticks; i++ )
	{
		ctrl.name.Format("gp%d_X1", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_Y1", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_X2", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_Y2", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_X3", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_Y3", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_X4", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_Y4", i+1); m_Controls.Add(ctrl);
	
		ctrl.name.Format("gp%d_B1", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B2", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B3", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B4", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B5", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B6", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B7", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B8", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B9", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B10", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B11", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B12", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B13", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B14", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B15", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_B16", i+1); m_Controls.Add(ctrl);
		
		ctrl.name.Format("gp%d_PovH", i+1); m_Controls.Add(ctrl);
		ctrl.name.Format("gp%d_PovV", i+1); m_Controls.Add(ctrl);
	}
}

string GetNameNoNumber(const string &name)
{
	long nameStart = name.FindSubStr("_");
	++nameStart;

	string str = "gp_";
	str += name.GetBuffer() + nameStart;
	return str;
}

void JoystickManager::AssignIndex(ControllerType type, long deviceIndex)
{
//	Assert((dword)deviceIndex < m_devices.Size());

	const dword ControlsPerDevice = 26; // 8 осей + 16 кнопок + PovV + PovH

	for( dword i = 0 ; i < ControlsPerDevice ; i++ )
	{
		m_Controls[type*ControlsPerDevice + i].realDeviceIndex = deviceIndex;

		if( deviceIndex != INVALID_CODE )
		{
			m_Controls[type*ControlsPerDevice + i].realControlIndex =
				m_devices[deviceIndex]->GetIndex(
					GetNameNoNumber(m_Controls[type*ControlsPerDevice + i].name).c_str());
		}
	}
}

void JoystickManager::Update(float deltaTime)
{
	for( int i = 0; i < m_devices ; i++ )
	{
		m_devices[i]->Update(deltaTime);

		if( m_index_1 < 0 )
		{
#ifdef _XBOX
			if( m_devices[i]->IsStartPressed())
			{
				m_index_1 = i;

				switch( i )
				{
					case 0:
						m_index_2 = 1;
						m_index_3 = 2;
						m_index_4 = 3; break;

					case 1:
						m_index_2 = 0;
						m_index_3 = 2;
						m_index_4 = 3; break;

					case 2:
						m_index_2 = 0;
						m_index_3 = 1;
						m_index_4 = 3; break;

					case 3:
						m_index_2 = 0;
						m_index_3 = 1;
						m_index_4 = 2; break;
				}

				AssignIndex(Primary	 ,m_index_1);
				AssignIndex(Secondary,m_index_2);
				AssignIndex(Third	 ,m_index_3);
				AssignIndex(Fourth	 ,m_index_4);

				m_isPaused = false;
			}
#else
			m_index_1 = 0;
			m_index_2 = 1;
			m_index_3 = 2;
			m_index_4 = 3;

			AssignIndex(Primary	 ,m_index_1);
			AssignIndex(Secondary,m_index_2);
			AssignIndex(Third	 ,m_index_3);
			AssignIndex(Fourth	 ,m_index_4);

			m_isPaused = false;
#endif
		}

		/////////////////////////////////////////////////////////////

		if( m_index_1 >= 0 && m_devices[m_index_1]->IsDisconnected())
		{
		//	m_index_1 = -1;
			m_isPaused = true;
		}

	/*	if( m_index_2 >= 0 && m_devices[m_index_2]->IsDisconnected())
		{
			m_index_2 = -1;
			m_isPaused = true;
		}

		if( m_index_3 >= 0 && m_devices[m_index_3]->IsDisconnected())
		{
			m_index_3 = -1;
			m_isPaused = true;
		}

		if( m_index_4 >= 0 && m_devices[m_index_4]->IsDisconnected())
		{
			m_index_4 = -1;
			m_isPaused = true;
		}*/
	}
}

long JoystickManager::GetIndex(const char *deviceControl)
{
	for( int i = 0; i < m_Controls ; i++ )
	{
		if( m_Controls[i].name == deviceControl )
			return i;
	}

	return INVALID_CODE;
}

float JoystickManager::GetRawValue(long controlIndex) const
{
	if ((dword)controlIndex >= m_Controls.Size())
		return 0.0f;

#ifdef _XBOX
	if( m_Controls[controlIndex].realDeviceIndex == INVALID_CODE )
		return 0.0f;
		
	return m_devices[m_Controls[controlIndex].realDeviceIndex]->GetRawValue(
					 m_Controls[controlIndex].realControlIndex);
#else
	// ищем первый джойстик у которого нажата данная кнопка (added by Nikita)
	const dword ControlsPerDevice = 26; // 8 осей + 16 кнопок + PovV + PovH
	long nControlNum = controlIndex % ControlsPerDevice;
	float fMaxVal = 0.f;
	for( long n=0; n<m_devices && n<4; n++ )
	{
		long nCurControlIndex = n * ControlsPerDevice + nControlNum;
		float fVal = 0.f;
		if( nCurControlIndex < m_Controls )
		{
			long nDevice = m_Controls[nCurControlIndex].realDeviceIndex;
			long nControl = m_Controls[nCurControlIndex].realControlIndex;
			if( nDevice != INVALID_CODE && nControl != INVALID_CODE && m_devices[nDevice] != NULL )
				fVal = m_devices[nDevice]->GetRawValue( nControl );
			if( fabs(fVal) > fabs(fMaxVal) )
				fMaxVal = fVal;
		}
	}
	return fMaxVal;
#endif
}

void JoystickManager::EndFrame()
{
	for( int i = 0; i < m_devices ; i++ )
	{
		m_devices[i]->EndFrame();
	}
}
