#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
	
	FILE *f;
	char buff[80];
	char id[80];
	int count = 0;
	int c;
	size_t i;
	
	if (argc != 2)
	{
		fprintf(stderr, "usage: bin2cpp [filename]\n");
		return 1;
	}

	f = fopen(argv[1], "rb");

	if (!f)
	{
		fprintf(stderr, "Unable to open %s for reading.\n", argv[1]);
		return 1;
	}

	strcpy(id, argv[1]);
	for (i = 0; i < strlen(id); ++i)
	{
		if (!isalnum(id[i]))
		{
			id[i] = '_';
		}
	}

	printf("const char *%s = \n", id);

	buff[0] = 0;

	while ((c = fgetc(f)) != -1)
	{
		char tmp[10];
		count++;
		if (strlen(buff) == 0)
		{
			sprintf(tmp, "\\x%02x", c);
		}
		else
		{
			sprintf(tmp, "\\x%03x", c);
		}
		if (strlen(tmp) + strlen(buff) < 70)
		{
			strcat(buff, tmp);
		}
		else
		{
			printf("\t\"%s\"\n", buff);
			sprintf(buff, "\\x%02x", c);
		}
	}

	fclose(f);

	printf("\t\"%s\";\n" ,buff);
	printf("size_t %s_len = %d;\n", id, count);

	return 0;

}

