int add_drifts(struct node* c_node, struct node* end)
{
  struct node *d1;
  struct element *drift;
  double pos, dl, el2;
  int cnt = 0;
  pos = c_node->position
        - c_node->length / two;
  while (c_node != NULL)
    {
     cnt++;
     el2 = c_node->length / two;
     dl = c_node->position - el2 - pos;
     if (dl + ten_m_6 < zero)
       {
        sprintf(c_dummy, "%s, length %e", c_node->name, dl);
        fatal_error("negative drift in front of", c_dummy);
       }
     else if (dl > ten_m_6)
       {
      cnt++;
        drift = get_drift(dl);
        d1 = new_elem_node(drift, 0);
        link_in_front(d1, c_node);
        d1->position = pos + dl / two;
       }
     pos = c_node->position + el2;
     if (c_node == end) break;
     c_node = c_node->next;
    }
  return cnt;
}

void add_table_vars(struct name_list* cols, struct command_list* select)
     /* 1: adds user selected variables to table - always type 2 = double
        2: adds aperture variables apertype (string) + aper_1, aper_2 etc. */
{
  int i, j, k, n, pos;
  char* var_name;
  char tmp[12];
  struct name_list* nl;
  struct command_parameter_list* pl;
  for (i = 0; i < select->curr; i++)
    {
     nl = select->commands[i]->par_names;
     pl = select->commands[i]->par;
     pos = name_list_pos("column", nl);
     if (nl->inform[pos])
       {
      for (j = 0; j < pl->parameters[pos]->m_string->curr; j++)
        {
         var_name = pl->parameters[pos]->m_string->p[j];
         if (strcmp(var_name, "apertype") == 0)
           {
            if ((n = aperture_count(current_sequ)) > 0)
       {
                 add_to_name_list(permbuff("apertype"), 3, cols);
                 for (k = 0; k < n; k++)
          {
           sprintf(tmp, "aper_%d", k+1);
                    add_to_name_list(permbuff(tmp), 2, cols);
          }
       }
           }
         else if (name_list_pos(var_name, cols) < 0) /* not yet in list */
              add_to_name_list(permbuff(var_name), 2, cols);
        }
       }
    }
}

void add_to_command_list(char* label, struct command* comm,
                         struct command_list* cl, int flag)
     /* adds command comm to the command list cl */
     /* flag for printing a warning */
{
  int pos, j;
  if ((pos = name_list_pos(label, cl->list)) > -1)
    {
     if (flag) put_info(label, "redefined");
     if (cl != defined_commands && cl != stored_commands)
         delete_command(cl->commands[pos]);
     cl->commands[pos] = comm;
    }
  else
    {
     if (cl->curr == cl->max) grow_command_list(cl);
     j = add_to_name_list(permbuff(label), 0, cl->list);
     cl->commands[cl->curr++] = comm;
    }
}

void add_to_command_list_list(char* label, struct command_list* cl,
                              struct command_list_list* sl)
     /* adds command list cl to comand-list list sl */
{
  int pos, j;
  if ((pos = name_list_pos(label, sl->list)) > -1)
    {
     delete_command_list(sl->command_lists[pos]);
     sl->command_lists[pos] = cl;
    }
  else
    {
     if (sl->curr == sl->max) grow_command_list_list(sl);
     j = add_to_name_list(permbuff(label), 0, sl->list);
     sl->command_lists[sl->curr++] = cl;
    }
}

void add_to_constraint_list(struct constraint* cs, struct constraint_list* cl)
     /* add constraint cs to the constraint list cl */
{
  if (cl->curr == cl->max) grow_constraint_list(cl);
  cl->constraints[cl->curr++] = cs;
}

void add_to_el_list( /* adds element to alphabetic element list */
        struct element** el, int inf, struct el_list* ell, int flag)
     /* inf is entered in the namelist */
     /*  flag < 0: do not delete if already present, do not warn */
     /*       = 0: delete, but do not warn */
     /*       = 1: delete & warn */
     /*       = 2: warn and ignore if already present - resets *el to old */
{
  int pos, j;
  if ((pos = name_list_pos((*el)->name, ell->list)) > -1)
    {
     if (flag > 1)
       {
        warning("element re-definition inside sequence ignored:", (*el)->name);
        *el = ell->elem[pos];
       }
     else
       {
        if (flag > 0) put_info("element redefined:", (*el)->name);
        if (flag >= 0 && ell == element_list) delete_element(ell->elem[pos]);
        ell->elem[pos] = *el;
       }
    }
  else
    {
     if (ell->curr == ell->max) grow_el_list(ell);
     j = add_to_name_list(permbuff((*el)->name), inf, ell->list);
     ell->elem[ell->curr++] = *el;
    }
}

void add_to_macro_list( /* adds macro to alphabetic macro list */
        struct macro* macro, struct macro_list* nll)
{
  int pos, j;
  if ((pos = name_list_pos(macro->name, nll->list)) > -1)
    {
     warning("macro redefined:", macro->name);
     delete_macro(nll->macros[pos]);
     nll->macros[pos] = macro;
    }
  else
    {
     if (nll->curr == nll->max) grow_macro_list(nll);
     j = add_to_name_list(permbuff(macro->name), 0, nll->list);
     nll->macros[nll->curr++] = macro;
    }
}

int add_to_name_list(char* name, int inf, struct name_list* vlist)
     /* adds name to alphabetic name list vlist */
     /* inf is an integer kept with name */
{
  int j, num, low = 0, mid, high = vlist->curr - 1, pos = 0, ret;

  if (name == NULL) return -1;
  if ((ret = name_list_pos(name, vlist)) < 0)
    {
     while (low <= high)
       {
        mid = (low + high) / 2;
        if ((num = strcmp(name, vlist->names[vlist->index[mid]])) < 0)
                          {high = mid - 1; pos = mid;}
        else if (num > 0) {low  = mid + 1; pos = low;}
       }
     ret = vlist->curr;
     if (vlist->curr == vlist->max) grow_name_list(vlist);
     for (j = vlist->curr; j > pos; j--) vlist->index[j] = vlist->index[j-1];
     vlist->index[pos] = vlist->curr;
     vlist->inform[vlist->curr] = inf;
     vlist->names[vlist->curr++] = name;
    }
  else  vlist->inform[ret] = inf;
  return ret;
}

void add_to_node_list( /* adds node to alphabetic node list */
        struct node* node, int inf, struct node_list* nll)
{
  int pos, j;
  if ((pos = name_list_pos(node->name, nll->list)) < 0)
    {
     if (nll->curr == nll->max) grow_node_list(nll);
     j = add_to_name_list(node->name, inf, nll->list);
     nll->nodes[nll->curr++] = node;
    }
}

void add_to_sequ_list(struct sequence* sequ, struct sequence_list* sql)
     /* adds sequence sequ to sequence list sql */
{
  int i;
  for (i = 0; i < sql->curr; i++) if (sql->sequs[i] == sequ)  return;
  for (i = 0; i < sql->curr; i++)
    {
     if (strcmp(sql->sequs[i]->name, sequ->name) == 0)
       {
        sql->sequs[i] = sequ;
        sql->list->names[i] = sequ->name;
        return;
       }
    }
  if (sql->curr == sql->max) grow_sequence_list(sql);
  sql->sequs[sql->curr++] = sequ;
  add_to_name_list(sequ->name, 0, sql->list);
}

void add_to_table_list(struct table* t, struct table_list* tl)
     /* adds table t to table list tl */
{
  int pos, j;
  if ((pos = name_list_pos(t->name, tl->names)) < 0)
    {
     if (tl->curr == tl->max) grow_table_list(tl);
     j = add_to_name_list(tmpbuff(t->name), 0, tl->names);
     tl->tables[tl->curr++] = t;
    }
  else
    {
     tl->tables[pos] = delete_table(tl->tables[pos]);
     tl->tables[pos] = t;
    }
}

void add_to_var_list( /* adds variable to alphabetic variable list */
        struct variable* var, struct var_list* varl, int flag)
     /* flag = 0: undefined reference in expression, 1: definition
               2: separate list, do not drop variable */
{
  int pos, j;
  if ((pos = name_list_pos(var->name, varl->list)) > -1)
    {
     if (flag == 1)
       {
        if (varl->list->inform[pos] == 1)
           put_info(var->name, "redefined");
      else varl->list->inform[pos] = flag;
       }
     if (flag < 2) delete_variable(varl->vars[pos]);
     varl->vars[pos] = var;
    }
  else
    {
     if (varl->curr == varl->max) grow_var_list(varl);
     j = add_to_name_list(permbuff(var->name), flag, varl->list);
     varl->vars[varl->curr++] = var;
    }
}

void add_vars_to_table(struct table* t)
     /* fills user-defined variables into current table_row) */
{
  int i;
  char* p;

  for (i = t->org_cols; i < t->num_cols; i++)
    {
     if (t->columns->inform[i] < 3)
       {
      if (strstr(t->columns->names[i], "aper_"))
          t->d_cols[i][t->curr]
          = get_aperture(current_node, t->columns->names[i]);
        else t->d_cols[i][t->curr] = get_variable(t->columns->names[i]);
       }
     else if ((p = command_par_string(t->columns->names[i],
         current_node->p_elem->def)) == NULL)
          t->s_cols[i][t->curr] = tmpbuff("none");
     else t->s_cols[i][t->curr] = tmpbuff(p);
    }
}

int char_cnt(char c, char* string)
     /* returns number of occurrences of character c in string */
{
  int i, k = 0;
  for (i = 0; i < strlen(string); i++) if(string[i] == c) k++;
  return k;
}

int char_p_pos(char* name, struct char_p_array* p)
     /* returns the position of name in character pointer array p,
        or -1 if not found */
{
  int i;
  for (i = 0; i < p->curr; i++) if (strcmp(name, p->p[i]) == 0) return i;
  return -1;
}

struct char_p_array* clone_char_p_array(struct char_p_array* p)
{
  int i;
  struct char_p_array* clone = new_char_p_array(p->max);
  for (i = 0; i < p->curr; i++) clone->p[i] = permbuff(p->p[i]);
  clone->curr = p->curr;
  return clone;
}

struct command* clone_command(struct command* p)
{
  int i;
  struct command* clone = new_command(p->name, 0, p->par->curr,
               p->module, p->group, p->link_type,
                                      p->mad8_type);
  copy_name_list(clone->par_names, p->par_names);
  clone->par->curr = p->par->curr;
  for (i = 0; i < p->par->curr; i++)
      clone->par->parameters[i] =
        clone_command_parameter(p->par->parameters[i]);
  return clone;
}

struct command_parameter* clone_command_parameter(struct command_parameter* p)
{
  struct command_parameter* clone = new_command_parameter(p->name, p->type);
  clone->call_def = p->call_def;
  switch (p->type)
    {
    case 4:
      clone->c_min = p->c_min;
      clone->c_max = p->c_max;
      clone->min_expr = clone_expression(p->min_expr);
      clone->max_expr = clone_expression(p->max_expr);
    case 0:
    case 1:
    case 2:
      clone->double_value = p->double_value;
      clone->expr = clone_expression(p->expr);
      break;
    case 3:
      clone->string = p->string;
      clone->expr = NULL;
      break;
    case 11:
    case 12:
      clone->double_array = clone_double_array(p->double_array);
      clone->expr_list = clone_expr_list(p->expr_list);
      break;
    case 13:
      clone->m_string = clone_char_p_array(p->m_string);
    }
  return clone;
}

struct double_array* clone_double_array(struct double_array* p)
{
  int i;
  struct double_array* clone = new_double_array(p->curr);
  clone->curr = p->curr;
  for (i = 0; i < p->curr; i++) clone->a[i] = p->a[i];
  return clone;
}

struct element* clone_element(struct element* el)
{
  struct element* clone = new_element(el->name);
  clone->length = el->length;
  clone->bv = el->bv;
  clone->def = el->def;
  clone->parent = el;
  clone->base_type = el->base_type;
  return clone;
}

struct expression* clone_expression(struct expression* p)
{
  struct expression* clone;
  if (p == NULL) return NULL;
  clone = new_expression(p->string, p->polish);
  clone->status = p->status;
  clone->value = p->value;
  return clone;
}

struct expr_list* clone_expr_list(struct expr_list* p)
{
  int i;
  struct expr_list* clone;
  if (p == NULL)  return NULL;
  clone = new_expr_list(p->curr);
  for (i = 0; i < p->curr; i++) clone->list[i] = clone_expression(p->list[i]);
  clone->curr = p->curr;
  return clone;
}

struct int_array* clone_int_array(struct int_array* p)
{
  int i;
  struct int_array* clone = new_int_array(p->curr);
  clone->curr = p->curr;
  for (i = 0; i < p->curr; i++) clone->i[i] = p->i[i];
  return clone;
}

struct macro* clone_macro(struct macro* org)
{
  int i;
  struct macro* clone
    = new_macro(org->n_formal, org->body->curr, org->tokens->curr);
  if (org->body->curr > 0) strcpy(clone->body->c, org->body->c);
  clone->body->curr = org->body->curr;
  for (i = 0; i < org->tokens->curr; i++)
    clone->tokens->p[i] = org->tokens->p[i];
  clone->tokens->curr = org->tokens->curr;
  for (i = 0; i < org->n_formal; i++)
    clone->formal->p[i] = org->formal->p[i];
  clone->n_formal = org->n_formal;
  return clone;
}

struct name_list* clone_name_list(struct name_list* p)
{
  int i, l = p->curr > 0 ? p->curr : 1;
  struct name_list* clone = new_name_list(l);
  for (i = 0; i < p->curr; i++) clone->index[i] = p->index[i];
  for (i = 0; i < p->curr; i++) clone->inform[i] = p->inform[i];
  for (i = 0; i < p->curr; i++) clone->names[i] = p->names[i];
  clone->curr = p->curr;
  return clone;
}

struct node* clone_node(struct node* p, int flag)
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
  if (flag)
    {
     clone->p_al_err = p->p_al_err;
     clone->p_fd_err = p->p_fd_err;
    }
  return clone;
}

void conv_char(char* string, struct int_array* tint)
     /*converts character string to integer array, using ascii code */
{
  int i, l = strlen(string),
  n = (l < tint->max-1) ? l : tint->max-1;
  tint->i[0] = n;
  for (i = 0; i < n; i++)  tint->i[i+1] = (int) string[i];
}

void copy_double(double* source, double* target, int n)
     /* copies n double precision values from source to target */
{
  int j;
  for (j = 0; j < n; j++)  target[j] = source[j];
}

void copy_name_list(struct name_list* out, struct name_list* in)
     /* copies namelist in to namelist out */
{
  int i, l = in->curr > 0 ? in->curr : 1;
  while (out->max < l) grow_name_list(out);
  for (i = 0; i < in->curr; i++) out->index[i]  = in->index[i];
  for (i = 0; i < in->curr; i++) out->inform[i] = in->inform[i];
  for (i = 0; i < in->curr; i++) out->names[i]  = in->names[i];
  out->curr = in->curr;
}

struct char_array* delete_char_array(struct char_array* pa)
{
  if (pa == NULL)  return NULL;
  if (stamp_flag && pa->stamp != 123456)
     fprintf(stamp_file, "d_c_a double delete --> %s\n", pa->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", pa->name);
  if (pa->c != NULL)  free(pa->c);
  free(pa);
  return NULL;
}

struct char_p_array* delete_char_p_array(struct char_p_array* pa, int flag)
     /* flag = 0: delete only pointer array, = 1: delete char. buffers, too */
{
  int i;
  if (pa == NULL)  return NULL;
  if (stamp_flag && pa->stamp != 123456)
     fprintf(stamp_file, "d_c_p_a double delete --> %s\n", pa->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", pa->name);
  if (flag)
    {
     for (i = 0; i < pa->curr; i++)  free(pa->p[i]);
    }
  if (pa->p != NULL)  free(pa->p);
  free(pa);
  return NULL;
}

struct command* delete_command(struct command* cmd)
{
  if (cmd == NULL) return NULL;
  if (stamp_flag && cmd->stamp != 123456)
     fprintf(stamp_file, "d_c double delete --> %s\n", cmd->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", cmd->name);
  if (cmd->par != NULL)  delete_command_parameter_list(cmd->par);
  if (cmd->par_names != NULL) delete_name_list(cmd->par_names);
  free(cmd);
  return NULL;
}

struct command_list* delete_command_list(struct command_list* cl)
{
  int i;
  if (cl == NULL) return NULL;
  if (stamp_flag && cl->stamp != 123456)
     fprintf(stamp_file, "d_c_l double delete --> %s\n", cl->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", cl->name);
  if (cl->list != NULL) delete_name_list(cl->list);
  for (i = 0; i < cl->curr; i++) delete_command(cl->commands[i]);
  if (cl->commands) free(cl->commands);
  free(cl);
  return NULL;
}

struct command_parameter*
       delete_command_parameter(struct command_parameter* par)
{
  if (par == NULL) return NULL;
  if (stamp_flag && par->stamp != 123456)
     fprintf(stamp_file, "d_c_p double delete --> %s\n", par->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", par->name);
  if (par->expr != NULL)         delete_expression(par->expr);
  if (par->min_expr != NULL)     delete_expression(par->min_expr);
  if (par->max_expr != NULL)     delete_expression(par->max_expr);
  if (par->double_array != NULL) delete_double_array(par->double_array);
  if (par->expr_list != NULL)    delete_expr_list(par->expr_list);
  if (par->m_string != NULL)     delete_char_p_array(par->m_string, 0);
  free(par);
  return NULL;
}

struct command_parameter_list*
       delete_command_parameter_list(struct command_parameter_list* parl)
{
  int i;
  if (parl == NULL) return NULL;
  if (stamp_flag && parl->stamp != 123456)
     fprintf(stamp_file, "d_c_p_l double delete --> %s\n", parl->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", parl->name);
  if (parl->parameters != NULL)
    {
     for (i = 0; i < parl->curr; i++)
      if (parl->parameters[i] != NULL)
        parl->parameters[i] = delete_command_parameter(parl->parameters[i]);
     if (parl->parameters)  free(parl->parameters);
    }
  free(parl);
  return NULL;
}

struct constraint* delete_constraint(struct constraint* cst)
{
  if (cst == NULL)  return NULL;
  if (stamp_flag && cst->stamp != 123456)
     fprintf(stamp_file, "d_c double delete --> %s\n", cst->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", "constraint");
  free(cst);
  return NULL;
}

struct constraint_list* delete_constraint_list(struct constraint_list* cl)
{
  if (cl == NULL)  return NULL;
  if (stamp_flag && cl->stamp != 123456)
     fprintf(stamp_file, "d_c_l double delete --> %s\n", cl->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", "constraint_list");
  free(cl);
  return NULL;
}

struct double_array* delete_double_array(struct double_array* a)
{
  if (a != NULL)
    {
     if (a->a != NULL) free(a->a);
     free(a);
    }
  return NULL;
}

struct element* delete_element(struct element* el)
{
  if (el == NULL)  return NULL;
  if (stamp_flag && el->stamp != 123456)
     fprintf(stamp_file, "d_e double delete --> %s\n", el->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", el->name);
  free(el);
  return NULL;
}

struct el_list* delete_el_list(struct el_list* ell)
{
  if (ell->list == NULL) return NULL;
  if (stamp_flag && ell->stamp != 123456)
     fprintf(stamp_file, "d_e_l double delete --> %s\n", ell->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", ell->name);
  delete_name_list(ell->list);
  if (ell->elem != NULL) free(ell->elem);
  free(ell);
  return NULL;
}

struct expression* delete_expression(struct expression* expr)
{
  if (expr == NULL) return NULL;
  if (stamp_flag && expr->stamp != 123456)
     fprintf(stamp_file, "d_ex double delete --> %s\n", expr->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", expr->name);
  if (expr->polish != NULL) expr->polish = delete_int_array(expr->polish);
  if (expr->string != NULL) free(expr->string);
  free(expr);
  return NULL;
}

struct expr_list* delete_expr_list(struct expr_list* exprl)
{
  int i;
  if (exprl == NULL) return NULL;
  if (stamp_flag && exprl->stamp != 123456)
     fprintf(stamp_file, "d_ex_l double delete --> %s\n", exprl->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", exprl->name);
  if (exprl->list != NULL)
    {
     for (i = 0; i < exprl->curr; i++)
      if (exprl->list[i] != NULL)  delete_expression(exprl->list[i]);
     free(exprl->list);
    }
  free(exprl);
  return NULL;
}

struct in_cmd* delete_in_cmd(struct in_cmd* cmd)
{
  if (cmd == NULL) return NULL;
  if (stamp_flag && cmd->stamp != 123456)
     fprintf(stamp_file, "d_i_c double delete --> %s\n", cmd->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", cmd->name);
  if (cmd->tok_list != NULL)
        cmd->tok_list = delete_char_p_array(cmd->tok_list, 0);
  free(cmd);
  return NULL;
}

struct int_array* delete_int_array(struct int_array* i)
{
  if (i == NULL)  return NULL;
  if (stamp_flag && i->stamp != 123456)
     fprintf(stamp_file, "d_i_a double delete --> %s\n", i->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", i->name);
  if (i->i != NULL) free(i->i);
  free(i);
  return NULL;
}

struct macro* delete_macro(struct macro* macro)
{
  if (macro == NULL)  return NULL;
  if (stamp_flag && macro->stamp != 123456)
     fprintf(stamp_file, "d_m double delete --> %s\n", macro->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", macro->name);
  if (macro->formal != NULL) delete_char_p_array(macro->formal, 0);
  if (macro->tokens != NULL) delete_char_p_array(macro->tokens, 0);
  if (macro->body != NULL) delete_char_array(macro->body);
  free(macro);
  return NULL;
}

struct name_list* delete_name_list(struct name_list* l)
{
  if (l == NULL) return NULL;
  if (stamp_flag && l->stamp != 123456)
     fprintf(stamp_file, "d_n_l double delete --> %s\n", l->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", l->name);
  if (l->index != NULL)  free(l->index);
  if (l->inform != NULL)  free(l->inform);
  if (l->names != NULL)   free(l->names);
  free(l);
  return NULL;
}

struct node* delete_node(struct node* p)
{
  if (p == NULL) return NULL;
  if (stamp_flag && p->stamp != 123456)
     fprintf(stamp_file, "d_n double delete --> %s\n", p->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", p->name);
  if (p->p_al_err) p->p_al_err = delete_double_array(p->p_al_err);
  if (p->p_fd_err) p->p_fd_err = delete_double_array(p->p_fd_err);
  free(p);
  return NULL;
}

struct node* delete_node_ring(struct node* start)
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

struct node_list* delete_node_list(struct node_list* l)
{
  if (l == NULL)  return NULL;
  if (stamp_flag && l->stamp != 123456)
     fprintf(stamp_file, "d_no_l double delete --> %s\n", l->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", l->name);
  if (l->nodes != NULL)  free(l->nodes);
  if (l->list != NULL)  delete_name_list(l->list);
  free(l);
  return NULL;
}

struct sequence* delete_sequence(struct sequence* sequ)
{
  if (sequ->ex_start != NULL)
    {
     sequ->ex_nodes = delete_node_list(sequ->ex_nodes);
     sequ->ex_start = delete_node_ring(sequ->ex_start);
     sequ->orbits = delete_vector_list(sequ->orbits);
     free(sequ->all_nodes);
    }
  if (sequ->l_expr) sequ->l_expr = delete_expression(sequ->l_expr);
  sequ->nodes = delete_node_list(sequ->nodes);
  sequ->start = delete_node_ring(sequ->start);
  if (sequ->cavities) sequ->cavities = delete_el_list(sequ->cavities);
  free(sequ);
  return NULL;
}

struct sequence_list* delete_sequence_list(struct sequence_list* sql)
{
  if (sql == NULL) return NULL;
  if (stamp_flag && sql->stamp != 123456)
     fprintf(stamp_file, "d_s_l double delete --> %s\n", sql->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", sql->name);
  if (sql->list != NULL) delete_name_list(sql->list);
  if (sql->sequs != NULL) free(sql->sequs);
  free(sql);
  return NULL;
}

struct table* delete_table(struct table* t)
{
  int i, j;
  if (t == NULL) return NULL;
  if (stamp_flag && t->stamp != 123456)
     fprintf(stamp_file, "d_t double delete --> %s\n", t->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", "table");
  if (t->header != NULL) t->header = delete_char_p_array(t->header, 1);
  if (t->col_out != NULL) t->col_out = delete_int_array(t->col_out);
  if (t->row_out != NULL) t->row_out = delete_int_array(t->row_out);
  if (t->node_nm != NULL) t->node_nm = delete_char_p_array(t->node_nm, 0);
  for (i = 0; i < t->curr; i++)
    {
     if (t->l_head[i] != NULL)
        t->l_head[i] = delete_char_p_array(t->l_head[i], 1);
    }
  if (t->l_head)  free(t->l_head);
  if (t->p_nodes) free(t->p_nodes);
  if (t->d_cols)
    {
     for (i = 0; i < t->num_cols; i++)
        if (t->columns->inform[i] < 3 && t->d_cols[i]) free(t->d_cols[i]);
     free(t->d_cols);
    }
  if (t->s_cols)
    {
     for (i = 0; i < t->num_cols; i++)
       {
        if (t->columns->inform[i] == 3 && t->s_cols[i])
        {
         for (j = 0; j < t->curr; j++)
              if (t->s_cols[i][j]) free(t->s_cols[i][j]);
           free(t->s_cols[i]);
        }
       }
     free (t->s_cols);
    }
  t->columns = delete_name_list(t->columns);
  free(t);
  return NULL;
}

struct variable* delete_variable(struct variable* var)
{
  if (var == NULL)  return NULL;
  if (stamp_flag && var->stamp != 123456)
     fprintf(stamp_file, "d_v double delete --> %s\n", var->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", var->name);
  if (var->expr != NULL) delete_expression(var->expr);
  if (var->string != NULL) free(var->string);
  free(var);
  return NULL;
}

struct var_list* delete_var_list(struct var_list* varl)
{
  if (varl == NULL) return NULL;
  if (stamp_flag && varl->stamp != 123456)
     fprintf(stamp_file, "d_v_l double delete --> %s\n", varl->name);
  if (watch_flag) fprintf(debug_file, "deleting --> %s\n", varl->name);
  if (varl->list != NULL) delete_name_list(varl->list);
  if (varl->vars != NULL) free(varl->vars);
  free(varl);
  return NULL;
}

struct vector_list* delete_vector_list(struct vector_list* vector)
{
  int j;
  if (vector == NULL) return NULL;
  if (vector->names != NULL)
    {
     for (j = 0; j < vector->names->curr; j++)
       if (vector->vectors[j]) delete_double_array(vector->vectors[j]);
     delete_name_list(vector->names);
    }
  if (vector->vectors != NULL) free(vector->vectors);
  free(vector);
  return NULL;
}

void dump_char_array(struct char_array* a)
{
  char* c = a->c;
  int n = 0, l_cnt = 60, k;
  while (n < a->curr)
    {
     k = a->curr - n; if (k > l_cnt) k = l_cnt;
     strncpy(c_dummy, c, k);
     c += k; n += k;
     c_dummy[k] = '\0';
     fprintf(prt_file, "%s\n", c_dummy);
    }
}

void dump_char_p_array(struct char_p_array* p)
{
  int i;
  for (i = 0; i < p->curr; i++) fprintf(prt_file, "%s\n", p->p[i]);
}

void dump_command(struct command* cmd)
{
  int i;
  fprintf(prt_file, "command: %s  module: %s\n",
          cmd->name, cmd->module);
  for (i = 0; i < cmd->par->curr; i++)
       dump_command_parameter(cmd->par->parameters[i]);
}

void dump_command_parameter(struct command_parameter* par)
{
  int i, k;
  char logic[2][8] = {"false", "true"};
  fprintf(prt_file, "parameter: %s   ", par->name);
  switch (par->type)
    {
    case 0:
     k = par->double_value;
     fprintf(prt_file, "logical: %s\n", logic[k]);
     break;
    case 1:
     if (par->expr != NULL)
       {
        dump_expression(par->expr);
        par->double_value = expression_value(par->expr, 2);
       }
     k = par->double_value;
     fprintf(prt_file, "integer: %d\n", k);
     break;
    case 2:
     if (par->expr != NULL)
       {
        dump_expression(par->expr);
        par->double_value = expression_value(par->expr, 2);
       }
     fprintf(prt_file, "double value: %e\n", par->double_value);
     break;
    case 11:
    case 12:
    if (par->double_array != NULL)
      {
       if (par->expr_list != NULL)
       {
          for (i = 0; i < par->double_array->curr; i++)
          {
           if (i < par->expr_list->curr && par->expr_list->list[i] != NULL)
               par->double_array->a[i]
                 = expression_value(par->expr_list->list[i], 2);
          }
       }
       fprintf(prt_file, "double array: ");
       for (i = 0; i < par->double_array->curr; i++)
            fprintf(prt_file, "%e ", par->double_array->a[i]);
       fprintf(prt_file, "\n");
      }
     break;
    case 3:
     fprintf(prt_file, "string: %s\n", par->string);
     break;
    case 13:
      dump_char_p_array(par->m_string);
    }
}

void dump_constraint(struct constraint* c)
{
  fprintf(prt_file, "name: %s type: %d value: %e min: %e max: %e weight: %e\n",
         c->name, c->type, c->value, c->c_min, c->c_max, c->weight);
}

void dump_constraint_list(struct constraint_list* cl)
{
  int i;
  for (i = 0; i < cl->curr; i++)
    {
     if (cl->constraints[i]) dump_constraint(cl->constraints[i]);
    }
}

void dump_element(struct element* el)
{
  fprintf(prt_file, "+++ dumping element %s  parent %s\n", el->name, el->parent->name);
  dump_command(el->def);
}

void dump_el_list(struct el_list* ell)
{
  int i;
  for (i = 0; i < ell->curr; i++) dump_element(ell->elem[i]);
}

void dump_expression(struct expression* ex)
{
 ex->value = expression_value(ex, 2);
 fprintf(prt_file, "expression: %s :: value: %e\n", ex->string, ex->value);
}

void dump_exp_sequ(struct sequence* sequ, int level)
     /* executes the command dumpsequ, ... */
{
  struct node* c_node;
  int j;
  double suml = zero;
  puts("+++++++++ dump expanded sequence +++++++++");
  c_node = sequ->ex_start;
  while(c_node != NULL)
    {
     suml += c_node->length;
     if (level > 2)
       {
        dump_node(c_node);
        if (c_node->p_al_err != NULL)
        {
         puts("alignment errors:");
           for (j = 0; j < c_node->p_al_err->curr; j++)
           printf("%e ", c_node->p_al_err->a[j]);
           printf("\n");
        }
        if (c_node->p_fd_err != NULL)
        {
         puts("field errors:");
           for (j = 0; j < c_node->p_fd_err->curr; j++)
           printf("%e ", c_node->p_fd_err->a[j]);
           printf("\n");
        }
        if (level > 3 && c_node->p_elem != NULL)  dump_element(c_node->p_elem);
       }
     else if (level > 0 && strcmp(c_node->base_name, "drift") != 0)
       fprintf(prt_file, "%s: at = %f  flag = %d\n", c_node->name,
              c_node->position, c_node->enable);
     if (c_node == sequ->ex_end)  break;
     c_node = c_node->next;
    }
  fprintf(prt_file, "=== sum of node length: %f\n", suml);
}

void dump_in_cmd(struct in_cmd* p_inp)
{
  fprintf(prt_file, "%s: type =%d, sub_type = %d, decl_start = %d\n",
          p_inp->label, p_inp->type, p_inp->sub_type, p_inp->decl_start);
  if (p_inp->cmd_def != NULL)
    {
     fprintf(prt_file, "defining command: %s\n", p_inp->cmd_def->name);
     /* dump_command(p_inp->cmd_def); */
    }
}

void dump_int_array(struct int_array* ia)
{
  int i;
  fprintf(prt_file, "dump integer array, length: %d\n", ia->curr);
  for (i = 0; i < ia->curr; i++)
    {
     fprintf(prt_file, "%d ", ia->i[i]);
     if ((i+1)%10 == 0) fprintf(prt_file, "\n");
    }
  if (ia->curr%10 != 0) fprintf(prt_file, "\n");
}

void dump_macro(struct macro* m)
{
  fprintf(prt_file, "name: %s\n", m->name);
  if (m->formal != NULL) dump_char_p_array(m->formal);
  dump_char_array(m->body);
  if (m->tokens != NULL) dump_char_p_array(m->tokens);
}

void dump_macro_list(struct macro_list* ml)
{
  int i;
  puts("++++++ dump of macro list");
  for (i = 0; i < ml->curr; i++) dump_macro(ml->macros[i]);
}

void dump_name_list(struct name_list* nl)
{
  int i;
  puts(" ");
  for (i = 0; i < nl->curr; i++)
    {
     fprintf(prt_file, "%-16s %d\n", nl->names[nl->index[i]], nl->inform[nl->index[i]]);
    }
}

void dump_node(struct node* node)
{
  int i;
  char pname[NAME_L] = "NULL", nname[NAME_L] = "NULL";
  if (node->previous != NULL) strcpy(pname, node->previous->name);
  if (node->next != NULL) strcpy(nname, node->next->name);
  fprintf(prt_file, "name: %s  occ: %d base: %s  position: %f\n", node->name,
          node->occ_cnt, node->base_name, node->position);
  fprintf(prt_file, "  names of - previous: %s  next: %s\n",
         pname, nname);
  if (node->cl != NULL)  for (i = 0; i < node->cl->curr; i++)
        dump_constraint(node->cl->constraints[i]);
}

void dump_sequ(struct sequence* c_sequ, int level)
{
  struct node* c_node;
  double suml = zero;
  fprintf(prt_file, "+++ dump sequence: %s\n", c_sequ->name);
  c_node = c_sequ->start;
  while(c_node != NULL)
    {
     suml += c_node->length;
     if (level > 2)
       {
        dump_node(c_node);
        if (level > 3 && c_node->p_elem != NULL)  dump_element(c_node->p_elem);
       }
     else if (level > 0 && strcmp(c_node->base_name, "drift") != 0)
       fprintf(prt_file, "%s: at = %f\n", c_node->name, c_node->position);
     if (c_node == c_sequ->end)  break;
     c_node = c_node->next;
    }
  fprintf(prt_file, "=== sum of node length: %f\n", suml);
}

void dump_variable(struct variable* v)
{
  fprintf(prt_file, "=== dumping variable %s\n", v->name);
}

void export_element(struct element* el, struct el_list* ell, FILE* file)
     /* recursive to have parents always in front for MAD-8 */
{
  int pos = name_list_pos(el->name, ell->list);
  char out[AUX_LG];
  if (pos >= 0)
    {
     if (ell->list->inform[pos] == 0)  /* not yet written */
       {
        export_element(el->parent, ell, file);
        strcpy(out, el->name);
        strcat(out, ": ");
        strcat(out, el->parent->name);
        export_el_def(el, out);
        write_nice(out, file);
        ell->list->inform[pos] = 1;
       }
    }
}

void export_comm_par(struct command_parameter* par, char* string)
     /* exports a command parameter */
{
  int i, k, last;
  char num[2*NAME_L];
  strcat(string, ",");
  strcat(string, par->name);
  switch(par->type)
    {
    case 0:
      strcat(string, "=");
      if (par->double_value == zero) strcat(string, "false");
      else                           strcat(string, "true");
      break;
    case 1:
    case 2:
      strcat(string, ":=");
      if (par->expr != NULL) strcat(string, par->expr->string);
      else
      {
       if (par->type == 1)
         {
          k = par->double_value; sprintf(num, "%d", k);
         }
         else sprintf(num, "%-23.15g", par->double_value);
         strcat(string, supp_tb(num));
      }
      break;
    case 3:
      if (par->string != NULL)
      {
         strcat(string, "=");
         strcat(string, par->string);
      }
      break;
    case 11:
    case 12:
      strcat(string, ":=");
      for (last = par->double_array->curr-1; last > 0; last--)
      {
       if (par->expr_list->list[last] != NULL)
         {
          if (zero_string(par->expr_list->list[last]->string) == 0) break;
         }
         else if (par->double_array->a[last] != zero) break;
      }
      strcat(string, "{");
      for (i = 0; i <= last; i++)
      {
       if (i > 0) strcat(string, ",");
         if (par->expr_list->list[i] != NULL)
            strcat(string, par->expr_list->list[i]->string);
         else
         {
          if (par->type == 11)
            {
             k = par->double_array->a[i]; sprintf(num, "%d", k);
            }
            else sprintf(num, "%-23.15g", par->double_array->a[i]);
            strcat(string, supp_tb(num));
         }
      }
      strcat(string, "}");
   }
}

void export_elem_8(struct element* el, struct el_list* ell, FILE* file)
     /* exports an element in mad-8 format */
     /* recursive to have parents always in front for MAD-8 */
{
  int pos = name_list_pos(el->name, ell->list);
  char out[AUX_LG];
  if (pos >= 0)
    {
     if (ell->list->inform[pos] == 0)  /* not yet written */
       {
        export_elem_8(el->parent, ell, file);
        strcpy(out, el->name);
        strcat(out, ": ");
        strcat(out, el->parent->name);
        export_el_def_8(el, out);
        write_nice_8(out, file);
        ell->list->inform[pos] = 1;
       }
    }
}

void export_el_def(struct element* el, char* string)
     /* exports an element definition in mad-X format */
{
  int i;
  struct command* def = el->def;
  struct command_parameter* par;
  for (i = 0; i < def->par->curr; i++)
    {
     par = def->par->parameters[i];
     if (def->par_names->inform[i]
         && par_out_flag(el->base_type->name, par->name))
       export_comm_par(par, string);
    }
}

void export_el_def_8(struct element* el, char* string)
     /* exports an element definition in mad-8 format */
{
  int i;
  struct command* def = el->def;
  struct command_parameter* par;
  for (i = 0; i < def->par->curr; i++)
    {
     par = def->par->parameters[i];
     if (def->par_names->inform[i]
         && par_out_flag(el->base_type->name, par->name))
       export_el_par_8(par, string);
    }
}

void export_el_par_8(struct command_parameter* par, char* string)
     /* exports an element parameter in mad-8 format */
{
  int i, k, lp, last, tilt = 0, vtilt = 0;
  char* const kskew[] = {"k1s", "k2s", "k3s", ""};
  char* const knorm[] = {"k1", "k2", "k3", ""};
  char num[2*NAME_L], tmp[8], tmpt[8];
  switch(par->type)
    {
    case 0:
      strcat(string, ",");
      strcat(string, par->name);
      strcat(string, "=");
      if (par->double_value == zero) strcat(string, "false");
      else                           strcat(string, "true");
      break;
    case 1:
    case 2:
      strcat(string, ",");
      lp = 0;
      while (strlen(kskew[lp]))
        {
         if (strcmp(kskew[lp], par->name) == 0)
           {
          strcat(string, knorm[lp]); tilt = 1; break;
           }
         lp++;
        }
      if (tilt == 0) strcat(string, par->name);
      strcat(string, "=");
      if (par->expr != NULL && strcmp(par->name, "harmon") != 0)
          strcat(string, par->expr->string);
      else
      {
       if (par->type == 1)
         {
          k = par->double_value; sprintf(num, "%d", k);
         }
         else sprintf(num, "%-23.15g", par->double_value);
         strcat(string, supp_tb(num));
      }
      break;
    case 3:
      strcat(string, ",");
      strcat(string, par->name);
      strcat(string, "=");
      strcat(string, par->string);
      break;
    case 11:
    case 12:
      vtilt = strcmp(par->name, "ks") == 0 ? 1 : 0;
      for (last = par->double_array->curr-1; last > 0; last--)
      {
       if (par->expr_list->list[last] != NULL)
         {
          if (zero_string(par->expr_list->list[last]->string) == 0) break;
         }
         else if (par->double_array->a[last] != zero) break;
      }
      for (i = 0; i <= last; i++)
      {
         if (par->expr_list->list[i] != NULL
             && !zero_string(par->expr_list->list[i]->string))
         {
            strcat(string, ",");
          sprintf(tmp, " k%dl =", i);
          sprintf(tmpt, ", t%d", i);
          strcat(string, tmp);
            strcat(string, par->expr_list->list[i]->string);
            if (vtilt) strcat(string, tmpt);
         }
         else if (par->double_array->a[i] != zero)
         {
            strcat(string, ",");
          sprintf(tmp, " k%dl =", i);
          sprintf(tmpt, ", t%d", i);
          if (par->type == 11)
            {
             k = par->double_array->a[i]; sprintf(num, "%d", k);
            }
            else sprintf(num, "%-23.15g", par->double_array->a[i]);
          strcat(string, tmp);
            strcat(string, supp_tb(num));
            if (vtilt) strcat(string, tmpt);
         }
      }
   }
  if (tilt) strcat(string, ", tilt");
}

void export_sequence(struct sequence* sequ, FILE* file)
     /* exports sequence in mad-X format */
{
  char num[2*NAME_L];
  struct element* el;
  struct sequence* sq;
  struct node* c_node = sequ->start;
  char rpos[3][6] = {"exit", "centre", "entry"};
  *c_dummy = '\0';
  if (sequ->share) strcat(c_dummy, "shared ");
  strcat(c_dummy, sequ->name);
  strcat(c_dummy, ": sequence");
  if (sequ->ref_flag)
    {
     strcat(c_dummy, ", refer = ");
     strcat(c_dummy, rpos[sequ->ref_flag+1]);
    }
  if (sequ->refpos != NULL)
    {
     strcat(c_dummy, ", refpos = ");
     strcat(c_dummy, sequ->refpos);
    }
  strcat(c_dummy, ", l = ");
  if (sequ->l_expr != NULL) strcat(c_dummy, sequ->l_expr->string);
  else
    {
     sprintf(num, "%-23.15g", sequ->length);
     strcat(c_dummy, supp_tb(num));
    }
  write_nice(c_dummy, file);
  while(c_node != NULL)
    {
     *c_dummy = '\0';
     if (strchr(c_node->name, '$') == NULL
         && strcmp(c_node->base_name, "drift") != 0)
       {
        if ((el = c_node->p_elem) != NULL)
          {
         if (c_node->p_elem->def_type)
           {
            strcat(c_dummy, el->name);
              strcat(c_dummy, ": ");
              strcat(c_dummy, el->parent->name);
           }
           else strcat(c_dummy, el->name);
          }
        else if ((sq = c_node->p_sequ) != NULL) strcat(c_dummy, sq->name);
        else fatal_error("save error: node without link:", c_node->name);
        strcat(c_dummy, ", at = ");
        if (c_node->at_expr != NULL) strcat(c_dummy, c_node->at_expr->string);
        else
          {
           sprintf(num, "%-23.15g", c_node->at_value);
           strcat(c_dummy, supp_tb(num));
          }
        if (c_node->from_name != NULL)
          {
         strcat(c_dummy, ", from = ");
           strcat(c_dummy, c_node->from_name);
          }
        write_nice(c_dummy, file);
       }
     if (c_node == sequ->end)  break;
     c_node = c_node->next;
    }
  strcpy(c_dummy, "endsequence");
  write_nice(c_dummy, file);
}

void export_sequ_8(struct sequence* sequ, struct command_list* cl, FILE* file)
     /* exports sequence in mad-8 format */
{
  char num[2*NAME_L];
  struct element* el;
  struct sequence* sq;
  struct node* c_node = sequ->start;
  if (pass_select_list(sequ->name, cl) == 0)  return;
  *c_dummy = '\0';
  strcat(c_dummy, sequ->name);
  strcat(c_dummy, ": sequence");
  write_nice_8(c_dummy, file);
  while(c_node != NULL)
    {
     *c_dummy = '\0';
     if (strchr(c_node->name, '$') == NULL
         && strcmp(c_node->base_name, "drift") != 0)
       {
        if ((el = c_node->p_elem) != NULL)
          {
         if (c_node->p_elem->def_type)
           {
            strcat(c_dummy, el->name);
              strcat(c_dummy, ": ");
              strcat(c_dummy, el->parent->name);
           }
           else strcat(c_dummy, el->name);
          }
        else if ((sq = c_node->p_sequ) != NULL) strcat(c_dummy, sq->name);
        else fatal_error("save error: node without link:", c_node->name);
        strcat(c_dummy, ", at = ");
        if (c_node->at_expr != NULL) strcat(c_dummy, c_node->at_expr->string);
        else
          {
           sprintf(num, "%-23.15g", c_node->at_value);
           strcat(c_dummy, supp_tb(num));
          }
        if (c_node->from_name != NULL)
          {
         strcat(c_dummy, ", from = ");
           strcat(c_dummy, c_node->from_name);
          }
        write_nice_8(c_dummy, file);
       }
     if (c_node == sequ->end)  break;
     c_node = c_node->next;
    }
  strcpy(c_dummy, sequ->name);
  strcat(c_dummy, "_end: marker, at = ");
  sprintf(num, "%-23.15g", sequ->length);
  strcat(c_dummy,num);
  write_nice_8(c_dummy, file);
  strcpy(c_dummy, "endsequence");
  write_nice_8(c_dummy, file);
}

void export_variable(struct variable* var, FILE* file)
     /* exports variable in mad-X format */
{
  int k;
  *c_dummy = '\0';
  if (var->status == 0) var->value = expression_value(var->expr, var->type);
  if (var->val_type == 0) strcat(c_dummy, "int ");
  if (var->type == 0) strcat(c_dummy, "const ");
  strcat(c_dummy, var->name);
  if (var->type < 2) strcat(c_dummy, " = ");
  else               strcat(c_dummy, " := ");
  if (var->expr != NULL) strcat(c_dummy, var->expr->string);
  else if (var->val_type == 0)
    {
     k = var->value; sprintf(c_join, "%d", k); strcat(c_dummy, c_join);
    }
  else
    {
     sprintf(c_join, "%-23.15g", var->value); strcat(c_dummy, supp_tb(c_join));
    }
  write_nice(c_dummy, file);
}

void export_var_8(struct variable* var, FILE* file)
     /* exports variable in mad-8 format */
{
  int k;
  *c_dummy = '\0';
  if (var->status == 0) var->value = expression_value(var->expr, var->type);
  if (var->type == 0)
    {
     strcat(c_dummy, var->name);
     strcat(c_dummy, ": constant = ");
    }
  else
    {
     strcat(c_dummy, var->name);
     if (var->type < 2) strcat(c_dummy, " = ");
     else               strcat(c_dummy, " := ");
    }
  if (var->expr != NULL) strcat(c_dummy, var->expr->string);
  else if (var->val_type == 0)
    {
     k = var->value; sprintf(c_join, "%d", k); strcat(c_dummy, c_join);
    }
  else
    {
     sprintf(c_join, "%-23.15g", var->value); strcat(c_dummy, supp_tb(c_join));
    }
  write_nice_8(c_dummy, file);
}

double find_value(char* name, int ntok, char** toks)
     /* returns value found in construct "name = value", or INVALID */
{
  double val = INVALID;
  int j;
  for (j = 0; j < ntok; j++)
    {
     if (strcmp(toks[j], name) == 0)
       {
      if (j+2 < ntok && *toks[j+1] == '=')
        {
         sscanf(toks[j+2], "%lf", &val);
           break;
        }
       }
    }
  return val;
}

double frndm()
     /* returns random number r with 0 <= r < 1 from flat distribution */
{
  const double one = 1;
  double scale = one / MAX_RAND;
  if (next_rand == NR_RAND)  irngen();
  return scale*irn_rand[next_rand++];
}

void ftoi_array(struct double_array* da, struct int_array* ia)
     /* converts and copies double array into integer array */
{
  int i, l = da->curr;
  while (l >= ia->max)  grow_int_array(ia);
  for (i = 0; i < l; i++) ia->i[i] = da->a[i];
  ia->curr = l;
}

void grow_char_array( /* doubles array size */
        struct char_array* p)
{
  char rout_name[] = "grow_char_array";
  char* p_loc = p->c;
  int j, new = 2*p->max;

  p->max = new;
  p->c = (char*) mymalloc(rout_name, new);
  for (j = 0; j < p->curr; j++) p->c[j] = p_loc[j];
  free(p_loc);
}

void grow_char_p_array( /* doubles array size */
        struct char_p_array* p)
{
  char rout_name[] = "grow_char_p_array";
  char** p_loc = p->p;
  int j, new = 2*p->max;

  p->max = new;
  p->p = (char**) mycalloc(rout_name,new, sizeof(char*));
  for (j = 0; j < p->curr; j++) p->p[j] = p_loc[j];
  free(p_loc);
}

void grow_command_list( /* doubles list size */
        struct command_list* p)
{
  char rout_name[] = "grow_command_list";
  struct command** c_loc = p->commands;
  int j, new = 2*p->max;

  p->max = new;
  p->commands
     = (struct command**) mycalloc(rout_name,new, sizeof(struct command*));
  for (j = 0; j < p->curr; j++) p->commands[j] = c_loc[j];
  free(c_loc);
}

void grow_command_list_list( /* doubles list size */
        struct command_list_list* p)
{
  char rout_name[] = "grow_command_list_list";
  struct command_list** c_loc = p->command_lists;
  int j, new = 2*p->max;

  p->max = new;
  p->command_lists = (struct command_list**)
                     mycalloc(rout_name,new, sizeof(struct command_list*));
  for (j = 0; j < p->curr; j++) p->command_lists[j] = c_loc[j];
  free(c_loc);
}

void grow_command_parameter_list( /* doubles list size */
        struct command_parameter_list* p)
{
  char rout_name[] = "grow_command_parameter_list";
  struct command_parameter** c_loc = p->parameters;
  int j, new = 2*p->max;

  p->max = new;
  p->parameters = (struct command_parameter**)
                   mycalloc(rout_name,new, sizeof(struct command_parameter*));
  for (j = 0; j < p->curr; j++) p->parameters[j] = c_loc[j];
  free(c_loc);
}

void grow_constraint_list( /* doubles list size */
        struct constraint_list* p)
{
  char rout_name[] = "grow_constraint_list";
  struct constraint** c_loc = p->constraints;
  int j, new = 2*p->max;

  p->max = new;
  p->constraints = (struct constraint**)
                    mycalloc(rout_name,new, sizeof(struct constraint*));
  for (j = 0; j < p->curr; j++) p->constraints[j] = c_loc[j];
  free(c_loc);
}

void grow_double_array( /* doubles array size */
        struct double_array* p)
{
  char rout_name[] = "grow_double_array";
  double* a_loc = p->a;
  int j, new = 2*p->max;

  p->max = new;
  p->a = (double*) mymalloc(rout_name,new * sizeof(double));
  for (j = 0; j < p->curr; j++) p->a[j] = a_loc[j];
  free(a_loc);
}

void grow_el_list( /* doubles list size */
        struct el_list* p)
{
  char rout_name[] = "grow_el_list";
  struct element** e_loc = p->elem;
  int j, new = 2*p->max;
  p->max = new;
  p->elem
     = (struct element**) mycalloc(rout_name,new, sizeof(struct element*));
  for (j = 0; j < p->curr; j++) p->elem[j] = e_loc[j];
  free(e_loc);
}

void grow_expr_list( /* doubles list size */
        struct expr_list* p)
{
  char rout_name[] = "grow_expr_list";
  struct expression** e_loc = p->list;
  int j, new = 2*p->max;
  p->max = new;
  p->list
   = (struct expression**) mycalloc(rout_name,new, sizeof(struct expression*));
  for (j = 0; j < p->curr; j++) p->list[j] = e_loc[j];
  free(e_loc);
}

void grow_in_buff_list( /* doubles list size */
        struct in_buff_list* p)
{
  char rout_name[] = "grow_in_buff_list";
  struct in_buffer** e_loc = p->buffers;
  FILE** f_loc = p->input_files;
  int j, new = 2*p->max;
  p->max = new;
  p->buffers
    = (struct in_buffer**) mycalloc(rout_name,new, sizeof(struct in_buffer*));
  for (j = 0; j < p->curr; j++) p->buffers[j] = e_loc[j];
  free(e_loc);
  p->input_files = mycalloc(rout_name, new, sizeof(FILE*));
  for (j = 0; j < p->curr; j++) p->input_files[j] = f_loc[j];
  free(f_loc);
}

void grow_in_cmd_list( /* doubles list size */
        struct in_cmd_list* p)
{
  char rout_name[] = "grow_in_cmd_list";
  struct in_cmd** c_loc = p->in_cmds;
  int j, new = 2*p->max;

  p->max = new;
  p->in_cmds
    = (struct in_cmd**) mycalloc(rout_name,new, sizeof(struct in_cmd*));
  for (j = 0; j < p->curr; j++) p->in_cmds[j] = c_loc[j];
  free(c_loc);
}

void grow_int_array( /* doubles array size */
        struct int_array* p)
{
  char rout_name[] = "grow_int_array";
  int* i_loc = p->i;
  int j, new = 2*p->max;

  p->max = new;
  p->i = (int*) mymalloc(rout_name,new * sizeof(int));
  for (j = 0; j < p->curr; j++) p->i[j] = i_loc[j];
  free(i_loc);
}

void grow_macro_list( /* doubles list size */
        struct macro_list* p)
{
  char rout_name[] = "grow_macro_list";
  struct macro** n_loc = p->macros;
  int j, new = 2*p->max;
  p->max = new;
  p->macros = (struct macro**) mycalloc(rout_name,new, sizeof(struct macro*));
  for (j = 0; j < p->curr; j++) p->macros[j] = n_loc[j];
  free(n_loc);
}

void grow_name_list( /* doubles list size */
        struct name_list* p)
{
  char rout_name[] = "grow_name_list";
  char** n_loc = p->names;
  int* l_ind = p->index;
  int* l_inf = p->inform;
  int j, new = 2*p->max;

  p->max = new;
  p->names = (char**) mycalloc(rout_name,new, sizeof(char*));
  p->index = (int*) mycalloc(rout_name,new, sizeof(int));
  p->inform = (int*) mycalloc(rout_name,new, sizeof(int));
  for (j = 0; j < p->curr; j++)
    {
     p->names[j] = n_loc[j];
     p->index[j] = l_ind[j];
     p->inform[j] = l_inf[j];
    }
  free(n_loc);
  free(l_ind);
  free(l_inf);
}

void grow_node_list( /* doubles list size */
        struct node_list* p)
{
  char rout_name[] = "grow_node_list";
  struct node** n_loc = p->nodes;
  int j, new = 2*p->max;
  p->max = new;
  p->nodes = (struct node**) mycalloc(rout_name,new, sizeof(struct node*));
  for (j = 0; j < p->curr; j++) p->nodes[j] = n_loc[j];
  free(n_loc);
}

void grow_sequence_list(struct sequence_list* l)
{
  char rout_name[] = "grow_sequence_list";
  struct sequence** sloc = l->sequs;
  int j, new = 2*l->max;
  l->max = new;
  l->sequs
    = (struct sequence**) mycalloc(rout_name,new, sizeof(struct sequence*));
  for (j = 0; j < l->curr; j++) l->sequs[j] = sloc[j];
  free(sloc);
}

void grow_table(struct table* t) /* doubles number of rows */
{
  char rout_name[] = "grow_table";
  int i, j, new = 2*t->max;
  char** s_loc;
  struct char_p_array* t_loc = t->node_nm;
  double* d_loc;
  struct node** p_loc = t->p_nodes;
  struct char_p_array** pa_loc = t->l_head;
  t->max = new;
  t->p_nodes = (struct node**) mycalloc(rout_name,new, sizeof(struct node*));
  t->l_head
      = (struct char_p_array**)
        mycalloc(rout_name,new, sizeof(struct char_p_array*));
  t->node_nm = new_char_p_array(new);

  for (i = 0; i < t->curr; i++)
    {
     t->node_nm->p[i] = t_loc->p[i];
     t->p_nodes[i] = p_loc[i];
     t->l_head[i] = pa_loc[i];
    }
  delete_char_p_array(t_loc, 0);
  free(pa_loc);
  t->node_nm->curr = t->curr; free(t_loc);
  for (j = 0; j < t->num_cols; j++)
    {
     if ((s_loc = t->s_cols[j]) != NULL)
       {
        t->s_cols[j] = (char**) mycalloc(rout_name,new, sizeof(char*));
        for (i = 0; i < t->curr; i++) t->s_cols[j][i] = s_loc[i];
        free(s_loc);
       }
    }
  for (j = 0; j < t->num_cols; j++)
    {
     if ((d_loc = t->d_cols[j]) != NULL)
       {
        t->d_cols[j] = (double*) mycalloc(rout_name,new, sizeof(double));
        for (i = 0; i < t->curr; i++) t->d_cols[j][i] = d_loc[i];
        free(d_loc);
       }
    }
}

void grow_table_list(struct table_list* tl)
{
  char rout_name[] = "grow_table_list";
  struct table** t_loc = tl->tables;
  int j, new = 2*tl->max;

  grow_name_list(tl->names);
  tl->max = new;
  tl->tables = (struct table**) mycalloc(rout_name,new, sizeof(struct table*));
  for (j = 0; j < tl->curr; j++) tl->tables[j] = t_loc[j];
  free(t_loc);
}

void grow_var_list( /* doubles list size */
        struct var_list* p)
{
  char rout_name[] = "grow_var_list";
  struct variable** v_loc = p->vars;
  int j, new = 2*p->max;

  p->max = new;
  p->vars
    = (struct variable**) mycalloc(rout_name,new, sizeof(struct variable*));
  for (j = 0; j < p->curr; j++) p->vars[j] = v_loc[j];
  free(v_loc);
}

void grow_vector_list( /* doubles list size */
        struct vector_list* p)
{
  char rout_name[] = "grow_vector_list";
  struct double_array** v_loc = p->vectors;
  int j, new = 2*p->max;

  p->max = new;
  p->vectors
    = (struct double_array**) mycalloc(rout_name,new,
                                       sizeof(struct double_array*));
  for (j = 0; j < p->curr; j++) p->vectors[j] = v_loc[j];
  free(v_loc);
}

double grndm()
     /* returns random number x from normal distribution */
{
  double xi1 = 2*frndm()-one, xi2=2*frndm()-one, zzr;
  while ((zzr = xi1*xi1+xi2*xi2) > one)
    {
     xi1 = 2*frndm()-one; xi2=2*frndm()-one;
    }
  zzr = sqrt(-2*log(zzr)/zzr);
  return xi1*zzr;
}

void init55(int seed)
     /* initializes random number algorithm */
{
  int i, ii, k = 1, j = abs(seed)%MAX_RAND;
  irn_rand[NR_RAND-1] = j;
  for (i = 0; i < NR_RAND-1; i++)
    {
     ii = (ND_RAND*(i+1))%NR_RAND;
     irn_rand[ii-1] = k;
     if ((k = j - k) < 0) k += MAX_RAND;
     j = irn_rand[ii-1];
    }
  /* warm up */
  for (i = 0; i < 3; i++) irngen();
}

int intrac()
     /* returns non-zero inf program is used interactively, else 0 */
{
    return ((int) isatty(0));
}

void irngen()
     /* creates random number for frndm() */
{
  int i, j;
  for (i = 0; i < NJ_RAND; i++)
    {
     if ((j = irn_rand[i] - irn_rand[i+NR_RAND-NJ_RAND]) < 0) j += MAX_RAND;
     irn_rand[i] = j;
    }
  for (i = NJ_RAND; i < NR_RAND; i++)
    {
     if ((j = irn_rand[i] - irn_rand[i-NJ_RAND]) < 0) j += MAX_RAND;
     irn_rand[i] = j;
    }
  next_rand = 0;
}

char* join(char** it_list, int n)
     /* joins n character strings into one */
{
  int j;
  *c_join = '\0';
  for (j = 0; j < n; j++) strcat(c_join, it_list[j]);
  return c_join;
}

char* join_b(char** it_list, int n)
     /* joins n character strings into one, blank separated */
{
  char* target;
  int j, k = 0;
  target = c_join;
  for (j = 0; j < n; j++)
    {
     strcpy(&target[k], it_list[j]);
     k += strlen(it_list[j]);
     target[k++] = ' ';
    }
  target[k] = '\0';
  return target;
}

void* mycalloc(char* caller, size_t nelem, size_t size)
{
  /* calls calloc, checks for memory granted */
  void* p;
  if ((p = calloc(nelem, size)) == NULL)
     fatal_error("memory overflow, called from routine:", caller);
  return p;
}

void mycpy(char* sout, char* sin)
     /* copies string, ends at any non-ascii character including 0 */
{
  char *p, *q;
  int l = 1;

  p = sin;  q = sout;
  while (*p > ' ' && *p <= '~' && l < 2*NAME_L)
      {
       *q++ = *p++;  l++;
      }
  *q = '\0';
}

void* mymalloc(char* caller, size_t size)
{
  /* calls malloc, checks for memory granted */
  void* p;
  if ((p = malloc(size)) == NULL)
    fatal_error("memory overflow, called from routine:", caller);
  return p;
}

char* mystrchr(char* string, char c)
     /* returns strchr for character c, but only outside strings included
        in single or double quotes */
{
  char quote = ' '; /* only for the compiler */
  int toggle = 0;
  while (*string != '\0')
    {
     if (toggle)
       {
      if (*string == quote) toggle = 0;
       }
     else if(*string == '\'' || *string == '\"')
       {
      quote = *string; toggle = 1;
       }
     else if (*string == c) return string;
     string++;
    }
  return NULL;
}

char* mystrstr(char* string, char* s)
     /* returns strstr for s, but only outside strings included
        in single or double quotes */
{
  char quote = ' '; /* only for the compiler */
  int toggle = 0, n = strlen(s);
  if (n == 0)  return NULL;
  while (*string != '\0')
    {
     if (toggle)
       {
      if (*string == quote) toggle = 0;
       }
     else if(*string == '\'' || *string == '\"')
       {
      quote = *string; toggle = 1;
       }
     else if (strncmp(string, s, n) == 0) return string;
     string++;
    }
  return NULL;
}

void my_repl(char* in, char* out, char* string_in, char* string_out)
     /* replaces all occurrences of "in" in string_in by "out"
        in output string string_out */
{
  int n, add, l_in = strlen(in), l_out = strlen(out);
  char* cp;
  char tmp[8];
  while ((cp = strstr(string_in, in)) != NULL)
    {
     while (string_in != cp) *string_out++ = *string_in++;
     string_in += l_in;
     if (*out == '$')
       {
      n = get_variable(&out[1]);
      sprintf(tmp,"%d", n); add = strlen(tmp);
        strncpy(string_out, tmp, add);
        string_out += add;
       }
     else
       {
        strncpy(string_out, out, l_out);
        string_out += l_out;
       }
    }
  strcpy(string_out, string_in);
}

int name_list_pos(char* p, struct name_list* vlist)
{
  int num, mid, low = 0, high = vlist->curr - 1;
  while (low <= high)
    {
     mid = (low + high) / 2;
     if ((num=strcmp(p, vlist->names[vlist->index[mid]])) < 0)  high = mid - 1;
     else if ( num > 0) low  = mid + 1;
     else               return vlist->index[mid];
    }
    return -1;
}

struct char_array* new_char_array(int length)
{
  char rout_name[] = "new_char_array";
  struct char_array* il =
       (struct char_array*) mycalloc(rout_name,1, sizeof(struct char_array));
  strcpy(il->name, "char_array");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->c = (char*) mymalloc(rout_name,length);
  return il;
}

struct char_array_list* new_char_array_list(int size)
{
  char rout_name[] = "new_char_array_list";
  struct char_array_list* tl = (struct char_array_list*)
      mycalloc(rout_name,1, sizeof(struct char_array_list));
  strcpy(tl->name, "char_array_list");
  tl->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", tl->name);
  tl->ca
  = (struct char_array**) mycalloc(rout_name,size, sizeof(struct char_array*));
  tl->max = size;
  return tl;
}

struct char_p_array* new_char_p_array(int length)
{
  char rout_name[] = "new_char_p_array";
  struct char_p_array* il
  = (struct char_p_array*) mycalloc(rout_name,1, sizeof(struct char_p_array));
  strcpy(il->name, "char_p_array");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->p = (char**) mycalloc(rout_name,length, sizeof(char*));
  return il;
}

struct command* new_command(char* name, int nl_length, int pl_length,
                            char* module, char* group, int link, int mad_8)
{
  char rout_name[] = "new_command";
  struct command* new
   = (struct command*) mycalloc(rout_name,1, sizeof(struct command));
  new->stamp = 123456;
  strcpy(new->name, name);
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", new->name);
  strcpy(new->module, module);
  strcpy(new->group, group);
  new->link_type = link;
  new->mad8_type = mad_8;
  if (nl_length == 0) nl_length = 1;
  new->par_names = new_name_list(nl_length);
  new->par = new_command_parameter_list(pl_length);
  return new;
}

struct command_list* new_command_list(int length)
{
  char rout_name[] = "new_command_list";
  struct command_list* il =
    (struct command_list*) mycalloc(rout_name,1, sizeof(struct command_list));
  strcpy(il->name, "command_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->list = new_name_list(length);
  il->commands
    = (struct command**) mycalloc(rout_name,length, sizeof(struct command*));
  return il;
}

struct command_list_list* new_command_list_list(int length)
{
  char rout_name[] = "new_command_list_list";
  struct command_list_list* il =
    (struct command_list_list*)
    mycalloc(rout_name,1, sizeof(struct command_list_list));
  strcpy(il->name, "command_list_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->list = new_name_list(length);
  il->command_lists
   = (struct command_list**)
     mycalloc(rout_name,length, sizeof(struct command_list*));
  return il;
}

struct command_parameter* new_command_parameter(char* name, int type)
{
  char rout_name[] = "new_command_parameter";
  struct command_parameter* new
  = (struct command_parameter*)
    mycalloc(rout_name,1, sizeof(struct command_parameter));
  strcpy(new->name, name); new->type = type;
  new->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", new->name);
  return new;
}

struct command_parameter_list* new_command_parameter_list(int length)
{
  char rout_name[] = "new_command_parameter_list";
  struct command_parameter_list* il =
       (struct command_parameter_list*)
        mycalloc(rout_name,1, sizeof(struct command_parameter_list));
  strcpy(il->name, "command_parameter_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  if (length > 0)
    {
     il->parameters = (struct command_parameter**)
       mycalloc(rout_name,length, sizeof(struct command_parameter*));
    }
  return il;
}

struct constraint* new_constraint(int type)
{
  char rout_name[] = "new_constraint";
  struct constraint* new = (struct constraint*)
                             mycalloc(rout_name,1, sizeof(struct constraint));
  strcpy(new->name, "constraint");
  new->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", new->name);
  new->type = type;
  return new;
}

struct constraint_list* new_constraint_list(int length)
{
  char rout_name[] = "new_constraint_list";
  struct constraint_list* il
    = (struct constraint_list*)
      mycalloc(rout_name,1, sizeof(struct constraint_list));
  strcpy(il->name, "constraint_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->constraints = (struct constraint**)
                     mycalloc(rout_name,length, sizeof(struct constraint*));
  return il;
}

struct double_array* new_double_array(int length)
{
  char rout_name[] = "new_double_array";
  struct double_array* il
   = (struct double_array*)
     mycalloc(rout_name,1, sizeof(struct double_array));
  il->stamp = 123456;
  il->curr = 0;
  il->max = length;
  il->a = (double*) mycalloc(rout_name,length, sizeof(double));
  return il;
}

struct element* new_element(char* name)
{
  char rout_name[] = "new_element";
  struct element* el
     = (struct element*) mycalloc(rout_name,1, sizeof(struct element));
  strcpy(el->name, name);
  el->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", el->name);
  return el;
}

struct el_list* new_el_list(int length)
{
  char rout_name[] = "new_el_list";
  struct el_list* ell
     = (struct el_list*) mycalloc(rout_name,1, sizeof(struct el_list));
  strcpy(ell->name, "el_list");
  ell->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", ell->name);
  ell->list = new_name_list(length);
  ell->elem
     = (struct element**) mycalloc(rout_name,length, sizeof(struct element*));
  ell->max = length;
  return ell;
}

struct node* new_elem_node(struct element* el, int occ_cnt)
{
  struct node* p;
  p = new_node(compound(el->name, occ_cnt));
  p->p_elem = el;
  p->length = el->length;
  p->base_name = el->base_type->name;
  return p;
}

struct expression* new_expression(char* in_string, struct int_array* polish)
{
  char rout_name[] = "new_expression";
  int j;
  struct expression* ex =
         (struct expression*) mycalloc(rout_name,1,sizeof(struct expression));
  strcpy(ex->name, "expression");
  ex->stamp = 123456;
  ex->string = (char*) mymalloc(rout_name,strlen(in_string)+1);
  strcpy(ex->string, in_string);
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", ex->name);
  if (polish != NULL)
    {
     ex->polish = new_int_array(polish->curr);
     ex->polish->curr = polish->curr;
     for (j = 0; j < polish->curr; j++) ex->polish->i[j] = polish->i[j];
    }
  return ex;
}

struct expr_list* new_expr_list(int length)
{
  char rout_name[] = "new_expr_list";
  struct expr_list* ell =
        (struct expr_list*) mycalloc(rout_name,1, sizeof(struct expr_list));
  strcpy(ell->name, "expr_list");
  ell->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", ell->name);
  ell->list  =
  (struct expression**) mycalloc(rout_name,length, sizeof(struct expression*));
  ell->max = length;
  return ell;
}

struct in_buffer* new_in_buffer(int length)
{
  char rout_name[] = "new_in_buffer";
  struct in_buffer* new =
         (struct in_buffer*) mycalloc(rout_name,1, sizeof(struct in_buffer));
  strcpy(new->name, "in_buffer");
  new->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", new->name);
  new->c_a = new_char_array(length);
  new->flag = -1;
  return new;
}

struct in_buff_list* new_in_buff_list(int length)
{
  char rout_name[] = "new_inbuf_list";
  struct in_buff_list* bll =
   (struct in_buff_list*) mycalloc(rout_name,1, sizeof(struct in_buff_list));
  strcpy(bll->name, "in_buff_list");
  bll->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", bll->name);
  bll->buffers =
   (struct in_buffer**) mycalloc(rout_name,length, sizeof(struct in_buffer*));
  bll->input_files =
     (FILE**) mycalloc(rout_name,length, sizeof(FILE*));
  bll->max = length;
  return bll;
}

struct in_cmd* new_in_cmd(int length)
{
  char rout_name[] = "new_in_cmd";
  struct in_cmd* new
    = (struct in_cmd*) mycalloc(rout_name,1, sizeof(struct in_cmd));
  strcpy(new->name, "in_cmd");
  new->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", new->name);
  new->tok_list = new_char_p_array(length);
  return new;
}

struct in_cmd_list* new_in_cmd_list(int length)
{
  char rout_name[] = "new_in_cmd_list";
  struct in_cmd_list* il =
       (struct in_cmd_list*) mycalloc(rout_name,1, sizeof(struct in_cmd_list));
  strcpy(il->name, "in_cmd_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->labels = new_name_list(length);
  il->in_cmds
    = (struct in_cmd**) mycalloc(rout_name,length, sizeof(struct in_cmd*));
  return il;
}

struct int_array* new_int_array(int length)
{
  char rout_name[] = "new_int_array";
  struct int_array* il =
       (struct int_array*) mycalloc(rout_name,1, sizeof(struct int_array));
  strcpy(il->name, "int_array");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->curr = 0;
  il->max = length;
  il->i = (int*) mymalloc(rout_name,length * sizeof(int));
  return il;
}

struct macro* new_macro(int n_formal, int length, int p_length)
{
  char rout_name[] = "new_macro";
  struct macro* m
    = (struct macro*) mycalloc(rout_name,1, sizeof(struct macro));
  strcpy(m->name, "macro");
  m->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", m->name);
  if ((m->n_formal  = n_formal) > 0) m->formal = new_char_p_array(n_formal);
  if (p_length > 0) m->tokens = new_char_p_array(p_length);
  m->body = new_char_array(++length);
  return m;
}

struct macro_list* new_macro_list(int length)
{
  char rout_name[] = "new_macro_list";
  struct macro_list* nll =
     (struct macro_list*) mycalloc(rout_name,1, sizeof(struct macro_list));
  strcpy(nll->name, "macro_list");
  nll->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", nll->name);
  nll->list = new_name_list(length);
  nll->macros
     = (struct macro**) mycalloc(rout_name,length, sizeof(struct macro*));
  nll->max = length;
  return nll;
}

struct name_list* new_name_list(int length)
{
  char rout_name[] = "new_name_list";
  struct name_list* il =
       (struct name_list*) mycalloc(rout_name,1, sizeof(struct name_list));
  strcpy(il->name, "name_list");
  il->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", il->name);
  il->names = (char**) mycalloc(rout_name,length, sizeof(char*));
  il->index = (int*) mycalloc(rout_name,length, sizeof(int));
  il->inform = (int*) mycalloc(rout_name,length, sizeof(int));
  il->max = length;
  return il;
}

struct node* new_node(char* name)
{
  char rout_name[] = "new_node";
  struct node* p = (struct node*) mycalloc(rout_name,1, sizeof(struct node));
  strcpy(p->name, name);
  p->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", p->name);
  return p;
}

struct node_list* new_node_list(int length)
{
  char rout_name[] = "new_node_list";
  struct node_list* nll =
    (struct node_list*) mycalloc(rout_name,1, sizeof(struct node_list));
  strcpy(nll->name, "node_list");
  nll->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", nll->name);
  nll->list = new_name_list(length);
  nll->nodes
     = (struct node**) mycalloc(rout_name,length, sizeof(struct node*));
  nll->max = length;
  return nll;
}

struct sequence* new_sequence(char* name, int ref)
{
  char rout_name[] = "new_sequence";
  struct sequence* s = mycalloc(rout_name,1, sizeof(struct sequence));
  strcpy(s->name, name);
  s->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", s->name);
  s->ref_flag = ref;
  s->nodes = new_node_list(10000);
  return s;
}

struct sequence_list* new_sequence_list(int length)
{
  char rout_name[] = "new_sequence_list";
  struct sequence_list* s
     = mycalloc(rout_name,length, sizeof(struct sequence_list));
  strcpy(s->name, "sequence_list");
  s->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", s->name);
  s->max = length;
  s->list = new_name_list(length);
  s->sequs
    = (struct sequence**) mycalloc(rout_name,length, sizeof(struct sequence*));
  return s;
}

struct node* new_sequ_node(struct sequence* sequ, int occ_cnt)
{
  struct node* p;
  p = new_node(compound(sequ->name, occ_cnt));
  p->p_sequ = sequ;
  p->length = sequ->length;
  p->base_name = permbuff("sequence");
  return p;
}

struct table* new_table(char* name, char* type, int rows,
                        struct name_list* cols)
{
  char rout_name[] = "new_table";
  int i, n = cols->curr;
  struct table* t
     = (struct table*) mycalloc(rout_name,1, sizeof(struct table));
  strcpy(t->name, name);
  strcpy(t->type, type);
  t->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", "table");
  t->columns = cols;
  t->num_cols = t->org_cols = n;
  t->s_cols = (char***) mycalloc(rout_name,n, sizeof(char**));
  t->d_cols = (double**) mycalloc(rout_name,n, sizeof(double*));
  t->max = ++rows; /* +1 because of separate augment_count */
  for (i = 0; i < n; i++)
    {
     if (cols->inform[i] < 3)
        t->d_cols[i] = (double*) mycalloc(rout_name,rows, sizeof(double));
     else if (cols->inform[i] == 3)
        t->s_cols[i] = (char**) mycalloc(rout_name,rows, sizeof(char*));
    }
  t->row_out = new_int_array(rows);
  t->col_out = new_int_array(n);
  t->node_nm = new_char_p_array(rows);
  t->p_nodes = (struct node**) mycalloc(rout_name,rows, sizeof(struct nodes*));
  t->l_head
    = (struct char_p_array**)
      mycalloc(rout_name,rows, sizeof(struct char_p_array*));
  return t;
}

struct table_list* new_table_list(int size)
{
  char rout_name[] = "new_table_list";
  struct table_list* tl
   = (struct table_list*) mycalloc(rout_name,1, sizeof(struct table_list));
  strcpy(tl->name, "table_list");
  tl->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", tl->name);
  tl->max = size;
  tl->names = new_name_list(size);
  tl->tables
    = (struct table**) mycalloc(rout_name,size, sizeof(struct table*));
  return tl;
}

struct variable* new_variable(char* name, double val, int val_type,
                              int type, struct expression* expr, char* string)
{
  char rout_name[] = "new_variable";
  struct variable* var =
    (struct variable*) mycalloc(rout_name,1, sizeof(struct variable));
  strcpy(var->name, name);
  var->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", var->name);
  var->value = val;
  var->type = type;
  var->val_type = val_type;
  if ((var->expr = expr) == NULL) var->status = 1;
  if (string != NULL) var->string = tmpbuff(string);
  return var;
}

struct var_list* new_var_list(int length)
{
  char rout_name[] = "new_var_list";
  struct var_list* var
    = (struct var_list*) mycalloc(rout_name,1, sizeof(struct var_list));
  strcpy(var->name, "var_list");
  var->stamp = 123456;
  if (watch_flag) fprintf(debug_file, "creating ++> %s\n", var->name);
  var->list = new_name_list(length);
  var->vars
    = (struct variable**) mycalloc(rout_name,length, sizeof(struct variable*));
  var->max = length;
  return var;
}

struct vector_list* new_vector_list(int length)
     /* creates a name list and pointer list
        for double arrays with initial length "length".
     */
{
  char rout_name[] = "new_vector_list";
  struct vector_list* vector
    = (struct vector_list*) mycalloc(rout_name,1, sizeof(struct vector_list));
  vector->max = length;
  vector->names = new_name_list(length);
  vector->vectors
    = (struct double_array**) mycalloc(rout_name, length,
                                       sizeof(struct double_array*));
  return vector;
}

char next_non_blank(char* string)
     /* returns next non-blank in string outside quotes, else blank */
{
  int i, toggle = 0, l = strlen(string);
  char quote = ' ';
  for (i = 0; i < l; i++)
    {
     if (toggle)
       {
      if (string[i] == quote)  toggle = 0;
       }
     else if (string[i] == '\'' || string[i] == '\"')
       {
      quote = string[i]; toggle = 1;
       }
     else if (string[i] != ' ')  return string[i];
    }
  return ' ';
}

int next_non_blank_pos(char* string)
     /* returns position of next non-blank in string outside quotes, else -1 */
{
  int i, toggle = 0, l = strlen(string);
  char quote = ' ';
  for (i = 0; i < l; i++)
    {
     if (toggle)
       {
      if (string[i] == quote)  toggle = 0;
       }
     else if (string[i] == '\'' || string[i] == '\"')
       {
      quote = string[i]; toggle = 1;
       }
     else if (string[i] != ' ')  return i;
    }
  return -1;
}

char* noquote(char* string)
{
  char* c = string;
  char* d = c;
  char k;
  if (string != NULL)
    {
     k = *c;
     if (k == '\"' || k == '\'')
       {
        d++;
        while (*d != k) *c++ = *d++;
        *c = '\0';
       }
    }
  return string;
}

int par_out_flag(char* base_name, char* par_name)
{
  /* marks the element parameters that are to be written on "save" */
  if (strcmp(par_name,"at") == 0 || strcmp(par_name,"from") == 0) return 0;
  if (strcmp(base_name, "multipole") == 0
      && strcmp(par_name,"l") == 0) return 0;
  if (strcmp(base_name, "rcollimator") == 0
      && strcmp(par_name,"lrad") == 0) return 0;
  if (strcmp(base_name, "ecollimator") == 0
      && strcmp(par_name,"lrad") == 0) return 0;
  return 1;
}

int predef_var(struct variable* var)
     /* return 1 for predefined variable, else 0 */
{
  int pos = name_list_pos(var->name, variable_list->list);
  return (pos < start_var ? 1 : 0) ;
}

void print_command(struct command* cmd)
{
  int i;
  fprintf(prt_file, "command: %s\n", cmd->name);
  for (i = 0; i < cmd->par->curr; i++)
    {
     print_command_parameter(cmd->par->parameters[i]);
     if ((i+1)%3 == 0) fprintf(prt_file, "\n");
    }
  if (i%3 != 0) fprintf(prt_file, "\n");
}

void print_command_parameter(struct command_parameter* par)
{
  int i, k;
  char logic[2][8] = {"false", "true"};
  switch (par->type)
    {
    case 0:
     k = par->double_value;
     fprintf(prt_file, "%s = %s, ", par->name, logic[k]);
     break;
    case 1:
     k = par->double_value;
     fprintf(prt_file, "%s = %d, ", par->name, k);
     break;
    case 2:
     fprintf(prt_file, "%s = %e, ", par->name, par->double_value);
     break;
    case 11:
    case 12:
    if (par->double_array != NULL)
      {
       fprintf(prt_file, "double array: ");
       for (i = 0; i < par->double_array->curr; i++)
            fprintf(prt_file, "%e, ", par->double_array->a[i]);
       fprintf(prt_file, "\n");
      }
     break;
    case 3:
     fprintf(prt_file, "%s = %s, ", par->name, par->string);
    }
}

void print_global(double delta)
{
  char tmp[NAME_L], trad[4];
  double alfa = get_value("probe", "alfa");
  double freq0 = get_value("probe", "freq0");
  double gamma = get_value("probe", "gamma");
  double beta = get_value("probe", "beta");
  double circ = get_value("probe", "circ");
  double bcurrent = get_value("probe", "bcurrent");
  double npart = get_value("probe", "npart");
  double energy = get_value("probe", "energy");
  int kbunch = get_value("probe", "kbunch");
  int rad = get_value("probe", "radiate");
  double gamtr = zero, t0 = zero, eta;
  get_string("probe", "particle", tmp);
  if (rad) strcpy(trad, "T");
  else     strcpy(trad, "F");
  if (alfa > zero) gamtr = sqrt(one / alfa);
  else if (alfa < zero) gamtr = sqrt(-one / alfa);
  if (freq0 > zero) t0 = one / freq0;
  eta = alfa - one / (gamma*gamma);
  puts(" ");
  printf(" Global parameters for %ss, radiate = %s:\n\n",
         tmp, trad);
  printf(" C         %16.8g m          f0        %16.8g MHz\n",circ, freq0);
  printf(" T0        %16.8g musecs     alfa      %16.8e \n", t0, alfa);
  printf(" eta       %16.8e            gamma(tr) %16.8g \n", eta, gamtr);
  printf(" Bcurrent  %16.8g A/bunch    Kbunch    %16d \n", bcurrent, kbunch);
  printf(" Npart     %16.8g /bunch     Energy    %16.8g GeV \n", npart,energy);
  printf(" gamma     %16.8g            beta      %16.8g\n", gamma, beta);
}

void print_rfc()
     /* prints the rf cavities present */
{
  double freq0, harmon, freq;
  int i, n = current_sequ->cavities->curr;
  struct element* el;
  if (n == 0)  return;
  freq0 = command_par_value("freq0", probe_beam);
  printf("\n RF system: \n");
  printf(" Cavity                    length[m]  voltage[MV]              lag          freq[MHz]         harmon\n");
  for (i = 0; i < n; i++)
    {
     el = current_sequ->cavities->elem[i];
     if ((harmon = el_par_value("harmon", el)) > zero)
       {
      freq = freq0 * harmon;
        printf(" %-16s  %14.6g  %14.6g  %14.6g  %18.10g  %12.0f\n",
               el->name, el->length, el_par_value("volt", el),
               el_par_value("lag", el), freq, harmon);
       }
    }
}

void print_table(struct table* t)
{
  int i, j, k, l, n, tmp, wpl = 4;
  if (t != NULL)
    {
     fprintf(prt_file, "\n");
     fprintf(prt_file, "++++++ table: %s\n", t->name);
     l = (t->num_cols-1) / wpl + 1;
     for (k = 0; k < l; k++)
       {
        n = wpl*(k+1) > t->num_cols ? t->num_cols : wpl*(k+1);
        fprintf(prt_file, "\n");
        for (i = wpl*k; i < n; i++)
           fprintf(prt_file, "%18s ", t->columns->names[i]);
        fprintf(prt_file, "\n");
        for (j = 0; j < t->curr; j++)
          {
         for (i = wpl*k; i < n; i++)
           {
            if (t->columns->inform[i] == 1)
       {
                 tmp = t->d_cols[i][j];
                 fprintf(prt_file, "%18d ", tmp);
       }
            else if (t->columns->inform[i] == 2)
                  fprintf(prt_file, "%18.10e ", t->d_cols[i][j]);
            else if (t->columns->inform[i] == 3)
                  fprintf(prt_file, "%18s ", t->s_cols[i][j]);
           }
           fprintf(prt_file, "\n");
        }
       }
    }
}

void print_value(struct in_cmd* cmd)
{
  char** toks = &cmd->tok_list->p[cmd->decl_start];
  int n = cmd->tok_list->curr - cmd->decl_start;
  int s_start = 0, end, type, nitem;
  while((type = loc_expr(toks, n, s_start, &end)) > 0)
    {
      nitem = end + 1 - s_start;
      if (polish_expr(nitem, &toks[s_start]) == 0)
         fprintf(prt_file, "%s = %-22.14g ;\n",
                 spec_join(&toks[s_start], nitem), polish_value(deco));
      else warning("invalid expression:", spec_join(&toks[s_start], nitem));
      s_start = end+1;
      if (s_start < n-1 && *toks[s_start] == ',') s_start++;
    }
}

int remove_colon(char** toks, int number, int start)
     /* removes colon behind declarative part for MAD-8 compatibility */
{
  int i, k = start;
  for (i = start; i < number; i++)  if (*toks[i] != ':') toks[k++] = toks[i];
  return k;
}

void replace(char* buf, char in, char out)
     /* replaces character in by character out in string buf */
{
  int j, l = strlen(buf);
  for (j = 0; j < l; j++)  if (buf[j] == in)  buf[j] = out;
}

int square_to_colon(char* string)
     /* sets occurrence count behind colon, possibly replacing [] */
{
  char* t;
  int k = strlen(string);
  if ((t = strchr(string, '[')) == NULL)
    {
     string[k++] = ':'; string[k++] = '1'; string[k] = '\0';
    }
  else
    {
     *t = ':';
     if ((t = strchr(string, ']')) == NULL)  return 0;
     else *t = '\0';
    }
  return strlen(string);
}

char* stolower(char* s)  /* converts string to lower in place */
{
  char *c = s;
  int j;
  for (j = 0; j < strlen(s); j++)
    {
     *c = (char) tolower((int) *c); c++;
    }
  return s;
}

void stolower_nq(char* s)
        /* converts string to lower in place outside quotes */
{
  char *c = s;
  int j, toggle = 0;
  char quote = ' '; /* just to suit the compiler */
  for (j = 0; j < strlen(s); j++)
    {
     if (toggle)
       {
      if (*c == quote) toggle = 0;
       }
     else if (*c == '\"' || *c == '\'')
       {
      toggle = 1; quote = *c;
       }
     else *c = (char) tolower((int) *c);
     c++;
    }
}

char* stoupper(char* s)  /* converts string to upper in place */
{
  char *c = s;
  int j;
  for (j = 0; j < strlen(s); j++)
    {
     *c = (char) toupper((int) *c); c++;
    }
  return s;
}

int string_cnt(char c, int n, char* toks[])
     /* returns number of strings in toks starting with character c */
{
  int i, k = 0;
  for (i = 0; i < n; i++) if(*toks[i] == c) k++;
  return k;
}

char* strip(char* name)
     /* strip ':' and following off */
{
  char* p;
  strcpy(tmp_key, name);
  if ((p = strchr(tmp_key, ':')) != NULL) *p = '\0';
  return tmp_key;
}

void supp_char(char c, char* string)
     /* suppresses character c in string */
{
  char* cp = string;
  while (*string != '\0')
    {
     if (*string != c)  *cp++ = *string;
     string++;
    }
  *cp = '\0';
}

int supp_lt(char* inbuf, int flag)
         /* suppress leading, trailing blanks and replace some special char.s*/
{
  int l = strlen(inbuf), i, j;
  replace(inbuf, '\x9', ' '); /* tab */
  replace(inbuf, '\xd', ' '); /* Windows e-o-l */
  if (flag == 0)  replace(inbuf, '\n', ' '); /* e-o-l */
  supp_tb(inbuf); /* suppress trailing blanks */
  if ((l = strlen(inbuf)) > 0)
    {
     for (j = 0; j < l; j++) if (inbuf[j] != ' ') break; /* leading blanks */
     if (j > 0)
       {
        for (i = 0; i < l - j; i++) inbuf[i] = inbuf[i+j];
        inbuf[i] = '\0';
       }
    }
  return strlen(inbuf);
}

char* supp_tb(char* string) /* suppress trailing blanks in string */
{
  int l = strlen(string), j;
  for (j = l-1; j >= 0; j--)
    {
     if (string[j] != ' ') break;
     string[j] = '\0';
    }
  return string;
}

double tgrndm(double cut)
     /* returns random variable from normal distribution cat at 'cut' sigmas */
{
  double ret = zero;
  if (cut > zero)
    {
     ret = grndm();
     while (fabs(ret) > fabs(cut))  ret = grndm();
    }
  return ret;
}

double vdot(int* n, double* v1, double* v2)
     /* returns dot product of vectors v1 and v2 */
{
  int i;
  double dot = 0;
  for (i = 0; i < *n; i++)  dot += v1[i] * v2[i];
  return dot;
}

double vmod(int* n, double* v)
{
  int i;
  double mod = 0;
  for (i = 0; i < *n; i++)  mod += v[i] * v[i];
  return sqrt(mod);
}

void write_elems(struct el_list* ell, struct command_list* cl, FILE* file)
{
  int i;
  for (i = 0; i < ell->curr; i++)
    {
     if (pass_select_list(ell->elem[i]->name, cl))
        export_element(ell->elem[i], ell, file);
    }
}

void write_elems_8(struct el_list* ell, struct command_list* cl, FILE* file)
{
  int i;
  for (i = 0; i < ell->curr; i++)
    {
     if (pass_select_list(ell->elem[i]->name, cl))
        export_elem_8(ell->elem[i], ell, file);
    }
}

void write_nice(char* string, FILE* file)
{
  int n, pos, ssc;
  char *c = string;
  char k;
  strcat(string, ";");
  n = strlen(string);
  while (n > LINE_FILL)
    {
     for (pos = LINE_FILL; pos > 10; pos--)
       {
      k = c[pos];
      if (strchr(" ,+-*/", k))  break;
       }
     c[pos] = '\0';
     fprintf(file, "%s\n", c);
     c[pos] = k;
     ssc = (int) &c[pos] - (int) c;
     n -= ssc;
     c = &c[pos];
    }
  fprintf(file, "%s\n", c);
}

void write_nice_8(char* string, FILE* file)
{
  int n, pos, comma, ssc;
  char *c = string;
  char k;
  strcat(string, ";");
  n = strlen(string);
  while (n > LINE_F_MAD8)
    {
     comma = 0;
     for (pos = LINE_F_MAD8; pos > 10; pos--)
       {
      k = c[pos];
      if (strchr(" ,+-*/", k))  break;
       }
     c[pos] = '\0';
     fprintf(file, "%s &\n", c);
     c[pos] = k;
     ssc = (int) &c[pos] - (int) c;
     n -= ssc;
     c = &c[pos];
    }
  fprintf(file, "%s\n", c);
}

void write_sequs(struct sequence_list* sql,struct command_list* cl, FILE* file)
{
  /* exports sequences in order of their nest level, flat first etc. */
  int i, j, max_nest = 0;
  for (i = 0; i < sql->curr; i++)
    if(sql->sequs[i]->nested > max_nest) max_nest = sql->sequs[i]->nested;
  for (j = 0; j <= max_nest; j++)
    {
     for (i = 0; i < sql->curr; i++)
       if(sql->sequs[i]->nested == j)
       {
          if (pass_select_list(sql->sequs[i]->name, cl))
             export_sequence(sql->sequs[i], file);
       }
    }
}

void write_vars(struct var_list* varl, struct command_list* cl, FILE* file)
{
  int i;
  for (i = 0; i < varl->curr; i++)
    {
     if (predef_var(varl->vars[i]) == 0
         && pass_select_list(varl->vars[i]->name, cl))
            export_variable(varl->vars[i], file);
    }
}

void write_vars_8(struct var_list* varl, struct command_list* cl, FILE* file)
{
  int i;
  for (i = 0; i < varl->curr; i++)
    {
     if (predef_var(varl->vars[i]) == 0
         && pass_select_list(varl->vars[i]->name, cl))
            export_var_8(varl->vars[i], file);
    }
}

void write_table(struct table* t, char* filename)
     /* writes rows with columns listed in row and col */
{
  char l_name[NAME_L];
  char sys_name[200];
  char* pc;
  struct int_array* col = t->col_out;
  struct int_array* row = t->row_out;
  int i, j, k, tmp;
  time_t now;
  struct tm* tm;
#ifndef _WIN32
  struct utsname u;
  i = uname(&u); /* get system name */
  strcpy(sys_name, u.sysname);
#endif
#ifdef _WIN32
  strcpy(sys_name, "Win32");
#endif
  time(&now);    /* get system time */
  tm = localtime(&now); /* split system time */
  if (strcmp(filename, "terminal") == 0) out_file = stdout;
  else if ((out_file = fopen(filename, "w")) == NULL)
    {
     warning("cannot open output file:", filename); return;
    }
  if (t != NULL)
    {
     strcpy(l_name, t->name);
     fprintf(out_file,
      "@ NAME             %%%02ds \"%s\"\n", strlen(t->name),
     stoupper(l_name));

     strcpy(l_name, t->type);
     fprintf(out_file,
      "@ TYPE             %%%02ds \"%s\"\n", strlen(t->type),
     stoupper(l_name));

     if (t->header != NULL)
       {
      for (j = 0; j < t->header->curr; j++)
         fprintf(out_file, "%s\n", t->header->p[j]);
       }
     if (title != NULL)
        fprintf(out_file,
         "@ TITLE            %%%02ds \"%s\"\n", strlen(title), title);

     fprintf(out_file,
      "@ ORIGIN           %%%02ds \"%s %s\"\n",
       strlen(myversion)+strlen(sys_name)+1, myversion, sys_name);

     fprintf(out_file,
      "@ DATE             %%08s \"%02d/%02d/%02d\"\n",
       tm->tm_mday, tm->tm_mon+1, tm->tm_year%100);

     fprintf(out_file,
      "@ TIME             %%08s \"%02d.%02d.%02d\"\n",
       tm->tm_hour, tm->tm_min, tm->tm_sec);
     fprintf(out_file, "* ");

     for (i = 0; i < col->curr; i++)
       {
      strcpy(l_name, t->columns->names[col->i[i]]);
        fprintf(out_file, "%-18s ", stoupper(l_name));
       }
     fprintf(out_file, "\n");

     fprintf(out_file, "$ ");
     for (i = 0; i < col->curr; i++)
       {
      if (t->columns->inform[col->i[i]] == 1)
            fprintf(out_file, "%%hd          ");
      else if (t->columns->inform[col->i[i]] == 2)
            fprintf(out_file, "%%le                ");
      else if (t->columns->inform[col->i[i]] == 3)
            fprintf(out_file, "%%s                 ");
       }
     fprintf(out_file, "\n");

     for (j = 0; j < row->curr; j++)
       {
      if (row->i[j])
        {
           if (t->l_head[j] != NULL)
             {
            for (k = 0; k < t->l_head[j]->curr; k++)
               fprintf(out_file, "%s\n", t->l_head[j]->p[k]);
           }
         for (i = 0; i < col->curr; i++)
           {
            if (t->columns->inform[col->i[i]] == 1)
       {
        tmp = t->d_cols[col->i[i]][j];
                 fprintf(out_file, " %-18d", tmp);
       }
            else if (t->columns->inform[col->i[i]] == 2)
                  fprintf(out_file, " %-18.10g", t->d_cols[col->i[i]][j]);
            else if (t->columns->inform[col->i[i]] == 3)
              {
               strcpy(c_dummy, t->s_cols[col->i[i]][j]);
                 stoupper(c_dummy);
                 pc = strip(c_dummy); /* remove :<occ_count> */
                 k = strlen(pc);
                 pc[k++] = '\"'; pc[k] = '\0';
                 fprintf(out_file, " \"%-18s", pc);
              }
           }
           fprintf(out_file, "\n");
        }
       }
     if (strcmp(filename, "terminal") != 0) fclose(out_file);
    }
}

void zero_double(double* a, int n)
     /* sets first n values in double array a to zero */
{
  int j;
  for (j = 0; j < n; j++)  a[j] = zero;
}

int zero_string(char* string) /* returns 1 if string defaults to '0', else 0 */
{
  int i, l = strlen(string);
  char c;
  for (i = 0; i < l; i++)
    if ((c = string[i]) != '0' && c != ' ' && c != '.') return 0;
  return 1;
}
