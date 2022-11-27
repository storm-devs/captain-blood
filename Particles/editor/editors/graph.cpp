#ifndef _XBOX

#include "graph.h"
#include "..\..\system\datasource\datagraph.h"

extern GUIGraphLine*	CopyGraphLineMax;
extern GUIGraphLine*	CopyGraphLineMin;
extern bool CopyBooleanRelative;


GraphEditor::GraphEditor ()
{
	bRelative = false;
	GraphLineMax = NEW GUIGraphLine (Vector (0.0f, 1.0f, 0.0f), Vector(99999.0f, 1.0f, 0.0f));
	GraphLineMin = NEW GUIGraphLine (Vector (0.0f), Vector(99999.0f, 0.0f, 0.0f));
	GraphLineMax->color = 0xFFFF0000;
	GraphLineMin->color = 0xFF0000FF;

	pGraphEditor = NULL;

	btnMove = NULL; 
	btnScale = NULL; 
	btnScaleX = NULL; 
	btnScaleY = NULL; 
	btnMoveLineY = NULL; 
	btnScaleLineY = NULL; 
	btnScaleLineX = NULL; 
	btnMoveLineX = NULL; 
	btnPaste = NULL;
	btnCopy = NULL;

	btnInsertParabols = NULL;
	btnCopyMaxToMin = NULL; 
	btnCopyMinToMax = NULL; 
	btnFindZeroAxis = NULL; 
	PercentGraph = NULL;

}

GraphEditor::~GraphEditor ()
{
	delete GraphLineMax;
	delete GraphLineMin;
}

void GraphEditor::Release ()
{
 delete this;
}

