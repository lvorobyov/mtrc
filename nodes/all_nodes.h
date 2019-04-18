#pragma once

#include "parameters.h"
#include "..\mtrc-object.h"

/* ----------------------- translarion unit -------------------------- */

class template_unit;

class template_call : public code_block
{
public:
	void setTemplate(template_unit* templUnit) { this->templUnit = templUnit; }
	void setArguments(argument_list* args) { this->args = args; }
	~template_call()
	{
		delete args;
	}
	int getCodeLength() { return (args->getCodeLength() + 5); }
	unsigned char* writeCode(unsigned char* ptr);
private:
	template_unit* templUnit;
	argument_list* args;
};

class subprogram_unit : public code_block
{
public:
	int getOffset(const unsigned char* current) const { return static_cast<int>(this->address - current); }
protected:
	unsigned char* address;
};

class translation_unit : public subprogram_unit
{
public:
	void setRules(rules_list* rules) { this->rules = rules; }
	void setBaseTemplate(template_call* inherit) { this->inherit = inherit;	}
	~translation_unit()
	{
		delete rules;
		delete inherit;
	}
	int getCodeLength() { return ((inherit ? inherit->getCodeLength() : 0) + (rules? rules->getCodeLength() : 0)); }
	unsigned char* writeCode(unsigned char* ptr);
	void writeGraph(FILE* gfile, int state_id) const
	{
		if(rules) rules->writeGraph(gfile, state_id);
	}
private:
	rules_list* rules;
	template_call* inherit;
};

class state_unit : public translation_unit
{
public:
	void setIndex(int index) { this->index = index; }
	void setBegin(int isBegin) { this->isBegin = isBegin; }
	bool isBeginState() const { return isBegin == 1; }
	int getIndex() const { return index; }
	int getCodeLength() { return (translation_unit::getCodeLength() + 1 /* + 7*/); }
	unsigned char* writeCode(unsigned char* ptr);
	void writeGraph(FILE* gfile) const
	{
		fprintf_s(gfile, "  %d [ label = < q<sub>%d</sub> > ]\n", index, index);
		translation_unit::writeGraph(gfile, index);
	}
private:
	int index;
	int isBegin;
};

class template_unit : public translation_unit
{
public:
	void setParameters(parameter_list* params) { this->params = params; }
	parameter_list* getParameters() const { return params; }
	~template_unit()
	{
		delete params;
	}
	int getCodeLength() { return (translation_unit::getCodeLength() + 3 /*9*/); }
	unsigned char* writeCode(unsigned char* ptr);
private:
	parameter_list* params;
};


class dispatch_table : public subprogram_unit
{
public:
	void setCount(int count) { this->count = count; }
	void setItem(state_unit* unit)
	{
		/*if (count >= 16)
		{
			dispatch_table* dsp_sub;
			int index = unit->getIndex() >> 4;
			if (sub_units[index] == NULL)
			{
				dsp_sub = new dispatch_table;
				dsp_sub->setCount(__min(count - index * 16, 16));
				sub_units[index] = dsp_sub;
			}
			else
			{
				dsp_sub = dynamic_cast<dispatch_table*>(sub_units[index]);
			}
			dsp_sub->sub_units[unit->getIndex() % 16] = unit;
		}
		else
		{*/
			int index = unit->getIndex();
			sub_units[index] = unit;
		//}
	}
	dispatch_table()
	{
		for (int i = 0; i < 16; i++)
			sub_units[i] = 0;
	}
	~dispatch_table()
	{
		/*if(count >= 16)
			for (int i = 0; i < 16; i++)
				delete sub_units[i];*/
	}
	int getCodeLength() { return (count*8 + 10 /* + 20*/); }
	unsigned char* writeCode(unsigned char* ptr);
	void setStartPtr(unsigned char* start_ptr)
	{
		this->start_ptr = start_ptr;
	}
	int getRelocOffset() const { return (int)(relocation_ptr - start_ptr); }
	int getEntryOffset() const { return this->getOffset(start_ptr); }
private:
	int count;
	subprogram_unit* sub_units[16];
private:
	unsigned char* start_ptr;
	unsigned char* relocation_ptr;
};

