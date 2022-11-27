#include "attrcreator.h"
//#include "..\gmx_pool.h"


objectsPool<BoolAttribute, 4096> boolPool/*("->Bool Attribute", __LINE__)*/;
objectsPool<LongAttribute, 4096> longPool/*("->Long Attribute", __LINE__)*/;
objectsPool<FloatAttribute, 4096> floatPool/*("->Float Attribute", __LINE__)*/;
objectsPool<StringAttribute, 4096> stringPool/*("->String Attribute", __LINE__)*/;
objectsPool<PositionAttribute, 4096> posPool/*("->Position Attribute", __LINE__)*/;
objectsPool<RotationAttribute, 4096> rotPool/*("->Rotation Attribute", __LINE__)*/;
objectsPool<ColorAttribute, 4096> colorPool/*("->Color Attribute", __LINE__)*/;
objectsPool<ArrayAttribute, 4096> arrayPool/*("->Array Attribute", __LINE__)*/;
objectsPool<EnumAttribute, 4096> enumPool/*("->Enum Attribute", __LINE__)*/;
objectsPool<GroupAttribute, 4096> groupPool/*("->Group Attribute", __LINE__)*/;
objectsPool<LocStringAttribute, 4096> lstrPool/*("->LocString Attribute", __LINE__)*/;


void FreeAllAttributesPools()
{
	groupPool.Destroy();
	arrayPool.Destroy();

	boolPool.Destroy();
	longPool.Destroy();
	floatPool.Destroy();
	stringPool.Destroy();
	posPool.Destroy();
	rotPool.Destroy();
	colorPool.Destroy();
	enumPool.Destroy();
	lstrPool.Destroy();
}


void DeleteAttribute (BaseAttribute* ptr)
{
	IMOParams::Type type = ptr->GetType();

	switch (type)
	{
	case IMOParams::t_bool:
		return boolPool.Free((BoolAttribute*)ptr);
	case IMOParams::t_long:
		return longPool.Free((LongAttribute*)ptr);
	case IMOParams::t_float:
		return floatPool.Free((FloatAttribute*)ptr);
	case IMOParams::t_string:
		return stringPool.Free((StringAttribute*)ptr);
	case IMOParams::t_position:
		return posPool.Free((PositionAttribute*)ptr);
	case IMOParams::t_angles:
		return rotPool.Free((RotationAttribute*)ptr);
	case IMOParams::t_color:
		return colorPool.Free((ColorAttribute*)ptr);
	case IMOParams::t_array:
		return arrayPool.Free((ArrayAttribute*)ptr);
	case IMOParams::t_enum:
		return enumPool.Free((EnumAttribute*)ptr);
	case IMOParams::t_group:
		return groupPool.Free((GroupAttribute*)ptr);
	case IMOParams::t_locstring:
		return lstrPool.Free((LocStringAttribute*)ptr);
	}
}


BaseAttribute* CreateEmptyAttribute (IMOParams::Type type)
{
  switch (type)
  {
    case IMOParams::t_bool:
      //return NEW BoolAttribute;
			return boolPool.Allocate();
    case IMOParams::t_long:
      //return NEW LongAttribute;
			return longPool.Allocate();
    case IMOParams::t_float:
      //return NEW FloatAttribute;
			return floatPool.Allocate();
    case IMOParams::t_string:
      //return NEW StringAttribute;
			return stringPool.Allocate();
    case IMOParams::t_position:
      //return NEW PositionAttribute;
			return posPool.Allocate();
    case IMOParams::t_angles:
      //return NEW RotationAttribute;
			return rotPool.Allocate();
    case IMOParams::t_color:
      //return NEW ColorAttribute;
			return colorPool.Allocate();
    case IMOParams::t_array:
      //return NEW ArrayAttribute;
			return arrayPool.Allocate();
    case IMOParams::t_enum:
      //return NEW EnumAttribute;
			return enumPool.Allocate();
    case IMOParams::t_group:
      //return NEW GroupAttribute;
			return groupPool.Allocate();
    case IMOParams::t_locstring:
      //return NEW LocStringAttribute;
			return lstrPool.Allocate();
      
  }

  return NULL;
}


BaseAttribute* CreateEmptyAttribute2 (IMOParams::Type type)
{
	switch (type)
	{
	case IMOParams::t_bool:
		//return NEW BoolAttribute;
		return boolPool.Allocate();
	case IMOParams::t_long:
		//return NEW LongAttribute;
		return longPool.Allocate();
	case IMOParams::t_float:
		//return NEW FloatAttribute;
		return floatPool.Allocate();
	case IMOParams::t_string:
		//return NEW StringAttribute;
		return stringPool.Allocate();
	case IMOParams::t_position:
		//return NEW PositionAttribute;
		return posPool.Allocate();
	case IMOParams::t_angles:
		//return NEW RotationAttribute;
		return rotPool.Allocate();
	case IMOParams::t_color:
		//return NEW ColorAttribute;
		return colorPool.Allocate();
	case IMOParams::t_array:
		//return NEW ArrayAttribute;
		return arrayPool.Allocate();
	case IMOParams::t_enum:
		//return NEW EnumAttribute;
		return enumPool.Allocate();
	case IMOParams::t_group:
		//return NEW GroupAttribute;
		return groupPool.Allocate();
	case IMOParams::t_locstring:
		//return NEW LocStringAttribute;
		return lstrPool.Allocate();

	}

	return NULL;

}



