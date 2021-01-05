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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

// check if the instruction match with the form, the unsignificative bit is represented by 'X'
int is(uint32_t ins, char * str) {
    int i = 31;
    int lsb;
    char c;
    while (i >= 0) {
        lsb = (ins >> i) & 1;
        c = str[31 - i];
        if ( (c == '0' && lsb) || (c == '1' && !lsb) ) return 0;
    	i--;
    }
    return 1;
}

//  Instruction set encoding see A3.1
static int arm_execute_instruction(arm_core p) {
	
	uint32_t ins;
	int result = arm_fetch(p, &ins);
 
	// miscellaneous instructions see Figure A3-4
	if (is(ins, "XXXX00010XX0XXXXXXXXXXXXXXX0XXXX")) {
	}
		
	// data processing immediate shift
	else if (is(ins, "XXXX000XXXXXXXXXXXXXXXXXXXX0XXXX")) {
		arm_data_processing_shift(p, ins);
	}

	// date processing register shift
	else if (is(ins, "XXXX000XXXXXXXXXXXXXXXXX0XX1XXXX")) {
		arm_data_processing_shift(p, ins);
	}

	// miscellaneous instructions see Figure A3-4
	else if (is(ins, "XXXX00010XX0XXXXXXXXXXXX0XX1XXXX")) {
	
	}
	
	// multeplies see A3-3
	// Extra load/store A3-5
	else if (is(ins, "XXXX000XXXXXXXXXXXXXXXXX1XX1XXXX")) {
		arm_load_store_miscellaneous(p, ins);
	}
	
	// undefined instruction
	else if (is(ins, "XXXX00110X00XXXXXXXXXXXXXXXXXXXX")) {
	
	}
	
	// move immediate to status register
	else if (is(ins, "XXXX00110X10XXXXXXXXXXXXXXXXXXXX")) {
		arm_data_processing_immediate_msr(p, ins);
	}
	
	// data processing immediate
	else if (is(ins, "XXXX001XXXXXXXXXXXXXXXXXXXXXXXXX")) {
		uint32_t shifter_operand = get_bits(ins, 11, 0);
		uint8_t shifter_carry_out;
		immediate(p, &shifter_operand, &shifter_carry_out);
		processing(p, ins, shifter_operand, shifter_carry_out);
	}
	
	// load/store immediate offset
	else if (is(ins, "XXXX010XXXXXXXXXXXXXXXXXXXXXXXXX")) {
		arm_load_store(p, ins);
	}
	
	// load/store register offset
	else if (is(ins, "XXXX011XXXXXXXXXXXXXXXXXXXX0XXXX")) {
		arm_load_store(p, ins);
	}

	// architecturaly undefined
	else if (is(ins, "XXXX01111111XXXXXXXXXXXX1111XXXX")) {
	
	}
		
	// media instruction see figure A3-2
	else if (is(ins, "XXXX011XXXXXXXXXXXXXXXXXXXX1XXXX")) {
	
	}

	// load/store multiple
	else if (is(ins, "XXXX100XXXXXXXXXXXXXXXXXXXXXXXXX")) {
		arm_load_store_multiple(p, ins);
	}
	
	// branch and branch with link
	else if (is(ins, "XXXX101XXXXXXXXXXXXXXXXXXXXXXXXX")) {
		arm_branch(p, ins);
	}
	
	// coprocessor load/store and double register transfers
	else if (is(ins, "XXXX110XXXXXXXXXXXXXXXXXXXXXXXXX")) {
		arm_coprocessor_load_store(p, ins);
	}
	
	// coprocessor data processing
	else if (is(ins, "XXXX1110XXXXXXXXXXXXXXXXXXX0XXXX")) {
	
	}
	
	// coprocessor register transfers
	else if (is(ins, "XXXX1110XXXXXXXXXXXXXXXXXXX1XXXX")) {
	
	}
	
	// software interrupt
	else if (is(ins, "XXXX1111XXXXXXXXXXXXXXXXXXXXXXXX")) {
		return arm_coprocessor_others_swi(p, ins);
	}
	
	//unconditionnal instruction A3-6
	else if (is(ins, "1111XXXXXXXXXXXXXXXXXXXXXXXXXXXX")) {
	
	}
    return result;
}

int arm_step(arm_core p) {
    int result;
    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
