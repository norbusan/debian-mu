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


#ifndef __MU_MSG_GMIME_INIT_H__
#define __MU_MSG_GMIME_INIT_H__

/** 
 * initialize the message parsing system; this function must be called
 * before doing any message parsing (ie., any of the other
 * mu_msg_gmime functions). when done with the message parsing system,
 * call mu_msg_gmime_uninit. Note: calling this function on an already
 * initialized system has no effect
 */
void     mu_msg_gmime_init            (void);

/** 
 * uninitialize the messge parsing system that has previously been
 * initialized with mu_msg_init. not calling mu_msg_uninit after
 * mu_msg_init has been called will lead to memory leakage. Note:
 * calling mu_msg_uninit on an uninitialized system has no
 * effect
 */
void     mu_msg_gmime_uninit          (void);  


#endif /*__MU_MSG_GMIME_INIT_H__*/
