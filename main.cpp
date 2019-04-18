
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "mtrc-options.h"
#include "mtrc-parser.h"
#include "nodes\all_nodes.h"


/*@null@*/ /*@only@*/
static char *obj_filename = NULL;
static char *in_filename = NULL;
static char *graph_filename = NULL;
static int special_options = 0;

/* Forward declarations: cmd line parser handlers */
static int opt_special_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_objfile_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_graphfile_handler(char *cmd, /*@null@*/ char *param, int extra);

size_t mtrc_splitpath(const char *path, /*@out@*/ const char **tail);
static /*@only@*/ char *replace_extension(const char *orig, /*@null@*/
	const char *ext, const char *def);
static void print_error(const char *fmt, ...);


/* values for special_options */
#define SPECIAL_SHOW_HELP 0x01
#define SPECIAL_SHOW_VERSION 0x02
#define SPECIAL_SHOW_LICENSE 0x04
#define SPECIAL_LISTED 0x08


/* command line options */
static opt_option options[] =
{
	{ 0, "version", 0, opt_special_handler, SPECIAL_SHOW_VERSION,
	("show version text"), NULL },
	{ 0, "license", 0, opt_special_handler, SPECIAL_SHOW_LICENSE,
	("show license text"), NULL },
	{ 'h', "help", 0, opt_special_handler, SPECIAL_SHOW_HELP,
	("show help text"), NULL },
	{ 'o', "objfile", 1, opt_objfile_handler, 0,
	("name of object-file output"), ("filename") },
	{ 'g', "graph", 1, opt_graphfile_handler, 0,
	("name of GraphVis graph-file output"), ("filename") }
};

#define PACKAGE_STRING "Turing Mashine Compiller Version 0.2 beta\n"

/* version message */
/*@observer@*/ static const char *version_msg[] = {
	PACKAGE_STRING,
	"Compiled on " __DATE__ ".",
	"Copyright (c) 2015 Lev Vorobuev.  All right reserved.",
	"Run mtrc --license for licensing overview and summary."
};

/* help messages */
/*@observer@*/ static const char *help_head = (
	"usage: mtrc [option]* file\n"
	"Options:\n");
/*@observer@*/ static const char *help_tail = (
	"\n"
	"Files are mtr sources to be compiled.\n"
	"\n"
	"Sample invocation:\n"
	"   mtrc -g graph.gv -o object.o source.mtr\n"
	/*"\n"
	"Report bugs to bug-mtrc@pi-knt.tk\n"*/);


static int do_compile()
{
	turing_mashine *turingMashine;
	mtrc_object object;

	const char *base_filename;

	/* determine the object filename if not specified */
	if (!obj_filename) {
		if (in_filename == NULL)
			/* Default to yasm.out if no obj filename specified */
			obj_filename = _strdup("yasm.out");
		else {
			/* replace (or add) extension to base filename */
			mtrc_splitpath(in_filename, &base_filename);
			if (base_filename[0] == '\0')
				obj_filename = _strdup("mtrc.obj");
			else
				obj_filename = replace_extension(base_filename,
					"obj",
					"mtrc.obj");
		}
	}



	object.obj_filename = obj_filename;
	object.src_filename = in_filename;

	/* Parse! */
	do_parse(turingMashine, in_filename);

	/* open the object file for output */
	FILE* obj = NULL;
	if (fopen_s(&obj, obj_filename, "wb"))
	{
		fprintf(stderr, "File %s can not open!", obj_filename);
		delete turingMashine;
		return EXIT_FAILURE;
	}
	

	/* Write the object file */
	turingMashine->getUnits()->getObject(object);
	write_object(object, obj ? obj : stderr);

	/* Close object file */
	if (obj)
		fclose(obj);


	/* Open and write the graph file */
	if (graph_filename)
	{
		FILE* gfile = NULL;
		if (fopen_s(&gfile, graph_filename, "wt"))
		{
			delete turingMashine;
			delete object.buffer;
			fprintf_s(stderr, "Graph file %s can not write.", graph_filename);
			return EXIT_FAILURE;
		}

		turingMashine->getUnits()->writeGraph(gfile);

		fclose(gfile);
	}

	delete object.buffer;
	delete turingMashine;

	return EXIT_SUCCESS;
}

#define NELEMS(array)   (sizeof(array) / sizeof(array[0]))

