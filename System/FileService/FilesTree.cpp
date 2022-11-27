

#include "FilesTree.h"
#include "PackFile.h"


FilesTree::Node::Node() : path(_FL_),
								children(_FL_)
{
	parent = null;
}


FilesTree::FilesTree() : paths(_FL_)
{
	root.parent = null;
	pathsCount = 0;
}

FilesTree::~FilesTree()
{
}

//Добавить объект отображающий путь
void FilesTree::AddMirrorPath(MirrorPath * mp, const char * on)
{
	//Получаем последний нод в пути
	Node * node = AddPath(root, on);
	Assert(node);
	//Регистрируемся в ноде
	node->path.Add(mp);
	mp->SetNode(node);
}

//Добавить новый путь и вернуть конечный узел
FilesTree::Node * FilesTree::AddPath(Node & node, const char * path)
{
	//Выделяем текущее имя
	for(long size = 0; path[size] != '\\' && path[size]; size++);
	//Если нет продолжения то возвращаем родительский нод
	if(size == 0)
	{
		return &node;
	}
	//Ищем среди детей
	for(long i = 0; i < node.children; i++)
	{
		Node & child = *node.children[i];
		if(IsEqual(child.name.c_str(), child.name.Len(), path, size))
		{
			if(path[size])
			{
				return AddPath(child, path + size + 1);
			}
			return &child;
		}
	}
	//Нет такого, добавляем
	Node * n = NEW Node();
	n->name.Reserve(size + 1);
	for(long i = 0; i < size; i++)
	{
		n->name += path[i];
	}
	node.children.Add(n);
	n->parent = &node;
	return AddPath(*n, path + size + 1);
}

//Удалить объект отображающий путь
void FilesTree::DelMirrorPath(MirrorPath * mp)
{
	Node * node = (Node *)mp->GetNode();
	Assert(node);
	node->path.Del(mp);
	//Удаляем цепочку пустых нодов
	while(node->children.Size() == 0 && node->path.Size() == 0 && node->parent)
	{
		Node * parent = node->parent;
		parent->children.Del(node);
		delete node;
		node = parent;
	}
}

//Получить список зеркальных путей
dword FilesTree::FindPaths(const char * fullPath)
{
	pathsCount = 0;
	//Ищем по зеркальным путям в паках
	TreeProcess(root, fullPath);
	return pathsCount;
}

//Получить путь из списка
const char * FilesTree::GetPath(dword index)
{
	if(index < pathsCount)
	{
		return paths[index].c_str();
	}
	return null;
}

//Поиск всех путей и проверка файлов в паке
void FilesTree::TreeProcess(Node & node, const char * fullPath)
{
	//Перебираем все отзеркаленые пути
	for(long i = 0; i < node.path; i++)
	{
		MirrorPath * p = node.path[i];
		if(pathsCount >= paths.Size())
		{
			paths.AddElements(16);
			for(dword i = pathsCount; i < paths.Size(); i++)
			{
				paths[i].Reserve(1024);
			}
		}
		string & path = paths[pathsCount++];
		path = node.path[i]->GetPath();
		path += fullPath;
	}
	if(*fullPath == 0)
	{
		//Закончились поиски
		return;
	}
	//Выделяем текущее имя
	for(long size = 0; fullPath[size] != '\\' && fullPath[size]; size++);
	//Ищем среди детей
	for(long i = 0; i < node.children; i++)
	{
		Node & child = *node.children[i];
		if(IsEqual(child.name.c_str(), child.name.Len(), fullPath, size))
		{
			const char * path = fullPath + size;
			if(*path == '\\') path++;
			TreeProcess(child, path);
			return;
		}
	}
}

//Сравнить строки
bool FilesTree::IsEqual(const char * str1, long len1, const char * str2, long len2)
{
	if(len1 != len2)
	{
		return false;
	}
	for(; len1 > 0; len1--, str1++, str2++)
	{
		if(*str1 != *str2)
		{
			return false;
		}
	}
	return true;
}

