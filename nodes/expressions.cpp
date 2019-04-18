#include "expressions.h"

/*	argument_expression
	code gen

00000020: 8B 4C 24 08        mov         ecx,dword ptr [esp+8]
*/

unsigned char* argument_expression::writeCode(unsigned char* ptr)
{
	*ptr++ = 0x8B;
	*ptr++ = 0x4C;
	*ptr++ = 0x24;
	*ptr++ = 0x04 + 4 * (unsigned char)arg_index;	// mov ecx,[esp+4]; arg_index = 0
	//unsigned char array[] =
	//{
	//	0x8B, 0x4C, 0x24, 0x04 + 4 * arg_index	
	//};
	//for (unsigned char var : array)
	//{
	//	*ptr++ = var;
	//}
	return ptr;
}

/*	going_arg
code generation example
----------- asm ------------
mov ecx, [ebp+12]	; arg_index = 0
cmp ecx, -1 		; -1 = end
je .q_end1
shl ecx, 24
and eax, ecx
jmp .end
.q_end1:
bts eax, 22
.end:
----------------------------
00000055: 8B 4D 0C           mov         ecx,dword ptr [ebp+0Ch]
00000058: 83 F9 FF           cmp         ecx,0FFFFFFFFh
0000005B: 74 07              je          00000064
0000005D: C1 E1 18           shl         ecx,18h
00000060: 09 C8              or          eax,ecx
00000062: EB 04              jmp         00000068
00000064: 0F BA E8 16        bts         eax,16h
00000068:
---------------------------
*/

unsigned char* going_arg::writeCode(unsigned char* ptr)
{
	ptr = argument_expression::writeCode(ptr);
	unsigned char array[] =
	{
		0x83, 0xF9, 0xFF,
		0x74, 0x07,
		0xC1, 0xE1, 0x18,
		0x09, 0xC8,
		0xEB, 0x04,
		0x0F, 0xBA, 0xE8, 0x16
	};
	for (unsigned char var : array)
	{
		*ptr++ = var;
	}
	return ptr;
}

/*	direction_arg
code generation example
---------- asm ----------------
00000000: 8B 4D 0C           mov         ecx,dword ptr [ebp+0Ch]
00000003: C1 E1 10           shl         ecx,10h
00000006: 01 C8              add         eax,ecx
-------------------------------
00000000: 8B 4D 0C           mov         ecx,dword ptr [ebp+0Ch]
00000003: 8D 0C 49           lea         ecx,[ecx+ecx*2]
00000006: C1 E1 10           shl         ecx,10h
00000009: 01 C8              add         eax,ecx

*/

unsigned char* direction_arg::writeCode(unsigned char* ptr)
{
	ptr = argument_expression::writeCode(ptr);
	if (line_dst == 1)
	{
		unsigned char array[] =
		{
			0x8D, 0x0C, 0x49
		};
		for (unsigned char var : array)
		{
			*ptr++ = var;
		}
	}
	unsigned char array2[] =
	{
		0xC1, 0xE1, 0x10,
		0x01, 0xC8
	};
	for (unsigned char var: array2)
	{
		*ptr++ = var;
	}
	return ptr;
}


/*	symbol_value
code_generation example
----------- asm -----------
------ line_dst = 0 -------
0000001C: B0 31              mov         al,31h
------ line_dst = 1 -------
0000001E: B4 00              mov         ah,0

*/

unsigned char * symbol_value::writeCode(unsigned char * ptr)
{
	*ptr++ = 0xB0 + ((line_dst == 0) ? 0x00 : 0x04);
	*ptr++ = value;
	return ptr;
}


/*	symbol_copy
code generation example
----------- asm -----------
------ line_src = 0 -------
00000009: 88 C0              mov         al,al	; line_dst = 0
0000000D: 88 C4              mov         ah,al	; line_dst = 1
------ line_src = 1 -------
0000000B: 88 E0              mov         al,ah	; line_dst = 0
0000000F: 88 E4              mov         ah,ah	; line_dst = 1
*/

