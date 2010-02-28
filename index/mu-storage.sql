-- database definition for storage

-- Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or
-- (at your option) any later version.
--  
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--  
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software Foundation,
-- Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  

-- go ahead!
BEGIN TRANSACTION;

-- the type of contact (to,cc,bcc,from)
-- these should *NOT* be changed; the code defines & depends on the
-- ids
CREATE TABLE IF NOT EXISTS contact_type (
       id       INTEGER PRIMARY KEY,
       descr    TEXT NOT NULL UNIQUE -- to, cc, bcc, from
);

-- some contact		
CREATE TABLE IF NOT EXISTS contact (
       id       INTEGER PRIMARY KEY,
       cname    TEXT DEFAULT '', -- sometimes, it's NULL; we set it
       address  TEXT DEFAULT '' -- to empty to prevent dups
);     		
CREATE INDEX IF NOT EXISTS contact_address_index ON contact (address);

-- table containing the information about a certain
-- message. note that the To/From/Cc info is also in the contact
-- table; this is for optimization reasons (handle both specific contacs
-- and message queries quickly)
CREATE TABLE IF NOT EXISTS message (
       id	  INTEGER PRIMARY KEY,
       msg_id     TEXT,                 -- the message id
       tstamp	  INTEGER DEFAULT 0,    -- timestamp
       mpath      TEXT NOT NULL UNIQUE, -- path to the message
       mdate      INTEGER DEFAULT 0,    -- message date (time_t)
       msize      INTEGER DEFAULT 0,    -- message size (in bytes)
       sender     VARCHAR,              -- message sender (From:)
       recipients TEXT,			-- message recipient (To:)
       cc         TEXT,			-- CC:
       subject    TEXT,                 -- message subject
       flags      INTEGER,              -- flags (MuMsgFlags)
       priority   INTEGER               -- priority (MuMsgPriority)
);
CREATE UNIQUE INDEX IF NOT EXISTS mpath_index ON message(mpath);
       	      	       
-- -- all the recipients for a certain message
CREATE TABLE IF NOT EXISTS message_contact (
       message_id           INTEGER,   -- points to message      
       contact_id           INTEGER,   -- points to contact
       contact_type_id      INTEGER    -- points to contact_type 
);

-- save to database
COMMIT;
