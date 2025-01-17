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
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
#include <stdint.h>

#define NBREG 37

typedef enum {
	// base
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13, // SP
	R14, // LR
	PC,
	RCPSR, // R rajouter a CPSR pour enlever l'embeguit� entre CPSR de trace.h et registre.h
	
	// supervisor variants
	R13_svc,
	R14_svc,
	SPSR_svc,
	// abort variants
	R13_abt,
	R14_abt,
	SPSR_abt,
	// undefined variants
	R13_und,
	R14_und,
	SPSR_und,
	// interrupt variants
	R13_irq,
	R14_irq,
	SPSR_irq,

	// fast interrupt variants
	R8_fiq,
	R9_fiq,
	R10_fiq,
	R11_fiq,
	R12_fiq,
	R13_fiq,
	R14_fiq,
	SPSR_fiq
} reg_name;

typedef struct registers_data *registers;

registers registers_create();
void registers_destroy(registers r);

uint16_t get_mode(registers r);
void set_mode(registers r, uint16_t mode);

int current_mode_has_spsr(registers r);
int in_a_privileged_mode(registers r);

uint32_t read_register(registers r, uint8_t reg);
uint32_t read_usr_register(registers r, uint8_t reg);
uint32_t read_cpsr(registers r);
uint32_t read_spsr(registers r);
void write_register(registers r, uint8_t reg, uint32_t value);
void write_usr_register(registers r, uint8_t reg, uint32_t value);
void write_cpsr(registers r, uint32_t value);
void write_spsr(registers r, uint32_t value);

#endif
