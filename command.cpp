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

#include <sys/time.h>
#include "command.hpp"

namespace Flux {
namespace resource_model {

using namespace std;
using namespace Flux::Jobspec;

struct command_t {
    string name;
    string abbr;
    cmd_func_f *cmd;
    string note;
};

command_t commands[] = {
    { "match",  "m", cmd_match, "Allocate or reserve matching resources (subcmd:"
"allocate | allocate_orelse_reserve): resource-query> match allocate jobspec"},
    { "cancel", "c", cmd_cancel, "Cancel an allocation or reservation: "
"resource-query> cancel jobid" },
    { "list", "l", cmd_list, "List all jobs: resource-query> list" },
    { "info", "i", cmd_info, "Print info on a jobid: resource-query> info jobid" },
    { "cat", "a", cmd_cat, "Print jobspec file: resource-query> cat jobspec" },
    { "help", "h", cmd_help, "Print help message: resource-query> help" },
    { "quit", "q", cmd_quit, "Quit the session: resource-query> quit" },
    { "NA", "NA", (cmd_func_f *)NULL, "NA" }
};

static double get_elapse_time (timeval &st, timeval &et)
{
    double ts1 = (double)st.tv_sec + (double)st.tv_usec/1000000.0f;
    double ts2 = (double)et.tv_sec + (double)et.tv_usec/1000000.0f;
    return ts2 - ts1;
}

static void print_schedule_info (resource_context_t *ctx, uint64_t jobid,
                                string &jobspec_fn, bool matched, int64_t at,
                                double elapse)
{
    if (matched) {
        string mode = (at == 0)? "ALLOCATED" : "RESERVED";
        string scheduled_at = (at == 0)? "Now" : to_string (at);
        cout << "INFO:" << " =============================" << endl;
        cout << "INFO:" << " JOBID=" << jobid << endl;
        cout << "INFO:" << " RESOURCES=" << mode << endl;
        cout << "INFO:" << " SCHEDULED AT=" << scheduled_at << endl;
        if (ctx->params.elapse_time)
            cout << "INFO:" << " ELAPSE=" << to_string (elapse) << endl;

        cout << "INFO:" << " =============================" << endl;
        ctx->jobs[jobid] = new job_info_t (jobid, (at != 0), at,
                                           jobspec_fn, elapse);
        if (at == 0)
            ctx->allocations[jobid] = jobid;
        else
            ctx->reservations[jobid] = jobid;
        ctx->jobid_counter++;
    } else {
        cout << "INFO:" << " =============================" << endl;
        cout << "INFO: " << "No matching resources found" << endl;
        cout << "INFO:" << " JOBID=" << jobid << endl;
        if (ctx->params.elapse_time)
            cout << "INFO:" << " ELAPSE=" << to_string (elapse) << endl;
        cout << "INFO:" << " =============================" << endl;
    }
}

int cmd_match (resource_context_t *ctx, vector<string> &args)
{
    if (args.size () != 3) {
        cerr << "ERROR: malformed command" << endl;
        return 0;
    }
    string subcmd = args[1];
    if (!(subcmd == "allocate" || subcmd == "allocate_orelse_reserve")) {
        cerr << "ERROR: unknown subcmd" << args[1] << endl;
        return 0;
    }

    try {
        int rc = 0;
        int64_t at = 0;
        uint64_t jobid = ctx->jobid_counter;
        string &jobspec_fn = args[2];
        ifstream jobspec_in;
        jobspec_in.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        jobspec_in.open (jobspec_fn);
        Flux::Jobspec::Jobspec job {jobspec_in};
        double elapse = 0.0f;
        struct timeval st, et;

        gettimeofday (&st, NULL);
        if (args[1] == "allocate")
            rc = ctx->traverser.run (job,
                                     match_op_t::MATCH_ALLOCATE, jobid, &at);
        else if (args[1] == "allocate_orelse_reserve")
            rc = ctx->traverser.run (job,
                                     match_op_t::MATCH_ALLOCATE_ORELSE_RESERVE,
                                     jobid, &at);
        gettimeofday (&et, NULL);
        elapse = get_elapse_time (st, et);
        print_schedule_info (ctx, jobid, jobspec_fn, (rc == 0), at, elapse);

    } catch (std::ifstream::failure &e) {
        cerr << "ERROR: Exception occurs for input file I/O" << e.what () << endl;
    }
    return 0;
}

int cmd_cancel (resource_context_t *ctx, vector<string> &args)
{
    cout << "ERROR: " << "Not yet implmented " << endl;
    return 0;
}

int cmd_list (resource_context_t *ctx, vector<string> &args)
{
    for (auto &kv: ctx->jobs) {
        job_info_t *info = kv.second;
        string mode = (!info->reserved)? "ALLOCATED" : "RESERVED";
        cout << "INFO: " << info->jobid << ", " << mode << ", "
             << info->scheduled_at << ", " << info->jobspec_fn << ", "
             << info->overhead << endl;
    }
    return 0;
}

int cmd_info (resource_context_t *ctx, vector<string> &args)
{
    if (args.size () != 2) {
        cerr << "ERROR: malformed command" << endl;
        return 0;
    }
    uint64_t jobid = (uint64_t)std::atoll(args[1].c_str ());
    if (ctx->jobs.find (jobid) == ctx->jobs.end ()) {
       cout << "ERROR: jobid doesn't exist: " << args[1] << endl;
       return 0;
    }
    job_info_t *info = ctx->jobs[jobid];
    string mode = (!info->reserved)? "ALLOCATED" : "RESERVED";
    cout << "INFO: " << info->jobid << ", " << mode << ", "
         << info->scheduled_at << ", " << info->jobspec_fn << ", "
         << info->overhead << endl;
    return 0;
}

int cmd_cat (resource_context_t *ctx, vector<string> &args)
{
    string &jspec_filename = args[1];
    ifstream jspec_in;
    jspec_in.open (jspec_filename);
    string line;
    while (getline (jspec_in, line))
        cout << line << endl;
    cout << "INFO: " << "Jobspec in " << jspec_filename << endl;
    return 0;
}

int cmd_help (resource_context_t *ctx, vector<string> &args)
{
    bool multi = true;
    bool found = false;
    string cmd = "unknown";

    if (args.size () == 2) {
        multi = false;
        cmd = args[1];
    }

    for (int i = 0; commands[i].name != "NA"; ++i) {
        if (multi || cmd == commands[i].name || cmd == commands[i].abbr) {
            cout << "INFO: " << commands[i].name << " (" << commands[i].abbr
                      << ")" << " -- " << commands[i].note << endl;
            found = true;
        }
    }
    if (!multi && !found)
        cout << "ERROR: unknown command: " << cmd << endl;

    return 0;
}

int cmd_quit (resource_context_t *ctx, vector<string> &args)
{
    cout << "INFO: " << "Bye bye " << endl;
    return -1;
}

cmd_func_f *find_cmd (const string &cmd_str)
{
    for (int i = 0; commands[i].name != "NA"; ++i) {
        if (cmd_str == commands[i].name)
            return commands[i].cmd;
        else if (cmd_str == commands[i].abbr)
            return commands[i].cmd;
    }
    return (cmd_func_f *)NULL;
}

} // namespace resource_model
} // namespace Flux

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */