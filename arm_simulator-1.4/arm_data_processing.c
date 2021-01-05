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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"
#include <assert.h>

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	//uint32_t cpsr = arm_read_cpsr(p);
	uint32_t shifter_operand = get_bits(ins, 11, 0);
	uint8_t shifter_carry_out;
	
	// registre shift
	if (get_bit(ins, 7) == 0 && get_bit(ins, 4) == 1) {
			if (get_bits(shifter_operand, 7, 4) == 1)
			rm_lsl_rs(p, &shifter_operand, &shifter_carry_out);
			else if (get_bits(shifter_operand, 7, 4) == 3)
			rm_lsr_rs(p, &shifter_operand, &shifter_carry_out);
			else if (get_bits(shifter_operand, 7, 4) == 5)
			rm_asr_rs(p, &shifter_operand, &shifter_carry_out);
			else if (get_bits(shifter_operand, 7, 4) == 7)
			rm_ror_rs(p, &shifter_operand, &shifter_carry_out);
	}
	// immediate shift (and register with 0 shift, so can be traited like immediate value)
	else if ( get_bit(ins, 4) == 0 ) {
	
		if (get_bits(shifter_operand, 11, 4) == 0)
			rm(p, &shifter_operand, &shifter_carry_out);
			
		else if (get_bits(shifter_operand, 6, 4) == 0)
			rm_lsl_shift_imm(p, &shifter_operand, &shifter_carry_out);
		else if (get_bits(shifter_operand, 6, 4) == 2)
			rm_lsr_shift_imm(p, &shifter_operand, &shifter_carry_out);
		else if (get_bits(shifter_operand, 6, 4) == 4)
			rm_asr_shift_imm(p, &shifter_operand, &shifter_carry_out);
			
		else if (get_bits(shifter_operand, 6, 4) == 6)
			rm_ror_shift_imm(p, &shifter_operand, &shifter_carry_out);

	}
	
	processing(p, ins, shifter_operand, shifter_carry_out);
    return UNDEFINED_INSTRUCTION;
}


// A4.1.39 MSR
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	uint8_t cond = get_bits(ins, 31, 28);
	uint32_t R = get_bit(ins, 22);
		
	uint32_t field_mask = get_bits(ins, 19, 16);
	// uint32_t SBO = get_bits(ins, 15, 12);
	uint32_t rotate_imm = get_bits(ins, 11, 8);
	uint32_t eight_bit_immediate = get_bits(ins, 7, 0);

	// uint32_t SBZ = get_bits(ins, 11, 8);
	uint32_t rm = get_bits(ins, 3, 0);
	rm = arm_read_register(p, rm);
		
	uint32_t byte_mask, mask, operand;
	uint32_t cpsr = arm_read_cpsr(p);


	if (condition(cpsr, cond)) {
		if (get_bit(ins, 25) == 1) operand = ror(eight_bit_immediate, (rotate_imm * 2));
		else operand = rm;

		if ((operand & UnallocMask) !=0) return 1; //UNPREDICTABLE;
		
		/* Attempt to set reserved bits */
		uint32_t a, b, c, d;
		if (get_bit(field_mask, 0) == 1) a = 0x000000FF; else a = 0x00000000;
		if (get_bit(field_mask, 1) == 1) b = 0x0000FF00; else b = 0x00000000;
		if (get_bit(field_mask, 2) == 1) c = 0x00FF0000; else c = 0x00000000;
		if (get_bit(field_mask, 3) == 1) d = 0xFF000000; else d = 0x00000000;
		byte_mask =  a | b | c | d;
		
		if (R == 0) {
			if (arm_in_a_privileged_mode(p)) {
				if ((operand & StateMask) != 0) return 1; //UNPREDICTABLE; /* Attempt to set non-ARM execution state */
				else mask = byte_mask & (UserMask | PrivMask);
				}
			else {
				mask = byte_mask & UserMask;
			}
			cpsr = (cpsr &  ~mask) | (operand & mask);
			arm_write_cpsr(p, cpsr);
		}
		else  { /* R == 1 */
			if (arm_current_mode_has_spsr(p)) {
				uint32_t spsr = arm_read_spsr(p);
				mask = byte_mask & (UserMask | PrivMask | StateMask);
				spsr = (spsr & ~mask) | (operand & mask);
				arm_write_spsr(p, spsr);
			}
			else return 1; //UNPREDICTABLE;
		}
	}
    return UNDEFINED_INSTRUCTION;
}

// processing
// borrowFrom is the negation of carryFrom
void processing(arm_core p, uint32_t ins, uint32_t shifter_operand, uint8_t shifter_carry_out) {
	// shifter_operand now is calculated
	uint32_t cond = get_bits(ins, 31, 28);
				
	uint32_t S = get_bit(ins, 20);		
	uint32_t rn = get_bits(ins, 19, 16);
	rn = arm_read_register(p, rn);
		
	uint32_t rd = get_bits(ins, 15, 12);
	uint64_t rd_value;
	uint32_t cpsr = arm_read_cpsr(p);
	uint8_t code = get_bits(ins, 24, 21);
	codeop opr = code;

	switch(opr) {
		case AND:
			if(condition(cpsr, cond)) {
				rd_value = rn & shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
					);
			}
		break;
		case EOR:
			if(condition(cpsr, cond)) {
				rd_value = rn ^ shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
				);
			}
		break;
		case SUB:
			if(condition(cpsr, cond)) {
				rd_value = rn - shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							!borrowFrom( rd_value ),
								overflowFrom(rn, shifter_operand, rd_value)
					);
			}			
		break;
		case RSB:
		if(condition(cpsr, cond)) {
				rd_value = shifter_operand - rn;
				arm_write_register(p, rd, rd_value );
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
			update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							!borrowFrom( rd_value ),
								overflowFrom(rn, shifter_operand, rd_value)
					);
			}
		break;
		case ADD:
			if(condition(cpsr, cond)) {
				rd_value = rn + shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							carryFrom( rd_value ),
								overflowFrom(rn, shifter_operand, rd_value)
					);
			}
		break;
		case ADC:
			if(condition(cpsr, cond)) {
			rd_value = rn + shifter_operand + get_bit(cpsr, C);
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							carryFrom( rd_value ),
								overflowFrom(rn, shifter_operand, rd_value)
					);
			}
		break;
		case SBC:
			if(condition(cpsr, cond)) {
				rd_value = rn - shifter_operand - (1 - get_bit(cpsr, C));
					arm_write_register(p, rd, rd_value );
					if (S == 1 && rd == 15) {
						if (arm_current_mode_has_spsr(p))
							arm_write_cpsr(p, arm_read_spsr(p));
						else  return; //UNPREDICTABLE
					}
					else if (S)
					update_flags(p,
						rd_value == 0,
							get_bit(rd_value, 31),
								!borrowFrom( rd_value ),
									overflowFrom(rn, shifter_operand, rd_value)
						);
				}
		break;
		case RSC:
			if(condition(cpsr, cond)) {
				rd_value = shifter_operand - rn - (1 - get_bit(cpsr, C));
				arm_write_register(p, rd, rd_value );
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							!borrowFrom( rd_value ),
								overflowFrom(rn, shifter_operand, rd_value)
					);
			}
		break;
		case TST:
			if(condition(cpsr, cond)) {
				uint32_t alu_out = rn & shifter_operand;
				update_flags(p,
					alu_out == 0,
						get_bit(alu_out, 31),
							shifter_carry_out,
							0
					);
			}
		break;
		case TEQ:
			if(condition(cpsr, cond)) {
				uint32_t alu_out = rn ^ shifter_operand;
				update_flags(p,
					alu_out == 0,
						get_bit(alu_out, 31),
							shifter_carry_out,
								0
					);
			}
		break;
		case CMP:
			if(condition(cpsr, cond)) {
				int64_t alu_out = rn - shifter_operand;
				update_flags(p,
					alu_out == 0,
						get_bit(alu_out, 31),
							!borrowFrom(alu_out),
								overflowFrom(rn, shifter_operand, alu_out)
					);
			}
		break;
		case CMN:
			if(condition(cpsr, cond)) {
				int64_t alu_out = rn + shifter_operand;
				update_flags(p,
					alu_out == 0,
						get_bit(alu_out, 31),
							carryFrom(alu_out),
								overflowFrom(rn, shifter_operand, alu_out)
					);
			}						
		break;
		case ORR:
			if(condition(cpsr, cond)) {
				rd_value = rn | shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
					);
			}
		break;
		case MOV:				
			if(condition(cpsr, cond)) {
				rd_value = shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
					);
			}
		break;
		case BIC:
			if(condition(cpsr, cond)) {
				rd_value = rn & ~shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
					);
			}
		break;
		case MVN:
			if(condition(cpsr, cond)) {
				rd_value = ~shifter_operand;
				arm_write_register(p, rd, rd_value);
				if (S == 1 && rd == 15) {
					if (arm_current_mode_has_spsr(p))
						arm_write_cpsr(p, arm_read_spsr(p));
					else  return; //UNPREDICTABLE
				}
				else if (S)
				update_flags(p,
					rd_value == 0,
						get_bit(rd_value, 31),
							shifter_carry_out,
								0
					);
			}	
		break;
	}
}

void update_flags(arm_core p, uint8_t z, uint8_t n, uint8_t c, uint8_t v) {
	uint32_t cpsr = arm_read_cpsr(p);
	// clear ZNCV flags
	cpsr = cpsr & ~(15 << 28);
	
	if (z) cpsr = set_bit(cpsr, Z); 
	if (n) cpsr = set_bit(cpsr, N);
	if (c) cpsr = set_bit(cpsr, C);
	if (v) cpsr = set_bit(cpsr, V);
	
	// update the cpsr register
	arm_write_cpsr(p, cpsr);
}

int carryFrom(uint64_t x) {
	return get_bit(x, 32);
}

int borrowFrom(uint64_t x) {
	return !carryFrom(x);
}

int overflowFrom(int32_t a, int32_t b, int64_t r) {
     return (a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0);
}

// shifter_operand's 11 formats -- see A5-1 in doc

// #<immediate>
void immediate(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
		uint8_t rotate_imm = get_bits(*shifter_operand, 11, 8);
		uint32_t immed_8 = get_bits(*shifter_operand, 7, 0);

		*shifter_operand = ror(immed_8, rotate_imm * 2);
		
		// update shifter_carry_out
		if (rotate_imm == 0)
			*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
		else
			*shifter_carry_out = get_bit(*shifter_operand, 31);
}

void rm(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
		//<Rm>
		uint32_t rm = get_bits(*shifter_operand, 3, 0);
		rm = arm_read_register(p, rm);
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
}


void rm_lsl_shift_imm(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
		// <Rm>, LSL #<shift_imm>
		uint8_t shift_imm = get_bits(*shifter_operand, 11, 7);
		uint32_t rm = get_bits(*shifter_operand, 3, 0);
		rm = arm_read_register(p, rm);
		if (shift_imm == 0) { 
			/* Register Operand */
			*shifter_operand = rm;
			*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
		}
		else {
			/* shift_imm > 0 */
			*shifter_operand = rm << shift_imm;
			*shifter_carry_out = get_bit(rm, (32 - shift_imm));
		}
}

void rm_lsl_rs(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	//<Rm>, LSL <Rs>
	uint32_t rs = get_bits(*shifter_operand, 11, 8);
	rs = arm_read_register(p, rs);
	uint32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
	
	uint8_t first_byte = get_bits(rs, 7, 0);
	if ( first_byte == 0) {
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
	}
	else if (first_byte < 32) {
		*shifter_operand = rm << first_byte;
		*shifter_carry_out = get_bit(rm, 32 - first_byte);
	}
	else if (first_byte == 32) {
		*shifter_operand = 0;
		*shifter_carry_out = get_bit(rm, 0);
	}
	else {
		/* first_byte > 32 */
		*shifter_operand = 0;
		*shifter_carry_out = 0;
	}
}

void rm_lsr_shift_imm(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	// <Rm>, LSR #<shift_imm>
	uint8_t shift_imm = get_bits(*shifter_operand, 11, 7);
	uint32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
	
	if (shift_imm == 0) {
		*shifter_operand = 0;
		*shifter_carry_out = get_bit(rm, 31);		
	}
	else { /* shift_imm > 0 */
		*shifter_operand = rm >> shift_imm;
		*shifter_carry_out = get_bit(rm, shift_imm - 1);
	}
}


void rm_lsr_rs(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	// <Rm>, LSR <Rs>
	uint32_t rs = get_bits(*shifter_operand, 11, 8);
	rs = arm_read_register(p, rs);
	uint32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
	uint8_t first_byte = get_bits(rs, 7, 0);
	
	if (first_byte == 0) {
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
	}
	else if (first_byte < 32) {
		*shifter_operand = rm >> first_byte;
		*shifter_carry_out = get_bit(rm, first_byte - 1);
	}
	else if (first_byte == 32) {
		*shifter_operand = 0;
		*shifter_carry_out = get_bit(rm, 31);
	}
	else { /* Rs[7:0] > 32 */
		*shifter_operand = 0;
		*shifter_carry_out = 0;
	}
}

// rm must be signed
void rm_asr_shift_imm(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	// <Rm>, ASR #<shift_imm>
	uint8_t shift_imm = get_bits(*shifter_operand, 11, 7);
	int32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
	uint8_t msb = get_bit(rm, 31);		
	if (shift_imm == 0) {
		if (msb == 0) {
			*shifter_operand = 0;
			*shifter_carry_out = msb;
		}
		else {
			/* Rm[31] == 1 */
			*shifter_operand = 0xFFFFFFFF;
			*shifter_carry_out = msb;
		}
	}
	else  {
		/* shift_imm > 0 */
		*shifter_operand = asr(rm,  shift_imm);
		*shifter_carry_out = get_bit(rm, shift_imm - 1);
	}
}

// rm must be signed
void rm_asr_rs(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	//<Rm>, ASR <Rs>
	uint32_t rs = get_bits(*shifter_operand, 11, 8);
	rs = arm_read_register(p, rs);
	int32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
		
	uint8_t msb = get_bit(rm, 31);
	uint8_t first_byte = get_bits(rs, 7, 0);
	if (first_byte == 0) {
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
	}
	else if (first_byte < 32) {
		*shifter_operand = asr(rm, first_byte);
		*shifter_carry_out = get_bit(rm, first_byte - 1);
	}
	else  {
		/* Rs[7:0] >= 32 */
		if (msb == 0) {
			*shifter_operand = 0;
			*shifter_carry_out = msb;
		}
		else { /* Rm[31] == 1 */
			*shifter_operand = 0xFFFFFFFF;
			*shifter_carry_out = msb;
		}
	}
}

void rm_ror_shift_imm(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	// <Rm>, ROR #<shift_imm>
	uint8_t shift_imm = get_bits(*shifter_operand, 11, 7);
	uint32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
	if (shift_imm == 0) {
		//See Data-processing operands - Rotate right with extend” on page A5-17
	}
	else { /* shift_imm > 0 */
		*shifter_operand = ror(rm, shift_imm);
		*shifter_carry_out = get_bit(rm, shift_imm - 1);
	}
}

void rm_ror_rs(arm_core p, uint32_t * shifter_operand, uint8_t * shifter_carry_out) {
	// <Rm>, ROR <Rs>
	uint32_t rs = get_bits(*shifter_operand, 11, 8);
	rs = arm_read_register(p, rs);
	uint32_t rm = get_bits(*shifter_operand, 3, 0);
	rm = arm_read_register(p, rm);
		
	uint8_t first_byte = get_bits(rs, 7, 0);
	if (first_byte == 0) {
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
	}
	else if (get_bits(rs, 4, 0) == 0) {
		*shifter_operand = rm;
		*shifter_carry_out = get_bit(rm, 31);
		}
	else { /* Rs[4:0] > 0 */
		*shifter_operand = ror(rm, get_bits(rs, 4, 0));
		*shifter_carry_out = get_bit(rm, get_bits(rs, 4, 0) - 1);
	}
}
