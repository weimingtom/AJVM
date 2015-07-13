#include <avm.h>
#include "ops.h"

#if !FREESTANDING

#include <stdio.h>
#include <string.h>
#include <signal.h>


static void die(char* e) {
	perror(e);
	exit(1);
}


#define DEFARG(x)	\
	static void x (char** argv, int* idx)



DEFARG(p_help);
DEFARG(p_version);
DEFARG(p_nostdlib);
DEFARG(p_library);
DEFARG(p_searchdir);
DEFARG(p_argv);
DEFARG(p_entry);

static int __argv_idx = -1;
static int __stdlib = 1;

struct {
	char* longopt;
	char* shortopt;
	char* docs;
	void (*handler) (char** argv, int* idx);
} args[] = {
	{ "--version", "-v", "Display AVM version information", p_version },
	{ "--help", "-h", "Display this information", p_help },
	{ "--nostdlib", "-n", "Use only libraries user-defined", p_nostdlib },
	{ "--library", "-l", "Load a library", p_library },
	{ "--searchdir", "-L", "Add a search directory for libraries", p_searchdir },
	{ "--argv", "-a", "Use the next arguments as parameters for main", p_argv },
	{ "--mainclass", "-e", "Override default Main-Class", p_entry },
	{ NULL, NULL, NULL, NULL }
};


DEFARG(p_help) {
	fprintf(stderr, APP_VERSION_FORMAT, APP_VERSION_ARGS);
	

	fprintf(stderr,
			"\nUsage:\n\tavm CLASS [ARGS] ...\n"
			"\t\tto invoke CLASS.main, or\n"
			"\tavm JPKFILE [ARGS] ...\n"
			"\t\tto execute a jpk file\n"
			"\nExample:\n\tavm echo.jpk --argv Hello World\n"
		);


	fprintf(stderr, "\nOptions:\n");

	int j;
	for(j = 0; args[j].handler; j++)
		fprintf(stderr, "%15s, %2s\t%s\n", args[j].longopt, args[j].shortopt, args[j].docs);


	exit(0);
}

DEFARG(p_version) {
	fprintf(stderr, APP_VERSION_FORMAT, APP_VERSION_ARGS);
	exit(0);
}


DEFARG(p_nostdlib) {
	__stdlib = 0;
}

DEFARG(p_library) {
	char* lname = NULL;
	if((strncmp(argv[*(idx)], "-l", 2) == 0) && (strlen(argv[*(idx)]) > 2))
		lname = &(argv[*(idx)] [2]);
	else {
		lname = argv[*(idx) + 1]; 
		*idx += 1; 
	}

	char buf[256];
	memset(buf, 0, sizeof(buf));

	strcat(buf, "lib");
	strcat(buf, lname);
	strcat(buf, ".jar");

	if(avm_open_library(buf) == J_OK)
		return;


	memset(buf, 0, sizeof(buf));

	strcat(buf, lname);
	strcat(buf, ".jar");

	if(avm_open_library(buf) == J_OK)
		return;

	die(lname);
}

DEFARG(p_searchdir) {
	char* lname = NULL;
	if((strncmp(argv[*(idx)], "-L", 2) == 0) && (strlen(argv[*(idx)]) > 2))
		lname = &(argv[*(idx)] [2]);
	else {
		lname = argv[*(idx) + 1]; 
		*idx += 1; 
	}

	avm_config_path_add(lname);
}

DEFARG(p_argv) {
	__argv_idx = *(idx) + 1;
}


DEFARG(p_entry) {
	char* lname = NULL;
	if((strncmp(argv[*(idx)], "-e", 2) == 0) && (strlen(argv[*(idx)]) > 2))
		lname = &(argv[*(idx)] [2]);
	else {
		lname = argv[*(idx) + 1]; 
		*idx += 1; 
	}

	avm_set_entrypoint(lname);
}

int main(int argc, char** argv) {

	if(argc < 2) {
		fprintf(stderr,
			"Usage:\tavm CLASS [ARGS] ...\n"
			"\t\tto invoke CLASS.main, or\n"
			"\tavm JPKFILE [ARGS] ...\n"
			"\t\tto execute a jpk file\n"
			"try 'avm --help' for more information.\n"
		);

		exit(1);
	}


	INITIALIZE_PATH();	/* see config.h */

	int i, j;
	for(i = 1; (i < argc) && (__argv_idx == -1); i++) {
		for(j = 0; args[j].handler; j++) {
			if(
				!((strcmp(argv[i], args[j].longopt) == 0) ||
				(strncmp(argv[i], args[j].shortopt, strlen(args[j].shortopt)) == 0))
			) continue;

			args[j].handler(argv, &i);
			break;
		}

		if(args[j].handler == NULL) {
			if(avm_open(argv[i]) == J_ERR) {
				fprintf(stderr, "avm: Invalid argument '%s'\n", argv[i]);
				exit(1);
			}
		}
	}	
	

	if(__stdlib)
		if(avm_open_library("rt.jar") == J_ERR)
			die("rt.jar");

	

	if(__argv_idx != -1) {
		argv = &argv[__argv_idx];
		argc -= __argv_idx;
	} else {
		argv = &argv[argc];
		argc = 0;
	}


	avm_begin();
	avm_main(argc, argv);
	avm_end();

 
	return 0;
}

#else
int main() { return 1; }
#endif
