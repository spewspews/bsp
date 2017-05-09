#include "../bspbitree.h"

#include <stdlib.h>
#include <stdio.h>

int
main(void)
{
	Bitree b;

	bitinit(&b, 20);
	bitupdate(&b, 3, 14);
	bitupdate(&b, 7, 5);
	printf("%d\n", bitsum(&b, 0, 10));
	printf("%d\n", bitsum(&b, 3, 4));
	printf("%d\n", bitsum(&b, 7, 8));
	exit(0);
}
