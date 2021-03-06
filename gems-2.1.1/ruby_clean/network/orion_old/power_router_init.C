/*
    Copyright (C) 1999-2008 by Mark D. Hill and David A. Wood for the
    Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
    http://www.cs.wisc.edu/gems/

    --------------------------------------------------------------------

    This file is part of Orion (Princeton's interconnect power model),
    a component of the Multifacet GEMS (General Execution-driven 
    Multiprocessor Simulator) software toolset originally developed at 
    the University of Wisconsin-Madison.

    Garnet was developed by Niket Agarwal at Princeton University. Orion was
    developed by Princeton University.

    Substantial further development of Multifacet GEMS at the
    University of Wisconsin was performed by Alaa Alameldeen, Brad
    Beckmann, Jayaram Bobba, Ross Dickson, Dan Gibson, Pacia Harper,
    Derek Hower, Milo Martin, Michael Marty, Carl Mauer, Michelle Moravan,
    Kevin Moore, Andrew Phelps, Manoj Plakal, Daniel Sorin, Haris Volos, 
    Min Xu, and Luke Yen.
    --------------------------------------------------------------------

    If your use of this software contributes to a published paper, we
    request that you (1) cite our summary paper that appears on our
    website (http://www.cs.wisc.edu/gems/) and (2) e-mail a citation
    for your published paper to gems@cs.wisc.edu.

    If you redistribute derivatives of this software, we request that
    you notify us and either (1) ask people to register with us at our
    website (http://www.cs.wisc.edu/gems/) or (2) collect registration
    information and periodically send it to us.

    --------------------------------------------------------------------

    Multifacet GEMS is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    Multifacet GEMS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Multifacet GEMS; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307, USA

    The GNU General Public License is contained in the file LICENSE.

### END HEADER ###
*/
#include <stdio.h>

#include "power_router_init.h"
#include "power_array.h"
#include "power_arbiter.h"
#include "power_crossbar.h"
#include "power_ll.h"
#include "parm_technology.h"
#include "SIM_port.h"
#include "power_static.h"
#include "power_utils.h"

/* -------------------------------------------------------------------------------------------- */
// Set buffer parameters
int buf_set_para(power_array_info *info, int is_fifo, unsigned n_read_port, unsigned n_write_port, unsigned n_entry, unsigned line_width, int outdrv)
{
	//general parameters
	info->share_rw = 0;
	info->read_ports = n_read_port;
	info->write_ports = n_write_port;
	info->n_set = n_entry;
	info->blk_bits = line_width;
	info->assoc = 1;
	info->data_width = line_width;
	info->data_end = PARM_data_end;

	//no array subpartition
	info->data_ndwl = 1;
	info->data_ndbl = 1;
	info->data_nspd = 1;

	info->data_n_share_amp =1;

	//MODEL parameters
	if(is_fifo) {
		info->row_dec_model = SIM_NO_MODEL;
		info->row_dec_pre_model = SIM_NO_MODEL;
	}
	else {
		info->row_dec_model = PARM_row_dec_model;
		info->row_dec_pre_model = PARM_row_dec_pre_model;
	}

	info->data_wordline_model = PARM_wordline_model;
	info->data_bitline_model = PARM_bitline_model;
	info->data_bitline_pre_model = PARM_bitline_pre_model;
	info->data_mem_model = PARM_mem_model;
	
	if(PARM_data_end == 2)
		info->data_amp_model = PARM_amp_model;
	else
		info->data_amp_model = SIM_NO_MODEL;
	if(outdrv)
		info->outdrv_model = PARM_outdrv_model;
	else
		info->outdrv_model = SIM_NO_MODEL;
	
	info->data_colsel_pre_model = SIM_NO_MODEL;
	info->col_dec_model = SIM_NO_MODEL;
	info->col_dec_pre_model = SIM_NO_MODEL;
	info->mux_model = SIM_NO_MODEL;
	
	//no tag array
	
	info->tag_wordline_model = SIM_NO_MODEL;
	info->tag_bitline_model = SIM_NO_MODEL;
	info->tag_bitline_pre_model = SIM_NO_MODEL;
	info->tag_mem_model = SIM_NO_MODEL;
	info->tag_attach_mem_model = SIM_NO_MODEL;
	info->tag_amp_model = SIM_NO_MODEL;
	info->tag_colsel_pre_model = SIM_NO_MODEL;
	info->comp_model = SIM_NO_MODEL;
	info->comp_pre_model = SIM_NO_MODEL;
	
	info->write_policy = 0; //no dirty bit

	//derived
	if(info->data_width != 0){
    info->n_item = info->blk_bits / info->data_width;
  }
  else{
    info->eff_data_cols = info->blk_bits * info->assoc * info->data_nspd;
  }

	return 0;
}	
/* -------------------------------------------------------------------------------------------- */


/* --------------- Router init --------------------------------------*/


