/*
 * modifier_registre.h
 */

#ifndef MODIFIER_REGISTRE_H_
#define MODIFIER_REGISTRE_H_

void modifier_registre_1(volatile uint32_t* ptr_registre, uint32_t index_bit_registre);
void modifier_registre_0(volatile uint32_t* ptr_registre, uint32_t index_bit_registre);
int tests_unitaires(volatile uint32_t* espace_memoire);

#endif /* MODIFIER_REGISTRE_H_ */