void GraphEditor::BeginEdit (GUIControl* form, DataGraph* EditedGraph)
{
	this->EditedGraph = EditedGraph;

	pGraphEditor = NEW GUIGraphEditor (form, 0, 30, form->GetDrawRect().w, form->GetDrawRect().h-30);


	int btnsHeight = 4;
	btnMove = NEW GUIButton(form, 4, btnsHeight, 24, 24);
	btnMove->Glyph->Load ("peditor\\move");
	btnMove->FontColor = 0xFF000000;
	btnMove->pFont->SetSize (16);
	btnMove->Down = true;
	btnMove->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnMovePressed;
	btnMove->Hint = "Move point";
	btnMove->GroupIndex = 1;
	btnMove->FlatButton = true;
	

	btnScale = NEW GUIButton(form, 4+27, btnsHeight, 24, 24);
	btnScale->Glyph->Load ("peditor\\scale");
	btnScale->FontColor = 0xFF000000;
	btnScale->pFont->SetSize (16);
	btnScale->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnScalePressed;
	btnScale->Hint = "Scale grid";
	btnScale->GroupIndex = 1;
	btnScale->FlatButton = true;



	
	btnScaleX = NEW GUIButton(form, 4+(27*2), btnsHeight, 24, 24);
	btnScaleX->Glyph->Load ("peditor\\scalex");
	btnScaleX->FontColor = 0xFF000000;
	btnScaleX->pFont->SetSize (16);
	btnScaleX->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnScaleXPressed;
	btnScaleX->Hint = "Scale grid, X-axis";
	btnScaleX->GroupIndex = 1;
	btnScaleX->FlatButton = true;



	btnScaleY = NEW GUIButton(form, 4+(27*3), btnsHeight, 24, 24);
	btnScaleY->Glyph->Load ("peditor\\scaley");
	btnScaleY->FontColor = 0xFF000000;
	btnScaleY->pFont->SetSize (16);
	btnScaleY->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnScaleYPressed;
	btnScaleY->Hint = "Scale grid, Y-axis";
	btnScaleY->GroupIndex = 1;
	btnScaleY->FlatButton = true;


	btnMoveLineY = NEW GUIButton(form, 4+(27*4), btnsHeight, 24, 24);
	btnMoveLineY->Glyph->Load ("peditor\\movey");
	btnMoveLineY->FontColor = 0xFF000000;
	btnMoveLineY->pFont->SetSize (16);
	btnMoveLineY->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnMoveYPressed;
	btnMoveLineY->Hint = "Move Line, Y-Axis";
	btnMoveLineY->GroupIndex = 1;
	btnMoveLineY->FlatButton = true;



	
	btnScaleLineY = NEW GUIButton(form, 4+(27*5), btnsHeight, 24, 24);
	btnScaleLineY->Glyph->Load ("peditor\\scalegy");
	btnScaleLineY->FontColor = 0xFF000000;
	btnScaleLineY->pFont->SetSize (16);
	btnScaleLineY->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnScaleLineYPressed;
	btnScaleLineY->Hint = "Scale Line, Y-Axis";
	btnScaleLineY->GroupIndex = 1;
	btnScaleLineY->FlatButton = true;


	btnScaleLineX = NEW GUIButton(form, 4+(27*6), btnsHeight, 24, 24);
	btnScaleLineX->Glyph->Load ("peditor\\scalegx");
	btnScaleLineX->FontColor = 0xFF000000;
	btnScaleLineX->pFont->SetSize (16);
	btnScaleLineX->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnScaleLineXPressed;
	btnScaleLineX->Hint = "Scale Line, X-Axis";
	btnScaleLineX->GroupIndex = 1;
	btnScaleLineX->FlatButton = true;

	 
	 
	btnMoveLineX = NEW GUIButton(form, 4+(27*7), btnsHeight, 24, 24);
	btnMoveLineX->Glyph->Load ("peditor\\movex");
	btnMoveLineX->FontColor = 0xFF000000;
	btnMoveLineX->pFont->SetSize (16);
	btnMoveLineX->OnMouseClick = (CONTROL_EVENT)&GraphEditor::btnMoveXPressed;
	btnMoveLineX->Hint = "Move Line, X-Axis";
	btnMoveLineX->GroupIndex = 1;
	btnMoveLineX->FlatButton = true;


	btnCopy = NEW GUIButton(form, 4+(27*8), btnsHeight, 24, 24);
	btnCopy->Glyph->Load ("peditor\\copy");
	btnCopy->FontColor = 0xFF000000;
	btnCopy->pFont->SetSize (16);
	btnCopy->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnCopyPressed;
	btnCopy->Hint = "Copy all graph lines";
	btnCopy->FlatButton = true;



	btnPaste = NEW GUIButton(form, 4+(27*9), btnsHeight, 24, 24);
	btnPaste->Glyph->Load ("peditor\\paste");
	btnPaste->FontColor = 0xFF000000;
	btnPaste->pFont->SetSize (16);
	btnPaste->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnPastePressed;
	btnPaste->Hint = "Paste all graph lines";
	btnPaste->FlatButton = true;


	btnCopyMaxToMin = NEW GUIButton(form, 4+(27*10), btnsHeight, 24, 24);
	btnCopyMaxToMin->Glyph->Load ("peditor\\minmax");
	btnCopyMaxToMin->Hint = "Copy Blue line to Red line";
	btnCopyMaxToMin->FontColor = 0xFF000000;
	btnCopyMaxToMin->pFont->SetSize (16);
	btnCopyMaxToMin->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnMax2Min;
	btnCopyMaxToMin->FlatButton = true;


	btnCopyMinToMax = NEW GUIButton(form, 4+(27*11), btnsHeight, 24, 24);
	btnCopyMinToMax->Glyph->Load ("peditor\\maxmin");
	btnCopyMinToMax->Hint = "Copy Red line to Blue line";
	btnCopyMinToMax->FontColor = 0xFF000000;
	btnCopyMinToMax->pFont->SetSize (16);
	btnCopyMinToMax->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnMin2Max;
	btnCopyMinToMax->FlatButton = true;


	btnFindZeroAxis = NEW GUIButton(form, 4+(27*12), btnsHeight, 24, 24);
	btnFindZeroAxis->Glyph->Load ("peditor\\zero");
	btnFindZeroAxis->FontColor = 0xFF000000;
	btnFindZeroAxis->pFont->SetSize (16);
	btnFindZeroAxis->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnFindZero;
	btnFindZeroAxis->Hint = "Find zero axis";
  btnFindZeroAxis->FlatButton = true;


	btnInsertParabols = NEW GUIButton(form, 4+(27*13), btnsHeight, 24, 24);
	btnInsertParabols->Glyph->Load ("peditor\\parabols");
	btnInsertParabols->FontColor = 0xFF000000;
	btnInsertParabols->pFont->SetSize (16);
	btnInsertParabols->OnMousePressed = (CONTROL_EVENT)&GraphEditor::btnInsertParabolsPressed;
	btnInsertParabols->Hint = "Insert parabol's";
  btnInsertParabols->FlatButton = true;


	PercentGraph = NEW GUICheckBox (form, 4+(27*15), btnsHeight, 56, 24);
	PercentGraph->ImageChecked->Load ("checked");
	PercentGraph->ImageNormal->Load ("normal");
	PercentGraph->Caption = "Relative graph";
	PercentGraph->OnChange = (CONTROL_EVENT)&GraphEditor::RelativeChanged;
	PercentGraph->pFont->SetName("arialcyrsmall");
	

	CreateDataForEdit ();
	
	

}

