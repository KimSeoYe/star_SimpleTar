#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

typedef struct _header {
    char type_id ;
    size_t name_size ;
    size_t data_size ;
} Header ;

void
get_parameters (int argc, int ** argv, char option, char * star_file, char * star_dir)
{
    /*
        Comman line interface
        ./star archive <archive-file-name> <target directoxry path> -> zip
        ./star list <archive-file-name> -> show the paths of all files aggregated in the <archive-file-name>
        ./star extract <archive-file-name> -> restore all files and directories archived in <archive-file-name>

        * invalid option 
            * do not pass the option
            * argv[1] not in "archive" or "extract" or "list" ? 
        * case 1. "archive"
            * # of parameter : 4
            * target file name is invalid : already exist
            * target directory name is invalid (not exist)
        * case 2. "extract"
            * # of parameter : 3
            * target file name is invalid : not exist
        * case 3. "list"
            * # of parameter : 3
            * target file name is invalid : not exist

        Q. timing for checking existance of target file name or directory ?
            CLI에서 먼저 파일이나 디렉토리가 있는지 없는지를 체크해서, 없으면 아예 프로그램을 종료시키는 것이
            나중에 file이나 directory를 열 때 검사하도록 미뤄두는 것보다 나을까?
            file이나 directory를 열 때는 존재 유무 말고도 다른 문제가 발생할 수도 있으니까
            CLI에서 먼저 해두는 게 나을 것 같다고 판단하긴 했지만,,,
        Q. 아예 각 파라미터를 받는 함수를 따로..? Ex) get_option, get_file, get_dir ...
            매 함수마다 option별 case를 나눠줘야 하는데, 이게 맞는 건지 사실 잘 모르겠다.
        Q. 파라미터를 받는 과정 전체를 함수로 만들어보려고 했는데, star_dir이 필요한 경우와 필요 없는 경우가 있어서 조금 고민..
            어차피 변수는 선언할거니까 일단 함수로 빼 보자.
    */

    if (argc < 3 || (strcmp(argv[1], "archive") != 0 && strcmp(argv[1], "extract") != 0 && strcmp(argv[1], "list")) != 0) {
        perror("ERROR: option: archive, extract, list\n") ;
        exit(1) ;
    }

    if (strcmp(argv[1], "archive") == 0) {
        if (argc != 4) {
            perror("ERROR: star archive <archive-file-name> <target directory path>\n") ;
            exit(1) ;
        }
        if (access(argv[2], F_OK) == 0)
        {
            perror("ERROR: archive-file-name already exist\n") ;
            exit(1) ;
        }
        if (access(argv[3], R_OK) == -1)
        {
            perror("ERROR: target directory path do not exist\n") ;
            exit(1) ;
        }

        option = 'a' ;
        star_file = argv[2] ;
        star_dir = argv[3] ;

        printf("It archives %s as %s\n", star_dir, star_file) ;
    }
    if (strcmp(argv[1], "extract") == 0) {
        if (argc != 3) {
            perror("ERROR: star extract <archive-file-name>\n") ;
            exit(1) ;
        }
        if (access(argv[2], R_OK) == -1)
        {
            perror("ERROR: archive-file-name do not exist\n") ;
            exit(1) ;
        }

        option = 'e' ;
        star_file = argv[2] ;

        printf("It extracts %s\n", star_file) ;
    }
    if (strcmp(argv[1], "list") == 0) {
        if (argc != 3) {
            perror("ERROR: star list <archive-file-name>\n") ;
            exit(1) ;
        }
        if (access(argv[2], R_OK) == -1)
        {
            perror("ERROR: archive-file-name do not exist\n") ;
            exit(1) ;
        }

        option = 'l' ;
        star_file = argv[2] ;

        printf("It lists %s\n", star_file) ;
    }
}

