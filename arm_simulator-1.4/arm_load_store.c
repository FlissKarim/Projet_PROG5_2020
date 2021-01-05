/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"
#include <assert.h>

uint8_t number_set_bits_in(uint16_t n) {
	uint8_t c = 0;
	while(n) {
	    c += n % 2;
	    n = n >> 1;
	}
	return c;
}

// encoding see doc A5.2.1

// LDR | LDRB | STR | STRB
int arm_load_store(arm_core p, uint32_t ins) {

	uint8_t cond = get_bits(ins, 31, 28);

	uint32_t rn = get_bits(ins, 19, 16);
	rn = arm_read_register(p, rn);
	
	// keep the register number to store value to or get value from
	uint32_t rd = get_bits(ins, 15, 12);
	
	uint32_t rm = get_bits(ins, 3, 0);
	
	uint32_t offset_12 = get_bits(ins, 11, 0);
	
	uint8_t P = get_bit(ins, 24);
	uint8_t U = get_bit(ins, 23);
	uint8_t B = get_bit(ins, 22);
	uint8_t W = get_bit(ins, 21);
	uint8_t L = get_bit(ins, 20);
	
	uint32_t address;
	// immediate offset/index
	if (get_bits(ins, 27, 25) == 2) {
	
		// Load and Store Word or Unsigned Byte - Immediate offset
		if (P == 1 && W == 0) {
			if (U == 1)
				address = rn + offset_12;
			else /* U == 0 */
				address = rn - offset_12;
		}
		
		// Load and Store Word or Unsigned Byte - Immediate pre-indexed
		else if (P == 1 && W == 1) {
			if (U == 1)
				address = rn + offset_12;
			else /* if U == 0 */
				address = rn - offset_12;
			if (condition(arm_read_cpsr(p), cond))
				rn = address;
		}
		// Load and Store Word or Unsigned Byte - Immediate post-indexed
		else if (P == 0 && W == 0) {
			address = rn;
			if (condition(arm_read_cpsr(p), cond)) {
				if (U == 1)
					rn = rn + offset_12;
				else /* U == 0 */
					rn = rn - offset_12;
			}
		}
	}
	
	//register offset/index
	else if (get_bits(ins, 27, 25) == 3 && get_bits(ins, 11, 4) == 0) {
		rm = arm_read_register(p, rm);
		
		// Load and Store Word or Unsigned Byte - Register offset
		if (P == 1 && W == 0) {
			if (U == 1)
				address = rn + rm;
			else /* U == 0 */
				address = rn - rm;
		}
		// Load and Store Word or Unsigned Byte - Register pre-indexed
		else if (P == 1 && W == 1) {
			if (U == 1)
				address = rn + rm;
			else /* U == 0 */
				address = rn - rm;
		if (condition(arm_read_cpsr(p), cond))
			rn = address;
		}
		// Load and Store Word or Unsigned Byte - Register post-indexed
		else if (P == 0 && W == 0) {
			address = rn;
			if (condition(arm_read_cpsr(p), cond)) {
				if (U == 1)
					rn = rn + rm;
				else /* U == 0 */
					rn = rn - rm;
			}
		}
	}

	// scaled register offset/index
	else if (get_bits(ins, 27, 25) == 3 && get_bit(ins, 4) == 0) {
		uint32_t shift_imm = get_bits(ins, 11, 7);
		uint8_t shift = get_bits(ins, 6, 5);
		uint32_t index;
		rm = arm_read_register(p, rm);
		
		//Load and Store Word or Unsigned Byte - Scaled register offset
		if (P == 1 && W == 0) {
			switch(shift) {
				case LSL: index = rm << shift_imm; break;
				case LSR: 
					if (shift_imm == 0)/* LSR #32 */ index = 0;
					else index = rm >> shift_imm;
				
				break;
				case ASR:
				if (shift_imm == 0) { /* ASR #32 */
					if (get_bit(rm, 31) == 1) index = 0xFFFFFFFF;
					else index = 0;
				}
				else
					index = asr(rm, shift_imm);
				break;
				case ROR:
					if (shift_imm == 0) /* RRX */
						index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
					else /* ROR */
						index = ror(rm, shift_imm);
				break;
				default: break;
			}
			if (U == 1)
				address = rn + index;
			else /* U == 0 */
				address = rn - index;
		}
		
		// Load and Store Word or Unsigned Byte - Scaled register pre-indexed
		else if (P == 1 && W == 1) {
			switch(shift) {
				case LSL: index = rm << shift_imm; break;
				case LSR: 
					if (shift_imm == 0) /* LSR #32 */
						index = 0;
					else
						index = rm >> shift_imm;
				break;
				
				case ASR:
					if (shift_imm == 0) { /* ASR #32 */
						if (get_bit(rm, 31) == 1)
							index = 0xFFFFFFFF;
						else
							index = 0;
						}
						else index = asr(rm, shift_imm);
				break;
				case ROR:
				/* ROR or RRX */
					if (shift_imm == 0) /* RRX */
						index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
					else /* ROR */
					 	index = ror(rm, shift_imm);
				break;
				default: break;
			}
		if (U == 1)
			address = rn + index;
		else /* U == 0 */
			address = rn - index;
		if (condition(arm_read_cpsr(p), cond))
			rn = address;
	}

	 	// Load and Store Word or Unsigned Byte - Scaled register post-indexed
	 	else if (P == 0 && W == 0) {
	 		address = rn;
	 		switch(shift) {
	 			case LSL: index = rm << shift_imm; break;
	 			case LSR:
	 				if (shift_imm == 0) /* LSR #32 */ index = 0;
	 				else index = rm >> shift_imm;
	 			break;
	 			case ASR:
	 				if (shift_imm == 0) /* ASR #32 */ {
	 					if (get_bit(rm, 31) == 1) index = 0xFFFFFFFF;
	 					else index = 0;
	 				}
	 				else index = asr(rm, shift_imm);
	 			break;
	 			
	 			case ROR:
	 				if (shift_imm == 0) /* RRX */
	 					index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
	 					else /* ROR */index = ror(rm, shift_imm);
	 			break;
	 			default: break;
	 		}
	 		if (condition(arm_read_cpsr(p), cond)) {
	 			if (U == 1) rn = rn + index;
	 			else /* U == 0 */ rn = rn - index;
	 		}
	 	}
 	}
 	
 	uint8_t byte;
 	uint32_t word;
 	// LOAD
 	if (L == 1) {
 		if (B == 1) {
 			// load unsigned byte
 			arm_read_byte(p, address, &byte);
 			arm_write_register(p, rd, (uint32_t) byte);
 		}
 		else {
 			// load word
 			arm_read_word(p, address, &word);
 			arm_write_register(p, rd, word);
 		}
 	}
 	// STORE
 	else {
 		if (B == 1) {
 			// store unsigned byte
 			byte = arm_read_register(p, rd);
 			arm_write_byte(p, address, byte);		
 		}
 		else {
 			// store word
 			word = arm_read_register(p, rd);
 			arm_write_word(p, address, word);
 		}
 	}
    return UNDEFINED_INSTRUCTION;
}

