//----------------------------------------------------
Когда создаешь новый тип аттрибута 
его также нужно прописать в следующих местах:

*********************
* attributelist.cpp *
*************************************************************
void AttributeList::CreateFromParams (IMOParams* params)
void AttributeList::AddParam (BaseAttribute* attr)

************
* load.cpp *
*************************************************************
void MissionLoad::Load (const char* filename)
