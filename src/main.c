#include <stdio.h>
#include <stdint.h>

#define TITLE_SIZE 0x0F

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s [ROM filename]\n", argv[0]);
        return 1;
    }

    FILE *image = fopen(argv[1], "rb");

    if (!image)
    {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
        return 1;
    }

    // Get game title
    fseek(image, 0x0134, SEEK_SET);

    char title[TITLE_SIZE];

    size_t bytes_read = fread(title, TITLE_SIZE, sizeof(uint8_t), image);

    if (bytes_read > 0)
    {
        printf("Title %s\n", title);
    }
    else 
    {
        printf("fread returned 0 bytes\n");
    }

    uint8_t cartridge_t;
    fseek(image, 0x0147, SEEK_SET);

    bytes_read = fread(&cartridge_t, 1, sizeof(uint8_t), image);

    if (bytes_read > 0)
    {
        printf("cartridge type: %u\n", cartridge_t);
    }

    return 0;
}