// load store multiple LDM(1) , STM(1)
int arm_load_store_multiple(arm_core p, uint32_t ins) {
	uint8_t cond = get_bits(ins, 31, 28);

	uint32_t rn = get_bits(ins, 19, 16);

	uint16_t register_list = get_bits(ins, 15, 0);
	uint8_t P = get_bit(ins, 24);
	uint8_t U = get_bit(ins, 23);
	// uint8_t S = get_bit(ins, 22);
	uint8_t W = get_bit(ins, 21);
	uint8_t L = get_bit(ins, 20);
	
	uint32_t cpsr = arm_read_cpsr(p);
	if (L == 1 && rn == 15) // PC
		cpsr = arm_read_spsr(p);

	rn = arm_read_register(p, rn);
	
	uint32_t start_address, end_address;
	int i = 0;
	
	// Load and Store Multiple - Increment after
	if (P == 0 && U == 1) {
		start_address = rn;
		end_address = rn + (number_set_bits_in(register_list) * 4) - 4;
		if (condition(cpsr, cond) && W == 1)
			rn = rn + (number_set_bits_in(register_list) * 4);
	}
	// Load and Store Multiple - Increment before
	else if (P == 1 && U == 1) {
		start_address = rn + 4;
		end_address = rn + (number_set_bits_in(register_list) * 4);
		if (condition(cpsr, cond) && W == 1)
			rn = rn + (number_set_bits_in(register_list) * 4);
	}
	// Load and Store Multiple - Decrement after
	else if (P == 0 && U == 0) {
		start_address = rn - (number_set_bits_in(register_list) * 4) + 4;
		end_address = rn;
		if (condition(cpsr, cond) && W == 1)
			rn = rn - (number_set_bits_in(register_list) * 4);
	}
	// Load and Store Multiple - Decrement before
	else if (P == 1 && U == 0) {
		start_address = rn - (number_set_bits_in(register_list) * 4);
		end_address = rn - 4;
		if (condition(cpsr, cond) && W == 1)
			rn = rn - (number_set_bits_in(register_list) * 4);
	}
	uint32_t address;
	// LDM
	if (L) {
		uint32_t value;
		// MemoryAccess(B-bit, E-bit)
		if (condition(cpsr, cond)) {
			address = start_address;
			for (i = 0; i <= 14; i++) {
				if (get_bit(register_list, i) == 1) {
					arm_read_word(p, address, &value);
					arm_write_register(p, i, value);
					address = address + 4;
				}
			}
			if (get_bit(register_list, 15) == 1) {
				value = arm_read_word(p, address, &value);
				arm_write_register(p, 15, value & 0xFFFFFFFE);
				//T Bit = value[0];
				address = address + 4;
			}
			assert(end_address == (address - 4));
		}
	}
	// STM
	else {
		// MemoryAccess(B-bit, E-bit)
		if (condition(cpsr, cond)) {
			address = start_address;
			for (i = 0; i <= 15; i++) {
				if (get_bit(register_list, i) == 1) {
					arm_write_word(p, address, arm_read_register(p, i));
					address = address + 4;
				}
			}
		assert(end_address == (address - 4));
		}
	}
    return UNDEFINED_INSTRUCTION;
}

