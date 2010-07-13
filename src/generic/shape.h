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
//This header file includes generic shape function classes

//Include guards to prevent multiple inclusions of the file
#ifndef OOMPH_SHAPE_HEADER
#define OOMPH_SHAPE_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

//oomph-lib includes
#include "Vector.h"
#include "matrices.h"
#include "orthpoly.h"

namespace oomph
{


//========================================================================
/// A Class for shape functions. In simple cases, the shape functions 
/// have only one index that can be thought of as corresponding to the
/// nodal points. In general, however, when quantities and 
/// their gradients are interpolated separately, the shape function have
/// two indicies: one corresponding to the nodal points, and the other
/// to the "type" of quantity being interpolated: function, derivative, &c
/// The second index can also represent the vector coordinate for 
/// vector-valued (Nedelec) shape functions.
///
/// The implementation of Shape functions is designed to permit fast
/// copying of entire sets of values by resetting the internal pointer
/// to the data, Psi;
/// functionality that is required, for example, 
///  when setting the test functions
/// in Galerkin elements and when reading pre-computed values of the shape 
/// functions.
/// In general, we cannot know at construction time whether the pointer to 
/// the values will be reset or not and, therefore,
/// whether the storage for values should be allocated by the object.
/// We choose to allocate storage on construction and store an
/// additional pointer Allocated_data that \b always addresses the storage
/// allocated by the object. If the Psi pointer is reset then this storage
/// will be "wasted", but only for the lifetime of the object. The cost for
/// non-copied Shape functions is one additional pointer.
//=========================================================================
class Shape
{
  private:

 /// \short Pointer that addresses the storage that will be used to read and
 /// set the shape functions. The shape functions are packed into 
 /// a flat array of doubles.
 double *Psi;

 /// \short Pointer that addresses the storage allocated by the object on
 /// construction. This will be the same as Psi if the object is not
 /// copied.
 double *Allocated_storage;

 ///Size of the first index of the shape function
 unsigned Index1;

 ///Size of the second index of the shape function
 unsigned Index2;

 ///Private function that checks whether the index is in range
 void range_check(const unsigned &i, const unsigned &j) const
  {
   //If an index is out of range, throw an error
   if((i >= Index1) || (j >= Index2))
    {
     std::ostringstream error_stream;
     error_stream << "Range Error: ";
     if(i >= Index1)
      {
       error_stream << i  << " is not in the range (0," << Index1-1 << ")" 
                    << std::endl;
      }
     if(j >= Index2)
      {
       error_stream << j  << " is not in the range (0," << Index2-1 << ")" 
                    << std::endl;
      }
     throw OomphLibError(error_stream.str(),
                         "Shape::range_check()",
                         OOMPH_EXCEPTION_LOCATION);
    }
  }

public:

 /// Constructor for a single-index set of shape functions.
 Shape(const unsigned &N) : Index1(N), Index2(1) 
  {Allocated_storage = new double[N]; Psi = Allocated_storage;}

 /// Constructor for a two-index set of shape functions.
 Shape(const unsigned &N, const unsigned &M) : Index1(N), Index2(M) 
  {Allocated_storage = new double[N*M]; Psi = Allocated_storage;}

 /// Broken copy constructor
 Shape(const Shape &shape) {BrokenCopy::broken_copy("Shape");}

 /// The assignment operator does a shallow copy 
 /// (resets the pointer to the data)
 void operator=(const Shape &shape)
  {
#ifdef PARANOID
   //Check the dimensions
   if((shape.Index1 != Index1) || 
      (shape.Index2 != Index2))
    {
     std::ostringstream error_stream;
     error_stream << "Cannot assign Shape object:" << std::endl
                  << "Indices do not match " 
                  << "LHS: " << Index1 << " " << Index2 
                  << ", RHS: " << shape.Index1 << " " << shape.Index2
                  << std::endl;
     throw OomphLibError(error_stream.str(),
                         "Shape::operator=()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   Psi = shape.Psi;
  }

 /// The assignment operator does a shallow copy 
 /// (resets the pointer to the data)
 void operator=(Shape* const &shape_pt)
  {
#ifdef PARANOID
   //Check the dimensions
   if((shape_pt->Index1 != Index1) || 
      (shape_pt->Index2 != Index2))
    {
     std::ostringstream error_stream;
     error_stream << "Cannot assign Shape object:" << std::endl
                  << "Indices do not match " 
                  << "LHS: " << Index1 << " " << Index2 
                  << ", RHS: " << shape_pt->Index1 << " " 
                  << shape_pt->Index2
                  << std::endl;
     throw OomphLibError(error_stream.str(),
                         "Shape::operator=()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   Psi = shape_pt->Psi;
  }

 /// Destructor, clear up the memory allocated by the object
 ~Shape() {delete[] Allocated_storage; Allocated_storage=0;}

 /// Overload the bracket operator to provide access to values.
 inline double & operator[](const unsigned &i) 
  {
#ifdef RANGE_CHECKING
   range_check(i,0);
#endif
   return Psi[i*Index2];
  }

 /// Overload the bracket operator (const version)
 inline const double & operator[](const unsigned &i) const 
  {
#ifdef RANGE_CHECKING
   range_check(i,0);
#endif
   return Psi[i*Index2];
  }

 /// Overload the round bracket operator to provide access to values.
 inline double &operator()(const unsigned &i) 
  {
#ifdef RANGE_CHECKING
   range_check(i,0);
#endif
   return Psi[i*Index2];
  }

 /// Overload the round bracket operator (const version)
 inline const double &operator()(const unsigned &i) const 
  {
#ifdef RANGE_CHECKING
   range_check(i,0);
#endif
   return Psi[i*Index2];
  }
 
 /// Overload the round bracket operator, allowing for two indicies
 inline double &operator()(const unsigned &i, const unsigned &j) 
  {
#ifdef RANGE_CHECKING
   range_check(i,j);
#endif
   return Psi[i*Index2 + j];
  }
 
 ///\short Overload the round bracket operator, allowing for two indices 
 /// (const version)
 inline const double &operator()(const unsigned &i, const unsigned &j)
  const{
#ifdef RANGE_CHECKING
  range_check(i,j);
#endif
  return Psi[i*Index2 + j];
 }

 /// Return the range of index 1 of the shape function object
 inline unsigned nindex1() const {return Index1;}

 /// Return the range of index 2 of the shape function object
 inline unsigned nindex2() const {return Index2;}

};

//================================================================
/// A Class for the derivatives of shape functions
/// The class design is essentially the same as Shape, but there is
/// on additional index that is used to indicate the coordinate direction in 
/// which the derivative is taken.
//================================================================
class DShape 
{
  private:

 /// \short Pointer that addresses the storage that will be used to read and
 /// set the shape-function derivatives. The values are packed into 
 /// a flat array of doubles.
 double *DPsi;

 /// \short Pointer that addresses the storage allocated by the object on
 /// construction. This will be the same as DPsi if the object is not
 /// copied.
 double *Allocated_storage;

 ///Size of the first index of the shape function
 unsigned Index1;

 ///Size of the second index of the shape function
 unsigned Index2;

 ///Size of the third index of the shape function
 unsigned Index3;
 
 /// Private function that checks whether the indices are in range
 void range_check(const unsigned &i, const unsigned &j,
                  const unsigned &k) const
  {
   //Check the first index
   if((i >= Index1) || (j >= Index2) || (k >= Index3))
    {
     std::ostringstream error_stream;
     error_stream << "Range Error: ";
     if(i >= Index1)
      {
       error_stream << i 
                    << " is not in the range (0," << Index1-1 << ")" 
                    << std::endl;
      }
     if(j >= Index2)
      {
       error_stream << j 
                    << " is not in the range (0," << Index2-1 << ")" 
                    << std::endl;
      }
     if(k >= Index3)
      {
       error_stream << k 
                    << " is not in the range (0," << Index3-1 << ")" 
                    << std::endl;
      }
     throw OomphLibError(error_stream.str(),
                         "DShape::range_check()",
                         OOMPH_EXCEPTION_LOCATION);
    }
  }

 
  public:

 /// Constructor with two parameters: a single-index shape function
 DShape(const unsigned &N, const unsigned &P) : Index1(N), Index2(1),
  Index3(P)
  {Allocated_storage = new double[N*P]; DPsi = Allocated_storage;}

 /// Constructor with three paramters: a two-index shape function
 DShape(const unsigned &N, const unsigned &M, const unsigned &P) :
  Index1(N), Index2(M), Index3(P)
  {Allocated_storage = new double[N*M*P]; DPsi = Allocated_storage;}

 /// Broken copy constructor
 DShape(const DShape &dshape) {BrokenCopy::broken_copy("DShape");}

 /// The assignment operator does a shallow copy 
 /// (resets the pointer to the data)
 void operator=(const DShape &dshape)
  {
#ifdef PARANOID
   //Check the dimensions
   if((dshape.Index1 != Index1) || 
      (dshape.Index2 != Index2) ||
      (dshape.Index3 != Index3))
    {
     std::ostringstream error_stream;
     error_stream << "Cannot assign DShape object:" << std::endl
                  << "Indices do not match " 
                  << "LHS: " << Index1 << " " << Index2 << " " 
                  << Index3
                  << ", RHS: " << dshape.Index1 << " " 
                  << dshape.Index2 << " " << dshape.Index3
                  << std::endl;
     throw OomphLibError(error_stream.str(),
                         "DShape::operator=()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   DPsi = dshape.DPsi;
  }

 /// The assignment operator does a shallow copy 
 /// (resets the pointer to the data)
 void operator=(DShape* const &dshape_pt)
  {
#ifdef PARANOID
   //Check the dimensions
   if((dshape_pt->Index1 != Index1) || 
      (dshape_pt->Index2 != Index2) ||
      (dshape_pt->Index3 != Index3))
    {
     std::ostringstream error_stream;
     error_stream << "Cannot assign Shape object:" << std::endl
                  << "Indices do not match " 
                  << "LHS: " << Index1 << " " << Index2 
                  << " " << Index3
                  << ", RHS: " << dshape_pt->Index1 << " " 
                  << dshape_pt->Index2 << " "
                  << dshape_pt->Index3
                  << std::endl;
     throw OomphLibError(error_stream.str(),
                         "DShape::operator=()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   DPsi = dshape_pt->DPsi;
  }



 /// Destructor, clean up the memory allocated by this object
 ~DShape() {delete[] Allocated_storage; Allocated_storage=0;}

 /// Overload the round bracket operator for access to the data
 inline double &operator()(const unsigned &i, const unsigned &k)
  {
#ifdef RANGE_CHECKING
   range_check(i,0,k);
#endif
   return DPsi[i*Index2*Index3 + k];
  }

 /// Overload the round bracket operator (const version)
 inline const double &operator()(const unsigned &i, const unsigned &k) const
  {
#ifdef RANGE_CHECKING
   range_check(i,0,k);
#endif
   return DPsi[i*Index2*Index3 + k];
  }

 /// Overload the round bracket operator, with 3 indices
 inline double &operator()(const unsigned &i, const unsigned &j, 
                    const unsigned &k)
  {
#ifdef RANGE_CHECKING
   range_check(i,j,k);
#endif
   return DPsi[(i*Index2 + j)*Index3 + k];
  }

 /// Overload the round bracket operator (const version)
 inline const double &operator()(const unsigned &i, const unsigned &j,
                          const unsigned &k) const
  {
#ifdef RANGE_CHECKING
   range_check(i,j,k);
#endif
   return DPsi[(i*Index2 + j)*Index3 + k];
  }

 /// \short Direct access to internal storage of data in flat-packed C-style 
 /// column-major format. WARNING: Only for experienced users. Only
 /// use this if raw speed is of the essence, as in the solid mechanics 
 /// problems.
 inline double& raw_direct_access(const unsigned long &i)
  {return DPsi[i];}

 /// \short Direct access to internal storage of data in flat-packed C-style 
 /// column-major format. WARNING: Only for experienced users. Only
 /// use this if raw speed is of the essence, as in the solid mechanics 
 /// problems.
 inline const double& raw_direct_access(const unsigned long &i) const
  {return DPsi[i];}

 /// \short Caculate the offset in flat-packed C-style, column-major format,
 /// required for a given i,j. WARNING: Only for experienced users. Only
 /// use this if raw speed is of the essence, as in the solid mechanics 
 /// problems.
 unsigned offset(const unsigned long &i,
                 const unsigned long &j) const
  {return (i*Index2 + j)*Index3 + 0;}



 /// Return the range of index 1 of the derivatives of the shape functions
 inline unsigned long nindex1() const {return Index1;}

 /// Return the range of index 2 of the derivatives of the shape functions
 inline unsigned long nindex2() const {return Index2;}

 /// Return the range of index 3 of the derivatives of the shape functions
 inline unsigned long nindex3() const {return Index3;}

};

////////////////////////////////////////////////////////////////////
//
// One dimensional shape functions and derivatives.
// empty -- simply establishes the template parameters.
//
////////////////////////////////////////////////////////////////////

namespace OneDimLagrange
{
 /// \short Definition for 1D Lagrange shape functions. The
 /// value of all the shape functions at the local coordinate s
 /// are returned in the array Psi.
 template<unsigned NNODE_1D>
  void shape(const double &s, double *Psi)
  {
   std::ostringstream error_stream;
   error_stream << "One dimensional Lagrange shape functions "
                << "have not been defined "
                << "for " << NNODE_1D << " nodes." << std::endl;
   throw OomphLibError(error_stream.str()<
                       "OneDimLagrange::shape()",
                       OOMPH_EXCEPTION_LOCATION);
  }
 

 /// \short Definition for derivatives of 1D Lagrange shape functions. The
 /// value of all the shape function derivatives at the local coordinate s
 /// are returned in the array DPsi.
 template<unsigned NNODE_1D>
  void dshape(const double &s, double *DPsi)
  {
   std::ostringstream error_stream;
   error_stream << "One dimensional Lagrange shape function derivatives "
                << "have not been defined "
                << "for " << NNODE_1D << " nodes." << std::endl;
   throw OomphLibError(error_stream.str()<
                       "OneDimLagrange::dshape()",
                       OOMPH_EXCEPTION_LOCATION);
  }
 

 /// \short Definition for second derivatives of 
 /// 1D Lagrange shape functions. The
 /// value of all the shape function derivatives at the local coordinate s
 /// are returned in the array DPsi.
 template<unsigned NNODE_1D>
  void d2shape(const double &s, double *DPsi)
  {
   std::ostringstream error_stream;
   error_stream << "One dimensional Lagrange shape function "
                << "second derivatives "
                << "have not been defined "
                << "for " << NNODE_1D << " nodes." << std::endl;
   throw OomphLibError(error_stream.str()<
                       "OneDimLagrangeShape::d2shape()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 /// \short 1D shape functions specialised to linear order (2 Nodes)
 // Note that the numbering is such that shape[0] is at s = -1.0.
 // and shape[1] is at s = 1.0
 template<>
  inline void shape<2>(const double &s, double *Psi)
  {
   Psi[0] = 0.5*(1.0-s);
   Psi[1] = 0.5*(1.0+s);
  }

 /// Derivatives of 1D shape functions specialised to linear order (2 Nodes)
 template<>
  inline void dshape<2>(const double &s, double *DPsi)
  {
   DPsi[0] = -0.5;
   DPsi[1] =  0.5;
  }
 
 /// \short Second Derivatives of 1D shape functions, 
 /// specialised to linear order  (2 Nodes)
 template<>
  inline void d2shape<2>(const double &s, double *DPsi)
  {
   DPsi[0] = 0.0;
   DPsi[1] = 0.0;
  }
 
 /// \short 1D shape functions specialised to quadratic order (3 Nodes)
 // Note that the numbering is such that shape[0] is at s = -1.0,
 // shape[1] is at s = 0.0 and shape[2] is at s = 1.0.
 template<> 
  inline void shape<3>(const double &s, double *Psi) 
  {
   Psi[0] = 0.5*s*(s-1.0);
   Psi[1] = 1.0 - s*s;
   Psi[2] = 0.5*s*(s+1.0);
  }
 
 /// Derivatives of 1D shape functions specialised to quadratic order (3 Nodes)
 template<>
  inline void dshape<3>(const double &s, double *DPsi)
  {
   DPsi[0] = s - 0.5;
   DPsi[1] = -2.0*s;
   DPsi[2] = s + 0.5;
  }


 /// Second Derivatives of 1D shape functions, specialised to quadratic order 
 /// (3 Nodes)
 template<>
  inline void d2shape<3>(const double &s, double *DPsi)
  {
   DPsi[0] =  1.0;
   DPsi[1] = -2.0;
   DPsi[2] =  1.0;
  }
 
 /// 1D shape functions specialised to cubic order (4 Nodes)
 template<>
  inline void shape<4>(const double &s, double *Psi) 
  {
   //Output from Maple
   double t1 = s*s;
   double t2 = t1*s;
   double t3 = 0.5625*t2;
   double t4 = 0.5625*t1;
   double t5 = 0.625E-1*s;
   double t7 = 0.16875E1*t2;
   double t8 = 0.16875E1*s;
   Psi[0] = -t3+t4+t5-0.625E-1;
   Psi[1] = t7-t4-t8+0.5625;
   Psi[2] = -t7-t4+t8+0.5625;
   Psi[3] = t3+t4-t5-0.625E-1;
  }


 /// Derivatives of 1D shape functions specialised to cubic order (4 Nodes)
 template<>
  inline void dshape<4>(const double &s, double *DPsi)
  {
   //Output from Maple
   double t1 = s*s;
   double t2 = 0.16875E1*t1;
   double t3 = 0.1125E1*s;
   double t5 = 0.50625E1*t1;
   DPsi[0] = -t2+t3+0.625E-1;
   DPsi[1] = t5-t3-0.16875E1;
   DPsi[2] = -t5-t3+0.16875E1;
   DPsi[3] = t2+t3-0.625E-1;
  }

 /// Second Derivatives of 1D shape functions specialised to cubic 
 /// order (4 Nodes)
 template<>
  inline void d2shape<4>(const double &s, double *DPsi)
  {
   //Output from Maple (modified by ALH, CHECK IT)
   double t1 = 2.0*s;
   double t2 = 0.16875E1*t1;
   double t5 = 0.50625E1*t1;
   DPsi[0] = -t2+0.1125E1;
   DPsi[1] = t5-0.1125E1;
   DPsi[2] = -t5-0.1125E1;
   DPsi[3] = t2+0.1125E1;
  }
 
};

//===============================================================
/// One Dimensional Hermite shape functions
//===============================================================
namespace OneDimHermite
{
 //Convention for polynomial numbering scheme
 //Type 0 is position, 1 is slope
 //Node 0 is at s=0 and 1 is s=1

 ///Constructor sets the values of the shape functions at the position s.
 inline void shape(const double &s, double Psi[2][2])
  {
   //Node 0
   Psi[0][0] = 0.25*(s*s*s - 3.0*s + 2.0);
   Psi[0][1] = 0.25*(s*s*s - s*s - s + 1.0);
   //Node 1
   Psi[1][0] = 0.25*(2.0 + 3.0*s - s*s*s);
   Psi[1][1] = 0.25*(s*s*s + s*s - s - 1.0);
  }


/// Derivatives of 1D Hermite shape functions
 inline void dshape(const double &s, double DPsi[2][2]) 
  {
   //Node 0
   DPsi[0][0] = 0.75*(s*s - 1.0);
   DPsi[0][1] = 0.25*(3.0*s*s - 2.0*s - 1.0);
    //Node 1
   DPsi[1][0] = 0.75*(1.0 - s*s);
   DPsi[1][1] = 0.25*(3.0*s*s + 2.0*s - 1.0);
 }

/// Second derivatives of the Hermite shape functions
 inline void d2shape(const double &s, double DPsi[2][2])
  {
   //Node 0
   DPsi[0][0] = 1.5*s;
   DPsi[0][1] = 0.5*(3.0*s - 1.0);
   //Node 1
   DPsi[1][0] = -1.5*s;
   DPsi[1][1] = 0.5*(3.0*s + 1.0);

  }

};

//=====================================================================
/// Class that returns the shape functions associated with legendre
//=====================================================================
template<unsigned NNODE_1D>
class OneDimensionalLegendreShape : public Shape
{
 static bool Nodes_calculated;

  public:
 static Vector<double> z;
 
 /// Static function used to populate the stored positions
 static inline void calculate_nodal_positions()
  {
   if(!Nodes_calculated) 
    {
     Orthpoly::gll_nodes(NNODE_1D,z);
     Nodes_calculated=true;
    }
  }

 static inline double nodal_position(const unsigned &n)
  {return z[n];}

 /// Constructor
 OneDimensionalLegendreShape(const double &s) : Shape(NNODE_1D)
  {
   using namespace Orthpoly;
   
   unsigned p = NNODE_1D-1;
   //Now populate the shape function
   for(unsigned i=0;i<NNODE_1D;i++)
    {
     //If we're at one of the nodes, the value must be 1.0
     if(std::abs(s-z[i]) < Orthpoly::eps) {(*this)[i] = 1.0;}
     //Otherwise use the lagrangian interpolant
     else
      {
       (*this)[i] = (1.0 - s*s)*dlegendre(p,s)/
        (p*(p+1)*legendre(p,z[i])*(z[i] - s));
      }
    }
  }
};

template<unsigned NNODE_1D>
Vector<double> OneDimensionalLegendreShape<NNODE_1D>::z;

template<unsigned NNODE_1D>
bool OneDimensionalLegendreShape<NNODE_1D>::Nodes_calculated=false;


template <unsigned NNODE_1D>
class OneDimensionalLegendreDShape : public Shape
{
  public:
 // Constructor 
 OneDimensionalLegendreDShape(const double &s) : Shape(NNODE_1D)
  {
   unsigned p = NNODE_1D - 1;
   Vector <double> z = OneDimensionalLegendreShape<NNODE_1D>::z;

   
   bool root=false;
   
   for(unsigned i=0;i<NNODE_1D;i++)
    {
     unsigned rootnum=0;
     for(unsigned j=0;j<NNODE_1D;j++){     // Loop over roots to check if
      if(std::abs(s-z[j])<10*Orthpoly::eps){ // s happens to be a root.
       root=true;
       break;
      }
		    rootnum+=1;
     }
     if(root==true){
      if(i==rootnum && i==0){(*this)[i]=-(1.0+p)*p/4.0;}
      else if(i==rootnum && i==p){(*this)[i]=(1.0+p)*p/4.0;}
		    else if(i==rootnum){(*this)[i]=0.0;}
      else{(*this)[i]=Orthpoly::legendre(p,z[rootnum])
            /Orthpoly::legendre(p,z[i])/(z[rootnum]-z[i]);}
     }
     else{
		    (*this)[i]=((1+s*(s-2*z[i]))/(s-z[i])* Orthpoly::dlegendre(p,s)
                                -(1-s*s)* Orthpoly::ddlegendre(p,s))
                     /p/(p+1.0)/Orthpoly::legendre(p,z[i])/(s-z[i]);
     }
     root = false;
    }
   
   
  }

};

}

#endif
