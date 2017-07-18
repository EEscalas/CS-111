#NAME: Elena Escalas, Susan Krkasharian
#EMAIL: elenaescalas@g.ucla.edu, skrkasharian@yahoo.com
#ID: 704560697, 104584663

import sys
import csv

found_error = False

class Superblock:
    def __init__(self,numBlocks,numInodes,blockSize,inodeSize,blocksPerGroup):
        self.num_blocks = numBlocks
        self.num_inodes = numInodes
        self.block_size = blockSize
        self.inode_size = inodeSize
        self.blocks_per_group = blocksPerGroup

class Group:
    def __init__(self,groupNum,blocksInGroup,inodesInGroup,numFreeBlocks,numFreeInodes):
        self.group_num = groupNum
        self.blocks_in_group = blocksInGroup
        self.inodes_in_group = inodesInGroup
        self.num_free_blocks = numFreeBlocks
        self.num_free_inodes = numFreeInodes

class Inode:
    def __init__(self,inodeNumber,Mode,linkCount):
        self.inode_number = inodeNumber
        self.mode = Mode
        self.link_count = linkCount
        self.pointers = dict()

class DirectoryEntry:
    def __init__(self,parentInodeNum,InodeNumOfRef,Name):
        self.parent_inode_num = parentInodeNum
        self.inode_num_of_ref = InodeNumOfRef
        self.name = Name

class IndirectEntry:
    def __init__(self,InodeOfOwning,Level,FileOffset,BlockNumberScanned,BlockNumReferenced):
        self.inode_of_owning = InodeOfOwning
        self.level = Level
        self.file_offset = FileOffset
        self.block_num_scanned = BlockNumberScanned
        self.block_num_referenced = BlockNumReferenced

class UsedBlockInfo:
    def __init__(self,Level,inode,Offset):
        self.inode_num = inode
        self.level = Level
        self.offset = Offset


free_blocks = list()
free_inodes = list()

groups = list()
inodes = list()
dir_entries = list()
indir_entries = list()
double_indir_entries = list()
triple_indir_entries = list()

from collections import defaultdict
used_block_dict = defaultdict(list)

block_dict = {'block_num':'STATE'}

filename=sys.argv[1]

with open(filename) as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        for column in row:
            if column == "SUPERBLOCK":
                super_block = Superblock(row[1], row[2], row[3], row[4], row[5]) 
            if column == "GROUP":
                group = Group(row[1], row[2], row[3],row[4],row[5])
                groups.append(group)
            if column == "BFREE":
                free_blocks.append(row[1])
            if column == "IFREE":
                free_inodes.append(row[1])
            if column == "INODE":
                inode = Inode(row[1],row[4],row[6])
                inode.pointers = {'0':row[12],'1':row[13],'2':row[14],'3':row[15],'4':row[16],'5':row[17],'6':row[18],'7':row[19],'8':row[20],'9':row[21],'10':row[22],'11':row[23],'12':row[24],'13':row[25],'14':row[26]}
                inodes.append(inode)
            if column == "DIRENT":
                dir_entry = DirectoryEntry(row[1],row[3],row[6])
                dir_entries.append(dir_entry)
            if column == "INDIRECT":
                indir_entry = IndirectEntry(row[1],row[2],row[3],row[4],row[5])
                entry_level = int(row[2])
                if (entry_level == 1):
                    indir_entries.append(indir_entry)
                elif(entry_level == 2):
                    double_indir_entries.append(indir_entry)
                elif(entry_level == 3):
                    triple_indir_entries.append(indir_entry)

num_blocks_per_group = int(super_block.blocks_per_group)


def isMetadataBlock(bn):
    b = int(bn)
    if (b < 8):
        return True
    elif (b % num_blocks_per_group < 8):
        return True
    else:
        return False

def isDataBlock(bn):
    b = int(bn)
    if (isValidBlock(bn) == False):
        return False
    #check if bn is superblock, block group desc talbe, block bitmap, inode bitmap or inode table
    if isMetadataBlock(bn):
        return False
    return True

def isFreeBlock(bn):
    if (isDataBlock(bn) == False):
        return False
    if str(bn) in free_blocks:
        return True
    else: 
        return False
        
def isFreeInode(inode_num):
    if str(inode_num) in free_inodes:
        return True
    else:
        return False

def isValidBlock(bn):
    if ((int(bn) < 0) or (int(bn) > num_blocks)):
        return False
    else:
        return True

def isReservedBlock(bn):
    if bn in block_dict.keys():
        if (block_dict[bn] == 'RESERVED'):
            return True
        else:
            return False
    else:
        return False

def isBlockUsed(bn):
    if bn in block_dict.keys():
        if (block_dict[bn] == 'RESERVED' or block_dict[bn] == 'FREE' or block_dict[bn] == 'USED'):
            return False
    return True