// A5.3.2 doc
//load store signed byte, halfword and doubleword
// LDRH |STRH
int arm_load_store_miscellaneous(arm_core p, uint32_t ins) {

	uint32_t cond = get_bits(ins, 31, 28);

	uint8_t  U = get_bit(ins, 23);
	uint8_t  L = get_bit(ins, 20);

	// uint8_t  S = get_bit(ins, 6);
	uint8_t  H = get_bit(ins, 5);
	
	uint32_t rn = get_bits(ins, 19, 16);
	rn = arm_read_register(p, rn);

	uint32_t rm = get_bits(ins, 3, 0);
	rm = arm_read_register(p, rn);
	
	uint32_t rd = get_bits(ins, 15, 12);

	uint32_t immedH = get_bits(ins, 11, 8);
	uint32_t immedL = get_bits(ins, 3, 0);
	
	uint32_t offset_8, address;
	uint32_t CP15_reg1_Ubit = 1;
	uint16_t data;
	
	uint32_t cpsr = arm_read_cpsr(p);
	
	// Miscellaneous Loads and Stores - Immediate offset
	if ( get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1 ) {
		offset_8 = (immedH << 4) | immedL;
		if (U == 1)
			address = rn + offset_8;
		else /* U == 0 */
			address = rn - offset_8;
	}
	
	// Miscellaneous Loads and Stores - Register offset
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 0 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		if (U == 1)
			address = rn + rm;
		else /* U == 0 */
			address = rn - rm;	
	}
	// Miscellaneous Loads and Stores - Immediate pre-indexed
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 3 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		offset_8 = (immedH << 4) | immedL;
		if (U == 1)
			address = rn + offset_8;
		else /* U == 0 */
			address = rn - offset_8;
		if (condition(cpsr, cond))
			rn = address;
		}

		// Miscellaneous Loads and Stores - Register pre-indexed
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 1 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		if (U == 1)
			address = rn + rm;
		else /* U == 0 */
			address = rn - rm;
		if (condition(cpsr, cond))
			rn = address;
	}

		// Miscellaneous Loads and Stores - Immediate post-indexed
	else if (get_bits(ins, 27, 24) == 0 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		address = rn;
		offset_8 = (immedH << 4) | immedL;
		if (condition(cpsr, cond)) {
			if (U == 1)
			rn = rn + offset_8;
			else /* U == 0 */
			rn = rn - offset_8;
		}
	}
	// Miscellaneous Loads and Stores - Register post-indexed
	else if (get_bits(ins, 27, 24) == 0 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		address = rn;
		if (condition(cpsr, cond)) {
			if (U == 1)
				rn = rn + rm;
			else /* U == 0 */
				rn = rn - rm;
		}
	}

	// Load
	if (L) {
		// LDRH
		if (H) {

			// MemoryAccess(B-bit, E-bit)
			if (condition(cpsr, cond)) {
				if (CP15_reg1_Ubit == 0) {
					if ( get_bits(address, 7, 0) == 0) arm_read_half(p, address, &data);
					// UNPREDICTABLE data
					else data = 0;
				}
				else /* CP15_reg1_Ubit == 1 */
					arm_read_half(p, address, &data);
				arm_write_register(p, rd, (uint32_t) data);
			}
		}
	}
	// Store
	else {
		// STRH
		if (H) {

			// MemoryAccess(B-bit, E-bit)
			if (condition(cpsr, cond)) {
				if (CP15_reg1_Ubit == 0) {
					if ( get_bits(address, 7, 0) == 0)
						arm_write_half(p, address, (uint16_t) get_bits(arm_read_register(p, rd), 15, 0));
					// UNPREDICTABLE data
					else
						arm_write_half(p, address, 0); 
					}
				else /* CP15_reg1_Ubit ==1 */
					arm_write_half(p, address, (uint16_t) get_bits(arm_read_register(p, rd), 15, 0));
			}
		}
	}
	return 0;
}

