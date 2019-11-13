#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

int mmapwrite(char *filename)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	umask (0);
	int fd_ziel = open (filename, O_RDWR | O_EXCL | O_CREAT, mode);
	if (lseek (fd_ziel, getpagesize() - 1, SEEK_SET) == -1) {
      perror ("lseek");
      exit (EXIT_FAILURE);
	}
	write (fd_ziel, " ", 1);
	char *ziel = NULL;
	if ((ziel =
        mmap (0, getpagesize(), PROT_READ | PROT_WRITE,
              MAP_SHARED, fd_ziel, 0)) == -1) {
      perror ("mmap");
      exit (EXIT_FAILURE);
	}
	strcpy(ziel, "Dies ist ein Beispieltext.\n");
	munmap(ziel, getpagesize());
	close(fd_ziel);
	return 0;
}

int main(int argc, char **argv)
{
	char * file = argv[1];
	mmapwrite(file);
}