void GraphEditor::CreateDataForEdit ()
{
	DWORD n = 0;

	pGraphEditor->NegativeValues = EditedGraph->GetNegative();

	if (EditedGraph->GetRelative())
	{
		pGraphEditor->FitViewWidth (0, 100);
		pGraphEditor->HorizontalAxeDesc = "Life %";
		PercentGraph->Checked = true;
		bRelative = true;
	} else
	{
		pGraphEditor->FitViewWidth (0, EditedGraph->GetMaxTime ());
		pGraphEditor->HorizontalAxeDesc = "time";
		PercentGraph->Checked = false;
		bRelative = false;
	}
	
	DWORD MaxCount = EditedGraph->GetMaxCount();
	GraphLineMax->SetSize(MaxCount);
	for (n = 0; n < MaxCount; n++)
	{
		const GraphVertex& gVrx = EditedGraph->GetMaxVertex(n);
		GraphLineMax->Change(n, Vector (gVrx.Time, gVrx.Val, 0.0f));
	}

	DWORD MinCount = EditedGraph->GetMinCount();
	GraphLineMin->SetSize(MinCount);
	for (n = 0; n < MinCount; n++)
	{
		const GraphVertex& gVrx = EditedGraph->GetMinVertex(n);
		GraphLineMin->Change(n, Vector (gVrx.Time, gVrx.Val, 0.0f));
	}

	pGraphEditor->Lines->Add(GraphLineMax);
	pGraphEditor->Lines->Add(GraphLineMin);


	pGraphEditor->FitViewHeight(pGraphEditor->GetMinValueY(), pGraphEditor->GetMaxValueY());
}



void GraphEditor::EndEdit ()
{
	GraphLineMax->SelectedPoint = -1;
	GraphLineMin->SelectedPoint = -1;
	GraphLineMax->bActive = false;
	GraphLineMin->bActive = false;

	delete pGraphEditor;

	delete btnMove; 
	delete btnScale; 
	delete btnScaleX; 
	delete btnScaleY; 
	delete btnMoveLineY; 
	delete btnScaleLineY; 
	delete btnScaleLineX; 
	delete btnMoveLineX; 
	delete btnPaste;
	delete btnCopy;

	delete btnInsertParabols;
	delete btnCopyMaxToMin; 
	delete btnCopyMinToMax; 
	delete btnFindZeroAxis; 
	delete PercentGraph;


	pGraphEditor = NULL;

	btnMove = NULL; 
	btnScale = NULL; 
	btnScaleX = NULL; 
	btnScaleY = NULL; 
	btnMoveLineY = NULL; 
	btnScaleLineY = NULL; 
	btnScaleLineX = NULL; 
	btnMoveLineX = NULL; 
	btnPaste = NULL;
	btnCopy = NULL;

	btnInsertParabols = NULL;
	btnCopyMaxToMin = NULL; 
	btnCopyMinToMax = NULL; 
	btnFindZeroAxis = NULL; 
	PercentGraph = NULL;


}


void _cdecl GraphEditor::btnMovePressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_EDIT;
}

