//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.90. August 3, 2009.
//LIC// 
//LIC// Copyright (C) 2006-2009 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
#ifndef OOMPH_DOUBLE_VECTOR_CLASS_HEADER
#define OOMPH_DOUBLE_VECTOR_CLASS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

// c++ headers
#include <algorithm>

// oomph headers
#include "linear_algebra_distribution.h"


namespace oomph{

class CRDoubleMatrix;

//=============================================================================
/// \short A vector in the mathematical sense, initially developed for
/// linear algebra type applications.\n
/// If MPI then this vector can be distributed - its distribution is 
/// described by the LinearAlgebraDistribution object at Distribution_pt. \n
/// Data is stored in a C-style pointer vector (double*)
//=============================================================================
 class DoubleVector : public DistributableLinearAlgebraObject
{                                                        
 
 public :                     
  
  /// \short Constructor for a DoubleVector of size ZERO.
  DoubleVector()
  : Values_pt(0), Internal_values(true)
  {}
 
 /// \short Constructor. Assembles a DoubleVector with a prescribed
 /// distribution. Additionally every entry can be set (with argument v - 
 /// defaults to 0).
 DoubleVector(const LinearAlgebraDistribution* const &dist_pt, 
              const double& v=0.0)
  : Values_pt(0), Internal_values(true)
  {
   this->build(dist_pt,v);
  }
   
 /// Destructor - just calls this->clear() to delete the distribution and data
 ~DoubleVector()
  {
   this->clear();
  }                 
 
 /// Copy constructor
 DoubleVector(const DoubleVector& new_vector)
  : Values_pt(0), Internal_values(true)
  {
   this->build(new_vector);
  }
 
 /// assignment operator
 void operator=(const DoubleVector& old_vector)
  {
   this->build(old_vector);
  }
 
 /// \short Just copys the argument DoubleVector
 void build(const DoubleVector& old_vector);

 /// \short Assembles a DoubleVector with distribution dist, if v is specified 
 /// each element is set to v, otherwise each element is set to 0.0
 void build(const LinearAlgebraDistribution* const &dist_pt, 
            const double& v);

 /// \short Assembles a DoubleVector with a distribution dist and coefficients
 /// taken from the vector v.\n
 /// Note. The vector v MUST be of length nrow()
 void build(const LinearAlgebraDistribution* const &dist_pt,
            const Vector<double>& v);

 /// \short initialise the whole vector with value v
 void initialise(const double& v);

 /// \short initialise the vector with coefficient from the vector v.\n
 /// Note: The vector v must be of length 
 void initialise(const Vector<double> v);

 /// \short wipes the DoubleVector
 void clear() 
  {
   if (Internal_values)
    {
     delete[] Values_pt;
    }
   Values_pt = 0;
   Distribution_pt->clear();
  }

 /// \short Allows are external data to be used by this vector. \n
 /// WARNING: The size of the external data must correspond to the 
 /// LinearAlgebraDistribution dist_pt argument.
 /// 1. When a rebuild method is called new internal values are created.\n
 /// 2. It is not possible to redistribute(...) a vector with external
 /// values \n.
 /// 3. External values are only deleted by this vector if
 /// delete_external_values = true.
 void set_external_values(const LinearAlgebraDistribution* const& dist_pt,
                          double* external_values,
                          bool delete_external_values)
  {
   // clean the memory
   this->clear();

   // Set the distribution
   Distribution_pt->rebuild(dist_pt);

   // set the external values
   set_external_values(external_values,delete_external_values);
  }
 
 /// \short Allows are external data to be used by this vector. \n
 /// WARNING: The size of the external data must correspond to the 
 /// distribution of this vector.
 /// 1. When a rebuild method is called new internal values are created.\n
 /// 2. It is not possible to redistribute(...) a vector with external
 /// values \n.
 /// 3. External values are only deleted by this vector if
 /// delete_external_values = true.
 void set_external_values(double* external_values, 
                          bool delete_external_values)
  {
#ifdef PARANOID
   // check that this distribution is setup
   if (!Distribution_pt->setup())
    {
    // if this vector does not own the double* values then it cannot be
    // distributed.
    // note: this is not stictly necessary - would just need to be careful 
    // with delete[] below.
     std::ostringstream error_message;    
     error_message << "The distribution of the vector must be setup before "
                   << "external values can be set"; 
     throw OomphLibError(error_message.str(),
                         "DoubleVector::set_external_values(...)",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   if (Internal_values)
    {
     delete[] Values_pt;
    }
   Values_pt = external_values;
   Internal_values = delete_external_values;
  }

 /// \short The contents of the vector are redistributed to match the new
 /// distribution. In a non-MPI rebuild this method works, but does nothing. \n
 /// \b NOTE 1: The current distribution and the new distribution must have
 /// the same number of global rows.\n
 /// \b NOTE 2: The current distribution and the new distribution must have
 /// the same Communicator.
 void redistribute(const LinearAlgebraDistribution* const& dist_pt);
   
 /// \short [] access function to the (local) values of this vector
 double& operator[](int i);

 /// \short == operator
 bool operator==(const DoubleVector& v);

 /// \short += operator
 void operator+=(DoubleVector v);

 /// -= operator
 void operator-=(DoubleVector v);

 /// \short [] access function to the (local) values of this vector
 const double& operator[](int i) const;

 /// \short returns the maximum coefficient
 double max();

 /// access function to the underlying values
 double* values_pt()
  {
   return Values_pt;
  }

 /// \short access function to the underlying values (const version)
 double* values_pt() const
  {
   return Values_pt;
  }

 /// output the contents of the vector
 void output(std::ostream &outfile);

 /// output the contents of the vector
 void output(std::string filename)
  {
    // Open file
    std::ofstream some_file;
    some_file.open(filename.c_str());
    output(some_file);
    some_file.close();
  }

 /// compute the 2 norm of this vector
 double dot(const DoubleVector& vec);

 /// compute the 2 norm of this vector 
 double norm();

 /// compute the A-norm using the matrix at matrix_pt
 double norm(CRDoubleMatrix* matrix_pt);

 private :
 
 /// the local vector
 double* Values_pt;

 /// \short Boolean flag to indicate whether the vector's data (values_pt) 
 /// is owned by this vector.
 bool Internal_values;

}; //end of DoubleVector                 
} // end of oomph namespace
#endif
