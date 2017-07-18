//NAME: Elena Escalas (Partner: Susan Krkasharian)
//EMAIL: elenaescalas@g.ucla.edu (Partner: skrkasharian@yahoo.com)
//ID: 704560697 (Partner: 104584663)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#include "ext2_kampe.h"

#define SUPER_BLOCK_OFFSET 1024

int image_fd;
int num_ptrs;
int block_size;
int blocks_count;
int inodes_count;
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
void print_free_inode_entries(char* free_bitmap, int num_inodes_in_group);
int get_num_inodes_in_group(int group_number);

// inode summary
void print_inode_summary(struct ext2_group_desc gdesc, char* free_inode_bitmap, int group_num);
struct ext2_inode get_inode(int x, int group_num);
char file_type(__u16 mode);
void print_time_string(time_t time);

// directory entries
void print_directory_entries(struct ext2_inode inode, int inode_number);

// indirect block references
void print_indirect_block_references(struct ext2_group_desc gdesc, char* free_inode_bitmap, int group_num);
int print_indirect_block(int inode_num, int offset, int level, int file_offset);

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
  if (image_fd < 0)
    {
      fprintf(stderr, "ERROR: Invalid filename\n");
      exit(1);
    }

  struct ext2_super_block superblock = get_super_block();
  block_size = get_block_size(superblock);
  num_ptrs = block_size/4;
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
  char* block_bitmap = (char*) malloc(block_size);
  char* inode_bitmap = (char*) malloc(block_size);
  for (group_num = 0; group_num < num_groups; group_num++)
    {
      pread(image_fd, &group_descriptor, sizeof(group_descriptor), (block_size*2)+(group_num*32));
      
      blocks_in_group = get_num_blocks_in_group(group_num);
      //fprintf(stdout, "inodes in group: %i\n", inodes_in_group);

      pread(image_fd, block_bitmap, blocks_in_group, block_size*group_descriptor.bg_block_bitmap);
      
      pread(image_fd, inode_bitmap, inodes_per_group, block_size*group_descriptor.bg_inode_bitmap);

      print_group_summary(superblock, group_descriptor, group_num);
      
      if (group_num == 0) block_offset_from_backups = 0;
      else block_offset_from_backups = 2;
      
      print_free_block_entries(block_bitmap,blocks_in_group);

      print_free_inode_entries(inode_bitmap, inodes_per_group);

      print_inode_summary(group_descriptor, inode_bitmap, group_num);

      print_indirect_block_references(group_descriptor, inode_bitmap, group_num);
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
  
  return 1 + (sblock.s_blocks_count - 1)/sblock.s_blocks_per_group;
}

int get_num_inodes(struct ext2_super_block sblock)
{
  return 1 + (sblock.s_inodes_count - 1)/sblock.s_inodes_per_group;
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


void print_free_inode_entries(char* free_bitmap, int num_inodes_in_group)
{
  int bitmap_bit = 0;
  int free = 1;
  int n = 0;
  int m;
  int current_inode_num = 1;
  while (current_inode_num < num_inodes_in_group)
    {
      bitmap_bit = free_bitmap[n];
      //fprintf(stdout, "current inode number: %i\n", current_inode_num);

      for(m = 0; m < 8; m++)
        {
          if (bitmap_bit & 0x01)
            free = 0;
          else free = 1;

	  if (free)
            fprintf(stdout,"IFREE,%i\n",current_inode_num);

	  //fprintf(stdout,"%i",free);

          bitmap_bit = bitmap_bit >> 1;
          current_inode_num++;
        }

      n++;
      //fprintf(stdout, "\n");
    }
}


void print_inode_summary(struct ext2_group_desc gdesc, char* free_inode_bitmap, int group_num)
{
  struct ext2_inode inode;
  time_t ctime, mtime, atime;
  int bitmap_bit = 0;
  int i = 0;
  int inode_num = 0;
  char ftype;
  int n;
  while(inode_num < inodes_per_group)
    {
      bitmap_bit = free_inode_bitmap[i];
      
      for (n = 0; n < 8; n++)
	{
	  if (bitmap_bit & 1) 
	    {
	      inode = get_inode(inode_num+1,group_num);
	      if ((inode.i_mode != 0) && (inode.i_links_count != 0))
		{
		  ftype = file_type(inode.i_mode);
		  
		  fprintf(stdout, "INODE,%i,", inode_num+2); // inode number
		  fprintf(stdout, "%c,", ftype); //file type
		  fprintf(stdout,"%o,", (inode.i_mode & 0x0fff)); //mode
		  fprintf(stdout,"%i,%i,", inode.i_uid, inode.i_gid); // ownder, group
		  fprintf(stdout, "%i,", inode.i_links_count); //link count
		  ctime = inode.i_ctime;
		  mtime = inode.i_mtime;
		  atime = inode.i_atime;
		  print_time_string(ctime);
		  print_time_string(mtime);
		  print_time_string(atime);
		  fprintf(stdout, "%i,", inode.i_size); //file size
		  fprintf(stdout, "%i,", inode.i_blocks); // number of blocks
		  
		  // print block addresses
		  int n;
		  for (n = 0; n < EXT2_N_BLOCKS; n++)
		    {
		      if (n == EXT2_N_BLOCKS - 1) fprintf(stdout, "%i\n", inode.i_block[n]);
		      else fprintf(stdout, "%i,", inode.i_block[n]);
		    }			
		  
	 	  
		  if (ftype == 'd')
		    print_directory_entries(inode,inode_num+2);
		  
		}
	    }
	  
	  bitmap_bit = bitmap_bit >> 1;
	  inode_num++;
	}
      i++;
    }
}


void print_time_string(time_t time)
{
  char time_string[18];
  struct tm * time_info = gmtime(&time);
  strftime(time_string,18,"%D %T", time_info);
  fprintf(stdout, "%s,", time_string);
}


struct ext2_inode get_inode(int x, int group_num)
{
  struct ext2_inode inode;
  int inode_table_block = 5;
  int bytes_per_inode = 128;
  int group0_starting_block = 2;
  int inode_table_offset = 3;
  int backups_offset = 2;
  if (group_num > 1)
    inode_table_block = group0_starting_block + blocks_per_group + backups_offset + (blocks_per_group*group_num); 
  pread(image_fd, &inode, sizeof(inode),((inode_table_block*block_size)+(x*bytes_per_inode)));
  return inode;
}

char file_type(__u16 mode)
{
  if ((mode & 0xA000) == 0xA000)
    return 's';
  else if ((mode & 0x8000) == 0x8000)
    return 'f';
  else if ((mode & 0x4000) == 0x4000)
    return 'd';
  else return '?';
}

void print_directory_entries(struct ext2_inode inode, int inode_number)
{
  
  __u32 cur_block = 0;
  int directories_present = 0;
  int cur_offset;
  int offset_within_dir = 0;
  struct ext2_dir_entry dir_entry;
  //    fprintf(stdout, "m = %i - current block: %i\n", m, inode.i_block[m]);
  offset_within_dir = 0;
  cur_block = inode.i_block[0];
  
  cur_offset = block_size*cur_block;
  directories_present = 1;
  while(offset_within_dir < block_size)
    {	      
      pread(image_fd, &dir_entry, sizeof(dir_entry), cur_offset);
      
      if (dir_entry.name_len > 256) fprintf(stdout, "LONG NAME ALERT\n");
      fprintf(stdout, "DIRENT,%i,", inode_number);
      fprintf(stdout, "%i,",offset_within_dir);      // 3. byte offset of this entry within the directory
      fprintf(stdout, "%i,",dir_entry.inode);        // 4. inode number of the referenced file
      fprintf(stdout, "%i,", dir_entry.rec_len);     // 5. entry length
      fprintf(stdout, "%i,", dir_entry.name_len);    // 6. name length
      fprintf(stdout, "'%s'\n", dir_entry.name);     // 7. name 
      
      cur_offset += dir_entry.rec_len;
      offset_within_dir += dir_entry.rec_len;

      
      //if (dir_entry.rec_len > block_size)
      //continue;
      //      if (offset_within_dir >= dir_entry.rec_len*block_size)
      //directories_present = 0;
      //fprintf(stdout, "offset within dir: %i - block size: %i\n", offset_within_dir, block_size);
      //if (offset_within_dir >= block_size)
      //{	
      //directories_present = 0;	  
      //}
    }
}



void print_indirect_block_references(struct ext2_group_desc gdesc, char* free_inode_bitmap, int group_num)
{
  struct ext2_inode inode;
  time_t ctime, mtime, atime;
  int bitmap_bit = 0;
  int i = 0;
  int inode_num = 0;
  char ftype;
  int n;
  while(inode_num < inodes_per_group)
    {
      bitmap_bit = free_inode_bitmap[i];
      
      for (n = 0; n < 8; n++)
        {
          if (bitmap_bit & 1)
            {
              inode = get_inode(inode_num+1,group_num);
              if ((inode.i_mode != 0) && (inode.i_links_count != 0))
                {
                  ftype = file_type(inode.i_mode);
		  
                  int n;
                  for (n = 0; n < EXT2_N_BLOCKS; n++)
                    {
                      if (inode.i_block[12] != 0 && n == 12)
			{
			  
			  print_indirect_block(inode_num+2,inode.i_block[n]*block_size, 1, n);
			  
			  /*int p;
			    for (p = 0; p < 256; p++)
                            {
			    pread(image_fd, &value_in_block, sizeof(value_in_block), (inode.i_block[n]*block_size)+(4*p));
			    fprintf(stdout, "Value in indirect inode table w/ 'offset %i': %i\n", 12+p, value_in_block);
			    }*/
			}
		      else if (inode.i_block[13] != 0 && n == 13)
			{
			  //fprintf(stdout, "INODE number: %i, has level of indirection 2\n", inode_num+2);  

			  print_indirect_block(inode_num+2, inode.i_block[n]*block_size, 2, 12+num_ptrs);
			}
		      else if (inode.i_block[14] != 0 && n == 14)
			{
			  //fprintf(stdout, "INODE number: %i, has level of indirection 3\n", inode_num+2);
			  print_indirect_block(inode_num+2, inode.i_block[n]*block_size, 3, 12+(num_ptrs*num_ptrs));
			}
		      
		    }	  
		}
	    }
	  bitmap_bit = bitmap_bit >> 1;
	  inode_num++;
	}
      i++;
    }
}

int print_indirect_block(int inode_num, int offset, int level, int file_offset)
{
  int in_level1_table = 1;
  __u32 value_in_block = 0;
  int next_ofset = 0;
  if (level == 1)
    {
      while (in_level1_table)
	{
	  pread(image_fd, &value_in_block, sizeof(value_in_block), offset);
	  
	  if (value_in_block == 0)
	    {
	      offset += 4;
	      file_offset++;
	      continue;
	    }
	  if (file_offset >= num_ptrs+12) 
	    {
	      in_level1_table = 0;
	      break;
	    }

	  fprintf(stdout, "INDIRECT,%i,", inode_num);
	  fprintf(stdout,"%i,", level);
	  fprintf(stdout, "%i,", file_offset);
	  fprintf(stdout, "%i,", offset/block_size);
	  fprintf(stdout,"%i\n", value_in_block);

	  offset += 4;
	  file_offset++;
	}
      return 1;
    }

  int in_table = 1;
  int end_of_table;
  int to_next_file_offset;
  int to_next_offset;
  int taken_block = 0;
  while(in_table)
    {
      pread(image_fd, &value_in_block, sizeof(value_in_block), offset);
      //fprintf(stdout, "Value in indirect inode table w/ 'offset 14': %i\n", value_in_block);

      if (level == 2)
	{
	  //fprintf(stdout, "In inode %i, in level 2 the value in block %i is %i\n", inode_num,file_offset,value_in_block);

	  end_of_table = 12 + num_ptrs + (num_ptrs*num_ptrs);
	  to_next_offset = num_ptrs*4;
	  to_next_file_offset = num_ptrs;
	}

      if (level == 3)
	{	
	  //fprintf(stdout, "In inode %i, in level 3 the value in block %i is %i\n", inode_num,file_offset,value_in_block);
	  // long long val_in_block;
	  //pread(image_fd, &val_in_block, sizeof(val_in_block), value_in_block*block_size);

	  end_of_table = 12 + num_ptrs + (num_ptrs*num_ptrs) + (num_ptrs*num_ptrs*num_ptrs);
	  to_next_offset = num_ptrs*num_ptrs*4;
	  to_next_file_offset = num_ptrs*num_ptrs;
      	}

      if (file_offset >= end_of_table) 
	{
	  in_table = 0;
	  return 0;
	}
      
      if(print_indirect_block(inode_num, value_in_block*block_size, level-1, file_offset))
	{
	  fprintf(stdout, "INDIRECT,%i,", inode_num);
	  fprintf(stdout, "%i,", level);
	  fprintf(stdout, "%i,", file_offset);
	  fprintf(stdout, "%i,", offset/block_size);
	  fprintf(stdout, "%i\n", value_in_block);
	  return 1;
	}

      //print_indirect_block(inode_num,value_in_block*block_size,level-1,file_offset);

      offset += to_next_offset;
      file_offset += to_next_file_offset; 

      //print_indirect_block(inode_num, value_in_block*block_size, level-1, file_offset);
    }
}
