//NAME: Elena Escalas (Partner: Susan Krkasharian)
//EMAIL: elenaescalas@g.ucla.edu (Partner: skrkasharian@ucla.edu)
//ID: 704560697 (Partner: 104584663)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

#include "ext2_kampe.h"

#define SUPER_BLOCK_OFFSET 1024
#define GROUP0_BLOCK_OFFSET 2
#define NUM_GROUP_METADATA_BLOCKS 4

int image_fd;
int block_size;
int blocks_count;
int num_groups;
int num_inodes;
int inodes_per_group;
int blocks_per_group;

// superblock 
struct ext2_super_block get_super_block();
void print_super_block_summary(struct ext2_super_block sblock);
int get_block_size(struct ext2_super_block sblock);

// group
void print_group_summary(struct ext2_super_block sblock, struct ext2_group_desc gdesc, int i);
int get_num_groups(struct ext2_super_block sblock);
int get_num_inodes(struct ext2_super_block sblock);
int get_num_blocks_in_group(int group_number);

// free block entries
void print_free_block_entries(char* free_bitmap, int num_blocks_in_group);

// free inode entries
void print_free_inode_entries();

// inode summary
void print_inode_summary();

// directory entries
void print_directory_entries();

// indirect block references
void print_indirect_block_references();

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

  struct ext2_super_block superblock = get_super_block();
  block_size = get_block_size(superblock);
  print_super_block_summary(superblock);

  blocks_count = superblock.s_blocks_count;
  blocks_per_group = superblock.s_blocks_per_group;
  inodes_per_group = superblock.s_inodes_per_group;
  num_groups = get_num_groups(superblock);
  num_inodes = get_num_inodes(superblock);

  struct ext2_group_desc group_descriptor;
  int group_num;
  int group_size;
  int blocks_in_group;
  int data_block_num;
  int block_offset_from_backups;
  char* bitmap = (char*) malloc(block_size);
  for (group_num = 0; group_num < num_groups; group_num++)
    {
      pread(image_fd, &group_descriptor, sizeof(group_descriptor), (block_size*2)+(group_num*32));
      
      blocks_in_group = get_num_blocks_in_group(group_num);

      pread(image_fd, bitmap, blocks_in_group, block_size*group_descriptor.bg_block_bitmap);

      print_group_summary(superblock, group_descriptor, group_num);
      
      if (group_num == 0) block_offset_from_backups = 0;
      else block_offset_from_backups = 2;
      
      print_free_block_entries(bitmap,blocks_in_group);

    }

  return 0;

}

struct ext2_super_block get_super_block()
{
  struct ext2_super_block super_block;
  pread(image_fd, &super_block, sizeof(super_block), SUPER_BLOCK_OFFSET);
  return super_block;
}

void print_super_block_summary(struct ext2_super_block sblock)
{
  fprintf(stdout, "SUPERBLOCK,");                    // 1. SUPERBLOCK
  fprintf(stdout, "%i,", sblock.s_blocks_count);     // 2. total number of blocks 
  fprintf(stdout, "%i,", sblock.s_inodes_count);     // 3. total number of i-nodes
  fprintf(stdout, "%i,", block_size);                // 4. block size
  fprintf(stdout, "%i,", sblock.s_inode_size);       // 5. inode size
  fprintf(stdout, "%i,", sblock.s_blocks_per_group); // 6. blocks per group
  fprintf(stdout, "%i,", sblock.s_inodes_per_group);        // 7. inodes per group
  fprintf(stdout, "%i\n", sblock.s_first_ino);              // 8. first non-reserved inode
}

int get_block_size(struct ext2_super_block sblock)
{
  return 1024 << sblock.s_log_block_size;
} 

int get_num_groups(struct ext2_super_block sblock)
{
  //fprintf(stdout, "block count: %i, blocks per group: %i\n",sblock.s_blocks_count, sblock.s_blocks_per_group);
  
  int number_of_groups = (sblock.s_blocks_count)/(sblock.s_blocks_per_group);
  if (sblock.s_blocks_count < sblock.s_blocks_per_group)
      number_of_groups = 1;
  return number_of_groups;
}

int get_num_inodes(struct ext2_super_block sblock)
{
  int number_of_inodes = (sblock.s_inodes_count)/(sblock.s_inodes_per_group);
  if (sblock.s_inodes_count < sblock.s_inodes_per_group)
    number_of_inodes = 1;
  return number_of_inodes;
}

int get_num_blocks_in_group (int group_num)
{
  if (num_groups > 1 && (group_num != num_groups - 1))
    {
      return blocks_per_group;
    }
  else return blocks_count % blocks_per_group;
}

 void print_group_summary(struct ext2_super_block sblock, struct ext2_group_desc gdesc, int i)
{
  int num_blocks_in_group;

  pread(image_fd, &gdesc, sizeof(gdesc), (block_size*2)+(i*32));

  fprintf(stdout,"GROUP,%i,",i);
  
  // 3. number of blocks in this group
  if (num_groups > 1 && (i != num_groups-1))
    num_blocks_in_group = sblock.s_blocks_per_group;
  else num_blocks_in_group = sblock.s_blocks_count % sblock.s_blocks_per_group;
  fprintf(stdout, "%i,", num_blocks_in_group);
  
  // 4. total number of inodes in this group
  fprintf(stdout, "%i,", inodes_per_group);
  
  // 5. number of free blocks
  fprintf(stdout, "%i,", gdesc.bg_free_blocks_count);
  
  // 6. number of free inodes
  fprintf(stdout, "%i,", gdesc.bg_free_inodes_count);

  // 7. block number of free block bitmap for this group
  fprintf(stdout, "%i,", gdesc.bg_block_bitmap);
  
  // 8. block number of free i-node bitmap for this group
  fprintf(stdout, "%i,", gdesc.bg_inode_bitmap);
  
  // 9. block number of first block of i-nodes in this group
  fprintf(stdout, "%i\n", gdesc.bg_inode_table);

}

void print_free_block_entries(char* free_bitmap, int num_blocks_in_group)
{
  int bitmap_bit = 0;
  int free = 1;
  int n = 0;
  int m;
  int current_block_num = 1;
  while (current_block_num < num_blocks_in_group)
    {
      bitmap_bit = free_bitmap[n];
      //fprintf(stdout, "current block number: %i\n", current_block_num);
      
      for(m = 0; m < 8; m++)
	{
	  if (bitmap_bit & 0x01)
	    free = 0;
	  else free = 1;
	  
	   if (free)
	    fprintf(stdout,"BFREE,%i\n",current_block_num);
	  
	   //fprintf(stdout,"%i",free);

	  bitmap_bit = bitmap_bit >> 1;
	  current_block_num++;
	}

      n++;
      //fprintf(stdout, "\n");
    }
}

void print_free_inode_entries();
void print_inode_summary();
void print_directory_entries();
void print_indirect_block_references();
