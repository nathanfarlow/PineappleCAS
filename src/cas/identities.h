#ifndef IDENTITIES_H_
#define IDENTITIES_H_

#include "../parser.h"

/*Neither the from or the to text can exceed 50 characters*/
#define ID_MAX_TEXT 50

typedef struct {
    char from_text[ID_MAX_TEXT], to_text[ID_MAX_TEXT];
    pcas_ast_t *from, *to;
} pcas_id_t;

#define ID_NUM_GENERAL 17
extern pcas_id_t id_general[ID_NUM_GENERAL];
#define ID_NUM_TRIG_IDENTITIES 20
extern pcas_id_t id_trig_identities[ID_NUM_TRIG_IDENTITIES];
#define ID_NUM_TRIG_CONSTANTS 40
extern pcas_id_t id_trig_constants[ID_NUM_TRIG_CONSTANTS];
#define ID_NUM_HYPERBOLIC 3
extern pcas_id_t id_hyperbolic[ID_NUM_HYPERBOLIC];
#define ID_NUM_COMPLEX 11
extern pcas_id_t id_complex[ID_NUM_COMPLEX];

bool id_Load(pcas_id_t *id);
void id_Unload(pcas_id_t *id);
bool id_Execute(pcas_ast_t *e, pcas_id_t *id, bool recursive);

bool id_ExecuteTable(pcas_ast_t *e, pcas_id_t *table, unsigned table_len, bool recursive);
void id_UnloadTable(pcas_id_t *table, unsigned table_len);
/*Calls unload table for all tables*/
void id_UnloadAll();

#endif