/*
    1개의 파일에 대해 동작하는 프로그램 만들기

    1. archive 인 경우

    

        * header 구조체 선언
        * stat 으로 star_dir 의 정보를 읽는다 (type_id, data_size) -> header 구조체에 저장한다
            * directory일 경우 length가 0이어야 한다!!
        * star_dir의 이름의 크기를 header 구조체에 저장한다.
        * return header...

        * star_file 이라는 이름으로 새로운 파일을 연다. (write)
        * header structure 의 정보를 쓴다
        
        * star_dir 파일의 이름을 쓴다
        * directory면 contents를 쓰지 않는다...
        * -> directory가 아니면
            * star_dir 파일을 연다 (read)
            * star_dir 의 내용을 읽으며 그 뒤에 바로 이어 쓴다.

        * closedir, fclose, ...
        
        Q. 읽으면서 바로 write 하는게 나을까? 아니면 파일을 읽는 과정과 쓰는 과정을 분리하는게 좋을까? -> 읽으면서 write 해보자
        Q. 파일 이름의 크기를 따로 저장해 둬야 할 것 같다 
        data strucrue : 파일명의 크기 - 파일명 - 파일 내용의 크기 - 파일 내용 ? (v)
                        or 파일명의 크기 - 파일 내용의 크기 - 파일명 - 파일 내용 ?

    2. extract 인 경우

        * star_file 이라는 이름의 파일을 연다.

        * header 에서 파일이름의 크기를 읽는다 -> 그 크기 만큼의 파일 이름을 읽는다.
        
        * 읽어낸 이름으로 새로운 파일을 연다.
        * header 에서 파일내용의 크기를 읽는다
        * 읽어낸 크기 만큼의 파일 내용을 읽으며 새로 열린 파일에 쓴다.

        * closedir, fclose, ...

    3. list 인 경우

        * star_file 이라는 이름의 파일을 연다.

        * header 에서 파일이름의 크기를 읽는다 -> 그 크기 만큼의 파일 이름을 읽는다
        * 읽어낸 파일 이름을 출력한다.

        * header 에서 파일 내용의 크기를 읽는다
        * 그 크기 만큼 file pointer를 이동시킨다 -> how ?
            * fseek(SEEKK_CUR, 읽어낸 크기) ?

    Q. 어느 정도를 함수로 만들어야 할까
    
*/

Header *
read_header_data(char * dir)  
{
    Header * new_header ;
    
    struct stat * st ;
    stat(dir, st) ;
    
    new_header->data_size = st->size ;
    
    if (S_ISDIR(st->st_mode)) {
        new_header->type_id = 0 ;
    }
    else {  /* Q. any other case..? */
        new_header->type_id = 1 ;
    }

    new_header->name_size = strlen(dir) ;

    return new_header ;
}

FILE *
write_header_data(FILE * w_fp, Header * h)
{
    if (sizeof(h->type_id) != fwrite(&h->type_id, 1, sizeof(h->type_id), w_fp)) {
        perror("ERROR: fwrite - h->type_id") ;
        exit(1) ;
    }
    if (sizeof(h->name_size) != fwrite(&h->name_size, 1, sizeof(h->name_size), w_fp)) {
        perror("ERROR: fwrite - h->name_size") ;
        exit(1) ;
    }
    if (sizeof(h->data_size) != fwrite(&h->data_size, 1, sizeof(h->data_size), w_fp)) {
        perror("ERROR: fwrite - h->data_size") ;
        exit(1) ;
    }

    return w_fp ;
}

FILE *
write_contents_data(FILE * w_fp, Header * h, char * target_path)
{
    if (h->name_size != fwrite(target_path, 1, h->name_size, w_fp)) {
        perror("ERROR: fwrite - star_dir") ;
        exit(1) ;
    }

    if (h->type_id == 1) {   // not a directory!
        FILE * r_fp = fopen(target_path , "rb") ;
        if (r_fp == NULL) {
            perror("ERROR: fopen - target_path") ; 
            exit(1) ;
        }

        size_t r_len ;
        char buffer[512] ;
        while (feof(r_fp) == 0) {
            r_len = fread(buffer, 1, r_len, w_fp) ;
            if (r_len != fwrite(buffer, 1, r_len, w_fp)) {
                perror("ERROR: fwrite - file contetns") ;
                exit(1) ;
            }
        }       
        fclose(r_fp) ;
    }

    fclose(w_fp) ;
}

void
archive (char * star_file, char * star_dir) 
{
    
    Header * new_header = read_header_data(star_dir) ;
    
    FILE * w_fp ;
    w_fp = fopen(star_file, 'wb') ;
    if(r_fp == NULL) {
        perror("ERROR: Failed to open a file") ;
        exit(1) ;
    }

    w_fp = write_header_data(w_fp, new_header) ;
    w_fp = write_contents_data(w_fp, new_header, star_dir) ;

    fclose(w_fp) ;
}

void
extract (char * star_file, char * star_dir)
{

}

void
list (char * star_file, char * star_dir)
{

}


int
main (int argc, char ** argv)
{
    
    char option  = '' ;   
    char * star_file = "";
    char * star_dir = "";
    get_parameters (argc, argv, option, star_file, star_dir) ;

    if (option == 'a') {
        archive(char * star_file, char * star_dir) ;
    }
    if (option == 'e') {
        extract(char * star_file, char * star_dir) ;
    }
    if (option == 'l') {
        list(char * star_file, char * star_dir) ;
    }


    

    return 0 ;
}