// STC , LDC
int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
	printf("x");
/*
    uint8_t P = get_bit(ins, 24);
    uint8_t U = get_bit(ins, 23);
    uint8_t N = get_bit(ins, 22);
    uint8_t W = get_bit(ins, 21);
    uint8_t L = get_bit(ins, 20);
    
    uint32_t rn = get_bits(ins, 19, 16);
    uint32_t CRd = get_bits(ins, 15, 12);
    uint32_t cp_num = get_bits(ins, 11, 8);
    uint32_t eight_bit_word_offset = get_bits(ins, 7, 0);

*/
    /* A4.1.96   STC
    
	MemoryAccess(B-bit, E-bit)
	processor_id = ExecutingProcessor()
	if ConditionPassed(cond) then
    	address = start_address
    	Memory[address,4] = value from Coprocessor[cp_num]
        if Shared(address) then // from ARMv6 //
        	physical_address = TLB(address)
            ClearExclusiveByAddress(physical_address,processor_id,4)
        while (NotFinished(coprocessor[cp_num]))
       		address = address + 4
       		Memory[address,4] = value from Coprocessor[cp_num]
       		if Shared(address) then    // from ARMv6 //
       			physical_address = TLB(address)
       			ClearExclusiveByAddress(physical_address,processor_id,4)
       			// See Summary of operation on page A2-49 //
      assert address == end_address
    */
    
    
    /* A4.1.19   LDC    

	MemoryAccess(B-bit, E-bit)
	if ConditionPassed(cond) then
		address = start_address
		load Memory[address,4] for Coprocessor[cp_num]
		while (NotFinished(Coprocessor[cp_num]))
			address = address + 4
			load Memory[address,4] for Coprocessor[cp_num]
		assert address == end_address
    */
    return UNDEFINED_INSTRUCTION;
}
