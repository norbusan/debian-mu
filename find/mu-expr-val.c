/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
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

#include <string.h>
#include <stdlib.h>

#include "path/mu-path.h" /* for MU_FILE...*/
#include "mu/mu-msg.h" /* for MU_MSG_....*/
#include "mu/mu-msg-flags.h" 

#include "mu-expr-val.h"
#include "mu-expr-error.h"
#include "mu-expr-type.h"

struct _MuExprVal {
	char*	   _str;
	int        _lower, _upper; /* interval */
	int        _int, _int_neg;
	gboolean   _neg;
	MuExprPred _pred;
};


/* numeric types describe interval, ie.
 * 1-2 for 'between 1 and 2', 1- for '>= 1' and -2 for '<= 2'
 * 1 means 'exactly 1' 
 * this function splits the type in these '1' and '2', or NULL
 * if they don't exist. 
 */
static gboolean
parse_interval (const char *str, char** left, char **right,
		gboolean *has_dash, GError **err)
{
	gchar *dash;
	
	dash = strchr (str, '-');
	if (!dash) { /* there is only one value, '1' */
		*left  = g_strdup(str);
		*right = NULL;
		*has_dash = FALSE;
		return TRUE;
	}

	if (strlen(str) == 1) { /* there's only a dash */
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_INTERVAL,
			     "expected: some values for interval");
		return FALSE;
	}
	
	if (strchr (dash + 1, '-')) { /* there can be only one dash */
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_INTERVAL,
			     "multiple dashes in interval expression");
		return FALSE;
	}

	if (dash == str) { /* something like '-2' */
		*left  = NULL;
		*right = g_strdup(dash + 1);
		*has_dash = TRUE;
		return TRUE;
	}
		
	if ((dash + 1)[0] == '\0') { /* '1-' */
		*left  = g_strndup(str, dash-str);
		*right = NULL;
		*has_dash = TRUE;
		return TRUE;
	}

	/* 1-2 */
	*left  = g_strndup(str, dash - str);
	*right = g_strdup(dash + 1);
	*has_dash = TRUE;

	return TRUE;
}

static gboolean
parse_size_val (const char* str, int *val, GError **err)
{
	int len;
	char k;
	int unit;
	double d;
	char *endptr;

	g_return_val_if_fail (val, FALSE);

	if (!str) {/* not an error; value is unset */
		*val = -1;
		return TRUE;
	}
	
	len = strlen(str);
	g_return_val_if_fail (len > 0, FALSE);

	k = str[len -1];
	if (g_ascii_isdigit (str[len -1]))
		unit = 1;
	/* http://en.wikipedia.org/wiki/Byte */
	else if (k == 'k') {
 		unit = 1000;        /* 1000 bytes */
		--len;
	} else if (k == 'M') {
		unit = 1000 * 1000; /* 1000000 bytes */
		--len;
	} else {
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_SIZE,
			     "invalid size '%s'; only k and M are valid units",
			     str);
		return FALSE;
	}
	
	d = strtod (str, &endptr);
	if (endptr - str != len) {
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_SIZE,
			     "invalid size '%s'", str);
		return FALSE;
	}

	*val = unit * d;
	
	return TRUE;
}


static gboolean
parse_size (MuExprVal *ev, const char *str, GError **err)
{
	char *left, *right;
	gboolean has_dash;
	int val1, val2;
	gboolean retval;

	if (!parse_interval (str, &left, &right, &has_dash, err))
		return FALSE;

	if (has_dash)
		ev->_pred = MU_EXPR_PRED_INTERVAL;

	do {
		if (!(retval = parse_size_val (left,  &val1, err))) 
			break;
		if (!(retval = parse_size_val (right, &val2, err))) 
			break;
		
		ev->_lower = val1;
		ev->_upper = val2;
		
		retval = TRUE;
		
	} while (0);
	
	g_free (left);
	g_free (right);
	
	return retval;
}


static time_t
day_start_time_t (GDate *date)
{
	struct tm tmbuf;
	g_date_to_struct_tm (date, &tmbuf);

	return mktime (&tmbuf);
}

static time_t
day_end_time_t (GDate *date)
{
	struct tm tmbuf;
	g_date_to_struct_tm (date, &tmbuf);

	/* end of the day...*/
	tmbuf.tm_hour  = 23;
	tmbuf.tm_min   = 59;
	tmbuf.tm_sec   = 59;
	tmbuf.tm_isdst = -1;

	return mktime (&tmbuf);
}


static GDate*
parse_date_gdate (const char* str, GError **err)
{
	GDate *date;

	date = g_date_new();
	g_date_set_parse (date, str);
	if (!g_date_valid (date)) {
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_DATE,
			     "invalid date: '%s'", str);
		g_date_free (date);
		return NULL;
	} else if (sizeof(time_t) <= 4) { 
		/* time_t won't cut it on 32 bit machines...*/
		if (g_date_year(date) < 1970 || g_date_year(date) > 2037) {
			g_set_error (err, 0,
				     MU_EXPR_ERROR_DATE_OUT_OF_RANGE,
				     "date must lie between 1970 and 2037");
			g_date_free (date);
			return NULL;
		}
	} 
	
	return date;
}