int power_router_init(power_router *router, power_router_info *info)
{   
	int outdrv;
	double req_len = 0;
	
	//general
//	info->n_in = PARM_in_port;
	info->n_total_in = info->n_in;
//	info->n_out = PARM_out_port;
	info->n_total_out = info->n_out;
//	info->flit_width = PARM_flit_width;

	//vc 
//	info->n_v_channel = MAX(PARM_v_channel, 1);
//	info->n_v_class = MAX(info->n_v_channel, PARM_v_class);

	if(info->n_v_class > 1) {
		info->in_share_buf = PARM_in_share_buf;
		info->out_share_buf = PARM_out_share_buf;
		info->in_share_switch = PARM_in_share_switch;
		info->out_share_switch = PARM_out_share_switch;
	}	
	else {
		info->in_share_buf = 0;
		info->out_share_buf = 0;
		info->in_share_switch = 0;
		info->out_share_switch = 0;
	}

	//xbar
	info->crossbar_model = PARM_crossbar_model;
	info->degree = PARM_crsbar_degree;
	info->connect_type = PARM_connect_type;
	info->trans_type = PARM_trans_type;
	info->crossbar_in_len = PARM_crossbar_in_len;
	info->crossbar_out_len = PARM_crossbar_out_len;

	//input buffer
	info->in_buf = PARM_in_buf;
	outdrv = !info->in_share_buf && info->in_share_switch;
	buf_set_para(&info->in_buf_info, 0, PARM_in_buf_rport, 1, PARM_in_buf_set, info->flit_width, outdrv);

	//vc arbiter
	if(info->n_v_class > 1) {
		info->vc_in_arb_model = PARM_vc_in_arb_model;
		info->vc_out_arb_model = PARM_vc_out_arb_model;
		if(info->vc_in_arb_model == QUEUE_ARBITER) {
			buf_set_para(&info->vc_in_arb_queue_info, 1, 1, 1, info->n_v_class, SIM_power_logtwo(info->n_v_class), 0);
			info->vc_in_arb_ff_model = SIM_NO_MODEL;
		}
		else
			info->vc_in_arb_ff_model = PARM_vc_in_arb_ff_model;	
	
		if(info->vc_out_arb_model == QUEUE_ARBITER) {
			buf_set_para(&info->vc_out_arb_queue_info, 1, 1, 1, info->n_total_in - 1, SIM_power_logtwo(info->n_total_in - 1), 0);
			info->vc_out_arb_ff_model = SIM_NO_MODEL;
		}
		else
			info->vc_out_arb_ff_model = PARM_vc_out_arb_ff_model;
	}
	else {
		info->vc_in_arb_model = SIM_NO_MODEL;
		info->vc_in_arb_ff_model = SIM_NO_MODEL;
		info->vc_out_arb_model = SIM_NO_MODEL;
		info->vc_out_arb_ff_model = SIM_NO_MODEL;
	}

	//switch arbiter
	if (info->n_in > 2) {
		info->sw_in_arb_model = PARM_sw_in_arb_model;
		info->sw_out_arb_model = PARM_sw_out_arb_model;
		if(info->sw_in_arb_model == QUEUE_ARBITER) {
			buf_set_para(&info->sw_in_arb_queue_info, 1, 1, 1, info->n_v_class, SIM_power_logtwo(info->n_v_class), 0);
			info->sw_in_arb_ff_model = SIM_NO_MODEL;
		}
		else
			info->sw_in_arb_ff_model = PARM_sw_in_arb_ff_model;	
		
		if(info->sw_out_arb_model == QUEUE_ARBITER) {
			buf_set_para(&info->sw_out_arb_queue_info, 1, 1, 1, info->n_total_in - 1, SIM_power_logtwo(info->n_total_in - 1), 0);
			info->sw_out_arb_ff_model = SIM_NO_MODEL;
		}
		else
			info->sw_out_arb_ff_model = PARM_sw_out_arb_ff_model;
	}
	else {
		info->sw_in_arb_model = SIM_NO_MODEL;
		info->sw_in_arb_ff_model = SIM_NO_MODEL;
		info->sw_out_arb_model = SIM_NO_MODEL;
		info->sw_out_arb_ff_model = SIM_NO_MODEL;
	}	
		
	if(info->in_buf) {
		if(info->in_share_buf)
			info->in_n_switch = info->in_buf_info.read_ports;
			else if(info->in_share_switch)
				info->in_n_switch = 1;
			else
				info->in_n_switch = info->n_v_class;
	}
	else
		info->in_n_switch = 1;	
			
	info->n_switch_in = info->n_in * info->in_n_switch;

	info->n_switch_out = info->n_out;

    //-------- call initialize functions -----------
	
	router->i_leakage = 0;

	//initialize crossbar
	power_crossbar_init(&router->crossbar, info->crossbar_model, info->n_switch_in, info->n_switch_out, info->flit_width, info->degree, info->connect_type, info->trans_type, info->crossbar_in_len, info->crossbar_out_len, &req_len);
	router->i_leakage += router->crossbar.i_leakage;
//      printf("xbar_leak %g", router->crossbar.i_leakage);

	//initialize input buffer
	if(info->in_buf) {
		power_array_init(&info->in_buf_info, &router->in_buf);
		router->i_leakage += router->in_buf.i_leakage * info->n_in;
//              printf("buffer_leak %g", router->in_buf.i_leakage);
	}
//    printf("initialize in buffer over\n");	
	
	//initialize vc arbiter
	if(info->vc_in_arb_model) 
		power_arbiter_init(&router->vc_in_arb, info->vc_in_arb_model, info->vc_in_arb_ff_model, PARM_VC_per_MC, 0, &info->vc_in_arb_queue_info);

	if(info->vc_out_arb_model) 
		power_arbiter_init(&router->vc_out_arb, info->vc_out_arb_model, info->vc_out_arb_ff_model, info->n_total_in - 1, req_len, &info->vc_out_arb_queue_info);
	
	//initialize switch arbiter
	if(info->sw_in_arb_model) 
		power_arbiter_init(&router->sw_in_arb, info->sw_in_arb_model, info->sw_in_arb_ff_model, info->n_v_class, 0, &info->sw_in_arb_queue_info);

	if(info->sw_out_arb_model) 
		power_arbiter_init(&router->sw_out_arb, info->sw_out_arb_model, info->sw_out_arb_ff_model, info->n_total_in - 1, req_len, &info->sw_out_arb_queue_info);
	
	return 0;
}
