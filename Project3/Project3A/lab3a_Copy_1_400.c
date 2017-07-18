//NAME: Elena Escalas (Partner: Susan Krkasharian)
//EMAIL: elenaescalas@g.ucla.edu (Partner: skrkasharian@ucla.edu)
//ID: 704560697 (Partner: 104584663)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

#include "ext2_kampe.h"

int image_fd;

int main(int argc, char** argv)
{

  // read and open image file
  char* filename;

  if (argc == 2)
    {
      filename = malloc(sizeof(argv[1]));
      filename = argv[1];
    }
  else 
    {
      if (argc > 2) fprintf(stderr, "ERROR: Too many arguments passed to function\n");
      else fprintf(stderr, "ERROR: No argument passed to function\n");
      exit(1);
    }

  image_fd = open(filename,O_RDONLY);
  fprintf(stdout, "image file descriptor: %i\n", image_fd);

  /****************************/
  /******  SUPERBLOCK  ********/
  /****************************/

  // variables to print out
  double SUP_tot_num_blocks = -1;               // 2
  double SUP_tot_num_inodes = -1;               // 3
  double SUP_block_size = -1;                   // 4
  double SUP_inode_size = -1;                   // 5
  double SUP_blocks_per_group = -1;             // 6
  double SUP_inodes_per_group = -1;             // 7
  double SUP_non_res_inode = -1;                // 8

  // collect information

  // output
  fprintf(stdout, "SUPERBLOCK,%f,%f,%f,%f,%f,%f,%f\n", SUP_tot_num_blocks, SUP_tot_num_inodes, SUP_block_size, SUP_inode_s
  /****************************/
  /*********  GROUP  **********/
  /****************************/

  // variables to print out
  double GRP_group_number = -1;                  // 2
  double GRP_tot_num_blocks_in_group = -1;       // 3
  double GRP_tot_num_inodes_in_group = -1;       // 4
  double GRP_num_free_blocks = -1;               // 5
  double GRP_num_free_inodes = -1;               // 6
  double GRP_block_num_free_block_bitmap = -1;   // 7
  double GRP_block_num_free_inode_bitmap = -1;   // 8
  double GRP_block_num_first_block_inode = -1;   // 9

  // collect information

  // output
  fprintf(stdout, "GROUP,%f,%f,%f,%f,%f,%f,%f,%f\n", GRP_group_number, GRP_tot_num_blocks_in_group, GRP_tot_num_inodes_in_group, GRP_num_free_blocks, GRP_num_free_inodes, GRP_block_num_free_block_bitmap, GRP_block_num_free_inode_bitmap, GRP_block_num_first_block_inode);

  /****************************/
  /***  FREE BLOCK ENTRIES  ***/
  /****************************/

  // variables to print out
  double FBE_free_block_num = -1;

  // collect information

  // output
  fprintf(stdout, "BFREE,%f\n",FBE_free_block_num);

  /****************************/
  /***  FREE INODE ENTRIES  ***/
  /****************************/

  // variables to print out
  double FIE_free_inode_num = -1;

  // collect information

  // output
  fprintf(stdout, "IFREE,%f\n", FIE_free_inode_num);

  /****************************/
  /*****  INODE SUMMARY  ******/
  /****************************/

  // variables to print out
  double ISM_inode_num = -1;          // 2
  char ISM_file_type = '?';           // 3
  //mode (octal)                      // 4
  double ISM_owner = -1;              // 5
  double ISM_group = -1;              // 6
  double ISM_link_count = -1;         // 7
  char ISM_creation_time[19]="";      // 8
  char ISM_modification_time[19]="";  // 9
  char ISM_time_last_access[19]="";   // 10
  double ISM_file_size = -1;          // 11
  double ISM_num_blocks = -1;         // 12

  // collect information

  // output
  fprintf(stdout, "INODE,%f,%c,%f,%f,%f,%s,%s,%s,%f,%f\n", ISM_inode_num, ISM_file_type, ISM_owner, ISM_link_count, ISM_creation_time, ISM_modification_time, ISM_time_last_access, ISM_file_size, ISM_num_blocks);

  /****************************/
  /***  DIRECTORY ENTRIES  ****/
  /****************************/

  // variables to print out
  double DIR_parent_inode_num = -1;   // 2
  double DIR_byte_offset = -1;        // 3
  double DIR_inode_num = -1;          // 4
  double DIR_entry_length = -1;       // 5
  double DIR_name_length = -1;        // 6
  char *DIR_name;                     // 7       // MALLOC LATER!

  // collect information

  // output
  fprintf(stdout, "DIRENT,%f,%f,%f,%f,%f,%f,%s\n", DIR_parent_inode_num, DIR_byte_offset, DIR_inode_num, DIR_entry_length, DIR_name_length, DIR_name);

  /****************************/
  /* INDIRECT BLOCK REFERENCES*/
  /****************************/

  // variables to print out

  // collect information

  // output
  fprintf(stdout, "INDIRECT,\n");


  return 0;

}