/* we understand:
 * 3d, 3w, 3m, 3y, for resp 3 days, weeks, monts years ago */
static GDate*
parse_date_shorthand (const char* str, GError **err)
{
	char *endptr;
	long int val;
	char kar;
	GDate *date;
	time_t t;

	val = strtol (str, &endptr, 10);
	if (val == 0)
		val = 1; /* we interpret 'd' as '1d' */
	kar = *endptr;
	
	if (*endptr != str[strlen(str) - 1]) {
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_DATE,
			     "invalid date expression");
		return NULL;
	}
	    
	switch (kar) {
 	case 'd': t = time(NULL) - val *  24 * 60 * 60; break; /* day */
	case 'w': t = time(NULL) - val *  7 * 24 * 60 * 60; break; /* week */
	case 'f': t = time(NULL) - val *  14 * 24 * 60 * 60; break; /* fortnight */
	case 'm': t = time(NULL) - val *  30 * 24 * 60 * 60; break; /* month */
	case 'y': t = time(NULL) - val * 365 * 24 * 60; break; /* year */
	default:
		g_set_error (err, 0,
			     MU_EXPR_ERROR_INVALID_DATE,
			     "invalid date expression");
		return NULL;
	}
	date = g_date_new();
	g_date_set_time_t (date, t);

	return date;
}


static GDate*
parse_date_shorthand_or_gdate (const char *str, GError **err)
{
	GDate *date;

	if (!str || strlen(str) == 0)
		return NULL;
	
	date = parse_date_shorthand (str, err);
	if (err && *err) { /* ignore errors; it probably wasn't shorthand */
		g_error_free (*err);
		*err = NULL;
	}
	if (!date) 
		date = parse_date_gdate (str, err);
	
	return date;
}

/* swap d1 and d2 if d2 comes before d1, otherwise, do nothing */
static gboolean
swap_earliest_date_first_maybe (GDate **d1, GDate **d2)
{
	if (!(*d1 && *d2))
		return TRUE; /* not an error */
	
	if (g_date_compare (*d1, *d2) > 0) {
		GDate *tmp = *d1;
		*d1 = *d2;
		*d2 = tmp;
		return TRUE;
	}

	return FALSE;
}


static gboolean
parse_date (MuExprVal *ev, const char *str, GError **err)
{
	char *left, *right;
	gboolean has_dash;
	GDate *left_date, *right_date;

	if (!parse_interval (str, &left, &right, &has_dash, err))
		return FALSE;

	ev->_pred = MU_EXPR_PRED_INTERVAL;
	
	left_date = parse_date_shorthand_or_gdate (left, err);
	g_free (left);
	if (*err) {
		g_free(right);
		return FALSE;
	}
	
	right_date = parse_date_shorthand_or_gdate (right, err);
	g_free (right);
	if (*err)
		return FALSE;

	/* swap? we need to check here, because we need beginning-of-day
	 * for smallest one, end-of-day for the other
	 */
	swap_earliest_date_first_maybe (&left_date, &right_date);
	
	if (has_dash) {
		ev->_lower = left_date  ?  day_start_time_t (left_date) : -1;
		ev->_upper = right_date ?  day_end_time_t   (right_date) : -1;
	} else { /* no dash; we turn 'left_date' into an interval */
		ev->_lower = day_start_time_t (left_date);
		ev->_upper = day_end_time_t (left_date);
	}
	
	if (left_date)
		g_date_free (left_date);
	if (right_date)
		g_date_free (right_date);
		
	return TRUE;
}



static const char*
determine_string_pred (MuExprVal *ev, const char* str)
{
	if (str[0] == '^') {
		ev->_neg = TRUE;
		++str;
	} else
		ev->_neg = FALSE;

	switch (str[0]) {
	case '=': ev->_pred = MU_EXPR_PRED_EXACT;
		++str;
		break;
	/* case '~': ev->_pred = EXPR_PRED_REGEXP;  */
	/* 	break; */
	default: ev->_pred = MU_EXPR_PRED_LIKE;
		break; /* nothing */
	}

	return str;
}

static gboolean
parse_string (MuExprVal *ev, const char *str, GError **err)
{
	const char* str2;
	
	str2 = determine_string_pred (ev, str);
	ev->_str  = g_strdup (str2); 	

	return TRUE;
}