def isDuplicate(bn):
    if bn in block_dict.keys():
        if (block_dict[bn] == 'USED'):
            return True
    return False

def printDuplicateBlocks(bn):
    if bn in used_block_dict.keys():
        location = 'BLOCK'
        for info in used_block_dict[bn]:
            if(info.level == 1):
                location = 'INDIRECT BLOCK'
            if(info.level == 2):
                location = 'DOUBLE INDIRECT BLOCK'
            if(info.level == 3):
                location = 'TRIPPLE INDIRECT BLOCK'
            print 'DUPLICATE', location, bn, 'IN INODE', info.inode_num, 'AT OFFSET', info.offset
        

def checkIfCorruptedBlockNumber(inode_num,offset,bn,level):
    if (int(level) == 0):
        if(isValidBlock(bn) == False):
            print 'INVALID BLOCK', bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            found_error = True
        elif (isDuplicate(bn) == True):
            print 'DUPLICATE BLOCK', bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            printDuplicateBlocks(bn)
            found_error = True
        elif (isFreeBlock(bn) == True):
            print 'ALLOCATED BLOCK', bn, 'ON FREELIST'
            found_error = True
        elif (isReservedBlock(bn) == True):
            print 'RESERVED BLOCK', bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            found_error = True
        elif (isBlockUsed(bn) == True):
            block_dict[bn] = 'USED'
            used_block = UsedBlockInfo(level,inode_num,offset)
            used_block_dict[bn].append(used_block)
    level_num = int(level)
    if (level_num > 0):
        location = 'INDIRECT BLOCK'
        if (level_num == 2):
            location = 'DOUBLE INDIRECT BLOCK'
        if (level_num == 3):
            location = 'TRIPPLE INDIRECT BLOCK'
        if (isValidBlock(bn) == False):
            print 'INVALID', location, bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            found_error = True
            return False
        elif (isReservedBlock(bn) == True):
            print 'RESERVED', location, bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            found_error = True
            return False
        elif(isDuplicate(bn) == True):
            print 'DUPLICATE', location, bn, 'IN INODE', inode_num, 'AT OFFSET', offset
            printDuplicateBlocks(bn)
            found_error = True
            return False
        else:
            block_dict[bn] = 'USED'
            used_indir_block = UsedBlockInfo(level,inode_num,offset)
            used_block_dict[bn].append(used_indir_block)
            return True

num_blocks = int(super_block.num_blocks)
num_inodes = int(super_block.num_inodes)
small_blocks = int(super_block.block_size)/4

for b in range(0, num_blocks):
    if (isDataBlock(b) == False):
        block_dict[str(b)] = 'RESERVED'
    if isFreeBlock(b):
        block_dict[str(b)] = 'FREE'

del block_dict['block_num']

triple_indir = 14
double_indir = 13
indir = 12

indirect_valid = True
double_indirect_valid = True
triple_indirect_valid = True

for i in inodes:
    if (i.mode == 0) or (i.link_count == 0):
        continue
    for ptr in i.pointers:
        value_in_block = i.pointers[ptr]
        if (int(ptr) < 12):
            if (int(value_in_block) != 0):
                checkIfCorruptedBlockNumber(i.inode_number,ptr,value_in_block,0)
        elif (int(ptr) == indir): # indirect block pointer
            if (int(i.pointers[ptr]) != 0):
                if(checkIfCorruptedBlockNumber(i.inode_number,ptr,value_in_block,1) == False):
                    indirect_valid = False
                    continue                
                for entry in indir_entries:
                    if (i.inode_number == entry.inode_of_owning):
                        checkIfCorruptedBlockNumber(i.inode_number,entry.file_offset,entry.block_num_referenced,0)
        elif(int(ptr) == double_indir): # doubly indirect block pointer
            if (int(i.pointers[ptr]) != 0):
                if(checkIfCorruptedBlockNumber(i.inode_number,str(small_blocks+12),value_in_block,2) == False):
                    double_indirect_valid = False
                    continue
                for d_entry in double_indir_entries:
                    if (i.inode_number == d_entry.inode_of_owning):
                        checkIfCorruptedBlockNumber(i.inode_number,d_entry.file_offset,d_entry.block_num_referenced,1)
                if (int(i.pointers[12]) == 0 or indirect_valid == False):
                    for entry in indir_entires:
                        if (i.inode_number == entry.inode_of_owning):
                            checkIfCorruptedBlockNumber(i.inode_number,entry.file_offset,entry.block_num_referenced,0)
        elif (int(ptr) == triple_indir): # triple indirect block pointer
            triple_offset = small_blocks + 12 + (small_blocks*small_blocks)
            if (int(i.pointers[ptr]) != 0):
                if(checkIfCorruptedBlockNumber(i.inode_number,str(triple_offset),value_in_block,3) == False):
                    continue
                for t_entry in triple_indir_entries:
                    if ((t_entry.inode_of_owning == i.inode_number)):
                        checkIfCorruptedBlockNumber(i.inode_number,t_entry.file_offset,t_entry.block_num_referenced,2)
                if (int(i.pointers[double_indir]) == 0 or double_indirect_valid == False):
                    for d_entry in double_indir_entries:
                        if (i.inode_number == d_entry.inode_of_owning):
                            checkIfCorruptedBlockNumber(i.inode_number,d_entry.file_offset,d_entry.block_num_referenced,1)
                    if (int(i.pointers[indir]) == 0 or indirect_valid == False):
                        for entry in indir_entires:
                            if (i.inode_number == entry.inode_of_owning):
                                checkIfCorruptedBlockNumber(i.inode_number,entry.file_offset,entry.block_num_referenced,0)
                if ((int(i.pointers[double_indir]) == 0 or double_indirect_valid == False) and (i.pointers[indir] == 0 or indirect_valid == False)):
                    for entry in indir_entires:
                        if (i.inode_number == entry.inode_of_owning):
                            checkIfCorruptedBlockNumber(i.inode_number,entry.file_offset,entry.block_num_referenced,0)
                            


