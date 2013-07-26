#include "madx.h"

static void
remove_from_node_list(struct node* node, struct node_list* nodes)
{
  int i;
  if ((i = remove_from_name_list(node->name, nodes->list)) > -1)
    nodes->nodes[i] = nodes->nodes[--nodes->curr];
}

#if 0 // not used...
static double
spec_node_value(char* par, int* number)
  /* returns value for parameter par of specified node (start = 1 !!) */
{
  double value = zero;
  struct node* node = current_node;
  int n = *number + current_sequ->start_node - 1;
  if (0 <= n && n < current_sequ->n_nodes)
  {
    current_node = current_sequ->all_nodes[n];
    value = node_value(par);
    current_node = node;
  }
  return value;
}
#endif

// public interface

struct node*
new_node(char* name)
{
  const char *rout_name = "new_node";
  struct node* p = mycalloc(rout_name, 1, sizeof *p);
  strcpy(p->name, name);
  p->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", p->name);
  return p;
}

struct node*
clone_node(struct node* p, int flag)
{
  /* Transfers errors from original nodes if flag != 0;
     this is needed for SXF input  */
  struct node* clone = new_node(p->name);
  strcpy(clone->name,p->name);
  clone->base_name = p->base_name;
  clone->occ_cnt = p->occ_cnt;
  clone->sel_err = p->sel_err;
  clone->position = p->position;
  clone->at_value = p->at_value;
  clone->length = p->length;
  clone->at_expr = p->at_expr;
  clone->from_name = p->from_name;
  clone->p_elem = p->p_elem;
  clone->p_sequ = p->p_sequ;
  clone->savebeta = p->savebeta;
  if (flag) {
    clone->p_al_err = p->p_al_err;
    clone->p_fd_err = p->p_fd_err;
    // AL: RF-Multipole errors (EFCOMP)
    clone->p_ph_err = p->p_ph_err;
  }
  // AL: RF-Multipole
  //clone->pnl = p->pnl;
  //clone->psl = p->psl;
  clone->rfm_lag = p->rfm_lag;
  clone->rfm_freq = p->rfm_freq;
  clone->rfm_volt = p->rfm_volt;
  clone->rfm_harmon = p->rfm_harmon;
  //  
  return clone;
}