static gboolean
parse_flags (MuExprVal *val, const char* str, GError **err)
{
	gboolean neg = FALSE;
	val->_int = val->_int_neg = 0;

	for (;str[0]; ++str) { 
		int *val_or_neg;
		
		if (str[0] == '^') {/* negate the next flag? */
			if (neg) {
				g_set_error (err, 0, MU_EXPR_ERROR_INVALID_FLAG,
					     "multiple negations are not allowed");
				return FALSE;
			} else 
				neg = TRUE;
			continue;
		}
		
		val_or_neg = neg ? &val->_int_neg : &val->_int;

		switch (str[0]) {
			/* this one is for messages in tmp */
		case 'N': *val_or_neg |= MU_MSG_FLAG_NEW; break;

			/* below are actually maildir flags */
		case 'P': *val_or_neg |= MU_MSG_FLAG_PASSED; break;
		case 'R': *val_or_neg |= MU_MSG_FLAG_REPLIED; break;
		case 'S': *val_or_neg |= MU_MSG_FLAG_SEEN; break;
		case 'T': *val_or_neg |= MU_MSG_FLAG_TRASHED; break;
		case 'D': *val_or_neg |= MU_MSG_FLAG_DRAFT; break;
		case 'F': *val_or_neg |= MU_MSG_FLAG_FLAGGED; break;

			/* content flags */
		case 'a': *val_or_neg |= MU_MSG_FLAG_HAS_ATTACH; break;
		case 's': *val_or_neg |= MU_MSG_FLAG_SIGNED; break;
		case 'x': *val_or_neg |= MU_MSG_FLAG_ENCRYPTED; break;

		default:
			g_set_error (err, 0, MU_EXPR_ERROR_INVALID_FLAG,
				     "'%c' is not a valid flag", str[0]);
			return FALSE;
		}
		neg = FALSE;
	}

	if (val->_int == 0 && val->_int_neg == 0) {
		g_set_error (err, 0, MU_EXPR_ERROR_INVALID_FLAG,
			     "flag expression invalid");
		return FALSE;
	}

	return TRUE;
}

static gboolean
parse_priority (MuExprVal *val, const char* str, GError **err)
{
	val->_int = 0;
	while (str[0]) {
		switch (str[0]) {
			/* this one is for messages in tmp */
		case 'L': val->_int |= MU_MSG_PRIORITY_LOW; break;
		case 'N': val->_int |= MU_MSG_PRIORITY_NORMAL; break;
		case 'H': val->_int |= MU_MSG_PRIORITY_HIGH; break;
		default:
			g_set_error (err, 0,
				     MU_EXPR_ERROR_INVALID_PRIORITY,
				     "'%c' is not a valid priority", str[0]);
			return FALSE;
		}
		++str;
	}
	return TRUE;
}



MuExprVal*
mu_expr_val_new (MuExprType type, const char* str, GError **err)
{
	MuExprVal *val;
	gboolean result;
	GError *my_err = NULL;

	g_return_val_if_fail (str, NULL);
	
	val = g_new0(MuExprVal,1);
	my_err = err ? *err : NULL;

	switch (type) {
	case MU_EXPR_TYPE_DATE:
		result = parse_date (val, str, &my_err); break;
	case MU_EXPR_TYPE_SIZE:
		result = parse_size (val, str, &my_err); break;
	case MU_EXPR_TYPE_FLAGS:
		result = parse_flags (val, str, &my_err); break;
	case MU_EXPR_TYPE_PRIORITY:
		result = parse_priority (val, str, &my_err); break;
	default: /* string type */	
		g_return_val_if_fail (mu_expr_type_is_string(type), NULL);
		result = parse_string (val, str, &my_err);
	}
	
	if (my_err && !err)
		g_error_free (my_err);

	if (!result) {
		mu_expr_val_destroy (val);
		return NULL;
	}

	return val;
}

void
mu_expr_val_destroy (MuExprVal *val)
{
	if (val) {
		g_free (val->_str);
		g_free (val);
	}
}


const char*
mu_expr_val_str (MuExprVal *val)
{
	g_return_val_if_fail (val, NULL);
	
	return val->_str;
}


int
mu_expr_val_int (MuExprVal *val)
{
	g_return_val_if_fail (val, -1);
	
	return val->_int;
}

int
mu_expr_val_int_neg (MuExprVal *val)
{
	g_return_val_if_fail (val, -1);
	
	return val->_int_neg;
}



int
mu_expr_val_lower (MuExprVal *val)
{
	g_return_val_if_fail (val, -1);
	
	return val->_lower;
}

int
mu_expr_val_upper (MuExprVal *val)
{
	g_return_val_if_fail (val, -1);
	
	return val->_upper;
}


MuExprPred
mu_expr_val_pred (MuExprVal *val)
{
	g_return_val_if_fail (val, -1);
	
	return val->_pred;
}


gboolean
mu_expr_val_neg (MuExprVal *val)
{
	g_return_val_if_fail (val, FALSE);
	
	return val->_neg;
}
