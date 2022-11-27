
#ifndef _FilesTree_h_
#define _FilesTree_h_

#include "MirrorPath.h"

class FilesTree
{
private:
	struct Node
	{
		Node();
		Node * parent;
		string name;
		array<MirrorPath *> path;
		array<Node *> children;
	};


public:
	FilesTree();
	~FilesTree();


public:
	//Добавить объект отображающий путь
	void AddMirrorPath(MirrorPath * mp, const char * on);
	//Удалить объект отображающий путь
	void DelMirrorPath(MirrorPath * mp);
	//Получить список зеркальных путей
	dword FindPaths(const char * fullPath);
	//Получить путь из списка
	const char * GetPath(dword index);

private:
	//Добавить новый путь и вернуть конечный узел
	Node * AddPath(Node & node, const char * path);
	//Поиск всех путей и проверка файлов в паке
	void TreeProcess(Node & node, const char * fullPath);

private:	
	//Сравнить строки
	static bool IsEqual(const char * str1, long len1, const char * str2, long len2);

private:
	//Дерево путей, на которое отображаются зеркальные пути
	Node root;
	//Поиск путей
	string path;
	array<string> paths;
	dword pathsCount;
};

#endif
