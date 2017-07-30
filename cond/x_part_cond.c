/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPIPM is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPIPM is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPIPM; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *                          
*                                                                                                 *
**************************************************************************************************/

void COMPUTE_QP_SIZE_OCP2OCP(int N, int *nx, int *nu, int *nb, int **idxb, int *ng, int N2, int *nx2, int *nu2, int *nb2, int *ng2)
	{

	int ii, jj, kk;

	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	int nbb; // box constr that remain box constr
	int nbg; // box constr that becomes general constr
	for(ii=0; ii<N2; ii++)
		{
		T1 = ii<R1 ? M1 : N1;
		nx2[ii] = nx[N_tmp+0];
		nu2[ii] = nu[N_tmp+0];
		nb2[ii] = nb[N_tmp+0];
		ng2[ii] = ng[N_tmp+0];
		for(jj=1; jj<T1; jj++)
			{
			nbb = 0;
			nbg = 0;
			for(kk=0; kk<nb[N_tmp+jj]; kk++)
				if(idxb[N_tmp+jj][kk]<nu[N_tmp+jj])
					nbb++;
				else
					nbg++;
			nx2[ii] += 0;
			nu2[ii] += nu[N_tmp+jj];
			nb2[ii] += nbb;
			ng2[ii] += ng[N_tmp+jj] + nbg;
			}
		N_tmp += T1;
		}
	nx2[N2] = nx[N];
	nu2[N2] = nu[N];
	nb2[N2] = nb[N];
	ng2[N2] = ng[N];

	return;

	}



int MEMSIZE_COND_QP_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp)
	{

	struct OCP_QP tmp_ocp_qp;
	struct DENSE_QP tmp_dense_qp;

	int ii;

	int N = ocp_qp->N;
	int N2 = part_dense_qp->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int size = 0;

	size += N2*sizeof(struct COND_QP_OCP2DENSE_WORKSPACE);

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_qp
		tmp_ocp_qp.N = T1;
		tmp_ocp_qp.nx = ocp_qp->nx+N_tmp;
		tmp_ocp_qp.nu = ocp_qp->nu+N_tmp;
		tmp_ocp_qp.nb = ocp_qp->nb+N_tmp;
		tmp_ocp_qp.ng = ocp_qp->ng+N_tmp;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;

		size += MEMSIZE_COND_QP_OCP2DENSE(&tmp_ocp_qp, &tmp_dense_qp); // TODO floag to avoid to condense the last stage !!!!!

		N_tmp += T1;

		}

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void CREATE_COND_QP_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp, struct COND_QP_OCP2OCP_WORKSPACE *cond_ws, void *mem)
	{

	struct OCP_QP tmp_ocp_qp;
	struct DENSE_QP tmp_dense_qp;

	int ii;

	int N = ocp_qp->N;
	int N2 = part_dense_qp->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	// cond workspace struct
	struct COND_QP_OCP2DENSE_WORKSPACE *cws_ptr = mem;
	cond_ws->cond_workspace = cws_ptr;
	cws_ptr += N2;

	// align to typicl cache line size
	size_t s_ptr = (size_t) cws_ptr;
	s_ptr = (s_ptr+63)/64*64;

	char *c_ptr = (char *) s_ptr;

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_qp
		tmp_ocp_qp.N = T1;
		tmp_ocp_qp.nx = ocp_qp->nx+N_tmp;
		tmp_ocp_qp.nu = ocp_qp->nu+N_tmp;
		tmp_ocp_qp.nb = ocp_qp->nb+N_tmp;
		tmp_ocp_qp.ng = ocp_qp->ng+N_tmp;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;

		CREATE_COND_QP_OCP2DENSE(&tmp_ocp_qp, &tmp_dense_qp, cond_ws->cond_workspace+ii, c_ptr);
		c_ptr += (cond_ws->cond_workspace+ii)->memsize;

		N_tmp += T1;

		}

	cond_ws->memsize = MEMSIZE_COND_QP_OCP2OCP(ocp_qp, part_dense_qp);

	return;

	}

	

void COND_QP_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp, struct COND_QP_OCP2OCP_WORKSPACE *part_cond_ws)
	{

	struct OCP_QP tmp_ocp_qp;

	int ii;

	int N = ocp_qp->N;
	int N2 = part_dense_qp->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_qp
		tmp_ocp_qp.N = T1;
		tmp_ocp_qp.nx = ocp_qp->nx+N_tmp;
		tmp_ocp_qp.nu = ocp_qp->nu+N_tmp;
		tmp_ocp_qp.nb = ocp_qp->nb+N_tmp;
		tmp_ocp_qp.ng = ocp_qp->ng+N_tmp;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;
		tmp_ocp_qp.BAbt = ocp_qp->BAbt+N_tmp;
		tmp_ocp_qp.b = ocp_qp->b+N_tmp;
		tmp_ocp_qp.RSQrq = ocp_qp->RSQrq+N_tmp;
		tmp_ocp_qp.rq = ocp_qp->rq+N_tmp;
		tmp_ocp_qp.DCt = ocp_qp->DCt+N_tmp;
		tmp_ocp_qp.d_lb = ocp_qp->d_lb+N_tmp;
		tmp_ocp_qp.d_ub = ocp_qp->d_ub+N_tmp;
		tmp_ocp_qp.d_lg = ocp_qp->d_lg+N_tmp;
		tmp_ocp_qp.d_ug = ocp_qp->d_ug+N_tmp;

		tmp_ocp_qp.N = T1;
		COND_BABT(&tmp_ocp_qp, part_dense_qp->BAbt+ii, part_dense_qp->b+ii, part_cond_ws->cond_workspace+ii);

		tmp_ocp_qp.N = T1-1; // TODO add flag to avoid condensing last stage
		COND_RSQRQ_N2NX3(&tmp_ocp_qp, part_dense_qp->RSQrq+ii, part_dense_qp->rq+ii, part_cond_ws->cond_workspace+ii);

		tmp_ocp_qp.N = T1-1; // TODO add flag to avoid condensing last stage
		COND_DCTD(&tmp_ocp_qp, part_dense_qp->idxb[ii], part_dense_qp->d_lb+ii, part_dense_qp->d_ub+ii, part_dense_qp->DCt+ii, part_dense_qp->d_lg+ii, part_dense_qp->d_ug+ii, part_cond_ws->cond_workspace+ii);

		N_tmp += T1;

		}

	// copy last stage
	int *nx = ocp_qp->nx;
	int *nu = ocp_qp->nu;
	int *nb = ocp_qp->nb;
	int *ng = ocp_qp->ng;

	GECP_LIBSTR(nu[N]+nx[N]+1, nu[N]+nx[N], ocp_qp->RSQrq+N, 0, 0, part_dense_qp->RSQrq+N2, 0, 0);
	VECCP_LIBSTR(nu[N]+nx[N], ocp_qp->rq+N, 0, part_dense_qp->rq+N2, 0);
	VECCP_LIBSTR(nb[N], ocp_qp->d_lb+N, 0, part_dense_qp->d_lb+N2, 0);
	VECCP_LIBSTR(nb[N], ocp_qp->d_ub+N, 0, part_dense_qp->d_ub+N2, 0);
	GECP_LIBSTR(nu[N]+nx[N], ng[N], ocp_qp->DCt+N, 0, 0, part_dense_qp->DCt+N2, 0, 0);
	VECCP_LIBSTR(ng[N], ocp_qp->d_lg+N, 0, part_dense_qp->d_lg+N2, 0);
	VECCP_LIBSTR(ng[N], ocp_qp->d_ug+N, 0, part_dense_qp->d_ug+N2, 0);

	return;

	}




