#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <linux/limits.h>


char *
get_path (char * dir_path, char * sub_dir)
{
    char * path = (char*) malloc(sizeof(char) * PATH_MAX);
    path = strdup(dir_path);
    strcat(path, "/");
    strcat(path, sub_dir);
    return path;
}


void 
copy_file (char * sub_path, char * new_path) 
{
    FILE * r_fp;
    FILE * w_fp;
    char buffer[512]; 
    
    r_fp = fopen(sub_path , "rb");
    if ( r_fp == NULL ) {
        perror("fopen"); 
        exit (1);
    }

    w_fp = fopen(new_path , "wb");
    if (w_fp == NULL) {
        perror("fopen"); 
        exit (1);
    }

    size_t r_len;
    while (feof(r_fp) == 0)  {
        r_len = fread (buffer, 1, sizeof(buffer), r_fp);
        if (r_len != fwrite (buffer, 1, r_len, w_fp)) {
            // can i reposition the file pointer..?
            perror("fwrite");
            exit(1);
        }
    }

}
    fclose (r_fp);


void 
copy_dir (char * dir, char * new_dir) 
{
    // 1. make a corresponding directory to the new (target) directory.
    if( mkdir (new_dir, 0766) == -1 ) {
        perror("mkdir");
        exit(1);
    }

    // 2. open and read source directory.
    DIR * dp;
    dp = opendir (dir);
    
    if (dp == NULL) {
        perror ("opendir: Couldn't open the directory");
        exit(1);
    }
    else {
        struct dirent * sub;
        for( ; sub = readdir (dp); ) {
            
            char * sub_path = get_path(dir, sub->d_name);
            char * new_path = get_path(new_dir, sub->d_name);

            if (sub->d_type == DT_LNK) {
                continue;
            }
            if (sub->d_type == DT_REG) {
                copy_file(sub_path, new_path);
            }
            if (sub->d_type == DT_DIR) {
                if( strcmp (sub->d_name, "..") != 0 && strcmp (sub->d_name, ".") != 0 ) 
                    copy_dir(sub_path, new_path);
            }

            free(sub_path);
            free(new_path);
        }
        closedir(dp);
    }
    
}

int 
main (int argc, char ** argv) 
{
    char * source = argv[1];
    char * target = argv[2];

    // handle invalid parameters -> more cases..?
    if (argc != 3) {
        perror("Invalid parameters.\ncmd: ./copy <source> <target>");
        exit(1);
    }

    // make 1st target directory
    if (mkdir(target, 0766) == -1) {
        perror("mkdir");
        exit(1); 
    }
    
    // make a path of 1st copied directory -> size of array..?
    strcat(target, "/");
    if (source[0] == '.' && source[1] == '/') // is it alright..?
        source = source + 2;   
    strcat(target, source);

    copy_dir(source, target);

    return 0;
}