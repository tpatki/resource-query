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

#ifndef DFU_MATCH_POWER_HPP
#define DFU_MATCH_POWER_HPP

#include <iostream>
#include <vector>
#include <numeric>
#include <map>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include "dfu_match_cb.hpp"
#include "jobspec.hpp"

namespace Flux {
namespace resource_model {

/* Power match policy: select resources if enough power is available
 * for the job. 
 */
class power_t : public dfu_match_cb_t
{
public:
    power_t () { }
    power_t (const std::string &name) : dfu_match_cb_t (name) { }
    power_t (const power_t &o) : dfu_match_cb_t (o) { }
    power_t &operator= (const power_t &o)
    {
        dfu_match_cb_t::operator= (o);
        return *this;
    }
    ~power_t () { }

    int dom_finish_graph (const subsystem_t &subsystem,
                          const std::vector<Flux::Jobspec::Resource> &resources,
                          const f_resource_graph_t &g, scoring_api_t &dfu)
    {
        int64_t score = MATCH_MET;
        for (auto &resource : resources) {
            const std::string &type = resource.type;
            unsigned int qc = dfu.qualified_count (subsystem, type);
            unsigned int count = select_count (resource, qc);
            if (count == 0) {
                score = MATCH_UNMET;
                break;
            }
            dfu.choose_accum_best_k (subsystem, type, count);
        }
        dfu.set_overall_score (score);
        return (score == MATCH_MET)? 0 : -1;
    }

    int dom_finish_slot (const subsystem_t &subsystem, scoring_api_t &dfu)
    {
        std::vector<std::string> types;
        dfu.resrc_types (subsystem, types);
        for (auto &type : types)
            dfu.choose_accum_all (subsystem, type);
        return 0;
    }

    int dom_finish_vtx (vtx_t u, const subsystem_t &subsystem,
                        const std::vector<Flux::Jobspec::Resource> &resources,
                        const f_resource_graph_t &g, scoring_api_t &dfu)
    {
        int64_t score = MATCH_MET;
        int64_t overall;

        for (auto &resource : resources) {
            if (resource.type != g[u].type)
                continue;

            // jobspec resource type matches with the visiting vertex
            for (auto &c_resource : resource.with) {
                // test children resource count requirements
                const std::string &c_type = c_resource.type;
                unsigned int qc = dfu.qualified_count (subsystem, c_type);
                unsigned int count = select_count (c_resource, qc);
                if (count == 0) {
                    score = MATCH_UNMET;
                    break;
                }
                dfu.choose_accum_best_k (subsystem, c_resource.type, count);
            }
        }

        // high id first policy (just a demo policy)
        overall = (score == MATCH_MET)? (score + g[u].id + 1) : score;
        dfu.set_overall_score (overall);
        decr ();
        return (score == MATCH_MET)? 0 : -1;
    }
}; // the end of class power_t


} // namespace resource_model
} // namespace Flux

#endif // DFU_MATCH_POWER_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
