#ifndef PARALUTION_OCL_KERNELS_CSR_HPP_
#define PARALUTION_OCL_KERNELS_CSR_HPP_

namespace paralution {

const char *ocl_kernels_csr =
	"// *************************************************************************\n"
	"//\n"
	"//    PARALUTION   www.paralution.com\n"
	"//\n"
	"//    Copyright (C) 2012-2013 Dimitar Lukarski\n"
	"//\n"
	"//    This program is free software: you can redistribute it and/or modify\n"
	"//    it under the terms of the GNU General Public License as published by\n"
	"//    the Free Software Foundation, either version 3 of the License, or\n"
	"//    (at your option) any later version.\n"
	"//\n"
	"//    This program is distributed in the hope that it will be useful,\n"
	"//    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"//    GNU General Public License for more details.\n"
	"//\n"
	"//    You should have received a copy of the GNU General Public License\n"
	"//    along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
	"//\n"
	"// *************************************************************************\n"
	"\n"
	"// Nathan Bell and Michael Garland\n"
	"// Efficient Sparse Matrix-Vector Multiplication on {CUDA}\n"
	"// NVR-2008-004 / NVIDIA Technical Report\n"
	"__kernel void kernel_csr_spmv_scalar(const int nrow, __global const int *row_offset, __global const int *col,\n"
	"                                     __global const ValueType *val, __global const ValueType *in,\n"
	"                                     __global ValueType *out) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    ValueType sum = 0;\n"
	"\n"
	"    for (int aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      sum += val[aj] * in[col[aj]];\n"
	"\n"
	"    out[ai] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"// Nathan Bell and Michael Garland\n"
	"// Efficient Sparse Matrix-Vector Multiplication on {CUDA}\n"
	"// NVR-2008-004 / NVIDIA Technical Report\n"
	"__kernel void kernel_csr_add_spmv_scalar(const int nrow, __global const int *row_offset,\n"
	"                                         __global const int *col, __global const ValueType *val,\n"
	"                                         const ValueType scalar, __global const ValueType *in,\n"
	"                                         __global ValueType *out) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    ValueType sum = out[ai];\n"
	"\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      sum += scalar * val[aj] * in[col[aj]];\n"
	"\n"
	"    out[ai] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_scale_diagonal(const int nrow, __global const int *row_offset, __global const int *col,\n"
	"                                        const ValueType alpha, __global ValueType *val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai == col[aj])\n"
	"        val[aj] = alpha * val[aj];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_scale_offdiagonal(const int nrow, __global const int *row_offset, __global const int *col,\n"
	"                                           const ValueType alpha, __global ValueType *val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai != col[aj])\n"
	"        val[aj] = alpha * val[aj];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_add_diagonal(const int nrow, __global const int *row_offset,\n"
	"                                      __global const int *col, const ValueType alpha, __global ValueType *val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai == col[aj])\n"
	"        val[aj] = val[aj] + alpha;\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_add_offdiagonal(const int nrow, __global const int *row_offset,\n"
	"                                         __global const int *col, const ValueType alpha, __global ValueType *val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai != col[aj])\n"
	"        val[aj] = val[aj] + alpha;\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_extract_diag(const int nrow, __global const int *row_offset, __global const int *col,\n"
	"                                      __global const ValueType *val, __global ValueType *vec) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai == col[aj])\n"
	"        vec[ai] = val[aj];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_extract_inv_diag(const int nrow, __global const int *row_offset,\n"
	"                                          __global const int *col, __global const ValueType *val,\n"
	"                                          __global ValueType *vec) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      if (ai == col[aj])\n"
	"        vec[ai] = 1.0 / val[aj];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_extract_submatrix_row_nnz(__global const int *row_offset, __global const int *col,\n"
	"                                                   __global const ValueType *val, const int smrow_offset,\n"
	"                                                   const int smcol_offset, const int smrow_size,\n"
	"                                                   const int smcol_size, __global int *row_nnz) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < smrow_size) {\n"
	"\n"
	"    int nnz = 0;\n"
	"    int ind = ai + smrow_offset;\n"
	"\n"
	"    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj)\n"
	"\n"
	"      if ( (col[aj] >= smcol_offset) &&\n"
	"           (col[aj] <  smcol_offset + smcol_size) )\n"
	"        ++nnz;\n"
	"\n"
	"    row_nnz[ai] = nnz;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_extract_submatrix_copy(__global const int *row_offset, __global const int *col,\n"
	"                                                __global const ValueType *val, const int smrow_offset,\n"
	"                                                const int smcol_offset, const int smrow_size,\n"
	"                                                const int smcol_size, __global const int *sm_row_offset,\n"
	"                                                __global int *sm_col, __global ValueType *sm_val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < smrow_size) {\n"
	"\n"
	"    int row_nnz = sm_row_offset[ai];\n"
	"    int ind = ai + smrow_offset;\n"
	"\n"
	"    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj) {\n"
	"\n"
	"      if ( (col[aj] >= smcol_offset) &&\n"
	"           (col[aj] <  smcol_offset + smcol_size) ) {\n"
	"\n"
	"        sm_col[row_nnz] = col[aj] - smcol_offset;\n"
	"        sm_val[row_nnz] = val[aj];\n"
	"        ++row_nnz;\n"
	"\n"
	"      }\n"
	"\n"
	"    }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_diagmatmult(const int nrow, __global const int *row_offset, __global const int *col,\n"
	"                                     __global const ValueType *diag, __global ValueType *val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow)\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      val[aj] *= diag[ col[aj] ];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_add_csr_same_struct(const int nrow, __global const int *out_row_offset,\n"
	"                                             __global const int *out_col, __global const int *in_row_offset,\n"
	"                                             __global const int *in_col, __global const ValueType *in_val,\n"
	"                                             const ValueType alpha, const ValueType beta, __global ValueType *out_val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj, ajj;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    int first_col = in_row_offset[ai];\n"
	"      \n"
	"    for (ajj=out_row_offset[ai]; ajj<out_row_offset[ai+1]; ++ajj)\n"
	"      for (aj=first_col; aj<in_row_offset[ai+1]; ++aj)\n"
	"        if (in_col[aj] == out_col[ajj]) {\n"
	"          \n"
	"          out_val[ajj] = alpha * out_val[ajj] + beta * in_val[aj];\n"
	"          ++first_col;\n"
	"          break;\n"
	"\n"
	"        }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_buffer_addscalar(const int size, const ValueType scalar, __global ValueType *buff) {\n"
	"\n"
	"  int gid = get_global_id(0);\n"
	"\n"
	"  if (gid < size)\n"
	"    buff[gid] += scalar;\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_reverse_index(const int size, __global const int *perm, __global int *out) {\n"
	"\n"
	"  int gid = get_global_id(0);\n"
	"\n"
	"  if (gid < size)\n"
	"    out[perm[gid]] = gid;\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_calc_row_nnz(const int nrow, __global const int *row_offset, __global int *row_nnz) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"\n"
	"  if (ai < nrow)\n"
	"    row_nnz[ai] = row_offset[ai+1]-row_offset[ai];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_permute_row_nnz(         const int  nrow,\n"
	"                                         __global const int *row_nnz_src,\n"
	"                                         __global const int *perm_vec,\n"
	"                                         __global       int *row_nnz_dst) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"\n"
	"  if (ai < nrow)\n"
	"    row_nnz_dst[perm_vec[ai]] = row_nnz_src[ai];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_permute_rows(       const       int  nrow,\n"
	"                                    __global const       int *row_offset,\n"
	"                                    __global const       int *perm_row_offset,\n"
	"                                    __global const       int *col,\n"
	"                                    __global const ValueType *data,\n"
	"                                    __global const       int *perm_vec,\n"
	"                                    __global const       int *row_nnz,\n"
	"                                    __global             int *perm_col,\n"
	"                                    __global       ValueType *perm_data) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    int num_elems = row_nnz[ai];\n"
	"    int perm_index = perm_row_offset[perm_vec[ai]];\n"
	"    int prev_index = row_offset[ai];\n"
	"\n"
	"    for (int i = 0; i < num_elems; ++i) {\n"
	"      perm_data[perm_index + i] = data[prev_index + i];\n"
	"      perm_col[perm_index + i]  = col[prev_index + i];\n"
	"    }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_csr_permute_cols(         const       int  nrow,\n"
	"                                      __global const       int *row_offset,\n"
	"                                      __global const       int *perm_vec,\n"
	"                                      __global const       int *row_nnz,\n"
	"                                      __global const       int *perm_col,\n"
	"                                      __global const ValueType *perm_data,\n"
	"                                      __global             int *col,\n"
	"                                      __global       ValueType *data) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int j;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    int num_elems = row_nnz[ai];\n"
	"    int elem_index = row_offset[ai];\n"
	"\n"
	"    for (int i = 0; i < num_elems; ++i) {\n"
	"\n"
	"      int comp = perm_vec[perm_col[elem_index+i]];\n"
	"\n"
	"      for (j = i-1; j >= 0 ; --j) {\n"
	"\n"
	"        if (col[elem_index+j]>comp) {\n"
	"          data[elem_index+j+1] = data[elem_index+j];\n"
	"          col[elem_index+j+1]  = col[elem_index+j];\n"
	"        } else\n"
	"          break;\n"
	"      }\n"
	"\n"
	"      data[elem_index+j+1] = perm_data[elem_index+i];\n"
	"      col[elem_index+j+1]  = comp;\n"
	"\n"
	"    }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n";
}

#endif // PARALUTION_OCL_KERNELS_CSR_HPP_