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

#ifndef NODE_DIST_HPP
#define NODE_DIST_HPP

#include <unordered_map>

namespace Flux {
namespace resource_model {

class node_dist_t {
public:
    node_dist_t () {};
   ~node_dist_t () {};

   //Need to handle errors....
    int set_dist (std::string f) {
       	std::ifstream node_config_f;
       	node_config_f.open(f);
       	std::string parse_str;
       	std::string first, second;
       	while (getline(node_config_f, parse_str))
       	{
       	    std::stringstream x(parse_str);
       	    getline(x, first, ',');
       	    getline(x, second);
//       	    std::cout << "first: " << first << ", second:" << second << std::endl;
       	 std::cout << "first: " << std::stoi(first) << ", second:" << std::stoi(second)<< std::endl;
       	    m_dist[std::stoi(first)] = std::stoi(second);
       	    std::cout << "SD Address: " << this << std::endl;
       	}
       	//Figure out error codes here
       	return 0;
       }

   int get_perf_class(int node_id) {
	    std::cout << " GPC Address: " << this << std::endl;
	   std::cout << "In perf class with input: " << node_id << std::endl;
	   std::unordered_map<int, int>::const_iterator val = m_dist.find(node_id);
	   if (val == m_dist.end())
		   return -1;
	   else
		   return val->second;
   }

private:
    std::unordered_map<int, int> m_dist;
};

} // namespace resource_model
} // namespace Flux

#endif // NODE_DIST_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
