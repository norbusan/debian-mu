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

#include <glib.h>
#include <string.h>

#include "mu/mu-msg-str.h"
#include "msg/mu-msg-gmime.h"

static void
test_mu_msg_01 (void)
{
	MuMsgGMime *msg;
	
	msg = mu_msg_gmime_new ("test01.msg");
	g_assert (msg);
	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"Test Test <testmail@example.com>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==,"gcc-help@gcc.gnu.org");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "3BE9E6535E30294486709ABABA798852173@example.com");
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg),==,"paradise lost");
	g_assert (mu_msg_gmime_get_body_html (msg)== NULL);
	g_assert (strlen(mu_msg_gmime_get_body_text (msg)) == 1110);
	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test01.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 1393);

	
	mu_msg_gmime_destroy (msg);
}

static void
test_mu_msg_02 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test02.msg");
	g_assert (msg);
	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,"Test Test <test@gmail.com>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
			"sqlite-announce@sqlite.org");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "376dd1950808220027r7f9f8182m6703783864618d88@"
			 "mail.gmail.com");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "Re: [sqlite-announce] database copying problem"
			 " when used mac book");
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_LOW);
	g_assert_cmpstr (mu_msg_gmime_get_mailing_list (msg),==,
			 "<sqlite-announce.sqlite.org>");

	g_assert_cmpuint(mu_msg_gmime_get_date (msg),==,1219390065);

	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);

	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test02.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 4623);

	mu_msg_gmime_destroy (msg);
}

static void
test_mu_msg_03 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test03.msg");
	g_assert (msg);
	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,"xxxxx <xxxxx.gr@gmail.com>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
			"sqlite-dev@sqlite.org");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "477821040808040038s381bf382p7411451e3c1a2e4e@"
			 "mail.gmail.com");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "Re: [sqlite-dev] SQLite exception");
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_LOW);
	g_assert_cmpstr (mu_msg_gmime_get_mailing_list (msg),==,
			 "<sqlite-dev.sqlite.org>");

	g_assert_cmpuint(mu_msg_gmime_get_date (msg),==,1217835502);

	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) != NULL);

	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test03.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 11790);
	
	mu_msg_gmime_destroy (msg);
}


static void
test_mu_msg_04 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test04.msg");
	g_assert (msg);
	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"Arvopaperi <arvopaperi@arvopaperi.fi>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
			"someone@gmail.com");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "27907688.1202456820176.JavaMail.oraweb@pontiac"
			 ".talentum.fi");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "EKP:n tiukka linja rakoilee");
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_NORMAL);
	g_assert (mu_msg_gmime_get_mailing_list (msg) == NULL);
	g_assert (mu_msg_gmime_get_cc (msg) == NULL);

	g_assert_cmpstr (mu_msg_gmime_get_header (msg,"Reply-To"), ==, 
			 "arvopaperi.noreply@talentum.com");

	g_assert (mu_msg_gmime_get_header (msg,"DoesNotExist") == NULL);
	
	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) != NULL);

	
	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test04.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 45067);

	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_ENCRYPTED),==, 
			  0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_HAS_ATTACH),==, 
			  0);
	
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_SIGNED),==, 
			  0);

	mu_msg_gmime_destroy (msg);
}



static void
test_mu_msg_05 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test05.msg");
	g_assert (msg);

	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"Walter Evans <4Coeducation@jenniferblock.com>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
	 		"diggleberry@gmail.com, someone@gmail.com, "
			"someone420@gmail.com, diggobiggo@gmail.com, "
			"diggreg@gmail.com");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "790z868s.6604073@alwaysbrilliant.com");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "Christmas Replica Watches");
	g_assert_cmpstr (mu_msg_gmime_get_user_agent(msg), ==, 
			 "Mediacomm Communicator 1.71");
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_NORMAL);
	g_assert (mu_msg_gmime_get_mailing_list (msg) == NULL);
	g_assert (mu_msg_gmime_get_cc (msg) == NULL);
	
	g_assert_cmpstr (mu_msg_gmime_get_header (msg,"X-Auth"), ==, "3-DES");
	g_assert (mu_msg_gmime_get_header (msg,"DoesNotExist") == NULL);
	
	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) == NULL);
	
	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test05.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 1914);

	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_ENCRYPTED),==, 
			  0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_HAS_ATTACH),==, 
			  0);

	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_SIGNED),==, 
			  0);

	
	mu_msg_gmime_destroy (msg);
}