/*
BaseAttribute* CreateEmptyAttribute2 (IMOParams::Type type)
{
  switch (type)
  {
  case IMOParams::t_bool:
    return NEW BoolAttribute;
  case IMOParams::t_long:
    return NEW LongAttribute;
  case IMOParams::t_float:
    return NEW FloatAttribute;
  case IMOParams::t_string:
    return NEW StringAttribute;
  case IMOParams::t_locstring:
    return NEW LocStringAttribute;
  case IMOParams::t_position:
    return NEW PositionAttribute;
  case IMOParams::t_angles:
    return NEW RotationAttribute;
  case IMOParams::t_color:
    return NEW ColorAttribute;
  case IMOParams::t_array:
    return NEW ArrayAttribute;
  case IMOParams::t_enum:
    return NEW EnumAttribute;
  case IMOParams::t_group:
    return NEW GroupAttribute;
  }

  return NULL;

}
*/

BaseAttribute* CreateAttributeCopy (BaseAttribute* source, BaseAttribute* dest)
{
  IMOParams::Type type = source->GetType();
  BaseAttribute* NewAttr = dest;

  if (!NewAttr) NewAttr = CreateEmptyAttribute2 (type);
  if (NewAttr == NULL) return NULL;

  NewAttr->SetHint(source->GetHint());

  switch (type)
  {

    case IMOParams::t_group:
      {
        //group creation
        //api->Trace("GROUP FOUND !!!!");
        GroupAttribute* copyTo = (GroupAttribute*)NewAttr;
        GroupAttribute* copyFrom = (GroupAttribute*)source;

        *copyTo = *copyFrom;
        return NewAttr;
      }



    case IMOParams::t_bool:
      {
        BoolAttribute* copyTo = (BoolAttribute*)NewAttr;
        BoolAttribute* copyFrom = (BoolAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_long:
      {
        LongAttribute* copyTo = (LongAttribute*)NewAttr;
        LongAttribute* copyFrom = (LongAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_float:
      {
        FloatAttribute* copyTo = (FloatAttribute*)NewAttr;
        FloatAttribute* copyFrom = (FloatAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_string:
      {
        StringAttribute* copyTo = (StringAttribute*)NewAttr;
        StringAttribute* copyFrom = (StringAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_locstring:
      {
        LocStringAttribute* copyTo = (LocStringAttribute*)NewAttr;
        LocStringAttribute* copyFrom = (LocStringAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_position:
      {
        PositionAttribute* copyTo = (PositionAttribute*)NewAttr;
        PositionAttribute* copyFrom = (PositionAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_angles:
      {
        RotationAttribute* copyTo = (RotationAttribute*)NewAttr;
        RotationAttribute* copyFrom = (RotationAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_color:
      {
        ColorAttribute* copyTo = (ColorAttribute*)NewAttr;
        ColorAttribute* copyFrom = (ColorAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_array:
      {
        ArrayAttribute* copyTo = (ArrayAttribute*)NewAttr;
        ArrayAttribute* copyFrom = (ArrayAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_enum:
      {
        EnumAttribute* copyTo = (EnumAttribute*)NewAttr;
        EnumAttribute* copyFrom = (EnumAttribute*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }

    default:
    {
      api->Trace("unkonwn type : %d", type);
    }
  }


  return NewAttr;
  
}


BaseAttribute* CreateFromMissionParam (const IMOParams::Param* source)
{
  IMOParams::Type type = source->type;
  BaseAttribute* NewAttr = CreateEmptyAttribute (type);
  if (NewAttr) NewAttr->SetHint(source->comment);
  //NewAttr
  if (NewAttr == NULL) return NULL;


  //api->Trace("attr = %d", type);

  switch (type)
  {
    case IMOParams::t_group:
    {
      //group creation
      //api->Trace("GROUP FOUND !!!!");
      GroupAttribute* copyTo = (GroupAttribute*)NewAttr;
      IMOParams::Group* copyFrom = (IMOParams::Group*)source;
      *copyTo = *copyFrom;
      return NewAttr;
    }

    case IMOParams::t_bool:
      {
        BoolAttribute* copyTo = (BoolAttribute*)NewAttr;
        IMOParams::Bool* copyFrom = (IMOParams::Bool*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_long:
      {
        LongAttribute* copyTo = (LongAttribute*)NewAttr;
        IMOParams::Long* copyFrom = (IMOParams::Long*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_float:
      {
        FloatAttribute* copyTo = (FloatAttribute*)NewAttr;
        IMOParams::Float* copyFrom = (IMOParams::Float*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_string:
      {
        StringAttribute* copyTo = (StringAttribute*)NewAttr;
        IMOParams::String* copyFrom = (IMOParams::String*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_locstring:
      {
        LocStringAttribute* copyTo = (LocStringAttribute*)NewAttr;
        IMOParams::LocString* copyFrom = (IMOParams::LocString*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_position:
      {
        PositionAttribute* copyTo = (PositionAttribute*)NewAttr;
        IMOParams::Position* copyFrom = (IMOParams::Position*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_angles:
      {
        RotationAttribute* copyTo = (RotationAttribute*)NewAttr;
        IMOParams::Angles* copyFrom = (IMOParams::Angles*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_color:
      {
        ColorAttribute* copyTo = (ColorAttribute*)NewAttr;
        IMOParams::Colors* copyFrom = (IMOParams::Colors*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_array:
      {
        ArrayAttribute* copyTo = (ArrayAttribute*)NewAttr;
        IMOParams::Array* copyFrom = (IMOParams::Array*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }
    case IMOParams::t_enum:
      {
        EnumAttribute* copyTo = (EnumAttribute*)NewAttr;
        IMOParams::Enum* copyFrom = (IMOParams::Enum*)source;
        *copyTo = *copyFrom;
        return NewAttr;
      }

    default:
      {
        api->Trace("unkonwn type : %d", type);
      }

  }


  return NewAttr;
}