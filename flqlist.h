#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"  
#include "ext_critical.h"
#include <math.h>

#define DFLT_MAX_LEN_QUEUE 256 

enum INLETS { I_INPUT, NUM_INLETS };
enum OUTLETS { O_OUTPUT, O_OUTPUT2, NUM_OUTLETS };

typedef struct {

	t_object obj;
	
	t_atom **h_queue;
	long *len_mssgs;
	long len_hqueue;
	long findex;
	long iindex;
	long total_elem;

	void *m_outlet;
	void *m_outlet2;

} t_fl_qlist;

void *fl_qlist_new(t_symbol *s, short argc, t_atom *argv);
void fl_qlist_assist(t_fl_qlist *x, void *b, long msg, long arg, char *dst);

void fl_qlist_free(t_fl_qlist *x);

void fl_qlist_int(t_fl_qlist *x, long n);
void fl_qlist_float(t_fl_qlist *x, double f);
void fl_qlist_bang(t_fl_qlist *x);
void fl_qlist_add2q(t_fl_qlist *x, t_symbol *s, long argc, t_atom *argv);
void fl_qlist_queuesize(t_fl_qlist *x, t_symbol *s, long argc, t_atom *argv);

static t_class *fl_qlist_class;