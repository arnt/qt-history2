/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** Licensees holding valid Qt Palmtop Developer license may use this 
** file in accordance with the Qt Palmtop Developer License Agreement 
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
** PURPOSE.
**
** email sales@trolltech.com for information about Qt Palmtop License 
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 * $Id: job.h,v 1.1 2001/09/16 17:20:32 lknoll Exp $
 * 
 * Copyright (C) 2000, 2001 by Martin Pool <mbp@samba.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


struct rs_job {
    int                 dogtag;

    /** Human-readable job operation name. */
    const char          *job_name;
    
    rs_buffers_t *stream;

    /** Callback for each processing step. */
    rs_result           (*statefn)(rs_job_t *);

    /** Final result of processing job.  Used by rs_job_s_failed(). */
    rs_result final_result;

    /* XXX: These next two are redundant with their equivalents in the
     * signature field.  Perhaps we should get rid of them, but
     * they're also used in the mksum operation. */
    int                 block_len;
    int                 strong_sum_len;

    /** Signature that's either being read in, or used for
     * generating a delta. */
    rs_signature_t      *signature;
    
    /** Command byte currently being processed, if any. */
    unsigned char       op;

    /** If in the middle of reading a signature (rs_loadsig_s_weak()),
     * or generating a delta, this contains the weak signature. */
    rs_weak_sum_t       weak_sig;

    /** If generating a delta, this is true if we have a valid weak signature and
     * can roll it forward. */
    int                 have_weak_sig;

    /** Lengths of expected parameters. */
    rs_long_t           param1, param2;
    
    struct rs_prototab_ent const *cmd;
    rs_mdfour_t      output_md4;

    /** Encoding statistics. */
    rs_stats_t          stats;

    /** Buffer of data left over in the scoop.  Allocation is
     * scoop_buf..scoop_alloc, and scoop_next[0..scoop_avail]
     * contains valid data. */
    char       *scoop_buf;
    char       *scoop_next;
    size_t      scoop_alloc;
    size_t      scoop_avail;
        
    /** If USED is >0, then buf contains that much write data to
     * be sent out. */
    char        write_buf[16];
    int         write_len;

    /** If \p copy_len is >0, then that much data should be copied
     * through from the input. */
    rs_long_t   copy_len;

    /** Copy from the basis position. */
    rs_long_t       basis_pos, basis_len;

    /** Callback used to copy data from the basis into the output. */
    rs_copy_cb      *copy_cb;
    void            *copy_arg;
};


rs_job_t * rs_job_new(const char *, rs_result (*statefn)(rs_job_t *));

void rs_job_check(rs_job_t *job);
const rs_stats_t *rs_job_statistics(rs_job_t *);

int rs_job_input_is_ending(rs_job_t *job);