class units_list : public code_list
{
public:
	int getCodeLength()
	{
		int max_index = 0;
		for (code_block* node : list)
		{
			state_unit* stt_ptr = dynamic_cast<state_unit*>(node);
			if (stt_ptr)
			{
				if (stt_ptr->getIndex() > max_index)
				{
					max_index = stt_ptr->getIndex();
				}
			}
		}
		disp_table->setCount(max_index+1);
		return (code_list::getCodeLength() + disp_table->getCodeLength());
	}
	unsigned char* writeCode(unsigned char* ptr)
	{
		disp_table->setStartPtr(ptr);
		//start_ptr = ptr;
		for (code_block* node : list)
		{
			state_unit* stt_ptr = dynamic_cast<state_unit*>(node);
			if (stt_ptr)
			{
				disp_table->setItem(stt_ptr);
			}
		}
		return disp_table->writeCode(code_list::writeCode(ptr));
	}
	state_unit* getState(int index) const
	{
		for (code_block* node : list)
		{
			state_unit* stt_ptr = dynamic_cast<state_unit*>(node);
			if (stt_ptr)
			{
				if (stt_ptr->getIndex() == index)
					return stt_ptr;
			}
		}
		return NULL;
	}
	int getStatesCount() const
	{
		int count = 0;
		for (code_block* node : list)
		{
			state_unit* stt_ptr = dynamic_cast<state_unit*>(node);
			if (stt_ptr)
			{
				count++;
			}
		}
		return count;
	}
	int getBeginIndex() const
	{
		for (code_block* node : list)
		{
			state_unit* stt_ptr = dynamic_cast<state_unit*>(node);
			if (stt_ptr)
			{
				if (stt_ptr->isBeginState())
					return stt_ptr->getIndex();
			}
		}
		return 0;
	}
	//void setStartPtr(unsigned char* start_ptr)
	//{
	//	//if (disp_table) disp_table->setStartPtr(start_ptr);
	//	this->start_ptr = start_ptr;
	//}
	units_list()
	{
		disp_table = new dispatch_table;
	}
	~units_list()
	{
		delete disp_table;
	}
	//int getDispIndex() const { return disp_table->getOffset(start_ptr); }
	//int getRelocOffset() const { return disp_table->getRelocOffset(start_ptr); }
	void getObject(mtrc_object& obj)
	{
		obj.lenght = getCodeLength();
		obj.buffer = new unsigned char[obj.lenght];
		writeCode(obj.buffer);
		obj.entry_offset = disp_table->getEntryOffset();
		obj.reloc_offset = disp_table->getRelocOffset();
		obj.begin_state = getBeginIndex();
	}
	void writeGraph(FILE* gfile) const
	{
		fprintf_s(gfile, "// Generated by mtrc.\n\n");
		fprintf_s(gfile, "digraph \"%s\"\n{\n", "G");
		fprintf_s(gfile, "  node [fontname = courier, shape = circle, colorscheme = paired6]\n");
		fprintf_s(gfile, "  edge [fontname = courier]\n\n");
		
		for (code_block* node : list)
		{
			state_unit* stt = dynamic_cast<state_unit*>(node);
			if (stt) stt->writeGraph(gfile);
		}
		fprintf_s(gfile, "\n}");
	}
private:
	dispatch_table* disp_table;
	//unsigned char* start_ptr;
};


class alpha_symbol_list;

class turing_mashine /*: public code_block */
{
public:
	void setUnits(units_list* units) { this->units = units; }
	void setMtrType(int multiline) { this->multiline = multiline; };
	bool isMultiline() const { return this->multiline == 1; }
	void setAlphaList(symbol_list* alphaValueSet) { this->alphaValueSet = alphaValueSet; }
	friend alpha_symbol_list;
	symbol_list* getAlphaList() const;
	~turing_mashine()
	{
		delete units;
		delete alphaValueSet;
	}
	//int getCodeLength() { return 0; }
	//unsigned char* writeCode(unsigned char* ptr) { return ptr; }
	//int getProgramIndex() const { return units->getDispIndex(); }
	//int getRelocOffset() const { return units->getRelocOffset(); }
	//int getBeginIndex() const { return units->getBeginIndex(); }
	units_list* getUnits() const { return units; }
private:
	units_list* units;
	int multiline;
	symbol_list* alphaValueSet;
};

class alpha_symbol_list : public symbol_list
{
public:
	alpha_symbol_list(const turing_mashine* owner) : symbol_list() { this->owner = owner; }
	void addNodeLast(code_block* node) { owner->alphaValueSet->addNodeLast(node); }
	int getCodeLength() { return owner->alphaValueSet->getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr)
	{
		owner->alphaValueSet->setElseIndex(this->else_index);
		owner->alphaValueSet->setTrueOffset(this->true_offset);
		owner->alphaValueSet->setLineSource(this->line_src);
		return owner->alphaValueSet->writeCode(ptr);
	}
private:
	const turing_mashine* owner;
};

