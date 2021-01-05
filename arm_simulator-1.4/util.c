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
#include "util.h"

int condition(uint32_t cpsr, uint8_t cond) {
	unsigned c, n, z, v;
	z = get_bit(cpsr, Z);
	n = get_bit(cpsr, N);
	c = get_bit(cpsr, C);
	v = get_bit(cpsr, V);
	switch(cond) {
		// EQ
		case 0: return z; break;
		// NE
		case 1: return !z; break;
		// CS/HS
		case 2: return c; break;
		// CC/LO
		case 3: return !c; break;
		// MI
		case 4: return n; break;
		// PL
		case 5: return !n; break;
		// VS
		case 6: return v; break;
		// VC
		case 7: return !v; break;
		// HI
		case 8: return c & !z; break;
		// LS
		case 9: return !c || z; break;
		// GE
		case 10: return n == v; break;
		// LT
		case 11: return n != v; break;
		// GT
		case 12: return (!z) && (n == v); break;
		// LE
		case 13: return (z || (n != v)); break;
		// AL
		case 14: return 1; break;
	}
	return 1;
}

/* We implement asr because shifting a signed is non portable in ANSI C */
uint32_t asr(uint32_t value, uint8_t shift) {
    return (value >> shift) | (get_bit(value, 31) ? ~0<<(32-shift) : 0);
}

uint32_t ror(uint32_t value, uint8_t rotation) {
    return (value >> rotation) | (value << (32-rotation));
}

int is_big_endian() {
    static uint32_t one = 1;
    return ((* (uint8_t *) &one) == 0);
}
