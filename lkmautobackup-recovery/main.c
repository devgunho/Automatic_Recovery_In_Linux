/*
*  2020.09 DevGun
*
*  1. Verify that there is a backup directory created by LKM
*  2. Outputs a list of files to restore to the original file
*  3. Restore the corresponding file to its original state
*/

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <sys/io.h>
#include <sys/stat.h>

// Directory check function
int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

// Backup File check
int isFileExists(const char *fpath)
{
    FILE *file;
    if ((file = fopen(fpath, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int main()
{
    char *dirname = "/backup_dir";
    char recovery_path[256];
    char *filename = NULL;
    char fullpath[256];
    DIR *dp;
    struct dirent *dir;
    int file_cnt = 1, file_size;

    FILE *fp, *recovery_fp;

    if (isDirectory(dirname) != 1)
        printf("[-] Backup Directory does not exist.\n");

    else
    {
        printf("[+] Directory check... OK!\n");

        dp = opendir(dirname);
        if (dp)
        {
            while (dir = readdir(dp))
            {
                filename = dir->d_name;
                printf(" [*] (%3d) | %s\n", file_cnt++, filename);

                if (strstr(filename, ".lkmautobackup"))
                {
                    printf("  [*] This is backup file.\n");
                    strcpy(fullpath, dirname);
                    strcat(fullpath, "/");
                    strcat(fullpath, filename);
                    printf("  [*] %s\n", fullpath);

                    char buf[256] = {
                        0,
                    };
                    fp = fopen(fullpath, "rb");

                    fseek(fp, 0, SEEK_END);     // seek to end of file
                    file_size = ftell(fp);      // get current file pointer
                    fseek(fp, 0, SEEK_SET);

                    fread(buf, 4, 1, fp);

                    int origin_filename_size = (int)strtol(buf, NULL, 16);
                    printf("  [*] filename info. / length: %d\n", origin_filename_size);
                    memset(buf, 0, sizeof(char) * 256);
                    fread(buf, origin_filename_size, 1, fp);
                    printf("  [*] filename info. / string: %s\n", buf);
                    memset(recovery_path, 0, sizeof(char) * 256);
                    strcat(recovery_path, "/recovery_dir");
                    mkdir(recovery_path, 0777);
                    strcat(recovery_path, "/");
                    strcat(recovery_path, buf);
                    printf("  [*] recovery path: %s\n", recovery_path);

                    if (isFileExists(recovery_path))
                    {
                        printf("  [*] A previously recovered file exists.\n");
                        remove(recovery_path);
                    }

                    char *data;
                    int cpyBuf, file_counter = 0;

                    recovery_fp = fopen(recovery_path, "w+");
                    fseek(recovery_fp, 0, SEEK_SET);

                    while ((cpyBuf = getc(fp)) != EOF)
                    {
                        file_counter++;

                        // Do not copy until the last "0A" of the backup file.
                        if(file_counter==(file_size-origin_filename_size-4))
                            break;
                        fprintf(recovery_fp, "%c", (char)cpyBuf);
                    }
                    //printf("  [*] file counter: %d %d\n", file_counter, file_size);
                }
            }
            printf("[+] List END.\n");
            closedir(dp);
            fclose(fp);
            fclose(recovery_fp);
        }
    }

    return 0;
}