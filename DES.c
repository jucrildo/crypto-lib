#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROUNDS 16
#define NUMBER_OF_S_BLOCKS 8
#define EACH_S_BLOCK_SIZE 6
#define EXPANDED_32BIT 48

#define MESSAGE_SIZE 64

#define KEY_64_SIZE 64
#define K_SUBKEY_SIZE 56
#define PC1_TABLE_SIZE 56
#define PC2_TABLE_SIZE 48

/*
typedef struct {
	uint64_t DES_block;
	uint64_t DES_key; 	// The key is actually 56 bits. The 8 bits left are for checking parity.((DES_key >> 55)&0xFF) 
} DES;

uint64_t encryption(uint64_t block, uint64_t key, int rounds) {
}

uint64_t decryption(uint64_t block, uint64_t key) {
}
*/

/* PC1 */
uint64_t pc1_permutation(uint64_t key) {
	const int pc1_table[PC1_TABLE_SIZE] = {57, 49, 41, 33, 25, 17,  9,
											1, 58, 50, 42, 34, 26, 18,
										   10,  2, 59, 51, 43, 35, 27,
										   19, 11,  3, 60, 52, 44, 36,
										   63, 55, 47, 39, 31, 23, 15,
										    7, 62, 54, 46, 38, 30, 22,
										   14,  6, 61, 53, 45, 37, 29,
										   21, 13,  5, 28, 20, 12,  4 };
	uint64_t pc1_permuted_key = 0;
	uint64_t tmp = 0;

	for (int i = 0; i < PC1_TABLE_SIZE; i++) {
		int bit_index = pc1_table[i];
		tmp = (key >> (KEY_64_SIZE - bit_index)) & 0x01;
		pc1_permuted_key = pc1_permuted_key | (tmp << (PC1_TABLE_SIZE - 1 - i));
	}

	return pc1_permuted_key;
}


/* PC2 */
uint64_t pc2_permutation(uint64_t concatenated_subkeys) {
	const int pc2_table[PC2_TABLE_SIZE] = {14, 17, 11, 24,  1,  5,
										    3, 28, 15,  6, 21, 10,
										   23, 19, 12,  4, 26,  8,
										   16,  7, 27, 20, 13,  2,
										   41, 52, 31, 37, 47, 55,
										   30, 40, 51, 45, 33, 48,
										   44, 49, 39, 56, 34, 53,
										   46, 42, 50, 36, 29, 32 };
	uint64_t pc2_permuted_key = 0;
	uint64_t bit_tmp = 0;

	for (int i = 0; i < PC2_TABLE_SIZE; i++) {
		int bit_index = pc2_table[i];
		bit_tmp = (concatenated_subkeys >> (K_SUBKEY_SIZE - bit_index)) & 0x01;
		pc2_permuted_key = pc2_permuted_key | (bit_tmp << (PC2_TABLE_SIZE - 1 - i));
	}

	return pc2_permuted_key;
}


#define HALF_KEY_SIZE 28
void create_16subkeys(uint32_t *left, uint32_t *right, int n_shifts) {
	uint32_t bit_tmp_l, bit_tmp_r = 0;
	
	if (n_shifts == 2) {
		for (int j = 1; j <= 2; j++) {
			bit_tmp_l = ((*left) >> (HALF_KEY_SIZE - 1)) & 0x01;
			bit_tmp_r = ((*right) >> (HALF_KEY_SIZE - 1)) & 0x01;
			*left  = ((*left) << 1) & 0xFFFFFFFF;
			*right = ((*right) << 1) & 0xFFFFFFFF;
			*left  |= bit_tmp_l;
			*right |= bit_tmp_r;
		}
	} else {
		bit_tmp_l = ((*left) >> (HALF_KEY_SIZE - 1)) & 0x01;
		bit_tmp_r = ((*right) >> (HALF_KEY_SIZE - 1)) & 0x01;
		*left  = ((*left) << 1) & 0xFFFFFFFF;
		*right = ((*right) << 1) & 0xFFFFFFFF;
		*left  |= bit_tmp_l;
		*right |= bit_tmp_r;
	}
}


uint8_t invert_8bits(uint8_t s_block) {
	uint8_t new_block = 0;
	for (int i = 0; i < EACH_S_BLOCK_SIZE; i++) {
		new_block |= ((s_block & (1 << i)) >> i) << ((EACH_S_BLOCK_SIZE - 1) - i);
	}
	return new_block;
}

uint64_t invert_48bits(uint64_t block48) {
	uint64_t block_inverted = 0;
	for (int i = 0; i < 48; i++) {
		block_inverted = (block_inverted << 1) | (block48 & 1);
		block48 >>= 1;
	}
	return block_inverted;
}

uint64_t invert_64bits(uint64_t block64) {
	uint64_t block_inverted = 0;
	for (int i = 0; i < 64; i++) {
		block_inverted = (block_inverted << 1) | (block64 & 1);
		block64 >>= 1;
	}
	return block_inverted;
}


/* This is the IP permutation. Follow the table to see how it goes. The idea is subtracting by 8 to find which original bit to copy */
uint64_t initial_permutation(uint64_t message) {
	uint64_t permuted_message = 0;
	int n = 58;
	int original_n = n;
	uint64_t find_bit, hold_bit = 0;

	for (int i = (MESSAGE_SIZE - 1); i >= 0; i--) {
		find_bit = ((message >> (n-1)) & 0x01);
		hold_bit = ((hold_bit >> i) | find_bit) << i;
		if ((n - 8) <= 0) {		// n becomes the original n+2, and after becomes 57 
			n = original_n + 2;
			original_n = n;
			if (n > MESSAGE_SIZE) {
				original_n = 57;
				n = original_n;
			}
		} else {
			n -= 8;
		}
		permuted_message = (permuted_message | hold_bit);
	}
	permuted_message = invert_64bits(permuted_message);	

	return permuted_message;//inverted_permuted_message;
}


/*uint64_t final_permutation() {  // inverse of the initial permutation
}*/

/* This function will get as parameter the original 64-bit key, and remove each multiple of 8 bit, returning a 56-bit key */
/*uint64_t transform_original_key(uint64_t key) {

}*/

/*
uint64_t create_subkeys() {

}*/



void print_s_blocks(uint8_t sblock) {
	for (int k = 7; k >= 0; k--) {
		printf("%d", (sblock >> k) & 0x01);
	}
	printf("\n");
} 


/* int count_blocks = 0; // Variable that will be used to see which blocks we're permutating. Ex: if count == 0..5, it's the 
 first block.
 Basically, the first bit should be a copy of the 32th bit, the second should be a copy of the first bit, and the 32th bit 
 should be a copy of the first bit.
# This function expands the 32-bit block into a 48-bit block */
/* NOT WORKING! 
uint64_t DES_expansion_permutation(uint32_t half_block, int count_blocks) {
	uint64_t expanded_half_block = 0;
	uint8_t s_blocks[NUMBER_OF_S_BLOCKS] = {0};
	uint8_t six_bit_block; //, next_six_bit_block;
	uint8_t bit, aux;
	uint8_t hold_last_2_bits[2];
	int count, total = 0;

	// This works only for the first 4-bit block, because the first bit will be the 32th-bit from the original block. 
	for (int i = 0; i < 6; i++) {
		if (i == 0 && count == 0) {
			six_bit_block = (half_block >> 31) & 0x01;
		} else {
			aux = (half_block >> (i-1)) & 0x01;
			bit = ((bit >> i) | aux) << i; // "<< i" is to shift back to corresponded position for then inserting that bit.
			six_bit_block = (six_bit_block | bit);
		}
		++count;
	}
	expanded_half_block = expanded_half_block | six_bit_block;

	// This array holds the last 2 bits of the previous block, which will be repeated as first 2 bits of the current block. 
	hold_last_2_bits[0] = (six_bit_block >> 4) & 0x01;
	hold_last_2_bits[1] = (six_bit_block >> 5) & 0x01;
//	count = count-1;
	total = count;

	uint8_t second_six_bit_block;
	uint8_t bit2, auxtemp;

//	expanded_half_block = (expanded_half_block << 6) | six_bit_block;

	// Each i iteration corresponds to each block, and each j iteration corresponds to each bit on each block. 
	s_blocks[0] = six_bit_block;
	for (int i = 1; i < NUMBER_OF_S_BLOCKS; i++) {
		for (int j = 0; j < EACH_S_BLOCK_SIZE; j++) {
			if (j == 0) {
				bit2 = hold_last_2_bits[0];
			//	bit2 = (s_blocks[i-1] >> 4) & 0x01;
				second_six_bit_block = second_six_bit_block | bit2;
				s_blocks[i] = (s_blocks[i] | bit2);
			} else if (j == 1) {
				bit2 = hold_last_2_bits[1];
				bit2 = bit2 << 1;
				second_six_bit_block = second_six_bit_block | bit2;
			//	bit = ((s_blocks[i-1] >> 5) & 0x01) << 1;
				s_blocks[i] = (s_blocks[i] | bit2);
			} else {
				//aux = (half_block >> (count-1)) & 0x01;
				auxtemp = (half_block >> (count-1)) & 0x01;
				bit2 = ((bit2 >> j) | auxtemp) << j;
				second_six_bit_block = second_six_bit_block | bit2;
			//	bit = ((bit >> j) | aux) << j;
			//	s_blocks[i] = (s_blocks[i] | bit);
				count++;
				if (i == (NUMBER_OF_S_BLOCKS-1) && j == (EACH_S_BLOCK_SIZE-1)) { //last bit of last block is same as first bit
					aux = (half_block >> 0) & 0x01;
					bit = ((bit >> 0) | aux) << j;
					s_blocks[i] = (s_blocks[i] | bit);
				}
			}
			total++;
		}
		count = count - 1;
		hold_last_2_bits[0] = (second_six_bit_block >> 4) & 0x01;
		hold_last_2_bits[1] = (second_six_bit_block >> 5) & 0x01;
		expanded_half_block = (expanded_half_block << 6) | second_six_bit_block;
	}
	
	for(int x = 0; x < NUMBER_OF_S_BLOCKS; x++) {
		s_blocks[x] = invert_8bits(s_blocks[x]);
		expanded_half_block = (expanded_half_block << 6) | s_blocks[x]; //now i need to invert all 48bits.
	}

	expanded_half_block = invert_64bits(expanded_half_block);	// inverting bits from 48-bit concatenated block

	for (int k = 0; k < NUMBER_OF_S_BLOCKS; k++) {
		print_s_blocks(s_blocks[k]);
	}
	
 	return expanded_half_block;
}
*/

// FINALLY WORKING!
uint64_t expansion_permutation2(uint32_t half_block) {//, uint8_t *s_block) {
	const int expansion_table[48] =             {32,  1,  2,  3,  4,  5,
	                                              4,  5,  6,  7,  8,  9,
												  8,  9, 10, 11, 12, 13,
												 12, 13, 14, 15, 16, 17,
												 16, 17, 18, 19, 20, 21,
												 20, 21, 22, 23, 24, 25,
												 24, 25, 26, 27, 28, 29,
												 28, 29, 30, 31, 32,  1};
//	uint64_t expanded_block = 0;
	uint64_t bot = 0;
	int bit_tmp = 0;

	for (int i = 0; i < 48; i++) {
		int bit_index = expansion_table[i];
		bit_tmp = ((half_block >> (bit_index - 1)) & 0x01);
		bot = ((bot << 1) | bit_tmp);
		printf("%d ", bit_tmp);
	}
	
	return bot;
}


// Working!
uint64_t xor_block48_key(uint64_t block48, uint64_t key48) {
	return block48 ^ key48;
}

// Not sure where to locate this table at. Maybe globally, maybe at main, or even a pointer that points to the function return
// I've decided to represent as a matrix. It looks like it'll be easier to get to right position.
/*const int s_table[4][16] = { 14,  3, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
							  0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
							  4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
							 15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13};
*/

