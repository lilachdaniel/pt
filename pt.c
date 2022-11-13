#include "os.h"
#include <math.h>
#include <stdio.h>



int find_entry(int l, uint64_t vpn);
int is_empty(uint64_t *node);

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
	int entry;
	uint64_t *curr_node;
	uint64_t curr_node_ppn = (pt << 12) + 1;
	
	for (int l = 0; l < 5; ++l) {
		if ((curr_node_ppn & 1) == 0) {
			return NO_MAPPING;
		}
		
		/* update node */
		curr_node = phys_to_virt(curr_node_ppn - 1);
		
		/* find entry */
		entry = find_entry(l, vpn);
		
		/* update node ppn */
		curr_node_ppn = curr_node[entry];


	}
	
	if ((curr_node_ppn & 1) == 0) {
			return NO_MAPPING;
	}
	
	return (curr_node_ppn - 1) >> 12;
}


int find_entry(int l, uint64_t vpn){
	int entry;
	
	vpn = vpn >> 9 * (4 - l);
	entry = vpn & 0x1ff;

	return entry;
}



void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
	uint64_t curr_node_ppn = (pt << 12) + 1;
	uint64_t *curr_node, *curr_parent;
	int curr_entry;
	uint64_t parents[5];
	
	for (int l = 0; l < 5; ++l) {
		parents[l] = curr_node_ppn; /* remember parent in case we want to free nodes later */
		
		curr_node = phys_to_virt(curr_node_ppn - 1);
		curr_entry = find_entry(l, vpn);
		
		if (l == 4) { /* it's a leaf! */
			 /* destroy mapping */
			if (ppn == NO_MAPPING) { 
				if ((curr_node[curr_entry] & 1) == 1) {
					curr_node[curr_entry] = 0 ; /* address is now invalid */
				}
				for (int i = 4; i > 0; --i) { /* free page table nodes (if needed) */
					curr_node_ppn = parents[i];
					curr_node = phys_to_virt(curr_node_ppn - 1);
					
					if (is_empty(curr_node)) { /* node is empty -> free frame of node */
						free_page_frame(curr_node_ppn);
						
						/* update parent */
						curr_parent = phys_to_virt(parents[i - 1]);
						curr_entry = find_entry(i - 1, vpn);
						curr_parent[curr_entry] = 0;
					}
					
					else { /* no more nodes to free */
						return;
					}
				}
			}
			/* create mapping */
			else {
				curr_node[curr_entry] = (ppn << 12) + 1;
			}
			return;
		}	
		
		else if ((curr_node[curr_entry] & 1) == 0) {
			/* destroy mapping */
			if (ppn == NO_MAPPING) { /* no mapping to destroy */
				return;
			}
			/* create mapping */ 
			else { /* create a node */ 
				curr_node[curr_entry] = (alloc_page_frame() << 12) + 1 ;
			}		
		}
		curr_node_ppn = curr_node[curr_entry];
		
	}
	return;		
}

int is_empty(uint64_t *node) {
	for (int i = 0; i < 512; ++i) {
		if ((node[i] & 1) == 0) {
			return 0; /* not empty! */
		}
	}
	return 1; /* empty! */
}

















