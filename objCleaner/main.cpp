#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

void usage()
{
    printf("required args: FILE_IN FILE_OUT\n");
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        usage();
        system("pause");
        return -1;
    }

    FILE *fd_in;
    fopen_s(&fd_in, argv[1], "r");
    if (fd_in == NULL)
    {
        printf("Could not open input file\n");
        usage();
        system("pause");
        return -1;
    }

    FILE *fd_out;
    fopen_s(&fd_out, argv[2], "w");
    if (fd_out == NULL)
    {
        printf("Could not open output file\n");
        usage();
        fclose(fd_in);
        system("pause");
        return -1;
    }

    // create a list of pairs
    // first item is the object name
    // second item is a counter to indicate number of times that name has been found
    std::vector<std::pair<std::string, unsigned int>> names;

    const unsigned int SIZEOF_BUFFERS = 512;
    char inBuffer[SIZEOF_BUFFERS];
    char nameBuffer[SIZEOF_BUFFERS];
    unsigned int objects = 0;
    unsigned int unique = 0;

    while (1)
    {
        if (fgets(inBuffer, SIZEOF_BUFFERS, fd_in) == NULL)
        {
            // EOF
            fclose(fd_in);
            fclose(fd_out);

            printf("Found %u objects, %u unique\n", objects, unique);

            system("pause");
            return 0;
        }
        unsigned int len = strlen(inBuffer);
        if (len == 0)
        {
            // ???
            printf("Error read 0 bytes, but didn't return NULL\n");
            fclose(fd_in);
            fclose(fd_out);
            system("pause");
            return -2;
        }
        else
        {
            if (inBuffer[len - 1] != '\n')
            {
                printf("Line length exceeds maximum supported (%u)\n", SIZEOF_BUFFERS);
                fclose(fd_in);
                fclose(fd_out);
                system("pause");
                return -2;
            }
        }

        // read a full line, check first character for object name flag
        if (inBuffer[0] == 'o')
        {
            objects++;

            // parse the name of the object
            // note this is safe as nameBuffer is the same size as inBuffer
            int ret = sscanf_s(inBuffer, "o %s", nameBuffer, SIZEOF_BUFFERS);
            if (ret != 1)
            {
                printf("Error, sscanf_s returned %d\n", ret);
                fclose(fd_in);
                fclose(fd_out);
                system("pause");
                return -2;
            }

            std::string name(nameBuffer);
            // check if it's already in our list
            auto pos = std::find_if(std::begin(names), std::end(names), 
                            [name](std::pair<std::string, unsigned int> const &b)
                            {
                                return (b.first.compare(name) == 0);
                            });

            if (pos == std::end(names))
            {
                // new name, we can keep it.
                printf("found new object: %s\n", nameBuffer);
                unique++;
                names.push_back(std::pair<std::string, unsigned int>(nameBuffer, 1));

                if (fwrite(inBuffer, 1, len, fd_out) != len)
                {
                    printf("failed to write all data\n");
                    fclose(fd_in);
                    fclose(fd_out);
                    system("pause");
                    return -3;
                }
            }
            else
            {
                // fonud a duplicate
                pos->second++;
                printf("Duplicate object: %s\n", nameBuffer);
                printf("\tInstance %u, renaming\n", pos->second);

                // write out the name (ignoring the \n this time)
                if (fwrite(inBuffer, 1, len - 1, fd_out) != len - 1)
                {
                    printf("failed to write all data\n");
                    fclose(fd_in);
                    fclose(fd_out);
                    system("pause");
                    return -3;
                }
                // write out a .NUM to give it a unique name
                char tempBuffer[32];
                int tempLen = snprintf(tempBuffer, 32, ".%u\n", pos->second);
                if (tempLen < 3 || tempLen > 32)
                {
                    printf("snprintf failed with %d\n", tempLen);
                    fclose(fd_in);
                    fclose(fd_out);
                    system("pause");
                    return -3;
                }
                if (fwrite(tempBuffer, 1, tempLen, fd_out) != tempLen)
                {
                    printf("failed to write all data\n");
                    fclose(fd_in);
                    fclose(fd_out);
                    system("pause");
                    return -3;
                }
            }
        }
        else
        {
            // not a new object, just write it out
            if (fwrite(inBuffer, 1, len, fd_out) != len)
            {
                printf("failed to write all data\n");
                fclose(fd_in);
                fclose(fd_out);
                system("pause");
                return -3;
            }
        }
    }
}