// This is probably very wrong. Maybe we don't actually need to convert right away. We could use the same technique of print.
uint8_t convert_dec_to_bin(int dec) {
	uint8_t bin = 0;
	while (dec % 2 > 0) {
		bin = dec % 2;
	}

	return bin;
}

#define ROW_SIZE 4
#define COL_SIZE 16
// the 4-bit number located at that address will be stored on the 6-bit number
// This seems to be working fine! Now I need to think of a way by sending the right matrix at each time, since they're all
// different from one to another.
uint8_t s_permutation(uint8_t s_block, int s_table[ROW_SIZE][COL_SIZE]) {
//	uint8_t first_bit = (s_block & 0x01);
//	uint8_t last_bit  = (s_block >> 5) & 0x01;
	uint8_t first_bit = (s_block >> 5) & 0x01;
	uint8_t last_bit =  (s_block & 0x01);

	uint8_t i = (first_bit << 1) | last_bit; // Holds first and last bits of the block. They named this 2bits as 'i'
	uint8_t j = ((s_block >> 1) & 0xF); // << 1; // Holds the 4 middle bits of the block. They named this 4bits as 'j'
		
	int bits4 = 0;
	int row = (int)i;
	printf("row: %d ", row);
	int col = (int)j;
	printf("col: %d\n", col);

	for (int r = 0; r < ROW_SIZE; r++) {
		for (int c = 0; c < COL_SIZE; c++) {
			if (r == row && c == col) {
				bits4 = s_table[row][col];
				break;
			}
		}
	}

	return bits4;	
}


/*
uint32_t combine_block_subkey(uint32_t R_half, uint64_t subkey) {
	const int arr_length = 8;
	uint8_t 4bits_vector[arr_length];			//R_half must be expanded to 48bits, using expansion_permutation permutation
    uint8_t 6bits_vector[arr_length];

	for (int i = 0; i < arr_length; i++) {
		6bits_vector[i] = expansion_permutation(4bits_vect[i]);
	}
	for (int i = 0; i < arr_length; ++i) {
		for (int j = 7; j >= 0; j--) {
			printf("%d", (4bits_vector[i] >> j) & 0x01); // prints each bit of an index
		}
		printf("\n"); 
	}

	return R_half ^ subkey;	// AND
}

void swap(uint32_t *left, uint32_t *right) {
	uint32_t *temp = left;
	left = right;
	right = temp;
}

uint64_t atoi64(const char *s) {
	uint64_t x = 0;
	int i = 0;

	while (s[i] && (s[i] >= '0' && s[i] <= '9')) {
		x = x * 10 + (s[i] - '0');
		++x;
	}
	
	return x;
}
*/

/*
void split_block(uint64_t permutated_message) {
	uint32_t L_block = ((permutated_message >> 8*0) & 0xFFFFFFFF);
	uint32_t R_block = ((permutated_message >> 8*4) & 0xFFFFFFFF);
	// ... This is after the initial permutation
}
*/

/*
void feistel_cipher(uint64_t block, uint64_t key, int rounds) {  // previously was a uint32_t block.
	uint32_t L_block = ((full_block >> 8*0) & 0xFFFFFFFF); // 32-bit first half
	uint32_t R_block = ((full_block >> 8*4) & 0xFFFFFFFF); // 32-bit second half
	uint64_t encrypted_text = 0;
	char *R_string = "";
	char *L_string = "";
//	bit_extension(&L_block); bit_extension(&R_block, sizeof(key));
	
	for (int i = 0; i < rounds; i++) {
		R_temp = R_block;
		R_block = combine_block_subkey(R_block, subkey);
		L_block = L_block ^ R_block;
		swap(&L_block, &R_block);
		L_block = R_temp;
	}
	
	sprintf(R_string, "%d", R_block);
	sprintf(L_string, "%d", L_block);
	strcat(R_string, L_string); 
	encrypted_text = atoi(
	
}
*/

/*uint32_t f_function(uint32_t half_block, uint64_t key, int n) {  // 32-bit block and 48-bit key
	uint32_t
}*/

