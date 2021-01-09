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
	uint16_t mode = get_mode(r);
	reg_name n = select_register(reg, mode);
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
	uint16_t mode = get_mode(r);
	reg_name n = select_register(reg, mode);
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

// select the register index depend on the actual cpu mode
reg_name select_register(uint8_t reg, uint16_t mode) {
	reg_name n = reg;
	switch(n) {
		case R13:
			switch(mode) {
				case IRQ: n = R13_irq; break;
				case ABT: n = R13_abt; break;							
				case UND: n = R13_und; break;							
				case SVC: n = R13_svc; break;
				case FIQ: n = R13_fiq; break;
				default:break;
			}
		case R14:
			switch(mode) {
				case IRQ: n = R14_irq; break;
				case ABT: n = R14_abt; break;							
				case UND: n = R14_und; break;							
				case SVC: n = R14_svc; break;
				case FIQ: n = R14_fiq; break;
				default:break;												
			}
		default:break;
	}
	switch(mode) {
		case FIQ:
			switch(n) {
				case R8: n = R8_fiq; break;
				case R9: n = R9_fiq; break;
				case R10: n = R10_fiq; break;
				case R11: n = R11_fiq; break;
				case R12: n = R12_fiq; break;
				default:break;
			}
		break;
		default: break;
	}
	return n;
}
