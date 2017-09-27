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

#include <cstdint>
#include <cstdlib>
#include <getopt.h>
#include <sys/time.h>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <readline/readline.h>
#include <readline/history.h>
#include <boost/algorithm/string.hpp>
#include "resource_graph.hpp"
#include "resource_gen.hpp"
#include "dfu_traverse.hpp"
#include "dfu_match_id_based.hpp"

using namespace std;
using namespace boost;
using namespace resource_model;

#define OPTIONS "G:S:P:g:o:h"
static const struct option longopts[] = {
    {"grug",             required_argument,  0, 'G'},
    {"match-subsystems", required_argument,  0, 'S'},
    {"match-policy",     required_argument,  0, 'P'},
    {"graph-format",     required_argument,  0, 'g'},
    {"output",           required_argument,  0, 'o'},
    {"help",             no_argument,        0, 'h'},
    { 0, 0, 0, 0 },
};

struct job_info_t {
    job_info_t (uint64_t j, bool r, int64_t at, string fn, double o)
        : jobid (j), reserved (r), scheduled_at (at), jobspec_fn (fn),
          overhead (o) { }
    uint64_t jobid = UINT64_MAX;
    bool reserved = false;
    int64_t scheduled_at = -1;
    string jobspec_fn;
    double overhead = 0.0f;
};

struct test_params_t {
    string grug;
    string matcher_name;
    string matcher_policy;
    string o_fname;
    string o_fext;
    emit_format_t o_format;
};

struct resource_context_t {
    test_params_t params;
    uint64_t jobid_counter;
    resource_graph_db_t db;
    multi_subsystems_t subsystems;
    map<string, f_resource_graph_t *> resource_graph_views;
    dfu_match_cb_t *matcher;
    dfu_traverser_t traverser;
    map<uint64_t, job_info_t *> jobs;
    map<uint64_t, uint64_t> allocations;
    map<uint64_t, uint64_t> reservations;
};

static int cmd_match (resource_context_t *ctx, vector<string> &args);
static int cmd_cancel (resource_context_t *ctx, vector<string> &args);
static int cmd_list (resource_context_t *ctx, vector<string> &args);
static int cmd_info (resource_context_t *ctx, vector<string> &args);
static int cmd_cat (resource_context_t *ctx, vector<string> &args);
static int cmd_quit (resource_context_t *ctx, vector<string> &args);
static int cmd_help (resource_context_t *ctx, vector<string> &args);

typedef int cmd_func_f (resource_context_t *, vector<string> &);

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
    { "info", "i", cmd_info, "Print the info on a jobid: resource-query> info jobid" },
    { "cat", "a", cmd_cat, "Print the jobspec file: resource-query> cat jobspec" },
    { "help", "h", cmd_help, "Print help message: resource-query> help" },
    { "quit", "q", cmd_quit, "Quit the session: resource-query> quit" },
    { "NA", "NA", (cmd_func_f *)NULL, "NA" }
};

