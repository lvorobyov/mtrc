#pragma once


#include <stdio.h>

typedef struct mtrc_object_s
{
	/*@owned@*/ char *src_filename;     /**< Source filename */
	/*@owned@*/ char *obj_filename;     /**< Object filename */

	unsigned char* buffer;				/**< �������� ��� */
	int lenght;							/**< ����� ��������� ���� */

	int reloc_offset;					/**< ����� ����� ��������� */

	int entry_offset;					/**< ����� ����� ����� _get_rule */

	int begin_state;					/**< ����� ���������� ��������� */

} mtrc_object;

int write_object(mtrc_object& obj, FILE* fout);