int main(int argc, char *argv[])
{

	if (parse_cmdline(argc, argv, options, NELEMS(options), print_error))
		return EXIT_FAILURE;

	switch (special_options) {
	case SPECIAL_SHOW_HELP:
		/* Does gettext calls internally */
		help_msg(help_head, help_tail, options, NELEMS(options));
		return EXIT_SUCCESS;
	case SPECIAL_SHOW_VERSION:
		for (int i = 0; i<NELEMS(version_msg); i++)
			printf("%s\n", version_msg[i]);
		return EXIT_SUCCESS;
	case SPECIAL_SHOW_LICENSE:
		/* for (i = 0; i<NELEMS(license_msg); i++)
			printf("%s\n", license_msg[i]); */
		return EXIT_SUCCESS;
	case SPECIAL_LISTED:
		/* Printed out earlier */
		return EXIT_SUCCESS;
	}

	/* Determine input filename and open input file. */
	if (!in_filename) {
		print_error(("No input files specified"));
		return EXIT_FAILURE;
	}


	return do_compile();
}

/*
*  Command line options handlers
*/
int not_an_option_handler(char *param)
{
	if (in_filename) {
		print_error(
			("warning: can open only one input file, only the last file will be processed"));
		delete [] in_filename;
	}

	in_filename = _strdup(param);

	return 0;
}

int other_option_handler(char *option)
{
	return 1;
}

static int
opt_special_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
	if (special_options == 0)
		special_options = extra;
	return 0;
}

static int
opt_objfile_handler(/*@unused@*/ char *cmd, char *param,
	/*@unused@*/ int extra)
{
	if (obj_filename) {
		print_error(
			("warning: can output to only one object file, last specified used"));
		delete [] obj_filename;
	}

	//assert(param != NULL);
	obj_filename = _strdup(param);

	return 0;
}

static int
opt_graphfile_handler(/*@unused@*/ char *cmd, char *param,
	/*@unused@*/ int extra)
{
	if (graph_filename) {
		print_error(
			("warning: can output to only one map file, last specified used"));
		delete [] graph_filename;
	}

	//assert(param != NULL);
	graph_filename = _strdup(param);

	return 0;
}

size_t
mtrc_splitpath(const char *path, /*@out@*/ const char **tail)
{
	const char *basepath = path;
	const char *s;

	/* split off drive letter first, if any */
	if (isalpha(path[0]) && path[1] == ':')
		basepath += 2;

	s = basepath;
	while (*s != '\0')
		s++;
	while (s >= basepath && *s != '\\' && *s != '/')
		s--;
	if (s < basepath) {
		*tail = basepath;
		if (path == basepath)
			return 0;   /* No head */
		else
			return 2;   /* Drive letter is head */
	}
	*tail = s + 1;
	/* Strip trailing .\ or ./ on path */
	while ((s - 1) >= basepath && *(s - 1) == '.' && (*s == '/' || *s == '\\')
		&& !((s - 2) >= basepath && *(s - 2) == '.'))
		s -= 2;
	/* Strip trailing slashes on path (except leading) */
	while (s>basepath && (*s == '/' || *s == '\\'))
		s--;
	/* Return length of head */
	return s - path + 1;
}


/* Replace extension on a filename (or append one if none is present).
* If output filename would be identical to input (same extension out as in),
* returns (copy of) def.
* A NULL ext means the trailing '.' should NOT be included, whereas a "" ext
* means the trailing '.' should be included.
*/
static char *
replace_extension(const char *orig, /*@null@*/ const char *ext,
	const char *def)
{
	char *out, *outext;
	size_t deflen, outlen;

	/* allocate enough space for full existing name + extension */
	outlen = strlen(orig) + 2;
	if (ext)
		outlen += strlen(ext) + 1;
	deflen = strlen(def) + 1;
	if (outlen < deflen)
		outlen = deflen;
	out = new char [outlen];

	strcpy_s(out, strlen(orig) + 1, orig);
	outext = strrchr(out, '.');
	if (outext) {
		/* Existing extension: make sure it's not the same as the replacement
		* (as we don't want to overwrite the source file).
		*/
		outext++;   /* advance past '.' */
		if (ext && strcmp(outext, ext) == 0) {
			outext = NULL;  /* indicate default should be used */
			print_error(
				("file name already ends in `.%s': output will be in `%s'"),
				ext, def);
		}
	}
	else {
		/* No extension: make sure the output extension is not empty
		* (again, we don't want to overwrite the source file).
		*/
		if (!ext)
			print_error(
				("file name already has no extension: output will be in `%s'"),
				def);
		else {
			outext = strrchr(out, '\0');    /* point to end of the string */
			*outext++ = '.';                /* append '.' */
		}
	}

	/* replace extension or use default name */
	if (outext) {
		if (!ext) {
			/* Back up and replace '.' with string terminator */
			outext--;
			*outext = '\0';
		}
		else
			strcpy_s(outext, strlen(ext) + 1, ext);
	}
	else
		strcpy_s(out, strlen(def) + 1, def);

	return out;
}

static void
print_error(const char *fmt, ...)
{
	va_list va;
	fprintf(stderr, "mtrc: ");
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fputc('\n', stderr);
}
