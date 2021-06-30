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
    char path_name[PATH_MAX] ;
} Header ;

void
get_parameters (int argc, char ** argv, char * option, char * star_file, char * star_dir)
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

        *option = 'a' ;
        strcpy(star_file, argv[2]) ;
        strcpy(star_dir, argv[3]) ;
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

        *option = 'e' ;
        strcpy(star_file, argv[2]) ;
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

        *option = 'l' ;
        strcpy(star_file, argv[2]) ;
    }
}

/*
    1개의 파일에 대해 동작하는 프로그램 만들기
    1. archive 인 경우
        # 디렉토리를 먼저 열어야 한다
            # 그 다음 하위 파일을 archive 해야한다...

        # 디렉토리 열기
        # 하위 내용 읽기
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
        * header structure를 읽는다
        * h->name_size 만큼의 이름을 읽는다
        * 읽어낸 이름으로 새로운 파일을 연다.
        * h->data_size 만큼의 파일내용의 크기를 읽는다
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

///////////////////////////////////////////// archive /////////////////////////////////////////////

void
make_header(char * dir, Header * h)  
{
    struct stat st ;
    stat(dir, &st) ;
    
    h->data_size = st.st_size ;
    
    if (S_ISDIR(st.st_mode)) {
        h->type_id = 0 ;
    }
    else {  /* Q. any other case..? */
        h->type_id = 1 ;
    }

    h->name_size = strlen(dir) ;

    strcpy(h->path_name, dir) ;
}

void
write_header_data(FILE * w_fp, Header * h)
{
    if (fwrite(&h->type_id, 1, sizeof(h->type_id), w_fp) != sizeof(h->type_id)) 
        goto err_exit ;
    if (fwrite(&h->name_size, 1, sizeof(h->name_size), w_fp) != sizeof(h->name_size)) 
        goto err_exit ;
    if (fwrite(&h->data_size, 1, sizeof(h->data_size), w_fp) != sizeof(h->data_size)) 
        goto err_exit ;
    if (fwrite(h->path_name, 1, h->name_size, w_fp) != h->name_size) 
        goto err_exit ;

    return ;

err_exit:
    perror("Error: fail to write a header data") ;
    exit(1) ;
}

void read_and_write (FILE * r_fp, FILE * w_fp, size_t size) {
    int s ;
    for(s = 0; s < size; ) {
        char buffer[512] ;
        size_t r_len = fread(buffer, 1, sizeof(buffer), r_fp) ;
        if (r_len != fwrite(buffer, 1, r_len, w_fp)) {
            // do something?
        }
        s += r_len ;
    }       
}


void
write_data (FILE * w_fp, Header * h, char * source)
{
    if (fwrite(&h->type_id, 1, sizeof(h->type_id), w_fp) != sizeof(h->type_id)) 
        goto err_exit ;
    if (fwrite(&h->name_size, 1, sizeof(h->name_size), w_fp) != sizeof(h->name_size)) 
        goto err_exit ;
    if (fwrite(&h->data_size, 1, sizeof(h->data_size), w_fp) != sizeof(h->data_size)) 
        goto err_exit ;
    if (fwrite(h->path_name, 1, h->name_size, w_fp) != h->name_size) 
        goto err_exit ;

    if (h->type_id == 1) {   // not a directory!

        FILE * r_fp = fopen(source , "rb") ;
        if (r_fp == NULL) {
            perror("ERROR: fopen") ; 
            exit(1) ;
        }

        read_and_write(r_fp, w_fp, h->data_size) ;

        fclose(r_fp) ;
    }
    
    return ;

err_exit:
    perror("Error: fail to write a header data") ;
    exit(1) ;
}

/*
    디렉토리 안에 있는 파일을 archive 해야 한다!

*/


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
archive (char * star_file, char * star_dir) 
{
    DIR * dp = opendir(star_dir) ;
    if(dp == NULL) {
        perror("ERROR: opendir") ;
        exit(1) ;
    }
    else {
        struct dirent * sub ;
        for( ; sub = readdir(dp) ; ) {
            if (strcmp(sub->d_name, "..") != 0 && strcmp(sub->d_name, ".") != 0) {
                Header head ;
                make_header(sub->d_name, &head) ;
                
                FILE * w_fp ;
                w_fp = fopen(star_file, "wb") ;
                if(w_fp == NULL) {
                    perror("ERROR: Failed to open a file") ;
                    exit(1) ;
                }

                char * src_path = get_path(star_dir, sub->d_name) ;
                write_data(w_fp, &head, src_path) ;

                fclose(w_fp) ;
            }            
        } 
    }
}

///////////////////////////////////////////// extract /////////////////////////////////////////////
// 헤더 구조에 맞춰 바꿔야 함!

void
read_header (FILE * r_fp, Header * h)
{
    // Header * h = (Header *) malloc(sizeof(Header)) ;

    if (sizeof(h->type_id) != fread(&h->type_id, 1, sizeof(h->type_id), r_fp)) 
        goto err_exit ;
    if (sizeof(h->name_size) != fread(&h->name_size, 1, sizeof(h->name_size), r_fp)) 
        goto err_exit ;
    if (sizeof(h->data_size) != fread(&h->data_size, 1, sizeof(h->data_size), r_fp)) 
        goto err_exit ;
    if (h->name_size != fread(h->path_name, 1, h->name_size, r_fp)) 
        goto err_exit ;

    return ;

err_exit:
    perror("Error: fail to read a header data") ;
    exit(1) ;
}


void
extract (char * star_file)
{
    FILE * r_fp = fopen(star_file , "rb") ;
    if (r_fp == NULL) {
        perror("ERROR: fopen - star_file") ; 
        exit(1) ;
    }

    Header head ;
    read_header(r_fp, &head) ;

    if (head.type_id == 0) {
        if (mkdir(head.path_name, 0766) == -1) {
            perror("ERROR: mkdir") ;
            exit(1) ;
        }
    } 
    if (head.type_id == 1) { 
        FILE * w_fp = fopen(head.path_name, "wb") ;
        if (w_fp == NULL) {
            perror("ERROR: fopen") ; 
            exit(1) ;
        }
        
        read_and_write(r_fp, w_fp, head.data_size) ;
    }
    fclose(r_fp) ;
}

///////////////////////////////////////////// list /////////////////////////////////////////////

void
list (char * star_file)
{

}


int
main (int argc, char ** argv)
{
    
    char option ;   
    // char * star_file ;
    // char * star_dir ;
    char star_file[PATH_MAX] ;
    char star_dir[PATH_MAX] ;

    get_parameters (argc, argv, &option, star_file, star_dir) ;

    if (option == 'a') {
        archive(star_file, star_dir) ;
    }
    if (option == 'e') {
        extract(star_file) ;
    }
    if (option == 'l') {
        list(star_file) ;
    }

    return 0 ;
}
