#include "flqlist.h"

void C74_EXPORT main() {

	t_class *c;

	c = class_new("flqlist",(method)fl_qlist_new, (method)fl_qlist_free,sizeof(t_fl_qlist), 0, 0);
	class_addmethod(c, (method)fl_qlist_assist,"assist", A_CANT, 0);
	class_addmethod(c,(method)fl_qlist_int, "int", A_LONG, 0);
	class_addmethod(c,(method)fl_qlist_float, "float", A_LONG, 0);
	class_addmethod(c,(method)fl_qlist_add2q, "list", A_GIMME, 0);
	class_addmethod(c,(method)fl_qlist_add2q, "anything", A_GIMME, 0);
	class_addmethod(c,(method)fl_qlist_queuesize, "queue_size", A_GIMME, 0);
	class_addmethod(c,(method)fl_qlist_bang, "bang", 0);

	class_register(CLASS_BOX, c);
	fl_qlist_class = c; 
}

void *fl_qlist_new(t_symbol *s, short argc, t_atom *argv) 
{
	t_fl_qlist *x = (t_fl_qlist *)object_alloc(fl_qlist_class);

	x->m_outlet2 = intout((t_object *)x);
	x->m_outlet = outlet_new((t_object *)x, NULL);

	x->len_hqueue = DFLT_MAX_LEN_QUEUE;
	x->h_queue = (t_atom **)sysmem_newptr(DFLT_MAX_LEN_QUEUE * sizeof(t_atom *));
	if (!x->h_queue) { object_error((t_object *)x, "no memory for queue"); }
	else{
		for (long i = 0; i < DFLT_MAX_LEN_QUEUE; i++) {
			x->h_queue[i] = (t_atom *)sysmem_newptr(sizeof(t_atom));
			if (!x->h_queue[i]) { object_error((t_object *)x, "no memory for message in queue"); }
		}
	}
	x->len_mssgs = (long *)sysmem_newptr(DFLT_MAX_LEN_QUEUE * sizeof(long));
	if (!x->len_mssgs) { object_error((t_object *)x, "no memory for messages sizes array"); }
	else {
		for (long i = 0; i < DFLT_MAX_LEN_QUEUE; i++) {
			x->len_mssgs[i] = 0;
		}
	}

	x->iindex = 0;
	x->findex = -1;
	
	x->total_elem = 0;

	return x;
}

void fl_qlist_assist(t_fl_qlist *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {										
		switch (arg) {
		case I_INPUT: sprintf(dst, "(bang)output list from queue/(message)any message to add to queue"); break;
		}

	}
	else if (msg == ASSIST_OUTLET) {    
		switch (arg) {
		case O_OUTPUT: sprintf(dst, "message from queue"); break;
		case O_OUTPUT2: sprintf(dst, "n° elements in queue"); break;
		}
	}
}

void fl_qlist_free(t_fl_qlist *x)
{
	for (long i = 0; i < x->len_hqueue; i++) { sysmem_freeptr(x->h_queue[i]); }
	sysmem_freeptr(x->h_queue);
	sysmem_freeptr(x->len_mssgs);
}

void fl_qlist_float(t_fl_qlist *x, double f) 
{}

void fl_qlist_int(t_fl_qlist *x, long n) 
{}

void fl_qlist_bang(t_fl_qlist *x)
{
	long len_q = x->len_hqueue;
	long findex = x->findex;
	long iindex = x->iindex;
	long total_elem = x->total_elem;
	long ac = 0;

	t_max_err m_error = MAX_ERR_NONE;

	t_atom *p_atom = (t_atom *)sysmem_newptr(sizeof(t_atom));

	//copy message
	critical_enter(0);
	if (x->total_elem <= 0) { m_error = -5; }
	else {
		ac = x->len_mssgs[findex];

		p_atom = (t_atom *)sysmem_resizeptr(p_atom, ac * sizeof(t_atom));
		if (!p_atom) { m_error = MAX_ERR_OUT_OF_MEM; }
		else {
			if (!x->h_queue[findex]) { m_error = MAX_ERR_INVALID_PTR; }
			else {
				m_error = atom_getatom_array(ac, x->h_queue[findex], ac, p_atom);
				if (!m_error) {
					x->findex = (findex + 1) % len_q;
					total_elem = --x->total_elem;
				}
			}
		}
	}
	critical_exit(0);
	//---------------

	sysmem_freeptr(p_atom);

	if (m_error == -5) { object_warn((t_object *)x, "queue is empty"); return; }
	if (m_error == MAX_ERR_OUT_OF_MEM) { object_error((t_object *)x, "out of memory for atom output"); return; }
	if (m_error == MAX_ERR_INVALID_PTR) { object_error((t_object *)x, "couldn't find a message"); return; }
	if (m_error) { object_error((t_object *)x, "couldn't fetch atom array"); return; }

	outlet_anything(x->m_outlet, gensym("queue"), (short)ac, p_atom);
	outlet_int(x->m_outlet2, total_elem);
}

void fl_qlist_add2q(t_fl_qlist *x, t_symbol *s, long argc, t_atom *argv)
{
	long ac = argc;
	t_atom *ap = argv;

	long len_q = x->len_hqueue;
	long findex = x->findex;
	long iindex = x->iindex;
	long total_elem = 0;

	t_max_err m_error = MAX_ERR_NONE;

	//first message added
	if (findex < 0) { findex = 0; }

	//add element
	critical_enter(0);
	if (x->total_elem >= len_q) { m_error = -5; }
	else {
		x->h_queue[iindex] = (t_atom *)sysmem_resizeptr(x->h_queue[iindex], ac * sizeof(t_atom));
		if (!x->h_queue[iindex]) { m_error = MAX_ERR_OUT_OF_MEM; }
		else {
			m_error = atom_getatom_array(ac, ap, ac, x->h_queue[iindex]);
			if (!m_error) {
				x->len_mssgs[iindex] = ac;
				x->iindex = (iindex + 1) % len_q;
				total_elem = ++x->total_elem;
			}
		}

	}
	critical_exit(0);
	//---------------

	if (m_error == -5) { object_warn((t_object *)x, "queue is full"); return; }	
	if (m_error == MAX_ERR_OUT_OF_MEM) { object_error((t_object *)x, "couldn't allocate memory for this message"); return; }	
	if (m_error) { object_error((t_object *)x, "couldn't fetch atom array"); return; }

	x->findex = findex;
	outlet_int(x->m_outlet2, total_elem);
}

void fl_qlist_queuesize(t_fl_qlist *x, t_symbol *s, long argc, t_atom *argv) 
{
	long ac = argc;
	t_atom *ap = argv;
	long new_queue_size = 0;
	long old_queue_size = x->len_hqueue;

	t_max_err m_error = MAX_ERR_NONE;

	if (ac != 1) { object_error((t_object *)x, "queue_size must have 1 argument"); return; }
	if (atom_gettype(ap) != A_LONG) { object_error((t_object *)x, "argument must be a number"); return; }

	new_queue_size = (long)atom_getlong(ap);
	if (new_queue_size <= 0) { object_error((t_object *)x, "argument must be a positive number"); return; }

	critical_enter(0);
	//free memory for every message
	for (long i = 0; i < new_queue_size; i++) { sysmem_freeptr(x->h_queue[i]); }

	//resize queue
	x->h_queue = (t_atom **)sysmem_resizeptr(x->h_queue, new_queue_size * sizeof(t_atom *));
	if (!x->h_queue) { m_error = MAX_ERR_OUT_OF_MEM; }
	else {
		for (long i = 0; i < new_queue_size; i++) {
			x->h_queue[i] = (t_atom *)sysmem_newptr(sizeof(t_atom));
			if (!x->h_queue[i]) { m_error = MAX_ERR_OUT_OF_MEM; }
		}
	}

	x->len_hqueue = new_queue_size;
	x->total_elem = 0;
	x->findex = -1;
	x->iindex = 0;

	critical_exit(0);

	if(m_error){ object_error((t_object *)x, "couldn't allocate memory for queue/messages"); return; }

	object_post((t_object *)x, "queue now has %d elements", new_queue_size);
}