static void usage (int code)
{
    cerr <<
"usage: resource-query [OPTIONS…]\n"
"\n"
"Command-line utility to select HPC resources in accordance with a\n"
"resource-matching policy, given a job specification written in\n"
"Flux's Canonical Job Specification (RFC 14).\n"
"\n"
"Read in a resource-graph generation recipe written in the GRUG format\n"
"and populate a resource-graph data store representing the compute and\n"
"other HPC resources and their relationships (RFC 4).\n"
"\n"
"Provide a simple command-line interface (cli) to allow users to allocate\n"
"or reserve the resource set in this resource-graph data store.\n"
"Traverse the resource graph in a predefined order for resource selection.\n"
"Currently only support one traversal type: depth-first traversal on the\n"
"dominant subsystem and up-walk traversal on one or more auxiliary subsystems.\n"
"\n"
"OPTIONS allow for using a predefined matcher that is configured\n"
"to use a different set of subsystems as its dominant and/or auxiliary\n"
"ones to perform matches on.\n"
"\n"
"OPTIONS allow for instantiating a different resource-matching\n"
"selection policy--e.g., select resources with high or low IDs first.\n"
"\n"
"OPTIONS also allow for exporting the filtered graph of the used matcher\n"
"in a selected graph format at the end of the cli session.\n"
"\n"
"To see cli commands, type in \"help\" in the cli: i.e., \n"
"  % resource-query> help"
"\n"
"\n"
"\n"
"OPTIONS:\n"
"    -h, --help\n"
"            Display the usage information\n"
"\n"
"    -G, --grug=<genspec>.graphml\n"
"            GRUG resource graph generator specification file in graphml\n"
"            (default=conf/default)\n"
"\n"
"    -S, --match-subsystems="
         "<CA|IBA|IBBA|PFS1BA|PA|C+IBA|C+PFS1BA|C+PA|IB+IBBA|"
              "C+P+IBA|VA|V+PFS1BA|ALL>\n"
"            Set the predefined matcher to use. Available matchers are:\n"
"                CA: Containment Aware\n"
"                IBA: InfiniBand connection-Aware\n"
"                IBBA: InfiniBand Bandwidth-Aware\n"
"                PFS1BA: Parallel File System 1 Bandwidth-aware\n"
"                PA: Power-Aware\n"
"                C+IBA: Containment and InfiniBand connection-Aware\n"
"                C+PFS1BA: Containment and PFS1 Bandwidth-Aware\n"
"                C+PA: Containment and Power-Aware\n"
"                IB+IBBA: InfiniBand connection and Bandwidth-Aware\n"
"                C+P+IBA: Containment, Power and InfiniBand connection-Aware\n"
"                VA: Virtual Hierarchy-Aware \n"
"                V+PFS1BA: Virtual Hierarchy and PFS1 Bandwidth-Aware \n"
"                ALL: Aware of everything.\n"
"            (default=CA).\n"
"\n"
"    -P, --match-policy=<low|high|locality>\n"
"            Set the resource match selection policy. Available policies are:\n"
"                high: Select resources with high ID first\n"
"                low: Select resources with low ID first\n"
"                locality: Select contiguous resources first in their ID space\n"
"            (default=high).\n"
"\n"
"    -g, --graph-format=<dot|graphml>\n"
"            Specify the graph format of the output file\n"
"            (default=dot)\n"
"\n"
"    -o, --output=<basename>\n"
"            Set the basename of the output file\n"
"            For AT&T Graphviz dot, <basename>.dot\n"
"            For GraphML, <basename>.graphml\n"
"\n";
    exit (code);
}

static double elapse_time (timeval &st, timeval &et)
{
    double ts1 = (double) st.tv_sec + (double) st.tv_usec/1000000.0f;
    double ts2 = (double) et.tv_sec + (double) et.tv_usec/1000000.0f;
    return ts2 - ts1;
}

static dfu_match_cb_t *create_match_cb (const string &policy)
{
    dfu_match_cb_t *matcher = NULL;
    if (policy == "high")
        matcher = (dfu_match_cb_t *)new high_first_t ();
    else if (policy == "low")
        matcher = (dfu_match_cb_t *)new low_first_t ();
    else if (policy == "locality")
        matcher = (dfu_match_cb_t *)new greater_interval_first_t ();
    return matcher;
}

static int cmd_match (resource_context_t *ctx, vector<string> &args)
{
    try {
        if (args.size () != 3) {
            cerr << "ERROR: malformed command" << endl;
            return 0;
        }
        string subcmd = args[1];
        if (!(subcmd == "allocate" || subcmd == "allocate_orelse_reserve")) {
            cerr << "ERROR: unknown subcmd" << args[1] << endl;
            return 0;
        }

        int rc = 0;
        int64_t at = 0;
        uint64_t jobid = ctx->jobid_counter;
        string &fn = args[2];
        ifstream jspec_in;
        jspec_in.open (fn);
        Jobspec job {jspec_in};
        double elapse = 0.0f;
        struct timeval st, et;

        gettimeofday (&st, NULL);

        if (args[1] == "allocate")
            rc = ctx->traverser.run (job, MATCH_ALLOCATE, jobid, &at);
        else if (args[1] == "allocate_orelse_reserve")
            rc = ctx->traverser.run (job, MATCH_ALLOCATE_ORELSE_RESERVE, jobid, &at);

        gettimeofday (&et, NULL);
        elapse = elapse_time (st, et);

        if (rc == 0) {
            string mode = (at == 0)? "ALLOCATED" : "RESERVED";
            cout << "INFO:" << " =============================" << endl;
            cout << "INFO:" << " JOBID=" << jobid << endl;
            cout << "INFO:" << " RESOURCES=" << mode << endl;
            cout << "INFO:" << " AT=" << at << endl;
            cout << "INFO:" << " SELECTED RESOURCE ABOVE" << endl;
            cout << "INFO:" << " ELAPSE=" << to_string (elapse) << endl;
            cout << "INFO:" << " =============================" << endl;
            ctx->jobs[jobid] = new job_info_t (jobid, (at != 0), at, fn, elapse);
            if (at == 0)
                ctx->allocations[jobid] = jobid;
            else
                ctx->reservations[jobid] = jobid;
            ctx->jobid_counter++;
        } else if (rc == -1) {
            cerr << "INFO:" << " =============================" << endl;
            cerr << "INFO: " << "No matching resources found" << endl;
            cerr << "INFO:" << " ELAPSE=" << to_string (elapse) << endl;
            cerr << "INFO:" << " =============================" << endl;
        }
    } catch (std::exception &e){
        cerr << "ERROR: " << e.what () << endl;
    }
    return 0;
}

