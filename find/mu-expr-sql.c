/*
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#include <stdlib.h>
#include <string.h>
#include "mu-expr.h"
#include "mu-expr-val.h"
#include "mu-expr-type.h"

static char*
get_select_line (void)
{
	GString *fields;
	int i = 0;

	fields = g_string_new ("SELECT   ");

	for (i = MU_EXPR_TYPE_TO; i != MU_EXPR_TYPE_NUM; ++i) {
		if (!mu_expr_type_db_field(i))
			continue;

		g_string_append (fields, mu_expr_type_db_field(i));

		if (i + 1 != MU_EXPR_TYPE_NUM) {
			g_string_append (fields, ", ");
			if ((i+1) % 5 == 0)
				g_string_append (fields, "\n         ");
		}
	}
	return g_string_free (fields, FALSE);
}


static const char*
get_from_line (gboolean needs_contacts)
{
	return "FROM     message m";
}

static char*
get_condition_bounded_interval (MuExprType type, int lower, int upper)
{
	char *str;
	char *strlower = NULL;
	char *strupper = NULL;
	
	if (lower != -1)
		strlower = g_strdup_printf
			("%s >= %d", mu_expr_type_db_field(type),
			 lower);
	if (upper != -1)
		strupper = g_strdup_printf
			("%s <= %d", mu_expr_type_db_field (type),
			 upper);
	
	str = g_strdup_printf ("%s%s%s",
			       strlower ? strlower : "",
				       strlower && strupper ? " AND " : "",
			       strupper ? strupper : "");
	g_free (strupper);
	g_free (strlower);

	return str;
}


static char*
get_condition_interval (MuExpr *expr)
{
	MuExprVal *val;
	MuExprType type;
	int lower, upper;

	val = mu_expr_get_val (expr);

	lower = mu_expr_val_lower (val);
	upper = mu_expr_val_upper (val);

	type = mu_expr_type (expr);

	/* swap */
	if (upper != -1 && upper < lower) {
		int tmp = lower;
		lower = upper;
		upper = tmp;
	}

	/* an 'interval' consisting of one value... */
	if (mu_expr_val_pred(val) == MU_EXPR_PRED_EXACT)
		return g_strdup_printf ("%s %s %d",
					mu_expr_type_db_field(type),
					mu_expr_val_neg(val) ? "<>" : "=",
					lower);

	else if (mu_expr_val_pred(val) == MU_EXPR_PRED_INTERVAL)
		return get_condition_bounded_interval (type, lower, upper);
		
	g_return_val_if_reached (NULL);
}

static char*
get_condition_number_logical_and (MuExpr *expr)
{
	char *str;
	int val, val_neg;

	val     = mu_expr_val_int(mu_expr_get_val(expr));
	val_neg = mu_expr_val_int_neg(mu_expr_get_val(expr));
	
	if (val == 0 && val_neg == 0) {
		g_warning ("%s: both val && val_neg are 0", __FUNCTION__);
		return NULL;
	}

	str = NULL;
	if (val)
		str = g_strdup_printf ("%s & %d = %d",
				       mu_expr_type_db_field(mu_expr_type(expr)),
				       val, val);
	if (val_neg)
		str = g_strdup_printf ("%s%s%s & %d = 0",
				       str ? str : "", str ? " AND ": "",
				       mu_expr_type_db_field(mu_expr_type(expr)), val_neg);
	return str;
}

static char*
sqlite_escape (const char *str)
{
	char *escaped;
	int c;

	escaped = (char*)malloc(2*strlen(str)+1);
	if (!escaped)
		return NULL;
	c = 0;
	while (str[0]) {
		if (str[0] == '\'') {
			strcpy (escaped + c, "''");
			c += 2;
		} else {
			strncpy (escaped + c, str, 1);
			c += 1;
		}
		++str;
	}
	escaped[c] = '\0';

	return escaped;
}


static char*
get_condition_string_single (const char* field, MuExprPred pred, gboolean neg,
			     const char* val)
{
	if (pred == MU_EXPR_PRED_EXACT)
		return g_strdup_printf ("%s %s '%s'", field, neg ? "<>" : "=", val);
	else
		return g_strdup_printf ("%s %sLIKE '%%%s%%'",
					field, neg ? "NOT " : "", val);
}

static char*
get_condition_string (MuExpr *expr)
{
	MuExprVal *val;
	MuExprCrit *crit;
	const char* str;
	char *escaped;
	GSList *conds, *cursor;
	GString *all;
	gboolean has_or;
	const MuExprType* types;

	crit = mu_expr_get_crit (expr);
	val  = mu_expr_get_val (expr);
	str  = mu_expr_val_str (val);

	g_return_val_if_fail (str, NULL);

	escaped = sqlite_escape (mu_expr_val_str(mu_expr_get_val(expr)));

	/* put all the 'foo = bar', 'cuux = bar', 'la = bar' in a list */
	conds = NULL;

	types = mu_expr_crit_types (crit);
	g_return_val_if_fail (types, NULL);

	while (types[0] != MU_EXPR_TYPE_NONE) {
		char *single = get_condition_string_single
			(mu_expr_type_db_field(types[0]),
			 mu_expr_val_pred(val),
			 mu_expr_val_neg(val),
			 escaped);
		if (single)
			conds = g_slist_append (conds, single);
		++types;
	}

	all = g_string_new (NULL);
	for (cursor = conds, has_or=FALSE; cursor; cursor = g_slist_next (cursor)) {
		g_string_append (all, (char*)cursor->data);
		if (cursor->next) {
			has_or = TRUE;
			g_string_append (all, " OR ");
		}
	}

	g_slist_foreach (conds, (GFunc)g_free, NULL);
	g_slist_free (conds);

	free (escaped);

	if (has_or) {
		g_string_prepend_c (all, '(');
		g_string_append_c  (all, ')');
	}

	return g_string_free (all, FALSE);
}


static char*
get_sort_line (const char* sortfields, gboolean ascending, gboolean quoted)
{
	GString *sortline;
	int i;

	if (!sortfields)
		return NULL;

	sortline = g_string_new ("ORDER BY ");

	for (i = 0; sortfields[i] != '\0'; ++i) {

		MuExprType type;
		type = mu_expr_type_from_char (sortfields[i]);
		if (type == MU_EXPR_TYPE_NONE) {
			g_warning ("invalid sortfield character '%c'", sortfields[i]);
			g_string_free (sortline, TRUE);
			return NULL;
		}
		
		if (quoted)
			g_string_append_printf (sortline, "\"%s\"",
						mu_expr_type_db_field(type));
		else 
			g_string_append_printf (sortline, "%s",
						mu_expr_type_db_field(type));

		if (sortfields[i + 1] != '\0')
			g_string_append (sortline, ", ");
	}
	g_string_append (sortline, ascending ? " ASC": " DESC");

	return g_string_free (sortline, FALSE);
}


char*
get_xapian_line (GHashTable *xapian_hash)
{
	GList *cursor, *keys;
	GString *str;
	int i = 0;

	if (!xapian_hash)
		return NULL;
	
	/* with no keys, we just get 'm.id IN ()' which is correct */
	keys = g_hash_table_get_keys (xapian_hash);
	
	str = g_string_sized_new (g_list_length(keys)*8); /* just a guess */
	g_string_append_printf (str, "%s IN (", 
				mu_expr_type_db_field(MU_EXPR_TYPE_ID));
	
	for (cursor = keys; cursor; cursor = cursor->next) {
		g_string_append (str, (const char*)cursor->data);
		if (cursor->next) {
			g_string_append_c (str, ',');
			if (cursor->next && (i+1) % 10 == 0)
				g_string_append (str, "\n                  ");
		} 
		++i;
	}
	g_string_append_c (str, ')');
	g_list_free (keys);

	return g_string_free (str, FALSE);
}


char*
get_condition_lines (MuExprList *exprs)
{	
	GString *str;

	if (!exprs)
		return NULL;

	str = g_string_sized_new(20*g_slist_length(exprs)); /* just a guess */
	
	while (exprs) {
		char *cond;
		MuExpr *expr = (MuExpr*)exprs->data;

		switch (mu_expr_type(expr)) {
		case MU_EXPR_TYPE_DATE:
		case MU_EXPR_TYPE_SIZE:
			cond = get_condition_interval (expr);
			break;
		case MU_EXPR_TYPE_FLAGS:
		case MU_EXPR_TYPE_PRIORITY:
			cond = get_condition_number_logical_and (expr);
			break;
		default:
			cond = get_condition_string (expr);
		}

		g_string_append (str, cond);
		g_free (cond);
		
		exprs = g_slist_next (exprs);
		if (exprs)
			g_string_append (str, "\nAND      ");
	}

	return g_string_free (str, FALSE);
}

char*
mu_expr_sql_generate (MuExprList *exprs, GHashTable *xapian_hash,
		      const char* sortfields, gboolean ascending)
{
	gchar   *select_line, 
		*xapian_line,
		*condition_lines,
		*sort_line;
	const gchar *from_line;
	
	GString *sql;

	select_line      = get_select_line ();
	xapian_line      = (xapian_hash) ? get_xapian_line (xapian_hash) : NULL;
	condition_lines  = (exprs) ? get_condition_lines (exprs) : NULL; 
	from_line        = get_from_line (FALSE);
	sort_line        = sortfields ? get_sort_line(sortfields,ascending,FALSE):NULL; 

	sql = g_string_sized_new (512); /* just a guess */
			
	g_string_append_printf (sql, "%s\n%s", select_line, from_line);

	if (xapian_line||condition_lines)
		g_string_append (sql, "\nWHERE    ");
	if (xapian_line) {
		g_string_append (sql, xapian_line);
		if (condition_lines) 
			g_string_append_printf (sql, 
						"\nAND      %s", condition_lines);
	} else if (condition_lines) 
		g_string_append (sql, condition_lines);
	
	if (sort_line)
		g_string_append_printf (sql, "\n%s", sort_line);
	
	g_string_append (sql, ";\n");

	g_free(select_line);
	g_free(xapian_line);
	g_free(condition_lines);
	g_free(sort_line);
	
	return g_string_free (sql, FALSE);
}


char*
mu_expr_sql_generate_sql_view_create (MuExprList *exprs, GHashTable *xapian_hash,
				      const char* tmpview)
{	
	GString *sql;
	char *str;
	
	g_return_val_if_fail (tmpview, NULL);

	sql = g_string_sized_new (512); /* just a guess */
	
	/*
	 * NOTE: testing showed that using temporary VIEWs is much faster than
	 * using temporary TABLEs
	 */
	str = mu_expr_sql_generate (exprs,xapian_hash, NULL, FALSE);
	g_string_append_printf (sql, "CREATE TEMP VIEW %s AS\n%s", 
				tmpview, str);

	g_free (str);
	return g_string_free (sql, FALSE);
}


char*
mu_expr_sql_generate_sql_view_read (const char* tmpview, const char* sortfields,
				    gboolean ascending)
{
	GString *sql;
	char *sortline;
	
	g_return_val_if_fail (tmpview, NULL);

	sql = g_string_sized_new (512); /* just a guess */
	g_string_append_printf (sql, "SELECT * FROM %s", tmpview);

	sortline  = sortfields ? get_sort_line(sortfields,ascending,TRUE) : NULL; 
	if (sortline)
		g_string_append_printf (sql, "\n%s", sortline);
	
	g_string_append (sql, ";\n");
	
	g_free (sortline);
	return g_string_free (sql, FALSE);
}
