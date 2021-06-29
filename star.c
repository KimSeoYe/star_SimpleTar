#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

int
main (int argc, char ** argv)
{
    /*
        Comman line interface
        ./star archive <archive-file-name> <target directory path> -> zip
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
            CLI에서 먼저 있는지 없는지를 체크해서, 없으면 아예 프로그램을 종료시키는 것이
            나중에 file이나 directory를 열 때 검사하도록 미뤄두는 것보다 나을까?
            file이나 directory를 열 때는 존재 유무 말고도 다른 문제가 발생할 수도 있으니까
            CLI에서 먼저 해두는 게 나을 것 같다고 판단하긴 했지만,,,
        Q. 아예 각 파라미터를 받는 함수를 따로..? Ex) get_option, get_file, get_dir ...
        Q. 파라미터를 받는 과정 전체를 함수로 만들어보려고 했는데, star_dir이 필요한 경우와 필요 없는 경우가 있어서 조금 고민..
    */

    if (argc < 3 || (strcmp(argv[1], "archive") != 0 && strcmp(argv[1], "extract") != 0 && strcmp(argv[1], "list")) != 0) {
        perror("ERROR: option: archive, extract, list\n") ;
        exit(1) ;
    }

    char option ;   
    char * star_file ;
    char * star_dir ;
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



    return 0 ;
}