static int cmd_cancel (resource_context_t *ctx, vector<string> &args)
{
    cout << "ERROR: " << "Not yet implmented " << endl;
    return 0;
}

static int cmd_list (resource_context_t *ctx, vector<string> &args)
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

static int cmd_info (resource_context_t *ctx, vector<string> &args)
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

static int cmd_cat (resource_context_t *ctx, vector<string> &args)
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

static int cmd_help (resource_context_t *ctx, vector<string> &args)
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

static int cmd_quit (resource_context_t *ctx, vector<string> &args)
{
    cout << "INFO: " << "Bye bye " << endl;
    return -1;
}

static void set_default_params (test_params_t &params)
{
    params.grug = "conf/default";
    params.matcher_name = "CA";
    params.matcher_policy = "high";
    params.o_fname = "";
    params.o_fext = "dot";
    params.o_format = GRAPHVIZ_DOT;
}

static int string_to_graph_format (string st, emit_format_t format)
{
    int rc = 0;
    if (iequals (st, string ("dot")))
        format = GRAPHVIZ_DOT;
    else if (iequals (st, string ("graphml")))
        format = GRAPH_ML;
    else
        rc = -1;

    return rc;
}

static int graph_format_to_ext (emit_format_t format, string &st)
{
    int rc = 0;
    switch (format) {
    case GRAPHVIZ_DOT:
        st = "dot";
        break;
    case GRAPH_ML:
        st = "graphml";
        break;
    default:
        rc = -1;
    }

    return rc;
}

static int subsystem_exist (resource_context_t *ctx, string n)
{
    int rc = 0;
    if (ctx->db.roots.find (n) == ctx->db.roots.end ())
        rc = -1;
    return rc;
}

