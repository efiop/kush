#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static char *username;
static char *hostname;
static char *dirname;

static int init(void)
{
	username = getlogin();
	if (!username) {
		perror("Can't get username");
		return -1;
	}

	hostname = malloc(HOST_NAME_MAX*sizeof(*hostname));
	if (!hostname) {
		perror("Can't allocate memory for hostname");
		return -1;
	}

	gethostname(hostname, HOST_NAME_MAX);
	if (!hostname) {
		perror("Can't get hostname");
		return -1;
	}

	dirname = get_current_dir_name();
	if (!dirname) {
		perror("Can't get cwd");
		return -1;
	}

	return 0;
}

static int parse_cmd(char *s)
{
	int ret;
	int argc = 0;
	char **argv = NULL;
	char *token;

	token = strtok(s, " \n");
	while (token != NULL) {
		argc++;
		argv = realloc(argv, sizeof(*argv)*argc);
		argv[argc-1] = strdup(token);
		token = strtok(NULL, " \n");
	}

	if (!strcmp(argv[0], "cd")) {
		if (argc > 2) {
			fprintf(stderr, "Too many arguments for cd\n");
			return -1;
		}

		if (chdir(argv[1])) {
			perror("Can't chdir");
			return -1;
		}

		return 0;
	}

	ret = fork();
	switch (ret) {
	case 0:
		if (execvp(argv[0], argv)) {
			perror("Can't exec cmd");
			exit(1);
		}
	case -1:
		perror("Can't fork");
		return -1;
	default:
		if (ret != wait(NULL)) {
			perror("Can't wait for child");
			return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	char *s = malloc(256);

	while (1) {
		init();
		printf("%s@%s %s $ ", username, hostname, dirname);
		fgets(s, 256, stdin);
		parse_cmd(s);
	}
}