static void
test_mu_msg_06 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test06.msg");
	g_assert (msg);

	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"Foo Bar <foobar@sympatico.ca>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
	 		"help-gnu-emacs@gnu.org");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "877iaonh3q.fsf@MagnumOpus.Mercurius");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "Re: Can anybody tell me how to send HTML-format mail in gnus");
	g_assert_cmpstr (mu_msg_gmime_get_user_agent(msg), ==, 
			 "Gnus/5.110011 (No Gnus v0.11) Emacs/23.0.60 (gnu/linux)");
	/* priority is 'list', so... */
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_LOW);
	g_assert_cmpstr (mu_msg_gmime_get_mailing_list (msg),==, 
		"Users list for the GNU Emacs text editor <help-gnu-emacs.gnu.org>");
	g_assert (mu_msg_gmime_get_cc (msg) == NULL);
	
	g_assert_cmpstr (mu_msg_gmime_get_header (msg,"X-Mailman-Version"), ==, "2.1.5");
	g_assert (mu_msg_gmime_get_header (msg,"DoesNotExist") == NULL);
	
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_ENCRYPTED),==, 
			  0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_HAS_ATTACH),==, 
			  0);

	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_SIGNED),==, 
			  MU_MSG_FLAG_SIGNED);
	
	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) == NULL);

	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test06.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 5143);

	
	mu_msg_gmime_destroy (msg);
}


static void
test_mu_msg_07 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test07.msg");
	g_assert (msg);

	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"ext def hello <foo.hello@wxs.nl>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
	 		"abc.hello@z123.com");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "000301c91b4d$8cb7d760$4b01a8c0@defLaptop");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "FW: Your Online Flight Ticket  N 60259");
	g_assert_cmpstr (mu_msg_gmime_get_user_agent(msg), ==, 
			 "Microsoft Office Outlook 11");
	/* priority is 'list', so... */
	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_NORMAL);
	g_assert_cmpstr (mu_msg_gmime_get_mailing_list (msg),==, NULL);
	g_assert (mu_msg_gmime_get_cc (msg) == NULL);
	g_assert_cmpstr (mu_msg_gmime_get_header (msg,"X-MimeOLE"), ==, 
			 "Produced By Microsoft MimeOLE V6.00.2900.3350");
	g_assert (mu_msg_gmime_get_header (msg,"DoesNotExist") == NULL);
	
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_SIGNED),==, 
			  0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_ENCRYPTED),==, 
			  0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_HAS_ATTACH),==, 
			  MU_MSG_FLAG_HAS_ATTACH);
	
	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) == NULL);

	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test07.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 2855);
	
	mu_msg_gmime_destroy (msg);
}


static void
test_mu_msg_08 (void)
{
	MuMsgGMime *msg;

	msg = mu_msg_gmime_new ("test08.msg");
	g_assert (msg);

	g_assert_cmpstr(mu_msg_gmime_get_from (msg),==,
			"Арсен Дерганенко  "
			"<kevin.gillettnn@eastbourne.audi.co.uk>");
	g_assert_cmpstr(mu_msg_gmime_get_to (msg),==, 
	 		"diggitee@gmail.com");
	g_assert_cmpstr (mu_msg_gmime_get_msgid (msg),==,
			 "003b01c93417$032a99ae$06e832b9@jmfqyg");	
	g_assert_cmpstr (mu_msg_gmime_get_subject (msg), ==, 
			 "[4]: Система антикризисного управления филиалами");
	g_assert_cmpstr (mu_msg_gmime_get_user_agent(msg), ==, 
			 "Microsoft Outlook Express 6.00.2900.3138");

	g_assert_cmpuint (mu_msg_gmime_get_priority (msg),==, MU_MSG_PRIORITY_NORMAL);
	g_assert_cmpstr (mu_msg_gmime_get_mailing_list (msg),==, NULL);
	g_assert (mu_msg_gmime_get_cc (msg) == NULL);
	g_assert_cmpstr (mu_msg_gmime_get_header (msg,"X-MimeOLE"), ==, 
			 "Produced By Microsoft MimeOLE V6.00.2900.3198");
	g_assert (mu_msg_gmime_get_header (msg,"DoesNotExist") == NULL);
	
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_SIGNED),==, 0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg) & MU_MSG_FLAG_ENCRYPTED),==,0);
	g_assert_cmpuint ((mu_msg_gmime_get_flags (msg)& MU_MSG_FLAG_HAS_ATTACH),==,0);
	
	g_assert (mu_msg_gmime_get_body_text(msg) != NULL);
	g_assert (mu_msg_gmime_get_body_html(msg) != NULL);
	
	g_assert_cmpstr (mu_msg_gmime_get_path(msg),==, "test08.msg");
	g_assert_cmpuint (mu_msg_gmime_get_file_size(msg),==, 9413);

	mu_msg_gmime_destroy (msg);
}





static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	
	int retval;
	g_test_init (&argc, &argv, NULL);

	mu_msg_gmime_init ();

	g_test_add_func ("/msg/read-msg-01", test_mu_msg_01);
	g_test_add_func ("/msg/read-msg-02", test_mu_msg_02);
	g_test_add_func ("/msg/read-msg-03", test_mu_msg_03);
	g_test_add_func ("/msg/read-msg-04", test_mu_msg_04);
	g_test_add_func ("/msg/read-msg-05", test_mu_msg_05);
	g_test_add_func ("/msg/read-msg-06", test_mu_msg_06);
	g_test_add_func ("/msg/read-msg-07", test_mu_msg_07);
	g_test_add_func ("/msg/read-msg-08", test_mu_msg_08);
	
	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL);

	retval = g_test_run ();

	mu_msg_gmime_uninit ();
	
	return retval;
}
