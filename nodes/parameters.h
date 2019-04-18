#pragma once

#include "rule_nodes.h"

class parameter	: public code_block
{
public:
	void setIdentifier(int id) { this->id = id; }
	int getIdentifier() const { return id; }
	void setType(int type) { this->type = type; }
	int getParam() const { return type; }
	int getCodeLength() { return 0; }
	unsigned char* writeCode(unsigned char* ptr) { return ptr; }
private:
	int id;
	int type; // 0 - value; 1 - dir; 2 - going;
};

class parameter_list : public code_list
{
public:
	parameter* getParameter(int id)
	{
		for (code_block* block: list)
		{
            auto par = dynamic_cast<parameter*>(block);
            if (par == nullptr)
                continue;
			if (par->getIdentifier() == id)
				return par;
		}
		return NULL;
	}
	int indexOf(int id)
	{
		int index = -1;
		for (code_block* block: list)
		{
			index++;
            auto par = dynamic_cast<parameter*>(block);
            if (par == nullptr)
                continue;
			if (par->getIdentifier() == id)
				return index;
		}
		return -1;
	}
	int getCodeLength() { return 0; }
	unsigned char* writeCode(unsigned char* ptr) { return ptr; }
	int getCount() const { return list.size(); }
private:

};



class going_argument_value : public going_expression
{
public:
	int getCodeLength() { return 5; }
	unsigned char* writeCode(unsigned char* ptr);
};/* -1  = end */

class direction_argument_value : public direction_expression
{
public:
	int getCodeLength() { return 2; }
	unsigned char* writeCode(unsigned char* ptr);
};/* 0 - not 1 - right; 2 - left */

class symbol_argument_value : public symbol_expression
{
public:
	int getCodeLength() { return 2; }
	unsigned char* writeCode(unsigned char* ptr);
};/* 0x15 - значение не менять; 0x16 - first; 0x17 - second */

class argument_list : public code_list
{
public:
	int getCodeLength() { return code_list::getCodeLength()/* + 3*/; }
	unsigned char* writeCode(unsigned char* ptr);
};