struct node*
delete_node(struct node* p)
{
  const char *rout_name = "delete_node";
  if (p == NULL) return NULL;
  if (stamp_flag && p->stamp != 123456)
    fprintf(stamp_file, "d_n double delete --> %s\n", p->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", p->name);
  if (p->p_al_err) p->p_al_err = delete_double_array(p->p_al_err);
  if (p->p_fd_err) p->p_fd_err = delete_double_array(p->p_fd_err);
  // AL: RF-Multipole errors (EFCOMP)
  if (p->p_ph_err) p->p_ph_err = delete_double_array(p->p_ph_err);
  // AL: RF-Multipole
  // if (p->pnl) p->pnl = delete_double_array(p->pnl);
  // if (p->psl) p->psl = delete_double_array(p->psl);
  //
  myfree(rout_name, p);
  return NULL;
}

struct node_list*
new_node_list(int length)
{
  const char *rout_name = "new_node_list";
  struct node_list* nll = mycalloc(rout_name, 1, sizeof *nll);
  strcpy(nll->name, "node_list");
  nll->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", nll->name);
  nll->list = new_name_list(nll->name, length);
  nll->nodes = mycalloc(rout_name, length, sizeof *nll->nodes);
  nll->max = length;
  return nll;
}

struct node_list*
delete_node_list(struct node_list* l)
{
  const char *rout_name = "delete_node_list";
  if (l == NULL)  return NULL;
  if (stamp_flag && l->stamp != 123456)
    fprintf(stamp_file, "d_no_l double delete --> %s\n", l->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", l->name);
  if (l->nodes != NULL)  myfree(rout_name, l->nodes);
  if (l->list != NULL)  delete_name_list(l->list);
  myfree(rout_name, l);
  return NULL;
}

struct node*
delete_node_ring(struct node* start)
{
  struct node *p, *q;
  if (start == NULL) return NULL;
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", "node_ring");
  q = start->next;
  while (q != NULL && q != start)
  {
    p = q; q = q->next; delete_node(p);
  }
  delete_node(start);
  return NULL;
}

static void
grow_node_list(struct node_list* p)
{
  const char *rout_name = "grow_node_list";
  p->max *= 2;
  p->nodes = myrecalloc(rout_name, p->nodes, p->curr * sizeof *p->nodes, p->max * sizeof *p->nodes);
}

void
dump_node(struct node* node)
{
  int i;
  char pname[NAME_L] = "NULL", nname[NAME_L] = "NULL", 
    from_name[NAME_L] = "NULL";
  if (node->previous != NULL) strcpy(pname, node->previous->name);
  if (node->next != NULL) strcpy(nname, node->next->name);
  if (node->from_name != NULL) strcpy(from_name, node->from_name);
  fprintf(prt_file, 
          v_format("name: %S  occ: %I base: %S  from_name: %S at_value: %F  position: %F\n"),
          node->name, node->occ_cnt, node->base_name, from_name, 
          node->at_value, node->position);
  fprintf(prt_file, v_format("  names of - previous: %S  next: %S\n"),
          pname, nname);
  if (node->cl != NULL)  for (i = 0; i < node->cl->curr; i++)
    dump_constraint(node->cl->constraints[i]);
}

struct node*
expand_node(struct node* node, struct sequence* top_sequ, struct sequence* sequ, double position)
  /* replaces a (sequence) node by a sequence of nodes - recursive */
{
  struct sequence* nodesequ = node->p_sequ;
  struct node *p, *q = nodesequ->start;
  int i;
  (void)sequ;

  int debug;
  debug=get_option("debug");

  p = clone_node(q, 0);
  
  if (debug) printf("\np->name: %s\n",p->name); 

  if ((i = name_list_pos(p->p_elem->name, occ_list)) < 0)
    i = add_to_name_list(p->p_elem->name, 1, occ_list);
  else strcpy(p->name, compound(p->p_elem->name, ++occ_list->inform[i]));
  add_to_node_list(p, 0, top_sequ->ex_nodes);
  p->previous = node->previous;
  p->previous->next = p;
  p->position = position + get_node_pos(p, nodesequ) - get_refpos(nodesequ);

  if (debug) printf("elem_name at position + get_node_pos - get_refpos = p->position : \n %s: %e + %e - %e = %e \n",
		     p->p_elem->name, position, get_node_pos(p, nodesequ), get_refpos(nodesequ), p->position); 
  
  while (p != NULL)
  {
    if (q == nodesequ->end) break;
    if (strcmp(p->base_name, "rfcavity") == 0 &&
        find_element(p->p_elem->name, top_sequ->cavities) == NULL)
      add_to_el_list(&p->p_elem, 0, top_sequ->cavities, 0);
    p->next = clone_node(q->next, 0);
    p->next->previous = p;
    p = p->next;
    q = q->next;
    p->position = position + get_node_pos(p, nodesequ) - get_refpos(nodesequ);

    if (debug) printf("elem name at position + get_node_pos - get_refpos = p->position : \n %s: %e + %e - %e = %e\n",
		      p->p_elem->name, position, get_node_pos(p, nodesequ), get_refpos(nodesequ), p->position); 

    if (p->p_sequ != NULL) 
      p = expand_node(p, top_sequ, p->p_sequ, p->position);
    else
    {
      p->share = top_sequ->share;
      add_to_node_list(p, 0, top_sequ->ex_nodes);
    }
  }
  delete_node(node);
  return p;
}

double
get_node_pos(struct node* node, struct sequence* sequ) /*recursive */
  /* returns node position from declaration for expansion */
{
  double fact = 0.5 * sequ->ref_flag; /* element half-length offset */
  double pos, from = 0;

  if (loop_cnt++ == MAX_LOOP)
  {
    sprintf(c_dum->c, "%s   occurrence: %d", node->p_elem->name, node->occ_cnt);
    fatal_error("circular call in position of", c_dum->c);
  }

  if (node->at_expr == NULL) pos = node->at_value;
  else                       pos = expression_value(node->at_expr, 2);

  if (get_option("debug")) printf("in get_node_pos: name: %s, pos: %e, fact: %e, length: %e",
	 node->p_elem->name, pos, fact, node->length); 

  pos += fact * node->length; /* make centre position */

  if (node->from_name != NULL)
  {
    if ((from = hidden_node_pos(node->from_name, sequ)) == INVALID)
      fatal_error("'from' reference to unknown element:", node->from_name);
  }

  loop_cnt--;
  pos += from;

  if (get_option("debug")) printf(", from: %e  ---> final pos: %e \n", from, pos);

  return pos;
}

double
node_value(char* par)
  /* returns value for parameter par of current element */
{
  double value;
  char lpar[NAME_L];
  mycpy(lpar, par);
  if (strcmp(lpar, "l") == 0) value = current_node->length;
/*  else if (strcmp(lpar, "dipole_bv") == 0) value = current_node->dipole_bv;*/
  else if (strcmp(lpar, "other_bv") == 0) value = current_node->other_bv;
  else if (strcmp(lpar, "chkick") == 0) value = current_node->chkick;
  else if (strcmp(lpar, "cvkick") == 0) value = current_node->cvkick;
  else if (strcmp(lpar, "obs_point") == 0) value = current_node->obs_point;
  else if (strcmp(lpar, "sel_sector") == 0) value = current_node->sel_sector;
  else if (strcmp(lpar, "enable") == 0) value = current_node->enable;
  else if (strcmp(lpar, "occ_cnt") == 0) value = current_node->occ_cnt;
  else if (strcmp(lpar, "pass_flag") == 0) value = current_node->pass_flag;
/* AL: added by A. Latina 16 Feb 2012 */
  else if (strcmp(lpar, "rfm_freq") == 0) value = current_node->rfm_freq;
  else if (strcmp(lpar, "rfm_volt") == 0) value = current_node->rfm_volt;
  else if (strcmp(lpar, "rfm_lag") == 0) value = current_node->rfm_lag;
  else if (strcmp(lpar, "rfm_harmon") == 0) value = current_node->rfm_harmon;
//
  else value =  element_value(current_node, lpar);
  return value;
}

void
link_in_front(struct node* new, struct node* el)
  /* links a node 'new' in front of a node 'el' */
{
  el->previous->next = new;
  new->previous = el->previous; new->next = el;
  el->previous = new;
}

double
hidden_node_pos(char* name, struct sequence* sequ) /*recursive */
  /* (for 'from' calculation:) returns the position of a node
     in the current or a sub-sequence (hence hidden) */
{
  double pos;
  struct node* c_node;
  char tmp[NAME_L];
  int i;
  strcpy(tmp, name);
  square_to_colon(tmp);

  if ((i = name_list_pos(tmp, sequ->nodes->list)) > -1)
    return get_node_pos(sequ->nodes->nodes[i], sequ);

  else
  {
    c_node = sequ->start;
    while (c_node != NULL)
    {
      if (c_node->p_sequ != NULL)
      {
        if ((pos = hidden_node_pos(name, c_node->p_sequ)) != INVALID)
        {
          pos += get_node_pos(c_node, sequ) - get_refpos(c_node->p_sequ);
          return pos;
        }
      }
      if (c_node == sequ->end) break;
      c_node = c_node->next;
    }
    return INVALID;
  }
}

void
resequence_nodes(struct sequence* sequ)
  /* resequences occurrence list */
{
  struct node* c_node = sequ->start;
  int i, cnt;
  while (c_node != NULL)
  {
    if (c_node->p_elem != NULL)
    {
      if ((i = name_list_pos(c_node->p_elem->name, occ_list)) < 0)
      {
        i = add_to_name_list(c_node->p_elem->name, 1, occ_list);
        cnt = 1;
      }
      else cnt = ++occ_list->inform[i];
      strcpy(c_node->name, compound(c_node->p_elem->name, cnt));
    }
    if (c_node == sequ->end) break;
    c_node = c_node->next;
  }
}

int
retreat_node(void)
  /* replaces current node by previous node; 0 = already at start, else 1 */
{
  if (current_node == current_sequ->range_start)  return 0;
  current_node = current_node->previous;
  return 1;
}

void
store_node_value(char* par, double* value)
  /* stores value for parameter par at current node */
{
  char lpar[NAME_L];
  struct element* el = current_node->p_elem;

  mycpy(lpar, par);
  if (strcmp(lpar, "chkick") == 0) current_node->chkick = *value;
  else if (strcmp(lpar, "cvkick") == 0) current_node->cvkick = *value;
/*  else if (strcmp(lpar, "dipole_bv") == 0) current_node->dipole_bv = *value;*/
  else if (strcmp(lpar, "other_bv") == 0) current_node->other_bv = *value;
  else if (strcmp(lpar, "obs_point") == 0) current_node->obs_point = *value;
  else if (strcmp(lpar, "sel_sector") == 0) current_node->sel_sector = *value;
  else if (strcmp(lpar, "enable") == 0) current_node->enable = *value;

  /* added by E. T. d'Amico 27 feb 2004 */

  else if (strcmp(lpar, "e1") == 0) store_comm_par_value("e1",*value,el->def);
  else if (strcmp(lpar, "e2") == 0) store_comm_par_value("e2",*value,el->def);
  else if (strcmp(lpar, "angle") == 0) store_comm_par_value("angle",*value,el->def);

  /* added by E. T. d'Amico 12 may 2004 */

  else if (strcmp(lpar, "h1") == 0) store_comm_par_value("h1",*value,el->def);
  else if (strcmp(lpar, "h2") == 0) store_comm_par_value("h2",*value,el->def);
  else if (strcmp(lpar, "fint") == 0) store_comm_par_value("fint",*value,el->def);
  else if (strcmp(lpar, "fintx") == 0) store_comm_par_value("fintx",*value,el->def);
  else if (strcmp(lpar, "hgap") == 0) store_comm_par_value("hgap",*value,el->def);
  else if (strcmp(lpar, "pass_flag") == 0) current_node->pass_flag = *value;

  /* AL: added by A. Latina 16 Feb 2012 */

  else if (strcmp(lpar, "rfm_freq") == 0) store_comm_par_value("rfm_freq", *value, el->def);
  else if (strcmp(lpar, "rfm_volt") == 0) store_comm_par_value("rfm_volt", *value, el->def);
  else if (strcmp(lpar, "rfm_lag") == 0) store_comm_par_value("rfm_lag", *value, el->def);
  else if (strcmp(lpar, "rfm_harmon") == 0) store_comm_par_value("rfm_harmon", *value, el->def);

 /* added by FRS 29 August 2012 */

  else if (strcmp(lpar, "rm11") == 0) store_comm_par_value("rm11",*value,el->def);
  else if (strcmp(lpar, "rm12") == 0) store_comm_par_value("rm12",*value,el->def);
  else if (strcmp(lpar, "rm13") == 0) store_comm_par_value("rm13",*value,el->def);
  else if (strcmp(lpar, "rm14") == 0) store_comm_par_value("rm14",*value,el->def);
  else if (strcmp(lpar, "rm15") == 0) store_comm_par_value("rm15",*value,el->def);
  else if (strcmp(lpar, "rm16") == 0) store_comm_par_value("rm16",*value,el->def);
  else if (strcmp(lpar, "rm21") == 0) store_comm_par_value("rm21",*value,el->def);
  else if (strcmp(lpar, "rm22") == 0) store_comm_par_value("rm22",*value,el->def);
  else if (strcmp(lpar, "rm23") == 0) store_comm_par_value("rm23",*value,el->def);
  else if (strcmp(lpar, "rm24") == 0) store_comm_par_value("rm24",*value,el->def);
  else if (strcmp(lpar, "rm25") == 0) store_comm_par_value("rm25",*value,el->def);
  else if (strcmp(lpar, "rm26") == 0) store_comm_par_value("rm26",*value,el->def);
  else if (strcmp(lpar, "rm31") == 0) store_comm_par_value("rm31",*value,el->def);
  else if (strcmp(lpar, "rm32") == 0) store_comm_par_value("rm32",*value,el->def);
  else if (strcmp(lpar, "rm33") == 0) store_comm_par_value("rm33",*value,el->def);
  else if (strcmp(lpar, "rm34") == 0) store_comm_par_value("rm34",*value,el->def);
  else if (strcmp(lpar, "rm35") == 0) store_comm_par_value("rm35",*value,el->def);
  else if (strcmp(lpar, "rm36") == 0) store_comm_par_value("rm36",*value,el->def);
  else if (strcmp(lpar, "rm41") == 0) store_comm_par_value("rm41",*value,el->def);
  else if (strcmp(lpar, "rm42") == 0) store_comm_par_value("rm42",*value,el->def);
  else if (strcmp(lpar, "rm43") == 0) store_comm_par_value("rm43",*value,el->def);
  else if (strcmp(lpar, "rm44") == 0) store_comm_par_value("rm44",*value,el->def);
  else if (strcmp(lpar, "rm45") == 0) store_comm_par_value("rm45",*value,el->def);
  else if (strcmp(lpar, "rm46") == 0) store_comm_par_value("rm46",*value,el->def);
  else if (strcmp(lpar, "rm51") == 0) store_comm_par_value("rm51",*value,el->def);
  else if (strcmp(lpar, "rm52") == 0) store_comm_par_value("rm52",*value,el->def);
  else if (strcmp(lpar, "rm53") == 0) store_comm_par_value("rm53",*value,el->def);
  else if (strcmp(lpar, "rm54") == 0) store_comm_par_value("rm54",*value,el->def);
  else if (strcmp(lpar, "rm55") == 0) store_comm_par_value("rm55",*value,el->def);
  else if (strcmp(lpar, "rm56") == 0) store_comm_par_value("rm56",*value,el->def);
  else if (strcmp(lpar, "rm61") == 0) store_comm_par_value("rm61",*value,el->def);
  else if (strcmp(lpar, "rm62") == 0) store_comm_par_value("rm62",*value,el->def);
  else if (strcmp(lpar, "rm63") == 0) store_comm_par_value("rm63",*value,el->def);
  else if (strcmp(lpar, "rm64") == 0) store_comm_par_value("rm64",*value,el->def);
  else if (strcmp(lpar, "rm65") == 0) store_comm_par_value("rm65",*value,el->def);
  else if (strcmp(lpar, "rm66") == 0) store_comm_par_value("rm66",*value,el->def);

  /* end of additions */
}

void
set_new_position(struct sequence* sequ)
  /* sets a new node position for all nodes */
{
  struct node* c_node = sequ->start;
  double zero_pos = c_node->position;
  int flag = 0;
  while (c_node != NULL)
  {
    if (c_node->from_name == NULL)
    {
      c_node->position -= zero_pos;
      if (c_node->position < zero || (flag && c_node->position == zero))
        c_node->position += sequence_length(sequ);
      if (c_node->position > zero) flag = 1;
      c_node->at_value = c_node->position;
      c_node->at_expr = NULL;
    }
    if (c_node == sequ->end) break;
    c_node = c_node->next;
  }
  c_node->position = c_node->at_value = sequence_length(sequ);
}

void
set_node_bv(struct sequence* sequ)
  /* sets bv flag for all nodes */
{
  struct node* c_node = sequ->ex_start;
  double beam_bv;
  beam_bv = command_par_value("bv", current_beam);
  while (c_node != NULL)
  {
    c_node->other_bv = beam_bv;
    c_node->dipole_bv = beam_bv;
/* dipole_bv kill initiative SF TR FS */
/*      if (c_node->p_elem->bv) c_node->dipole_bv = beam_bv;
        else                    c_node->dipole_bv = 1;*/
    if (c_node == sequ->ex_end) break;
    c_node = c_node->next;
  }
}

void
add_to_node_list( /* adds node to alphabetic node list */
  struct node* node, int inf, struct node_list* nll)
{
  // int j, pos; // not used
  if (name_list_pos(node->name, nll->list) < 0) // (pos = not used
  {
    if (nll->curr == nll->max) grow_node_list(nll);
    add_to_name_list(node->name, inf, nll->list); // j = not used
    nll->nodes[nll->curr++] = node;
  }
}

int
count_nodes(struct sequence* sequ)
{
  int count = 1;
  struct node* c_node = sequ->start;
  while(c_node != sequ->end)
  {
    c_node = c_node->next;
    count++;
  }
  return count;
}

void
current_node_name(char* name, int* lg)
/* returns node_name blank padded up to lg */
{
  int i;
  strncpy(name, current_node->name, *lg);
  for (i = strlen(current_node->name); i < *lg; i++)  name[i] = ' ';
}

void
node_name(char* name, int* l)
  /* returns current node name in Fortran format */
  /* l is max. allowed length in name */
{
  int ename_l = strlen(current_node->name);
  int i, ncp = ename_l < *l ? ename_l : *l;
  int nbl = *l - ncp;
  for (i = 0; i < ncp; i++) name[i] = toupper(current_node->name[i]);
  for (i = 0; i < nbl; i++) name[ncp+i] = ' ';
}

int
get_node_count(struct node* node)
  /* finds the count of a node in the current expanded sequence */
{
  int cnt = 0;
  current_node = current_sequ->ex_start;
  while (current_node != NULL)
  {
    if (current_node == node) return cnt;
    cnt++;
    if (current_node == current_sequ->ex_end) break;
    current_node = current_node->next;
  }
  return -1;
}

void
store_node_vector(char* par, int* length, double* vector)
  /* stores vector at node */
{
  char lpar[NAME_L];
  mycpy(lpar, par);
  if (strcmp(lpar, "orbit0") == 0)  copy_double(vector, orbit0, 6);
  else if (strcmp(lpar, "orbit_ref") == 0)
  {
    if (current_node->orbit_ref)
    {
      while (*length > current_node->orbit_ref->max)
        grow_double_array(current_node->orbit_ref);
    }
    else current_node->orbit_ref = new_double_array(*length);
    copy_double(vector, current_node->orbit_ref->a, *length);
    current_node->orbit_ref->curr = *length;
  }
  else if (strcmp(lpar, "surv_data") == 0)  
        copy_double(vector, current_node->surv_data, *length);

}

int
store_no_fd_err(double* errors, int* curr)
{
  if (current_node->p_fd_err == NULL) {
    current_node->p_fd_err = new_double_array(FIELD_MAX);
    current_node->p_fd_err->curr = FIELD_MAX;
  }
  else {
    if(current_node->p_fd_err->curr < *curr)
      grow_double_array(current_node->p_fd_err);
  }
  current_node->p_fd_err->curr = *curr;
  copy_double(errors, current_node->p_fd_err->a,current_node->p_fd_err->curr);
  return current_node->p_fd_err->curr;
}

int
advance_node(void)
  /* advances to next node in expanded sequence;
     returns 0 if end of range, else 1 */
{
  if (current_node == current_sequ->range_end)  return 0;
  current_node = current_node->next;
  return 1;
}

int
advance_to_pos(char* table, int* t_pos)
  /* advances current_node to node at t_pos in table */
{
  struct table* t;
  int pos, cnt = 0, ret = 0;
  mycpy(c_dum->c, table);
  if ((pos = name_list_pos(c_dum->c, table_register->names)) > -1)
  {
    ret = 1;
    t = table_register->tables[pos];
    if (t->origin == 1)  return 0; /* table is read, has no node pointers */
    while (current_node)
    {
      if (current_node == t->p_nodes[*t_pos-1]) break;
      if ((current_node = current_node->next)
          == current_sequ->ex_start) cnt++;
      if (cnt > 1) return 0;
    }
  }
  return ret;
}

double
line_nodes(struct char_p_array* flat)
  /* creates a linked node list from a flat element list of a line */
{
  int i, j, k;
  double pos = zero, val;
  struct element* el;
  for (j = 0; j < flat->curr; j++)
  {
    if ((el = find_element(flat->p[j], element_list)) == NULL)
      fatal_error("line contains unknown element:", flat->p[j]);
    if (strcmp(el->base_type->name, "rfcavity") == 0 &&
        find_element(el->name, current_sequ->cavities) == NULL)
      add_to_el_list(&el, 0, current_sequ->cavities, 0);
    val = el_par_value("l", el);
    pos += val / 2;
    k = 1;
    if ((i = name_list_pos(el->name, occ_list)) < 0)
      i = add_to_name_list(el->name, k, occ_list);
    else k = ++occ_list->inform[i];
    make_elem_node(el, k);
    current_node->at_value = current_node->position = pos;
    pos += val / 2;
  }
  return pos;
}

void
node_string(char* key, char* string, int* l)
  /* returns current node string value for "key" in Fortran format */
  /* l is max. allowed length in string */
{
  char tmp[2*NAME_L];
  char* p;
  int i, l_p, nbl, ncp = 0;
  mycpy(tmp, key);
  if ((p = command_par_string(tmp, current_node->p_elem->def)))
  {
    l_p = strlen(p);
    ncp = l_p < *l ? l_p : *l;
  }
  nbl = *l - ncp;
  for (i = 0; i < ncp; i++) string[i] = p[i];
  for (i = 0; i < nbl; i++) string[ncp+i] = ' ';
}

int
remove_one(struct node* node)
{
  int pos;
  /* removes a node from a sequence being edited */
  if ((pos = name_list_pos(node->p_elem->name, occ_list)) < 0)  return 0;
  if (node->previous != NULL) node->previous->next = node->next;
  if (node->next != NULL) node->next->previous = node->previous;
  if (occ_list->inform[pos] == 1)
  {
    remove_from_node_list(node, edit_sequ->nodes);
    remove_from_name_list(node->p_elem->name, occ_list);
  }
  else --occ_list->inform[pos];
  /* myfree(rout_name, node); */
  return 1;
}

void
replace_one(struct node* node, struct element* el)
  /* replaces an existing node by a new one made from el */
{
  int i, k = 1;
  remove_from_node_list(node, edit_sequ->nodes);
  if ((i = name_list_pos(el->name, occ_list)) < 0)
    i = add_to_name_list(el->name, 1, occ_list);
  else k = ++occ_list->inform[i];
  strcpy(node->name, compound(el->name, k));
  add_to_node_list(node, 0, edit_sequ->nodes);
  node->p_elem = el;
  node->base_name = el->base_type->name;
  node->length = el->length;
  if (strcmp(el->base_type->name, "rfcavity") == 0 &&
      find_element(el->name, edit_sequ->cavities) == NULL)
    add_to_el_list(&el, 0, edit_sequ->cavities, 0);
}