void _cdecl GraphEditor::btnScalePressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_SCALE;
}

void _cdecl GraphEditor::btnScaleXPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_SCALEX;
}

void _cdecl GraphEditor::btnScaleYPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_SCALEY;
}

void _cdecl GraphEditor::btnMoveYPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_MOVELINEY;
}

void _cdecl GraphEditor::btnScaleLineXPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_SCALELINEX;
}

void _cdecl GraphEditor::btnScaleLineYPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_SCALELINEY;
}

void _cdecl GraphEditor::btnMoveXPressed (GUIControl* sender)
{
	pGraphEditor->Mode = GEM_MOVELINEX;
}


void _cdecl GraphEditor::btnMin2Max (GUIControl* sender)
{
	GraphLineMin->Copy(GraphLineMax);
}

void _cdecl GraphEditor::btnMax2Min (GUIControl* sender)
{
	GraphLineMax->Copy(GraphLineMin);
}


void _cdecl GraphEditor::btnFindZero (GUIControl* sender)
{
	pGraphEditor->OffsetY = -10;
	pGraphEditor->OffsetX = 10;
}

void _cdecl GraphEditor::btnInsertParabolsPressed (GUIControl* sender)
{
	GUIGraphLine* GraphLineP = NEW GUIGraphLine (Vector (0.0f, 1.0f, 0.0f), Vector(99999.0f, 1.0f, 0.0f));
	float speed = 1.0f;
	float starty = GraphLineP->GetPoint(0).y;
	float maxy = starty + (10.0f * 10.0f);
	float y = 0;

	for (int n = 0; n < 10; n++)
	{
		y = maxy-(starty+((float)(9.0f-n) * (float)(9.0f-n)));
		GraphLineP->Insert(n+1, Vector((float)n+1.0f, y, 0.0f));
	}

	for ( n = 0; n < 10; n++)
	{
		y = maxy-(starty+((float)n * (float)n));
		GraphLineP->Insert(n+11, Vector((float)n+11.0f, y, 0.0f));
	}

	GraphLineMax->Copy(GraphLineP);
	delete GraphLineP;
}

void _cdecl GraphEditor::RelativeChanged (GUIControl* sender)
{

	if (PercentGraph->Checked)
	{
		bRelative = true;
		pGraphEditor->FitViewWidth (0, 100);
		pGraphEditor->HorizontalAxeDesc = "Life %";
		return;
	}

	bRelative = false;
	pGraphEditor->FitViewWidth (0, EditedGraph->GetMaxTime ());
	pGraphEditor->HorizontalAxeDesc = "time";

}


void GraphEditor::Apply ()
{
	EditedGraph->SetRelative(bRelative);

	DWORD n = 0;
	DWORD MaxCount = GraphLineMax->GetCount();
	DWORD MinCount = GraphLineMin->GetCount();

	GraphVertex* pMaxArray = NEW GraphVertex[MaxCount];
	GraphVertex* pMinArray = NEW GraphVertex[MinCount];

	for (n = 0; n < MaxCount; n++)
	{
		Vector ptMax = GraphLineMax->GetPoint(n);
		pMaxArray[n].Time = ptMax.x;
		pMaxArray[n].Val = ptMax.y;
	}

	for (n = 0; n < MinCount; n++)
	{
		Vector ptMin = GraphLineMin->GetPoint(n);
		pMinArray[n].Time = ptMin.x;
		pMinArray[n].Val = ptMin.y;
	}

	EditedGraph->SetValues(pMinArray, MinCount, pMaxArray, MaxCount);

	delete pMaxArray;
	delete pMinArray;
}

void _cdecl GraphEditor::btnCopyPressed (GUIControl* sender)
{
	CopyGraphLineMax->Copy(GraphLineMax);
	CopyGraphLineMin->Copy(GraphLineMin);
	CopyBooleanRelative = bRelative;
}

void _cdecl GraphEditor::btnPastePressed (GUIControl* sender)
{
	GraphLineMax->Copy(CopyGraphLineMax);
	GraphLineMin->Copy(CopyGraphLineMin);
	bRelative = CopyBooleanRelative;
}


#endif