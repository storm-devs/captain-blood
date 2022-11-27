#include "GraphEditor.h"

#include "..\..\Common_h\IForce.h"

extern IForce *force;

void GraphEditor::Draw()
{
	GUIGraphEditor::Draw();

	float off = force ? force->GetPosition() : 0.0f;

	if( off < 0.0f )
		off = 0.0f;

	off = OffsetX + off/ScaleX*100.0f;

	if( off > 0.0f )
	{
		GUIRectangle &r = DrawRect;

		GUIHelper::DrawVertLine(r.y,r.h - 10,r.x + 30 + (int)off + 1,0xff000000);
	}

	GUIGraphLine *line = Lines->GetLine(0);

	if( line )
	{
		off = line->GetPoint(line->GetCount() - 1).x;

		if( off < 0.0f )
			off = 0.0f;

		off = OffsetX + off/ScaleX*100.0f;

		if( off > 0.0f )
		{
			GUIRectangle &r = DrawRect;

			GUIHelper::DrawVertLine(r.y,r.h - 10,r.x + 30 + (int)off + 0,0xff000000);
		}
	}
}

bool GraphEditor::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	switch( message )
	{
		case GUIMSG_WHEEL_UP:
			ScaleX *= 1.01f;
			break;

		case GUIMSG_WHEEL_DOWN:
			ScaleX /= 1.01f;
			break;
	}

	return GUIGraphEditor::ProcessMessages(message,lparam,hparam);
}
