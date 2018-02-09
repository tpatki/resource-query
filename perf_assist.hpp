/*****************************************************************************\
 *  Copyright (c) 2014 Lawrence Livermore National Security, LLC.  Produced at
 *  the Lawrence Livermore National Laboratory (cf, AUTHORS, DISCLAIMER.LLNS).
 *  LLNL-CODE-658032 All rights reserved.
 *
 *  This file is part of the Flux resource manager framework.
 *  For details, see https://github.com/flux-framework.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license, or (at your option)
 *  any later version.
 *
 *  Flux is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#ifndef PERF_ASSIST_HPP
#define PERF_ASSIST_HPP

#include <iostream>


class perf_assist_t
{
public:
    perf_assist_t () {
    		m_worst_perf_class = -1;
    		// Assume that each performance class degrades by 3%.
    		// TODO: We will want to make the number of performance classes as well as the slowdown configurable
    		// but I'll worry about that later.
    		per_class_slowdown = 0.03;

    }

    ~perf_assist_t () { }

    //Additional public function to pass around performance class
    int get_worst_perf_class() {
    		return m_worst_perf_class;
    }

    void set_worst_perf_class (int wpc) {
    		if (wpc > m_worst_perf_class)
    			m_worst_perf_class = wpc;
    }

    float get_duration_multiplier() {
    		return (1 + (m_worst_perf_class - 1) * per_class_slowdown);
    }


private:
     int m_worst_perf_class;
     float per_class_slowdown;

}; // the end of class perf_assist_t


// Extern declaration so dfu_power and dfu_traverse can access perf_obj.
// Object is created in dfu_traverse_impl.cpp
// TODO: Think of a better way to pass this info around.
extern perf_assist_t perf_obj;


#endif //PERF_ASSIST_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */