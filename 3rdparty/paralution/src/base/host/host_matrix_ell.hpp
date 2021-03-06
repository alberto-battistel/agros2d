// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2013 Dimitar Lukarski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#ifndef PARALUTION_HOST_MATRIX_ELL_HPP_
#define PARALUTION_HOST_MATRIX_ELL_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "../matrix_formats.hpp"

namespace paralution {

template <typename ValueType>
class HostMatrixELL : public HostMatrix<ValueType> {
  
public:

  HostMatrixELL();
  HostMatrixELL(const Paralution_Backend_Descriptor local_backend);
  virtual ~HostMatrixELL();

  inline int get_max_row(void) const { return mat_.max_row; }

  virtual void info(void) const;
  virtual unsigned int get_mat_format(void) const { return  ELL; }

  virtual void Clear(void);
  virtual void AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row);

  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat);

  virtual void CopyFrom(const BaseMatrix<ValueType> &mat);
  virtual void CopyTo(BaseMatrix<ValueType> *mat) const;

  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar, 
                        BaseVector<ValueType> *out) const; 
  
private:
  
  MatrixELL<ValueType, int> mat_;

  friend class BaseVector<ValueType>;  
  friend class HostVector<ValueType>;  
  friend class HostMatrixCSR<ValueType>;
  friend class HostMatrixCOO<ValueType>;  
  friend class HostMatrixHYB<ValueType>;  
  friend class HostMatrixDENSE<ValueType>;

  friend class GPUAcceleratorMatrixELL<ValueType>;
  friend class OCLAcceleratorMatrixELL<ValueType>;
  friend class MICAcceleratorMatrixELL<ValueType>;

};


}

#endif // PARALUTION_HOST_MATRIX_ELL_HPP_