int *get_tables(int i) {

	const int s1[4][16] = { 
		{14,  3, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
		{ 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
		{ 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
		{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
	};
	const int s2[4][16] = { 
		{15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
		{ 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
	    { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
		{13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
	};
	const int s3[4][16] = { 
		{10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
		{13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
	    {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
		{ 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
	};
	const int s4[4][16] = { 
		{ 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
		{13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
	    {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
		{ 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
	};
	const int s5[4][16] = { 
		{ 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
		{14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
		{ 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
		{11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
	};
	const int s6[4][16] = { 
		{12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
	    {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
		{ 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
		{ 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
	};
	const int s7[4][16] = { 
		{ 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
	    {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
		{ 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
		{ 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
	};
	const int s8[4][16] = { 
		{13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
	    { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
		{ 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
		{ 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
	};

	// very bad written code. Maybe what we should do is use pointers somehow. Need a better way on getting the tables on the
	// function. Maybe a string param, that tries to find the table name as string, then retrieves the matrix correspondent

	int *table = NULL;
	if (i == 0) { table = &s1 }
	else if (i == 1) { table = &s2 }
	else if (i == 2) { table = &s3 }
	else if (i == 3) { table = &s4 }
	else if (i == 4) { table = &s5 }
	else if (i == 5) { table = &s6 }
	else if (i == 6) { table = &s7 }
	else { table = &s8 }

	return table
} 

void print_sblocks_4midbits(uint8_t s_blocks[], uint8_t s_blocks_4[]) {
	for (int i = 0; i < 8; i++) {
		printf("\ns_block[%d]:\n", i);
		for (int k = 5; k >= 0; k--) {
			printf("%u", (s_blocks[i] >> k) & 0x01);
		}
		printf("\n4 middle bits:\n");
		for (int k = 3; k >= 0; k--) {
			printf("%u", (s_blocks_4[i] >> k) & 0x01);
		}
	}
}

#define P_TABLE_SIZE 32
uint32_t p_permutation(uint32_t s_blocks_4_combined) {
	const int p_table[P_TABLE_SIZE] = {16,  7, 20, 21,
						     		   29, 12, 28, 17,
						      			1, 15, 23, 26,
						                5, 18, 31, 10,
						      			2,  8, 24, 14,
						     		   32, 27,  3,  9,
						     		   19, 13, 30,  6,
						     		   22, 11,  4, 25};
	
	uint32_t new_block = 0;
	int bit_tmp = 0;

	for (int i = 0; i < P_TABLE_SIZE; i++) {
		int bit_index = p_table[i];
		bit_tmp = (s_blocks_4_combined >> (P_TABLE_SIZE - bit_index)) & 0x01;
		new_block = new_block | (bit_tmp << (P_TABLE_SIZE - 1 - i));
	}

	return new_block;
}

uint64_t ip-1_permutation(uint32_t p_permuted_block) {
	const int p-1_table[64] = {
		40, 8, 48, 16, 56, 24, 64, 32,
		39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30,
		37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26,
		33, 1, 41,  9, 49, 17, 57, 25
	};

	uint64_t permuted_block = 0;
	int bit_tmp = 0;

	for (int i = 0; i < 64; i++) {
		int bit_index = p-1_table[i];
		bit_tmp = (p_permuted_block >> (64 - bit_index)) & 0x01;
		permuted_block = permuted_block | (bit_tmp << (64 - 1 - i));
	}

	return permuted_block;
}


#define S_BLOCKS 8


int main() {
/* ---------- STEP 1: KEYS ---------- */
	uint64_t key_example = 0b0001001100110100010101110111100110011011101111001101111111110001;
	uint64_t key_56bits = pc1_permutation(key_example);
	uint32_t left_key = (key_56bits >> 28) & 0xFFFFFFFF;
	uint32_t right_key = key_56bits & 0xFFFFFFFF;
	uint64_t concat_left_right_keys[ROUNDS] = {0};

/* ---------- creating 16 subkeys ---------- */
	const int n_shifts[ROUNDS] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
	uint32_t l[ROUNDS], r[ROUNDS] = {0};
	for (int i = 0; i < ROUNDS; i++) {
		create_16subkeys(&left_key, &right_key, n_shifts[i]);
		l[i] = left_key;
		r[i] = right_key;
		concat_left_right_keys[i] = ((uint64_t)l[i] << 28) | (uint64_t)r[i];
	}

/* ----------- PC2 ---------- */
	uint64_t pc2_subkeys[ROUNDS] = {0};

	for (int i = 0; i < ROUNDS; i++) {
		pc2_subkeys[i] = pc2_permutation(concat_left_right_keys[i]);
	}

/* --------------- PRINTS ---------------- */
	printf("Permuted key:\n");
	for (int k = 55; k >= 0; k--) {
		printf("%llu", (key_56bits >> k) & 0x01);
	}
	printf("\n");

	printf("leftkey c1..c16:\n");
	for (int i = 0; i < ROUNDS; i++) {
		for (int k = 27; k >= 0; k--) {
			printf("%u", (l[i] >> k) & 0x01);
		}
		printf("\n");
	}
	printf("\n");

	printf("rightkey d1..d16:\n");
	for (int i = 0; i < ROUNDS; i++) {
		for (int k = 27; k >= 0; k--) {
			printf("%u", (r[i] >> k) & 0x01);
		}
		printf("\n");
	}
	printf("\n");

	printf("Concatenated keys c1d1..cNdN:\n");
	for (int i = 0; i < ROUNDS; i++) {
		for (int k = 55; k >= 0; k--) {
			printf("%llu", (concat_left_right_keys[i] >> k) & 0x01);
		}
		printf("\n");
	}
	printf("\n");
	
	printf("PC2 48 bit key: \n");
	for (int i = 0; i < ROUNDS; i++) {
		for (int k = 47; k >= 0; k--) {
			printf("%llu", (pc2_subkeys[i] >> k) & 0x01);
		}
		printf("\n");
	}
	printf("\n");


/* ---------- STEP 2: MESSAGE ---------- */
	uint64_t message = 0b0000000100100011010001010110011110001001101010111100110111101111;
	uint64_t ip_message = initial_permutation(message);

	printf("Message:\n");
	for (int k = 63; k >= 0; k--) {
		printf("%llu", (message >> k) & 0x01);
	}
	printf("\n");

	printf("IP Message:\n");
	for (int k = 63; k >= 0; k--) {
		printf("%llu", (ip_message >> k) & 0x01);
	}
	printf("\n");

	
	uint32_t ip_message_left  = (ip_message >> 32) & 0xFFFFFFFF;
	uint32_t ip_message_right = (ip_message & 0xFFFFFFFF);

	printf("left ip_message:\n");
	for(int k = 31; k >= 0; k--) {
		printf("%u", (ip_message_left >> k) & 0x01);
	}
	printf("\n");

	printf("right ip_message:\n");
	for(int k = 31; k >= 0; k--) {
		printf("%u", (ip_message_right >> k) & 0x01);
	}
	printf("\n");


	// Expansion permutation
	
	uint32_t decoy = 0b11110000101010101111000010101010; // delete after
	uint64_t r0 = invert_48bits(expansion_permutation2(decoy));

	printf("block:\n");
	for (int k = 31; k >= 0; k--) {
		printf("%u", (decoy >> k) & 0x01);
	}
	printf("\n");

	printf("expanded block:\n");
	for (int i = 47; i >= 0; i--) {
		printf("%llu", (r0 >> i) & 0x01);
	}
	printf("\n");


	// Xor key, block
	uint64_t key_test = 0b000110110000001011101111111111000111000001110010; // these are just for testing the xor function.
	uint64_t block_test = 0b011110100001010101010101011110100001010101010101;
	uint64_t xor_block = xor_block48_key(block_test, key_test); // working
	uint8_t s_blocks[8] = {0};	
	
	const int s_table[4][16] = { 
		{14,  3, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
		{ 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
		{ 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
		{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
	};
//	uint8_t s_block_4bits = s_permutation(s_blocks[0], s_table);
// split the xor48 block into 8 s_blocks of length 6.
	int j = 48;
	for (int i = 0; i < 8; i++) {
		s_blocks[i] = (xor_block >> (j-6)) & 0x3F;
		j -= 6;
	}
	uint8_t s_block_4bits = s_permutation(s_blocks[1], s_table);
	
	

	printf("s_blocks:\n");
	for (int i = 0; i < 8; i++) {
		for (int k = 5; k >= 0; k--) {
			printf("%u", (s_blocks[i] >> k) & 0x01);
		}
		printf("\n");
	}

	printf("xor block48, key 48:\n");
	for (int k = 47; k >= 0; k--) {
		printf("%llu", (xor_block >> k) & 0x01);
	}
	printf("\n");


	printf("s_block[0]:\n");
	for (int k = 5; k >= 0; k--) {
		printf("%u", (s_blocks[1] >> k) & 0x01);
	}
	printf("\n");
	printf("Only the 4 middle bits:\n");
	for (int k = 3; k >= 0; k--) {
		printf("%u", (s_block_4bits >> k) & 0x01);
	}
	printf("\n");



/* This is an idea of how to pass each matrix to s_permutation. Basically I created a vector of pointers that each of them
   points to an index of an array. Each index of this array is a matrix (aka si table). Should I do those in a separated
   function? */

// All this tables are for the s_permutation. Each si table must be "permutated" with si_block.
// Need to find a good place to store those tables. Thinking on how the function could work to retrieve the right table each
// time.
	const int s1[4][16] = { 
		{14,  3, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
		{ 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
		{ 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
		{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
	};
	const int s2[4][16] = { 
		{15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
		{ 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
	    { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
		{13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
	};
	const int s3[4][16] = { 
		{10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
		{13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
	    {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
		{ 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
	};
	const int s4[4][16] = { 
		{ 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
		{13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
	    {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
		{ 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
	};
	const int s5[4][16] = { 
		{ 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
		{14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
		{ 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
		{11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
	};
	const int s6[4][16] = { 
		{12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
	    {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
		{ 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
		{ 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
	};
	const int s7[4][16] = { 
		{ 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
	    {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
		{ 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
		{ 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
	};
	const int s8[4][16] = { 
		{13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
	    { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
		{ 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
		{ 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
	};

#define NUM_S_BLOCKS 8
	uint64_t matrix_array[NUM_S_BLOCKS] = {s1, s2, s3, s4, s5, s6, s7, s8}; // Each of these si is a matrix 
	
	uint8_t sblocks_4[8] = {0};
	for (int x = 0; x < 8; x++) {
		sblocks_4[x] = s_permutation(s_blocks[x], matrix_array[x]);
	}
	print_sblocks_4midbits(s_blocks, sblocks_4); // prints all s_blocks and all sblocks_4 (4 mid bits gotten on s_permut.)
	uint32_t concat_sblocks_4 = 0;

	for (int i = 0; i < 8; i++) {
		concat_sblocks_4 = (concat_sblocks_4 << 4) | sblocks_4[i];
	}
	uint32_t permuted_concat_sblocks_4 = p_permutation(concat_sblocks_4);

	printf("\n Concat block: \n");
	for (int k = 31; k >= 0; k--) {
		printf("%u", (concat_sblocks_4 >> k) & 0x01);
	}
	printf("\n P permuted concat block: \n");
	for (int k = 31; k >= 0; k--) {
		printf("%u", (permuted_concat_sblocks_4 >> k) & 0x01);
	}

	return 0;
}

/* TODO :
- Expansion and Xor function are working. Now, need to figure it out which blocks and keys to pass as params.
- Also, maybe I need a function that will retrieve a correspondent s_table, since there are many, which each time is a 
different one to be permuted at.
- Right now, many processes in this code work only for one parameter. It should work for many others, like the s_permutation
blocks, which I need to give as an argument each block at a time.
- Also, need to organize the code a little bit better. It's messy and disgusting.
- Now, I need to make the p-1 permutation to work. 
- Need to organize the code and make it neat. Also, need to check the rules of left and right block at each iteration. Maybe
looking at other pages will help more.
- Then after all that, the decription is the only left, I suppose.
*/











