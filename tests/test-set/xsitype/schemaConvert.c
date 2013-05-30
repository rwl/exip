
/*
	Converts EXIProcessor generated schemas into EXIP supported shemas
*/

#include <stdio.h>

int main() {
	int	c;

	c	= fgetc(stdin);
	//fputc (c, stdout);
	fputc (0xa0, stdout);

	c	= fgetc(stdin);
	fputc (0x09, stdout);

	while ((c=fgetc(stdin)) != EOF)
		fputc (c, stdout);
	return	0;
}
