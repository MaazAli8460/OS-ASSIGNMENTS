#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

void menu();

int main()
{

    int maxname = 20;
    int max_content = 1000;
    char fname[maxname];
    printf("Enter the filename: ");
    // read filename
    scanf("%s", fname);
    int i = 0;
    while (fname[i] != '\0')
    {
        i++;
    }
    fname[i] = '.';
    i++;
    fname[i] = 'c';
    i++;
    fname[i] = '\0';
    int count = 1;
    char choice[4] = "1";
    int save = 0;
    int compile = 0;
    char input;
    char file_content[max_content];
    int bytes_read = 0;
    menu();
    printf("Enter text. Press ESC and then ENTER to execute commands.\n");
    int pre = 1;

    FILE *fp = fopen(fname, "r+");
    if (!fp)
    {
        fp = fopen(fname, "w+");
        if (fp == NULL)
        {
            printf("Error creating file \"%s\"\n", fname);
            return 1;
        }
    }
    else
    {
        printf("\nFile Exists");
        if (fp == NULL)
        {
            printf("Error opening file \"%s\"\n", fname);
        }

        // Read file contents into a buffer

        bytes_read = fread(file_content, sizeof(char), max_content, fp);
        if (bytes_read == 0)
        {
            printf("File may be empty\"%s\"\n", fname);
        }
        else
        {
            printf("File content:\n");
            for (int i = 0; i < bytes_read; i++)
            {
                if (file_content[i] == '\n') //|| file_content[i] == EOF)
                {
                    printf("\n");
                    printf("%d >> ", count++);
                    continue;
                }
                printf("%c", file_content[i]);
            }
            count--;
        }
        pre = 0;
    }
    while (1)
    {

        while ((input = getchar()) != 27)
        {

            if (input == '\n' && pre != 0)
            {
                printf("%d >> ", count);
                file_content[bytes_read] = '\n';
                bytes_read++;
                count++;
                continue;
            }
            pre++;
            file_content[bytes_read] = input;
            bytes_read++;
        }
        count--;
        printf("MinEdit @copyrights Reserved:");
        scanf("%s", choice);
        if (strcmp(choice, "0") == 0)
        {
            break;
        }
        if (strcmp(choice, ":s") == 0 || strcmp(choice, ":S") == 0)
        {
            /* Save code */
            if (fp)
            {
                fclose(fp); // Close the file
                /* code */
            }
            fp = fopen(fname, "w+"); // Open in write mode to overwrite existing file
            printf("Code Saved\n");
            for (int i = 0; i < bytes_read; i++)
            {
                fputc(file_content[i], fp);
                printf("%c", file_content[i]);
            }
            fclose(fp);
            save = 1;
        }
        else if (strcmp(choice, ":!c") == 0 || strcmp(choice, ":!C") == 0)
        {
            /* Compile code */
            if (save == 0)
            {
                printf("\nSave file first to compile.\n");
            }
            else
            {
                printf("Running Compile command..........\n");
                char compile_cmd[100];
                sprintf(compile_cmd, "gcc -o file12 %s", fname);
                system(compile_cmd);
                compile = 1;
            }
        }
        else if (strcmp(choice, ":e") == 0 || strcmp(choice, ":E") == 0)
        {
            if (compile == 1)
            {
                printf("Running code..........\n\n\n\n");
                system("./file12");
                compile = 0;
                printf("\n\n\n\nRunning completed..........\n");
            }
            else
            {
                printf("Run failed......\n");
                printf("Try saving, debuging and compiling file again......\n");
            }
            break;
        }
        else if (strcmp(choice, ":x") == 0 || strcmp(choice, ":X") == 0)
        {
            if (save == 1)
            {
                printf("\nDo you confirm(y/n):");
                scanf("%s", choice);
                if (strcmp(choice, "y") == 0)
                {
                    printf("\nFile closed.\n");
                    fclose(fp);
                    break;
                }
                else
                {
                    printf("\nFile is open.\n");
                    continue;
                }
            }
            else
            {
                fclose(fp);
                printf("\nFile closed.\n");
            }
        }
        else
        {
            printf("\nInput Not Recognised.....\n");
        }
        printf("\n");
    }

    printf("Text entered. Saved to \"%s\"\n", fname);
    return 0;
}
void menu()
{
    printf("\nMenu");
    printf("\t0.  To Exit the Min Editor.\n");
    printf("\t:!c or :!C To Compile code.\n");
    printf("\t:s  or :S  To Save code.\n");
    printf("\t:e  or :E  To Run code.\n");
    printf("\t:x  or :X  To Close code file.\n");
}
