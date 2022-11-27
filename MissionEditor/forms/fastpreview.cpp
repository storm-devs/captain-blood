
#include "fastpreview.h"
#include "mainwindow.h"
#include "..\Attributes\BaseAttr.h"
#include "..\attributes\attributes.h"



#define WINDOW_POSX   10
#define WINDOW_POSY   10
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 480

extern IRender * pRS;
extern TMainWindow* MainWindow;

extern char* IntToStr (int val);
extern char* FloatToStr (float val);

TFastPreviewWindow::TFastPreviewWindow (MissionEditor::tCreatedMO* pObject) : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
  int newHeight = pRS->GetScreenInfo3D().dwHeight;
  newHeight -= 20;
  SetHeight(newHeight);
  dword dwHeight = newHeight;


  bAlwaysOnTop = true;
  bSystemButton = false;

  
  Caption = "Object : ";
  Caption += pObject->pObject.SPtr()->GetObjectID().c_str();


  btnOK = NEW GUIButton (this, 4, dwHeight-60, WINDOW_WIDTH-9, 24);
  btnOK->Glyph->Load ("ok");
  btnOK->Caption = "Close window";
  btnOK->pFont->SetSize (12);
  btnOK->OnMousePressed = (CONTROL_EVENT)&TFastPreviewWindow::onOKPressed;
  btnOK->FlatButton = true;


  pAttrTable = NEW GUIListBox(this, 4, 2, WINDOW_WIDTH-9, dwHeight-70);

  //pAttrTable->Items

  int AttrCount = pObject->AttrList->GetCount();

  for (int n = 0 ; n < AttrCount; n++)
  {
    BaseAttribute* pBase = pObject->AttrList->Get(n);

    AddBaseAttr(pBase, 0);

  }
  
}


TFastPreviewWindow::~TFastPreviewWindow ()
{
}


void _cdecl TFastPreviewWindow::onOKPressed (GUIControl* sender)
{
  Close (this);
}


void TFastPreviewWindow::AddBaseAttr (BaseAttribute* pBase, int Level)
{
  string v = "";

  for (int lvl = 0; lvl < Level; lvl++)
  {
    v+= "  ";
  }



  v += "#b";
  v += pBase->GetName();
  v += "@b : ";

  bool bNeedAddLine = true;

  IMOParams::Type AttrType = pBase->GetType();

  switch (AttrType)
  {
    //--------------------------------------
  case IMOParams::t_bool:
    {
      BoolAttribute* pBoolAttr = (BoolAttribute*)pBase;
      if (pBoolAttr->GetValue())
      {
        v += "true";
      } else
      {
        v += "false";
      }
      break;
    }

    //--------------------------------------
  case IMOParams::t_long:
    {
      LongAttribute* pLongAttr = (LongAttribute*)pBase;
      v += string (IntToStr(pLongAttr->GetValue()));
      break;
    }

    //--------------------------------------
  case IMOParams::t_float:
    {
      FloatAttribute* pFloatAttr = (FloatAttribute*)pBase;
      v += string (FloatToStr(pFloatAttr->GetValue()));
      break;
    }


    //--------------------------------------
  case IMOParams::t_string:
    {
      StringAttribute* pStrAttr = (StringAttribute*)pBase;
      v+= "\"";
      v += pStrAttr->GetValue();
      v+= "\"";
      break;
    }

    //--------------------------------------
  case IMOParams::t_locstring:
    {
      LocStringAttribute* pStrAttr = (LocStringAttribute*)pBase;
      v+= "\"";
      v += pStrAttr->GetValue();
      v+= "\"";
      break;
    }
    
    

    //--------------------------------------
  case IMOParams::t_position:
    {
      PositionAttribute* pPosAttr = (PositionAttribute*)pBase;
      v += string (FloatToStr(pPosAttr->GetValue().x));
      v += ", ";
      v += string (FloatToStr(pPosAttr->GetValue().y));
      v += ", ";
      v += string (FloatToStr(pPosAttr->GetValue().z));
      break;
    }

    //--------------------------------------
  case IMOParams::t_angles:
    {
      RotationAttribute* pPosAttr = (RotationAttribute*)pBase;
      v += string (FloatToStr(pPosAttr->GetValue().x));
      v += ", ";
      v += string (FloatToStr(pPosAttr->GetValue().y));
      v += ", ";
      v += string (FloatToStr(pPosAttr->GetValue().z));
      break;
    }


    //--------------------------------------
  case IMOParams::t_color:
    {
      ColorAttribute* pClrAttr = (ColorAttribute*)pBase;

      v+= "RGBA (";
      v += string (FloatToStr(pClrAttr->GetValue().r));
      v += ", ";
      v += string (FloatToStr(pClrAttr->GetValue().g));
      v += ", ";
      v += string (FloatToStr(pClrAttr->GetValue().b));
      v += ", ";
      v += string (FloatToStr(pClrAttr->GetValue().a));
      v+= ")";
      break;

    }


    //--------------------------------------
  case IMOParams::t_enum:
    {
      EnumAttribute* pEnumAttr = (EnumAttribute*)pBase;
      v += "\"";
      v += pEnumAttr->GetStringValue();
      v += "\"";

      break;
    }


  case IMOParams::t_array:
    {
      ArrayAttribute* pArrayAttr = (ArrayAttribute*)pBase;

      int ValCount = pArrayAttr->GetValuesCount();

      v += string (IntToStr(ValCount));
      v += " items";
      pAttrTable->Items.Add(v);
      bNeedAddLine = false;

      for (int j = 0; j < ValCount; j++)
      {
        ArrayAttribute::ArrayElement* pElement = pArrayAttr->GetValue(j);
        for (dword q = 0; q < pElement->elements.Size(); q++)
        {
          AddBaseAttr(pElement->elements[q], Level+1);
          //
        }
      }

      break;
    }

  case IMOParams::t_group:
    {
      GroupAttribute* pGroupAttr = (GroupAttribute*)pBase;

      pAttrTable->Items.Add(v);
      bNeedAddLine = false;

      for (DWORD i = 0 ; i < pGroupAttr->Childs.Size(); i++)
      {
        AddBaseAttr(pGroupAttr->Childs[i], Level+1);
      }
      break;
    }


  default:
    {
      v+= "unknown";
    }
  }

  if (bNeedAddLine) pAttrTable->Items.Add(v);

}



void TFastPreviewWindow::Draw ()
{
  GUIWindow::Draw();

  if (GetAsyncKeyState(VK_ESCAPE) < 0)
  {
    Close(this);
  }

}