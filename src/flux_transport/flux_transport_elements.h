//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.85. June 9, 2008.
//LIC// 
//LIC// Copyright (C) 2006-2008 Matthias Heil and Andrew Hazel
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
//Header file for flux transport elements base class

#ifndef OOMPH_FLUX_TRANSPORT_ELEMENTS_HEADER
#define OOMPH_FLUX_TRANSPORT_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "../generic/elements.h"


namespace oomph
{

 //==================================================================
 ///Base class for the flux transport equations templated by the
 ///dimension DIM.  The equations that are solved are
 /// \f $ \frac{\partial u_{i}}{\partial t} + \frac{\partial}{\partial x_{j}} 
 /// \left(F_{ij}(u_{k})\right), \f$
 /// where \f$ F_{ij} \f$ is a matrix of flux components.
 //==================================================================
template<unsigned DIM>
class FluxTransportEquations : public virtual FiniteElement
{

protected:

 /// \short Return the number of fluxes (default zero) 
 virtual inline unsigned nflux() const {return 0;}

 /// \short Return the index at which the i-th unknown value
 /// is stored. The default value, i, is appropriate for single-physics
 /// problems.
 /// In derived multi-physics elements, this function should be overloaded
 /// to reflect the chosen storage scheme. Note that these equations require
 /// that the unknowns are always stored at the same indices at each node.
 virtual inline unsigned u_index_flux_transport(const unsigned &i) 
  const {return i;}

 /// \short Return the flux as a function of the unknown. This interface
 /// could (should) be generalised)
 virtual void flux(const Vector<double> &u, DenseMatrix<double> &f)
  {
   std::ostringstream error_stream;
   error_stream << "Default empty flux function called\n"
                << "This should be overloaded with a specific flux function\n"
                << "in a derived class\n";
   throw OomphLibError(error_stream.str(),
                       "FluxTransportEquations::flux()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 /// \short Return the flux derivatives as a funciton of the unknowns
 /// Default finite-different implementation
 virtual void dflux_du(const Vector<double> &u, 
                       RankThreeTensor<double> &df_du);

 /// \short Shape/test functions and derivs w.r.t. to global coords at 
 /// local coord. s; return  Jacobian of mapping
 virtual double dshape_and_dtest_eulerian_flux_transport(
  const Vector<double> &s, 
  Shape &psi, 
  DShape &dpsidx, 
  Shape &test, 
  DShape &dtestdx) const=0;

 /// \short Shape/test functions and derivs w.r.t. to global coords at 
 /// integration point ipt; return  Jacobian of mapping
 virtual double dshape_and_dtest_eulerian_at_knot_flux_transport(
  const unsigned &ipt, 
  Shape &psi, 
  DShape &dpsidx,
  Shape &test, 
  DShape &dtestdx) 
  const=0;


public:

 ///Constructor
 FluxTransportEquations() : FiniteElement() { }

 /// Compute the element's residual Vector
 void fill_in_contribution_to_residuals(Vector<double> &residuals)
  {
   //Call the generic residuals function with flag set to 0
   //and using a dummy matrix argument
   fill_in_generic_residual_contribution_flux_transport(
    residuals,GeneralisedElement::Dummy_matrix,
    GeneralisedElement::Dummy_matrix,0);
  }

 ///\short Compute the element's residual Vector and the jacobian matrix
 /// Virtual function can be overloaded by hanging-node version
 void fill_in_contribution_to_jacobian(Vector<double> &residuals,
                                       DenseMatrix<double> &jacobian)
  {
   //Call the generic routine with the flag set to 1
   fill_in_generic_residual_contribution_flux_transport(
    residuals,jacobian,GeneralisedElement::Dummy_matrix,1);
  }

 /// Add the element's contribution to its residuals vector,
 /// jacobian matrix and mass matrix
 void fill_in_contribution_to_jacobian_and_mass_matrix(
  Vector<double> &residuals, DenseMatrix<double> &jacobian, 
  DenseMatrix<double> &mass_matrix)
  {
   //Call the generic routine with the flag set to 2
   fill_in_generic_residual_contribution_flux_transport(
    residuals,jacobian,mass_matrix,2);
  }

  ///Assemble the contributions to the mass matrix and residuals
 void fill_in_contribution_to_mass_matrix(Vector<double> &residuals,
                                          DenseMatrix<double> &mass_matrix)
  {
   fill_in_generic_residual_contribution_flux_transport(
    residuals,GeneralisedElement::Dummy_matrix,mass_matrix,3);
  }

 
 ///\short Compute the residuals for the Navier--Stokes equations; 
 /// flag=1(or 0): do (or don't) compute the Jacobian as well. 
 virtual void fill_in_generic_residual_contribution_flux_transport(
  Vector<double> &residuals, DenseMatrix<double> &jacobian, 
  DenseMatrix<double> &mass_matrix, unsigned flag);

 //Get the value of the unknowns
 double interpolated_u_flux_transport(const Vector<double> &s, 
                                      const unsigned &i);

 /// \short i-th component of du/dt at local node n. 
 /// Uses suitably interpolated value for hanging nodes.
 double du_dt_flux_transport(const unsigned &n, const unsigned &i) const;

 //Default output function
 void output(std::ostream &outfile)
  {
   unsigned nplot=5;
   output(outfile,nplot);
  }

 void output(std::ostream &outfile, const unsigned &nplot);

};

}

#endif
