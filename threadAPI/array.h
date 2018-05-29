/***************************************************************************//**
 * @file array.h
 * @author Dorian Weber
 * @brief Enthält die Schnittstelle eines generalisierten Arrays.
 * @details
 * Hier ist ein Beispiel für die Benutzung des Arrays:
 * @code
 * int* array;
 * 
 * arrayInit(array);
 * arrayPush(array) = 1;
 * arrayPush(array) = 2;
 * arrayPush(array) = 3;
 * 
 * while (!arrayIsEmpty(array))
 * 	printf("%i\n", arrayPop(array));
 * 
 * arrayRelease(array);
 * @endcode
 * mit der Ausgabe
 * @code
 * 3
 * 2
 * 1
 * @endcode
 * 
 * Jede Art von Daten kann auf diese Weise als Array organisiert werden, sogar
 * komplexe Strukturen. Eine Einschränkung ist, dass man keine Zeiger auf
 * Arrayelemente halten sollte, da das Array beim Einfügen neuer Elemente unter
 * Umständen relokalisiert werden muss, was absolute Zeiger auf Elemente
 * invalidiert. Um auf Elemente zu verweisen sollten Indizes verwendet werden,
 * es sei denn du weißt was du tust!
 ******************************************************************************/

#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

/* *** includes ************************************************************* */

#include <stddef.h>

/* *** structures *********************************************************** */

/**@brief Arrayheader.
 * 
 * Diese Struktur wird jedem Array im Speicher vorangestellt und beinhaltet
 * Informationen über Kapazität und aktuelle Auslastung des Arrays. Die
 * Arrayelemente schließen sich dieser Struktur unmittelbar an, so dass der
 * Nutzer von dieser versteckten Information nichts bemerkt.
 */
typedef struct array_hdr_s
{
	unsigned int len; /**<@brief Anzahl der Elemente im Array. */
	unsigned int cap; /**<@brief Kapazität des reservierten Speichers. */
} array_hdr_t;

/* *** interface ************************************************************ */

/**@internal
 * @brief Initialisiert und gibt einen Zeiger auf den Start des Arrays zurück.
 * @param size      Größe der Arrayelemente
 * @param capacity  initiale Kapazität
 * @return ein Zeiger auf den Start des Arrays, falls erfolgreich,\n
 *      \c NULL im Falle eines Speicherfehlers
 */
extern void* arrayInit(unsigned int capacity, size_t size);

/**@brief Initialisiert ein neues Array.
 * @param self  das zu initialisierende Array
 * @return 0, falls keine Fehler bei der Initialisierung aufgetreten sind\n
 *      != 0 ansonsten
 */
#define arrayInit(self) \
	((self = arrayInit(8, sizeof((self)[0]))) == NULL ? -1 : 0)

/**@brief Gibt das Array und alle assoziierten Strukturen frei.
 * @param self  das freizugebende Array
 */
extern void arrayRelease(void* self);

/**@brief Setzt die Länge des Arrays auf 0 zurück.
 * @param self  das zu leerende Array
 */
extern void arrayClear(void* self);

/**@internal
 * @brief Reserviert Platz für einen neuen Wert im Array.
 * @param self  das Array
 * @param size  Größe der Arrayelemente
 * @return der neue Zeiger auf den Start des Arrays
 */
extern void* arrayPush(void* self, size_t size);

/**@brief Hängt ein Element an das Ende des Arrays.
 * @param self  das Array
 */
#define arrayPush(self) \
	(self = arrayPush(self, sizeof((self)[0])), (self)+arrayCount(self)-1)[0]

/**@brief Entfernt das oberste Element des Arrays.
 * @param self  das Array
 */
extern void arrayPop(void* self);

/**@brief Entfernt und liefert das oberste Element des Arrays.
 * @param self  das Array
 * @return das oberste Element von \p self
 */
#define arrayPop(self) \
	(arrayPop(self), (self)+arrayCount(self))[0]

/**@brief Gibt das oberste Element des Arrays zurück.
 * @param self  das Array
 * @return das oberste Element von \p self
 */
#define arrayTop(self) \
	(self)[arrayCount(self) - 1]

/**@brief Gibt zurück, ob das Array leer ist.
 * @param self  das Array
 * @return 0, falls nicht leer\n
        != 0, falls leer
 */
extern int arrayIsEmpty(const void* self);

/**@brief Gibt die Anzahl der Elemente im Array zurück.
 * @param self  das Array
 * @return Anzahl der Elemente in \p self
 */
extern unsigned int arrayCount(const void* self);

#endif /* ARRAY_H_INCLUDED */
