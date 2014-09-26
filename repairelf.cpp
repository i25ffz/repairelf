/****************************************************************************
 * 360 crack elf header repair tool
 *                                         (c) 2014 martin of USTeam
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "elf.h"

unsigned char ELF_MAGIC[] = {0x7f, 0x45, 0x4c, 0x46};

int main(int argc, const char** argv) {
	if (argc < 2)
        printf("Usage:\n\t%s <so file>", argv[0]);

    FILE *file = fopen(argv[1], "rb+");
    if (!file) {
        printf("input file not exists!\n");

        return -1;
    }

    fseek(file, 0L, SEEK_END);
    int size = ftell(file);

    Elf32_Ehdr header;

    //seek to begin && read
    fseek(file, 0L, SEEK_SET);
    fread(&header, 1, sizeof(Elf32_Ehdr), file);

	if (memcmp(&header, ELF_MAGIC, 4)) {
        printf("Invalid elf file[%s]!\n", argv[1]);

        return 0;
	}

	if (header.e_type == ET_EXEC) {
        printf("ELF execute file[%s]\n", argv[1]);
	} else if (header.e_type == ET_DYN) {
        printf("ELF shared file[%s]\n", argv[1]);
	} else {
		printf("Unknown e_type : %d\n", header.e_type);
		printf("Aborting repair...\n");
        fclose(file);

		return -1;
	}

	if (header.e_phentsize == 32 && header.e_shentsize == 40 &&
            (header.e_shoff + header.e_shentsize * header.e_shnum) == size) {
        printf("The elf header is ok!\n");
        fclose(file);

        return 0;
	} else {
        printf("Invalid elf header, start repairing...\n");
	}

    //read all sections &&get string table index
    fseek(file, header.e_shoff, SEEK_SET);
    Elf32_Shdr sechd;
    int idx = 0;
    while (fread(&sechd, 1, sizeof(Elf32_Shdr), file)) {
        if (sechd.sh_type == SHT_STRTAB && sechd.sh_name == 1) {
            printf("find string table index:%d\n", idx);
            break;
        }

        idx++;
    }

	header.e_phentsize = 32;
	header.e_shentsize = 40;
	header.e_shnum = (size - header.e_shoff) / 40;
    header.e_shstrndx = idx;

    fseek(file, 0L, SEEK_SET);
    fwrite(&header, 1, sizeof(Elf32_Ehdr), file);

    fclose(file);

    printf("repair so OK!\n");

	return 0;
}
