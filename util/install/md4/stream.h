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
 * librsync -- library for network deltas
 * $Id: stream.h,v 1.1 2001/09/16 17:20:32 lknoll Exp $
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

                /* Two wars in a lifetime bear hard on the little places.
                 * In winter when storms come rushing out of the dark,
                 * And the bay boils like a cauldron of sharks,
                 * The old remember the trenches at Paschendale
                 * And sons who died on the Burma Railway. */


int rs_buffers_is_empty(rs_buffers_t *stream);
int rs_buffers_copy(rs_buffers_t *stream, int len);

int rs_tube_catchup(rs_job_t *);
void rs_tube_write(rs_job_t *, void const *buf, size_t len);
void rs_tube_copy(rs_job_t *, int len);
int rs_tube_is_idle(rs_job_t const *);
void rs_check_tube(rs_job_t *);

void rs_buffers_check_exit(rs_buffers_t const *);

void rs_scoop_advance(rs_job_t *, size_t len);
rs_result rs_scoop_readahead(rs_job_t *, size_t len, void **ptr);
rs_result rs_scoop_read(rs_job_t *, size_t len, void **ptr);
rs_result rs_scoop_read_rest(rs_job_t *, size_t *len, void **ptr);
size_t rs_scoop_total_avail(rs_job_t *job);
void rs_scoop_input(rs_job_t *job, size_t len);
