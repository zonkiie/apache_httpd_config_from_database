#include <includes.h>

int main(int argc, char **argv)
{
	char * file = argv[1];
	mmapwrite(file, "Dies ist ein Test.\n");
}