static int set_subsystems_use (resource_context_t *ctx, string n)
{
    int rc = 0;
    ctx->matcher->set_matcher_name (n);
    dfu_match_cb_t &matcher = *(ctx->matcher);
    const string &matcher_type = matcher.matcher_name ();

    if (iequals (matcher_type, string ("CA"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "*");
    }
    else if (iequals (matcher_type, string ("IBA"))) {
        if ( (rc = subsystem_exist (ctx, "ibnet")) == 0)
            matcher.add_subsystem ("ibnet", "*");
    }
    else if (iequals (matcher_type, string ("IBBA"))) {
        if ( (rc = subsystem_exist (ctx, "ibnetbw")) == 0)
            matcher.add_subsystem ("ibnetbw", "*");
    }
    else if (iequals (matcher_type, string ("PFS1BA"))) {
        if ( (rc = subsystem_exist (ctx, "pfs1bw")) == 0)
            matcher.add_subsystem ("pfs1bw", "*");
    }
    else if (iequals (matcher_type, string ("PA"))) {
        if ( (rc = subsystem_exist (ctx, "power")) == 0)
            matcher.add_subsystem ("power", "*");
    }
    else if (iequals (matcher_type, string ("C+PFS1BA"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "contains");
        if ( !rc && (rc = subsystem_exist (ctx, "pfs1bw")) == 0)
            matcher.add_subsystem ("pfs1bw", "*");
    }
    else if (iequals (matcher_type, string ("C+IBA"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "contains");
        if ( !rc && (rc = subsystem_exist (ctx, "ibnet")) == 0)
            matcher.add_subsystem ("ibnet", "connected_up");
    }
    else if (iequals (matcher_type, string ("C+PA"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "*");
        if ( !rc && (rc = subsystem_exist (ctx, "power")) == 0)
            matcher.add_subsystem ("power", "*");
    }
    else if (iequals (matcher_type, string ("IB+IBBA"))) {
        if ( (rc = subsystem_exist (ctx, "ibnet")) == 0)
            matcher.add_subsystem ("ibnet", "connected_down");
        if ( !rc && (rc = subsystem_exist (ctx, "ibnetbw")) == 0)
            matcher.add_subsystem ("ibnetbw", "*");
    }
    else if (iequals (matcher_type, string ("C+P+IBA"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "contains");
        if ( (rc = subsystem_exist (ctx, "power")) == 0)
            matcher.add_subsystem ("power", "drawn");
        if ( !rc && (rc = subsystem_exist (ctx, "ibnet")) == 0)
            matcher.add_subsystem ("ibnet", "connected_up");
    } else if (iequals (matcher_type, string ("V+PFS1BA"))) {
        if ( (rc = subsystem_exist (ctx, "virtual1")) == 0)
            matcher.add_subsystem ("virtual1", "*");
        if ( !rc && (rc = subsystem_exist (ctx, "pfs1bw")) == 0)
            matcher.add_subsystem ("pfs1bw", "*");
    } else if (iequals (matcher_type, string ("VA"))) {
        if ( (rc = subsystem_exist (ctx, "virtual1")) == 0)
            matcher.add_subsystem ("virtual1", "*");
    } else if (iequals (matcher_type, string ("ALL"))) {
        if ( (rc = subsystem_exist (ctx, "containment")) == 0)
            matcher.add_subsystem ("containment", "*");
        if ( !rc && (rc = subsystem_exist (ctx, "ibnet")) == 0)
            matcher.add_subsystem ("ibnet", "*");
        if ( !rc && (rc = subsystem_exist (ctx, "ibnetbw")) == 0)
            matcher.add_subsystem ("ibnetbw", "*");
        if ( !rc && (rc = subsystem_exist (ctx, "pfs1bw")) == 0)
            matcher.add_subsystem ("pfs1bw", "*");
        if ( (rc = subsystem_exist (ctx, "power")) == 0)
            matcher.add_subsystem ("power", "*");
    } else
        rc = -1;

    return rc;
}

static void write_to_graphviz (f_resource_graph_t &fg, subsystem_t ss, fstream &o)
{
    f_res_name_map_t vmap = get (&resource_pool_t::name, fg);
    f_edg_infra_map_t emap = get (&resource_relation_t::idata, fg);
    label_writer_t<f_res_name_map_t, vtx_t> vwr (vmap);
    edg_label_writer_t ewr (emap, ss);
    write_graphviz (o, fg, vwr, ewr);
}

static void write_to_graphml (f_resource_graph_t &fg, fstream &o)
{
    dynamic_properties dp;
    dp.property ("name", get (&resource_pool_t::name, fg));
    dp.property ("name", get (&resource_relation_t::name, fg));
    write_graphml(o, fg, dp, true);
}

static void write_to_graph (resource_context_t *ctx)
{
    fstream o;
    string fn, mn;
    mn = ctx->matcher->matcher_name ();
    f_resource_graph_t &fg = *(ctx->resource_graph_views[mn]);
    fn = ctx->params.o_fname + "." + ctx->params.o_fext;
    o.open (fn, fstream::out);
    cout << "INFO: Write the target graph of the matcher..." << endl;
    switch (ctx->params.o_format) {
    case GRAPHVIZ_DOT:
        write_to_graphviz (fg, ctx->matcher->dom_subsystem (), o);
        break;
    case GRAPH_ML:
        write_to_graphml (fg, o);
        break;
    default:
        cout << "ERROR: Graph format is not yet implemented:" << endl;
        break;
    }
    o.close ();
}

static cmd_func_f *find_cmd (const string &cmd_str)
{
    for (int i = 0; commands[i].name != "NA"; ++i) {
        if (cmd_str == commands[i].name)
            return commands[i].cmd;
        else if (cmd_str == commands[i].abbr)
            return commands[i].cmd;
    }
    return (cmd_func_f *)NULL;
}

void control_loop (resource_context_t *ctx)
{
    cmd_func_f *cmd = NULL;
    while(1) {
        char *line = readline("resource-query> ");
        if (line == NULL)
            continue;
        else if(*line)
            add_history(line);

        vector<string> tokens;
        istringstream iss (line);
        copy(istream_iterator<string>(iss), istream_iterator<string>(),
             back_inserter (tokens));
        free(line);
        if (tokens.empty ())
            continue;

        string &cmd_str = tokens[0];
        cmd = find_cmd (cmd_str);
        if (!cmd)
            continue;
        if (cmd (ctx, tokens) != 0)
            break;
    }

    return;
}

int main (int argc, char *argv[])
{
    int rc;
    int ch;
    resource_context_t *ctx = new resource_context_t ();
    set_default_params (ctx->params);

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'h': /* --help */
                usage (0);
                break;
            case 'G': /* --grug*/
                ctx->params.grug = optarg;
                rc = 0;
                break;
            case 'S': /* --match-subsystems */
                ctx->params.matcher_name = optarg;
                break;
            case 'P': /* --match-policy */
                ctx->params.matcher_policy = optarg;
                break;
            case 'g': /* --graph-format */
                rc = string_to_graph_format (optarg, ctx->params.o_format);
                if ( rc != 0) {
                    cerr << "[ERROR] unknown output format for --graph-format: ";
                    cerr << optarg << endl;
                    usage (1);
                }
                graph_format_to_ext (ctx->params.o_format, ctx->params.o_fext);
                break;
            case 'o': /* --output */
                ctx->params.o_fname = optarg;
                break;
            default:
                usage (1);
                break;
        }
    }

    if (optind != argc)
        usage (1);

    // Create matcher and traverser objects
    ctx->matcher = create_match_cb (ctx->params.matcher_policy);
    if (ctx->matcher == NULL) {
        cerr << "ERROR: unknown match policy " << endl;
        cerr << "ERROR: " << ctx->params.matcher_policy << endl;
        return EXIT_FAILURE;
    }

    // Generate a resource graph data store
    resource_generator_t rgen;
    if ( (rc = rgen.read_graphml (ctx->params.grug, ctx->db)) != 0) {
        cerr << "ERROR: error in generating resources" << endl;
        cerr << "ERROR: " << rgen.err_message () << endl;
        return EXIT_FAILURE;
    }
    resource_graph_t &g = ctx->db.resource_graph;

    // Configure the matcher and its subsystem selector
    cout << "INFO: Loading a matcher: " << ctx->params.matcher_name << endl;
    if (set_subsystems_use (ctx, ctx->params.matcher_name) != 0) {
        cerr << "ERROR: Not all subsystems found" << endl;
        return EXIT_FAILURE;
    }
    vtx_infra_map_t vmap = get (&resource_pool_t::idata, g);
    edg_infra_map_t emap = get (&resource_relation_t::idata, g);
    const multi_subsystemsS &filter = ctx->matcher->subsystemsS ();

    subsystem_selector_t<vtx_t, f_vtx_infra_map_t> vtxsel (vmap, filter);
    subsystem_selector_t<edg_t, f_edg_infra_map_t> edgsel (emap, filter);

    f_resource_graph_t *fg = new f_resource_graph_t (g, edgsel, vtxsel);
    ctx->resource_graph_views[ctx->params.matcher_name] = fg;
    ctx->jobid_counter = 1;

    const string &dom = ctx->matcher->dom_subsystem ();
    auto &aggr_update_types = ctx->matcher->sdau_resource_types[dom];

    // scheduler-driven aggregate-updates optimization is configured with
    // the following resource types.
    aggr_update_types.insert ("node");
    aggr_update_types.insert ("core");
    aggr_update_types.insert ("gpu");
    aggr_update_types.insert ("memory");

    ctx->traverser.initialize (fg, &(ctx->db.roots), ctx->matcher);

    control_loop (ctx);

    // Output the filtered resource graph
    if (ctx->params.o_fname != "")
        write_to_graph (ctx);

    return EXIT_SUCCESS;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
