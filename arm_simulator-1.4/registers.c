/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {
	uint32_t tab[NBREG];
};

registers registers_create() {
    registers r = (struct registers_data *) malloc (sizeof(struct registers_data));
    write_cpsr(r, USR);
    return r;
}

void registers_destroy(registers r) {
	if (r) free(r);
}

uint16_t get_mode(registers r) {
    return read_cpsr(r) & 31;
}

void set_mode(registers r, uint16_t mode) {
    write_cpsr(r, ((read_cpsr(r) >> 5) << 5) | mode);
}

int current_mode_has_spsr(registers r) {
	uint16_t mode = get_mode(r);
    return (mode != USR) && (mode != SYS);
}

int in_a_privileged_mode(registers r) {
	uint16_t mode = get_mode(r);
    return mode != USR;
}

uint32_t read_register(registers r, uint8_t reg) {
	reg_name n = reg;
	uint16_t mode = get_mode(r);
	switch(mode) {
		case IRQ:
			switch(n) {
				case R13: n = R13_irq; break;
				case R14: n = R14_irq; break;
				default:break;
			}
		break;
		case ABT:
			switch(n) {
				case R13: n = R13_abt;break;
				case R14: n = R14_abt;break;
				default:break;
			}
		case UND:
			switch(n) {
				case R13: n = R13_und; break;
				case R14: n = R14_und; break;
				default:break;
			}
		break;
		case SVC:
			switch(n) {
				case R13: n = R13_svc; break;
				case R14: n = R14_svc; break;
				default:break;
			}
		break;
		case FIQ:
			switch(n) {
				case R8: n = R8_fiq; break;
				case R9: n = R9_fiq; break;
				case R10: n = R10_fiq; break;
				case R11: n = R11_fiq; break;
				case R12: n = R12_fiq; break;
				case R13: n = R13_fiq; break;
				case R14: n = R14_fiq; break;
				default:break;
			}
		break;
		default: break;
	}
	return r->tab[n];
}

uint32_t read_usr_register(registers r, uint8_t reg) {
	return r->tab[reg];
}

uint32_t read_cpsr(registers r) {
	reg_name n = RCPSR;
    return r->tab[n];
}

uint32_t read_spsr(registers r) {
	reg_name n;
	uint16_t mode = get_mode(r);
	switch(mode) {
		case IRQ: n = SPSR_irq; break;
		case FIQ: n = SPSR_fiq;break;
		case SVC: n = SPSR_svc;break;
		case ABT: n = SPSR_abt;break;
		case UND: n = SPSR_und;break;
		default: break;
	}
	return r->tab[n];
}

void write_register(registers r, uint8_t reg, uint32_t value) {
	reg_name n = reg;
	uint16_t mode = get_mode(r);
	switch(mode) {
		case IRQ:
			switch(n) {
				case R13: n = R13_irq; break;
				case R14: n = R14_irq; break;
				default:break;
			}
		break;
		case ABT:
			switch(n) {
				case R13: n = R13_abt; break;
				case R14: n = R14_abt; break;
				default:break;
			}
		case UND:
			switch(n) {
				case R13: n = R13_und; break;
				case R14: n = R14_und; break;
				default:break;
			}
		break;
		case SVC:
			switch(n) {
				case R13: n = R13_svc; break;
				case R14: n = R14_svc; break;
				default:break;
			}
		break;
		case FIQ:
			switch(n) {
				case R8: n = R8_fiq; break;
				case R9: n = R9_fiq; break;
				case R10: n = R10_fiq; break;
				case R11: n = R11_fiq; break;
				case R12: n = R12_fiq; break;
				case R13: n = R13_fiq; break;
				case R14: n = R14_fiq; break;
				default:break;
			}
		break;
		default: break;
	}
	r->tab[n] = value;
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
	r->tab[reg] = value;
}

void write_cpsr(registers r, uint32_t value) {
	reg_name n = RCPSR;
	r->tab[n] = value;
}

void write_spsr(registers r, uint32_t value) {
	uint16_t mode = get_mode(r);
	reg_name n;
	switch(mode) {
		case IRQ: n = SPSR_irq; r->tab[n] = value; break;
		case FIQ: n = SPSR_fiq; r->tab[n] = value; break;
		case SVC: n = SPSR_svc; r->tab[n] = value; break;
		case ABT: n = SPSR_abt; r->tab[n] = value; break;
		case UND: n = SPSR_und; r->tab[n] = value; break;
		default: break;
	}
	r->tab[n] = value;
}
