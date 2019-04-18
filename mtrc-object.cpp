#include "mtrc-object.h"

#include <windows.h>
#include <winnt.h>
#include <time.h>
#include <string.h>


int write_object(mtrc_object& obj, FILE* fout)
{
	if (fout == stderr)
	{
		return 1;
	}

	time_t cur;
	time(&cur);
	IMAGE_FILE_HEADER objhdr;
	objhdr.Machine = IMAGE_FILE_MACHINE_I386;
	objhdr.NumberOfSections = 2;
	objhdr.TimeDateStamp = cur;
	objhdr.PointerToSymbolTable = sizeof(IMAGE_FILE_HEADER) +
		sizeof(IMAGE_SECTION_HEADER) * 2 + obj.lenght + sizeof(IMAGE_BASE_RELOCATION) + sizeof(WORD) + sizeof(int);
	objhdr.SizeOfOptionalHeader = 0;
	objhdr.Characteristics = IMAGE_FILE_LINE_NUMS_STRIPPED
		| IMAGE_FILE_LOCAL_SYMS_STRIPPED
		| IMAGE_FILE_32BIT_MACHINE;

	IMAGE_SECTION_HEADER textsct;
	char textsect[] = ".text\0\0";
	for (int i = 0; i < 8; i++)
		textsct.Name[i] = textsect[i];
	textsct.Misc.PhysicalAddress = 0;
	textsct.VirtualAddress = 0;
	textsct.SizeOfRawData = obj.lenght;
	textsct.PointerToRawData = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER) * 2;
	textsct.PointerToRelocations = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER) * 2 + obj.lenght;
	textsct.PointerToLinenumbers = 0; /* возможно другое значение */
	textsct.NumberOfRelocations = 1;
	textsct.NumberOfLinenumbers = 0;
	textsct.Characteristics = IMAGE_SCN_CNT_CODE
		| IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;


	IMAGE_SECTION_HEADER datasct;
	char datasect[] = ".data\0\0";
	for (int i = 0; i < 8; i++)
		datasct.Name[i] = datasect[i];
	datasct.Misc.PhysicalAddress = obj.lenght;
	datasct.VirtualAddress = 0;
	datasct.SizeOfRawData = 4;
	datasct.PointerToRawData = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER) * 2 +
		obj.lenght + sizeof(IMAGE_BASE_RELOCATION) + sizeof(WORD);
	datasct.PointerToRelocations = 0;
	datasct.PointerToLinenumbers = 0;
	datasct.NumberOfRelocations = 0;
	datasct.NumberOfLinenumbers = 0;
	datasct.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA
		| IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;


	IMAGE_BASE_RELOCATION reloc;
	reloc.VirtualAddress = obj.reloc_offset;
	WORD TypeOffset = 6;

	IMAGE_SYMBOL symbols[6];
	char filesym[] = ".file\0\0";
	for (int i = 0; i < 8; i++)
		symbols[0].N.ShortName[i] = filesym[i];
	symbols[0].Value = 0;
	symbols[0].SectionNumber = IMAGE_SYM_DEBUG;
	symbols[0].Type = IMAGE_SYM_TYPE_NULL;
	symbols[0].StorageClass = IMAGE_SYM_CLASS_FILE;

	int filename_size = strlen(obj.obj_filename);
	int n_symbs = filename_size / sizeof(IMAGE_AUX_SYMBOL) + 1;
	objhdr.NumberOfSymbols = 8 + n_symbs;
	reloc.SizeOfBlock = 2 + n_symbs;
	symbols[0].NumberOfAuxSymbols = n_symbs;
	IMAGE_AUX_SYMBOL *aux_sym_file = new IMAGE_AUX_SYMBOL[n_symbs];
	int i = 0;
	for (int j = 0; j < n_symbs; j++)
	{
		for (i = 0; i < 18 && obj.obj_filename[i + j * 18] != 0; i++)
			aux_sym_file[j].File.Name[i] = obj.obj_filename[i + j * 18];
		for (; i < 18; i++)
			aux_sym_file[j].File.Name[i] = 0;
	}


	char featsym[] = "@feat.00";
	for (i = 0; i < 8; i++)
		symbols[1].N.ShortName[i] = featsym[i];
	symbols[1].Value = 1;
	symbols[1].SectionNumber = IMAGE_SYM_ABSOLUTE;
	symbols[1].Type = IMAGE_SYM_TYPE_NULL;
	symbols[1].StorageClass = IMAGE_SYM_CLASS_STATIC;
	symbols[1].NumberOfAuxSymbols = 0;

	for (i = 0; i < 8; i++)
		symbols[2].N.ShortName[i] = textsect[i];
	symbols[2].Value = 0;
	symbols[2].SectionNumber = 1;
	symbols[2].Type = IMAGE_SYM_TYPE_NULL;
	symbols[2].StorageClass = IMAGE_SYM_CLASS_STATIC;
	symbols[2].NumberOfAuxSymbols = 1;


	IMAGE_AUX_SYMBOL aux_symbol_text;
	aux_symbol_text.Section.Length = obj.lenght;
	aux_symbol_text.Section.NumberOfRelocations = 1;
	aux_symbol_text.Section.NumberOfLinenumbers = 0;
	aux_symbol_text.Section.CheckSum = 0;
	aux_symbol_text.Section.Number = 0;
	aux_symbol_text.Section.Selection = 0;

	char proc_name[] = "_get_rule";
	char data_name[] = "_start_state";


	symbols[3].N.Name.Short = 0;
	symbols[3].N.Name.Long = 4;
	symbols[3].Value = obj.entry_offset;
	symbols[3].SectionNumber = 1;
	symbols[3].Type = IMAGE_SYM_TYPE_NULL;
	symbols[3].StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
	symbols[3].NumberOfAuxSymbols = 0;

	for (i = 0; i < 8; i++)
		symbols[4].N.ShortName[i] = datasect[i];
	symbols[4].Value = 0;
	symbols[4].SectionNumber = 2;
	symbols[4].Type = IMAGE_SYM_TYPE_NULL;
	symbols[4].StorageClass = IMAGE_SYM_CLASS_STATIC;
	symbols[4].NumberOfAuxSymbols = 1;

	IMAGE_AUX_SYMBOL aux_symbol_data;
	aux_symbol_data.Section.Length = 4;
	aux_symbol_data.Section.NumberOfRelocations = 0;
	aux_symbol_data.Section.NumberOfLinenumbers = 0;
	aux_symbol_data.Section.CheckSum = 0;
	aux_symbol_data.Section.Number = 0;
	aux_symbol_data.Section.Selection = 0;

	symbols[5].N.Name.Short = 0;
	symbols[5].N.Name.Long = 4 + strlen(proc_name) + 1;
	symbols[5].Value = 0;
	symbols[5].SectionNumber = 2;
	symbols[5].Type = IMAGE_SYM_TYPE_INT;
	symbols[5].StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
	symbols[5].NumberOfAuxSymbols = 0;

	fwrite(&objhdr, sizeof(IMAGE_FILE_HEADER), 1, fout);
	fwrite(&textsct, sizeof(IMAGE_SECTION_HEADER), 1, fout);
	fwrite(&datasct, sizeof(IMAGE_SECTION_HEADER), 1, fout);
	fwrite(obj.buffer, sizeof(unsigned char), obj.lenght, fout);
	fwrite(&reloc, sizeof(IMAGE_BASE_RELOCATION), 1, fout);
	fwrite(&TypeOffset, sizeof(WORD), 1, fout);
	fwrite(&obj.begin_state, sizeof(int), 1, fout);
	fwrite(symbols, sizeof(IMAGE_SYMBOL), 1, fout);
	fwrite(aux_sym_file, sizeof(IMAGE_AUX_SYMBOL), n_symbs, fout);
	delete[] aux_sym_file;
	fwrite(&symbols[1], sizeof(IMAGE_SYMBOL), 2, fout);
	fwrite(&aux_symbol_text, sizeof(IMAGE_AUX_SYMBOL), 1, fout);
	fwrite(&symbols[3], sizeof(IMAGE_SYMBOL), 2, fout);
	fwrite(&aux_symbol_data, sizeof(IMAGE_AUX_SYMBOL), 1, fout);
	fwrite(&symbols[5], sizeof(IMAGE_SYMBOL), 1, fout);
	int string_size = strlen(proc_name) + 1 + strlen(data_name) + 1 + 4;
	fwrite(&string_size, sizeof(int), 1, fout);
	fwrite(proc_name, sizeof(char), strlen(proc_name) + 1, fout);
	fwrite(data_name, sizeof(char), strlen(data_name) + 1, fout);

	return 0;
}