for b in range(0,num_blocks):
    if str(b) not in block_dict.keys():
        print 'UNREFERENCED BLOCK', b
        found_error = True

inode_dict = {'inode_num':'STATE'}
allocated_dict = list()
linkcount = {'inode_num':int(0)}

for i in inodes: #should be allocated
    if i.inode_number in free_inodes:
        print 'ALLOCATED INODE', i.inode_number, 'ON FREELIST'
        found_error = True
    else:
        inode_dict[i.inode_number] = 'USED'
        allocated_dict.append(i.inode_number)

for n in free_inodes:
    inode_dict[n] = 'FREE'
    

for x in range(11,num_inodes):
    if str(x) not in inode_dict.keys():
        print 'UNALLOCATED INODE', x, 'NOT ON FREELIST'
        found_error = True

for inode in allocated_dict:
    linkcount[inode] = int(0)

del linkcount['inode_num']

parent_dict = {'sub_dir':'parent_dir'}

del parent_dict['sub_dir']

for de in dir_entries:
    if (int(de.inode_num_of_ref) > num_inodes):
        print 'DIRECTORY INODE', de.parent_inode_num, 'NAME', de.name, 'INVALID INODE', de.inode_num_of_ref
        found_error = True
    elif (de.inode_num_of_ref not in allocated_dict):
        if (int(de.inode_num_of_ref) > 10):
            print 'DIRECTORY INODE', de.parent_inode_num, 'NAME', de.name, 'UNALLOCATED INODE', de.inode_num_of_ref
            found_error = True
    else:
        linkcount[de.inode_num_of_ref]+=1
        if (de.inode_num_of_ref != de.parent_inode_num):
            if (de.parent_inode_num != 2): # root inode
                parent_dict[de.inode_num_of_ref] = de.parent_inode_num

for de in dir_entries:
    if (de.name == "'.'"):
        if (de.inode_num_of_ref != de.parent_inode_num):
            print 'DIRECTORY INODE', de.parent_inode_num, 'NAME',de.name,'LINK TO INODE',de.inode_num_of_ref,'SHOULD BE',de.parent_inode_num
            found_error = True
    if (de.name == "'..'"):
        if ((int(de.parent_inode_num) == 2) and (de.parent_inode_num != de.inode_num_of_ref)):
            print 'DIRECTORY INODE', de.parent_inode_num, 'NAME', de.name, 'LINK TO INODE', de.inode_num_of_ref, 'SHOULD BE', de.parent_inode_num
            found_error = True
        #elif (parent_dict[de.parent_inode_num] != de.inode_num_of_ref):
            #print 'DIRECTORY INODE', de.parent_inode_num, 'NAME', de.name, 'LINK TO INODE', parent_dict[de.parent_inode_num], 'SHOULD BE',de.inode_num_of_ref
            #found_error = True
            #print '\'.\'\n', 'DE.NAME:', de.name
                    
for inode in inodes:
    if inode.inode_number in linkcount.keys():
        if (linkcount[inode.inode_number] != int(inode.link_count)):
            print 'INODE', inode.inode_number,'HAS', linkcount[inode.inode_number], 'LINKS BUT LINKCOUNT IS', inode.link_count
            found_error = True        
        






if (found_error):
    sys.exit(2)

#print de.inode_num_of_ref
        
#from pprint import pprint    

#pprint(block_dict)
