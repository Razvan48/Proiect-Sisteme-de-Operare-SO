#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool isOption(const char* user_option, const char* option1, const char* option2) {
	return strcmp(user_option, option1) == 0 || strcmp(user_option, option2) == 0;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "da: missing operand\n");
        fprintf(stderr, "Try \'da --help\' for more information\n");
        return 1;
    }

    if (isOption(argv[1], "-h", "--help")) {
    	printf("Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory at [DIR]\n"
            "  -a, --add           analyze a new directory path for disk usage\n"
            "  -p, --priority      set priority for the new analysis (works only with -a argument)\n"
            "  -S, --suspend <id>  suspend task with <id>\n"
            "  -R, --resume <id>   resume task with <id>\n"
            "  -r, --remove <id>   remove the analysis with the given <id>\n"
            "  -i, --info <id>     print status about the analysis with <id> (pending, progress, d\n"
            "  -l, --list          list all analysis tasks, with their ID and the corresponding root p\n"
            "  -p, --print <id>    print analysis report for those tasks that are \"done\"\n"
            "  -t, --terminate     terminates daemon\n\n"
        );
        return 0;
    }

   	fprintf(stderr, "da: invalid option \'%s\'\n", argv[1]);
   	fprintf(stderr, "Try \'da --help\' for more information\n");
	return 0;
}
