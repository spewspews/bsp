#define BSP_REGEXP_IMPLEMENTATION
#include "../bspregexp.h"

#include <errno.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

void
regerror(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(1);
}

enum {
	NMATCH = 10,
};

int
main(int argc, char **argv)
{
	char buf[8192];
	size_t l;
	Reprog *re;
	Resub matches[NMATCH], *mp;

	if(argc != 2) {
		printf("enter regex\n");
		exit(1);
	}

	setlocale(LC_ALL, "en_US.UTF-8");

	re = regcomp(argv[1]);
	if(re == NULL) {
		printf("regexp compilation returned NULL: %s\n", strerror(errno));
		exit(1);
	}

	while(fgets(buf, sizeof(buf), stdin)) {
		l = strlen(buf);
		if(buf[l-1] == '\n')
			buf[l-1] = '\0';
		memset(matches, 0, NMATCH*sizeof(*matches));
		if(regexec(re, buf, matches, NMATCH)) {
			printf("match!\n");
			for(mp = matches; mp < matches+NMATCH; mp++) {
				if(mp->ep)
					printf("%.*s\n", (int)(mp->ep - mp->sp), mp->sp);
			}
		}
	}
	exit(0);
}
