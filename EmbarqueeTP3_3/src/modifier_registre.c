/*
 * modifier_registre.c
 */
#include <stdint.h> // uint32_t

/* Une fonction qui passe à 1 l'un des bits d'un registre.
 * @param ptr_registre       Un pointeur volatile sur uint32_t (pointeur sur le registre)
 * @param index_bit_registre le numéro d'index du bit visé.
 */
void modifier_registre_1(volatile uint32_t* ptr_registre, uint32_t index_bit_registre) {
	// Masquage via un ou logique. Si le bit est à 0, il passe à 1, si il est déjà a 1, il reste a 1.
	// On fait un OU logique, on va positionner un 1, on fait un décalage sur la gauche.
	// 1 << 0 nous donne 1, 1 << 1 nous donne 2, 1 << 2 nous donne 4 ...
	*ptr_registre = *ptr_registre | (1 << index_bit_registre);
}


/* Une fonction qui passe à 0 l'un des bits d'un registre.
 * @param ptr_registre       Un pointeur volatile sur uint32_t (pointeur sur le registre)
 * @param index_bit_registre le numéro d'index du bit visé.
 */
void modifier_registre_0(volatile uint32_t* ptr_registre, uint32_t index_bit_registre) {
	*ptr_registre = *ptr_registre & ~(1 << index_bit_registre);
}

/* Une fonction de tests unitaires pour tester les fonctions modifier_registre_1()
 * et modifier_registre_0()
 * @param espace_memoire Le pointeur sur lequel les tests seront effectués
 * @return Cette fonction ne retourne 0 que si tous les tests ont l'effet attendu.
 */
int tests_unitaires(volatile uint32_t* espace_memoire) {
	int erreurs = 0;
	int i;

	uint32_t espace_memoire_etat_precedent = *espace_memoire;
	for(i = -64 ; i <= 64 ; i++) {
		modifier_registre_1(espace_memoire, i);

		// Chaque décalage à gauche <=> *2, sauf si i % 32 == 0
		// 1 << 0 nous donne 1, 1 << 1 nous donne 2, 1 << 2 nous donne 4 ...
		erreurs = erreurs + (i % 32 != 0 && ((*espace_memoire) / espace_memoire_etat_precedent) != 2 );

		espace_memoire_etat_precedent = *espace_memoire;
		modifier_registre_0(espace_memoire, i);
	}

	return erreurs;
}