unsigned char* symbol_copy::writeCode(unsigned char* ptr)
{
	//unsigned char array[] =
	//{
	//	/*0x8B, 0x4D, 0x08,*/
	//	0x88, ((line_dst == 0) ? 0x00 : 0x04) + ((line_src == 0) ? 0xC0 : 0xE0)
	//};for (unsigned char var : array)
	//{
	//	*ptr++ = var;
	//}
	*ptr++ = 0x88;
	*ptr++ = ((line_dst == 0) ? 0x00 : 0x04) + ((line_src == 0) ? 0xC0 : 0xE0);
	
	return ptr;
}

/* symbol_argument
code generation example
----------- asm -----------
00000000: 8B 4D 0C           mov         ecx,dword ptr [ebp+0Ch]
00000003: 80 F9 15           cmp         cl,15h
00000006: 74 0E              je          00000016
00000008: 80 F9 16           cmp         cl,16h
0000000B: 74 0F              je          0000001C
0000000D: 80 F9 17           cmp         cl,17h
00000010: 74 11              je          00000023
00000012: 88 C8              mov         al,cl
00000014: EB 12              jmp         00000028
00000016: 0F BA E8 14        bts         eax,14h
0000001A: EB 0C              jmp         00000028
0000001C: 8B 4D 08           mov         ecx,dword ptr [ebp+8]
0000001F: 88 C8              mov         al,cl
00000021: EB 05              jmp         00000028
00000023: 8B 4D 08           mov         ecx,dword ptr [ebp+8]
00000026: 88 E8              mov         al,ch

*/

unsigned char* symbol_argument::writeCode(unsigned char* ptr)
{
	ptr = argument_expression::writeCode(ptr);
	unsigned char array[] =
	{
		0x80, 0xF9, 0x15,
		0x74, 0x0E,
		0x80, 0xF9, 0x16,
		0x74, 0x0F,
		0x80, 0xF9, 0x17,
		0x74, 0x11,
		0x88, ((line_dst == 0) ? 0x08 : 0x0C) + 0xC0,
		0xEB, 0x12,
		0x0F, 0xBA, 0xE8, 0x14 + (unsigned char)line_dst,
		0xEB, 0x0C,
		0x8B, 0x4D, 0x08,
		0x88, ((line_dst == 0) ? 0x08 : 0x0C) + 0xC0,
		0xEB, 0x05,
		0x8B, 0x4D, 0x08,
		0x88, ((line_dst == 0) ? 0x08 : 0x0C) + 0xE0
	};
	for (unsigned char var : array)
	{
		*ptr++ = var;
	}
	return ptr;
}

unsigned char* actions_block::writeCode(unsigned char* ptr)
{
	*ptr++ = 0xEB;
	*ptr++ = end_offset;
	return ptr;
}


/*	rule_action
code generation example

00000019: 0D 00 2A 14 02     or         eax,2142A00h
...

*/

unsigned char* rule_action::writeCode(unsigned char* ptr)
{
	rule_expression* rule_ptr;
	unsigned long rule = ((rule_ptr = dynamic_cast<rule_expression*>(settingStmt))? rule_ptr->getRuleInfo() : 0) +
		((rule_ptr = dynamic_cast<rule_expression*>(movingStmt))? rule_ptr->getRuleInfo() : 0) +
		((rule_ptr = dynamic_cast<rule_expression*>(goingStmt))? rule_ptr->getRuleInfo() : 0);
	*ptr++ = 0x0D;	// 0xB8 - mov 
	*ptr++ = ((unsigned char)rule);
	rule >>= 8;
	*ptr++ = ((unsigned char)rule);
	rule >>= 8;
	*ptr++ = ((unsigned char)rule);
	rule >>= 8;
	*ptr++ = ((unsigned char)rule);
	rule >>= 8;
	if (settingStmt) ptr = settingStmt->writeCode(ptr);
	if (movingStmt) ptr = movingStmt->writeCode(ptr);
	if (goingStmt) ptr = goingStmt->writeCode(ptr);
	ptr = actions_block::writeCode(ptr);
	return ptr;
}
