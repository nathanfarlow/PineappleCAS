#ifndef IDENTITIES_H_
#define IDENTITIES_H_

#include "../parser.h"

/*Neither the from or the to text can exceed 50 characters*/
#define ID_MAX_TEXT 50

typedef struct {
    char from_text[ID_MAX_TEXT], to_text[ID_MAX_TEXT];
    ast_t *from, *to;
} id_t;

#define ID_NUM_INVERSES 16
extern id_t id_inverses[ID_NUM_INVERSES];
#define ID_NUM_TRIG_IDENTITIES 18
extern id_t id_trig_identities[ID_NUM_TRIG_IDENTITIES];
#define ID_NUM_TRIG_CONSTANTS 46
extern id_t id_trig_constants[ID_NUM_TRIG_CONSTANTS];

bool id_Load(id_t *id);
void id_Unload(id_t *id);
bool id_Execute(ast_t *e, id_t *id);

bool id_ExecuteTable(ast_t *e, id_t *table, unsigned table_len);
void id_UnloadTable(id_t *table, unsigned table_len);
/*Calls unload table for all tables*/
void id_UnloadAll();

#endif