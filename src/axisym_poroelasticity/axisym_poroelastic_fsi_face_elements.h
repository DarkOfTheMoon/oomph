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
//Header file for elements that are used to integrate fluid tractions
#ifndef OOMPH_AXISYM_POROELASTIC_FSI_TRACTION_ELEMENTS_HEADER
#define OOMPH_AXISYM_POROELASTIC_FSI_TRACTION_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif


//OOMPH-LIB headers
#include "../generic/shape.h"
#include "../generic/elements.h"
#include "../generic/element_with_external_element.h"

#include "axisym_poroelasticity_elements.h"



namespace oomph
{

//=======================================================================
/// Namespace containing the default Strouhal number of axisymmetric
/// linearised poroelastic FSI.
//=======================================================================
namespace LinearisedAxisymPoroelasticBJS_FSIHelper
 {
  /// Default for fluid Strouhal number
  double Default_strouhal_number=1.0;

  /// Default for inverse slip rate coefficient: no slip
  double Default_inverse_slip_rate_coefficient=0.0;
 }



//======================================================================
/// \short A class for elements that allow the imposition of the linearised 
/// poroelastic FSI
/// slip condition (according to the Beavers-Joseph-Saffman condition) from an
/// adjacent poroelastic axisymmetric medium. The element geometry is obtained
/// from the FaceGeometry<ELEMENT> policy class.
//======================================================================
template <class FLUID_BULK_ELEMENT, class POROELASTICITY_BULK_ELEMENT>
class LinearisedAxisymPoroelasticBJS_FSIElement :
 public virtual FaceGeometry<FLUID_BULK_ELEMENT>,
 public virtual FaceElement,
 public virtual ElementWithExternalElement
{
public:

 /// \short Constructor, takes the pointer to the "bulk" element and the
 /// face index identifying the face to which the element is attached.
 /// The optional identifier can be used
 /// to distinguish the additional nodal values created by
 /// this element from thos created by other FaceElements.
 LinearisedAxisymPoroelasticBJS_FSIElement(
   FiniteElement* const &bulk_el_pt,
   const int& face_index,
   const unsigned &id=0);

 /// Broken copy constructor
 LinearisedAxisymPoroelasticBJS_FSIElement(
   const LinearisedAxisymPoroelasticBJS_FSIElement& dummy)
  {
   BrokenCopy::broken_copy(
     "LinearisedAxisymPoroelasticBJS_FSIElement");
  }

 /// Broken assignment operator
 void operator=(const LinearisedAxisymPoroelasticBJS_FSIElement&)
  {
   BrokenCopy::broken_assign(
     "LinearisedAxisymPoroelasticBJS_FSIElement");
  }

 /// \short Access function for the pointer to the fluid Strouhal number
 /// (if not set, St defaults to 1)
 double* &st_pt() {return St_pt;}

 /// Access function for the fluid Strouhal number
 double st() const
  {
   return *St_pt;
  }

 /// Inverse slip rate coefficient
 double inverse_slip_rate_coefficient() const
 {
  return *Inverse_slip_rate_coeff_pt;
 }


 /// Pointer to inverse slip rate coefficient
 double*& inverse_slip_rate_coefficient_pt()
 {
  return Inverse_slip_rate_coeff_pt;
 }


 /// Add the element's contribution to its residual vector
 void fill_in_contribution_to_residuals(Vector<double> &residuals)
  {
   //Call the generic residuals function with flag set to 0
   //using a dummy matrix argument
   fill_in_generic_residual_contribution_axisym_poroelastic_fsi(
    residuals,GeneralisedElement::Dummy_matrix,0);
  }
 

 // hieher need to add derivs w.r.t external data (the 
 // bulk velocity dofs
 /* /// \short Add the element's contribution to its residual vector and its */
 /* /// Jacobian matrix */
 /* void fill_in_contribution_to_jacobian( */
 /*   Vector<double> &residuals, */
 /*   DenseMatrix<double> &jacobian) */
 /*  { */
 /*   //Call the generic routine with the flag set to 1 */
 /*   fill_in_generic_residual_contribution_fpsi_bjs_axisym */
 /*    (residuals,jacobian,1); */

 /*   //Derivatives w.r.t. external data */
 /*   fill_in_jacobian_from_external_interaction_by_fd(residuals,jacobian); */
 /*  } */
 

 /// Output function
 void output(std::ostream &outfile)
  {
   //Dummy
   unsigned nplot=0;
   output(outfile,nplot);
  }

 /// Output function: Output at Gauss points; n_plot is ignored.
 void output(std::ostream &outfile, const unsigned &n_plot)
 {
  //Get the value of Nintpt
  const unsigned n_intpt = integral_pt()->nweight();
  
  // Tecplot header info
  outfile << this->tecplot_zone_string(n_intpt);
  
  //Set the Vector to hold local coordinates
  Vector<double> s(Dim-1);
  Vector<double> x_bulk(Dim);

  // Cache the Strouhal number
  const double local_st=st();
  
  // Cache the slip rate coefficient
  const double local_inverse_slip_rate_coeff=inverse_slip_rate_coefficient(); 
  
  //Loop over the integration points
  for(unsigned ipt=0;ipt<n_intpt;ipt++)
   {
    //Assign values of s
    for(unsigned i=0;i<(Dim-1);i++)
     {
      s[i] = integral_pt()->knot(ipt,i);
     }

    // Get the outer unit normal
    Vector<double> interpolated_normal(Dim);
    outer_unit_normal(ipt,interpolated_normal);
    
    // Calculate the unit tangent vector
    Vector<double> interpolated_tangent(Dim);
    interpolated_tangent[0]=-interpolated_normal[1];
    interpolated_tangent[1]= interpolated_normal[0];
    
    // Get solid velocity and porous flux from adjacent solid
    POROELASTICITY_BULK_ELEMENT* ext_el_pt=
     dynamic_cast<POROELASTICITY_BULK_ELEMENT*>(
      external_element_pt(0,ipt));
    Vector<double> s_ext(external_element_local_coord(0,ipt));
    Vector<double> du_dt(3);
    Vector<double> q(2);
    ext_el_pt->interpolated_du_dt(s_ext,du_dt);
    ext_el_pt->interpolated_q(s_ext,q);
    x_bulk[0]=ext_el_pt->interpolated_x(s_ext,0);
    x_bulk[1]=ext_el_pt->interpolated_x(s_ext,1);
    
#ifdef PARANOID
    // Get own coordinates:
    Vector<double> x(Dim);
    this->interpolated_x(s,x);

    double error=sqrt((x[0]-x_bulk[0])*(x[0]-x_bulk[0])+
                      (x[1]-x_bulk[1])*(x[1]-x_bulk[1]));
    double tol=1.0e-10;
    if (error>tol)
     {
      std::stringstream junk;
      junk 
       << "Gap between external and face element coordinate\n"
       << "is suspiciously large: "
       << error << "\nBulk/external at: " 
       << x_bulk[0] << " " << x_bulk[1] << "\n" 
       << "Face at: " << x[0] << " " << x[1] << "\n";
      OomphLibWarning(junk.str(),
                      OOMPH_CURRENT_FUNCTION,
                      OOMPH_EXCEPTION_LOCATION);
     }
#endif
  

    // Get permeability from the bulk poroelasticity element
    const double permeability=ext_el_pt->permeability();
    const double local_permeability_ratio=ext_el_pt->permeability_ratio();

    // Local coordinate in bulk element
    Vector<double> s_bulk(Dim);
    s_bulk=local_coordinate_in_bulk(s);

    // Get the fluid traction from the NSt bulk element 
    Vector<double> traction_nst(3);
    dynamic_cast<FLUID_BULK_ELEMENT*>(bulk_element_pt())->traction(
     s_bulk,interpolated_normal,traction_nst);
    
    // Get fluid velocity from bulk element
    Vector<double> fluid_veloc(Dim+1,0.0);
    dynamic_cast<FLUID_BULK_ELEMENT*>(bulk_element_pt())->
     interpolated_u_axi_nst(s_bulk,fluid_veloc);

    // Calculate the normal components
    double scaled_normal_wall_veloc=0.0;
    double scaled_normal_poro_veloc=0.0;
    double scaled_tangential_wall_veloc=0.0;
    double scaled_tangential_poro_veloc=0.0;
    double normal_nst_veloc=0.0;
    for(unsigned i=0;i<Dim;i++)
     {
      scaled_normal_wall_veloc+=
      local_st*du_dt[i]*interpolated_normal[i];

      scaled_normal_poro_veloc+=
      local_st*permeability*q[i]*interpolated_normal[i];

      scaled_tangential_wall_veloc+=
       local_st*du_dt[i]*interpolated_tangent[i];

      scaled_tangential_poro_veloc+=
       -traction_nst[i]*sqrt(local_permeability_ratio)*
       local_inverse_slip_rate_coeff*interpolated_tangent[i];

      normal_nst_veloc+=fluid_veloc[i]*interpolated_normal[i];
     }
    
    // Calculate the combined poroelasticity "velocity" (RHS of BJS BC).
    double total_poro_normal_component=
     scaled_normal_wall_veloc+scaled_normal_poro_veloc;
    double total_poro_tangential_component=
     scaled_tangential_wall_veloc+scaled_tangential_poro_veloc;
    Vector<double> poro_veloc(2,0.0);
    for(unsigned i=0;i<Dim;i++)
     {
      poro_veloc[i]+=
       total_poro_normal_component*interpolated_normal[i]+
       total_poro_tangential_component*interpolated_tangent[i];
     }

    // Output
    outfile << x_bulk[0] << " "
            << x_bulk[1] << " "
            << fluid_veloc[0] << " "
            << fluid_veloc[1] << " "
            << poro_veloc[0] << " "
            << poro_veloc[1] << " "
            << normal_nst_veloc*interpolated_normal[0] << " "
            << normal_nst_veloc*interpolated_normal[1] << " "
            << total_poro_normal_component*interpolated_normal[0] << " " 
            << total_poro_normal_component*interpolated_normal[1] << " " 
            << scaled_normal_wall_veloc*interpolated_normal[0] << " " 
            << scaled_normal_wall_veloc*interpolated_normal[1] << " " 
            << scaled_normal_poro_veloc*interpolated_normal[0] << " " 
            << scaled_normal_poro_veloc*interpolated_normal[1] << " " 
            << std::endl;
   }
 }


 /// C-style output function
 void output(FILE* file_pt)
  {FaceGeometry<FLUID_BULK_ELEMENT>::output(file_pt);}

 /// C-style output function
 void output(FILE* file_pt, const unsigned &n_plot)
  {FaceGeometry<FLUID_BULK_ELEMENT>::output(file_pt,n_plot);}


protected:

 /// \short Function to compute the shape and test functions and to return
 /// the Jacobian of mapping between local and global (Eulerian)
 /// coordinates
 double shape_and_test(
   const Vector<double> &s,
   Shape &psi,
   Shape &test) const
  {
   // Find number of nodes
   unsigned n_node = nnode();

   // Get the shape functions
   shape(s,psi);

   // Set the test functions to be the same as the shape functions
   for(unsigned i=0;i<n_node;i++) {test[i] = psi[i];}

   // Return the value of the jacobian
   return J_eulerian(s);
  }


 /// \short Function to compute the shape and test functions and to return
 /// the Jacobian of mapping between local and global (Eulerian)
 /// coordinates
 double shape_and_test_at_knot(
   const unsigned &ipt,
   Shape &psi,
   Shape &test) const
  {
   //Find number of nodes
   unsigned n_node = nnode();

   //Get the shape functions
   shape_at_knot(ipt,psi);

   //Set the test functions to be the same as the shape functions
   for(unsigned i=0;i<n_node;i++) {test[i] = psi[i];}

   //Return the value of the jacobian
   return J_eulerian_at_knot(ipt);
  }

private:

 /// \short Add the element's contribution to its residual vector.
 /// flag=1(or 0): do (or don't) compute the contribution to the
 /// Jacobian as well.
 void fill_in_generic_residual_contribution_axisym_poroelastic_fsi(
  Vector<double> &residuals, DenseMatrix<double> &jacobian,
  const unsigned& flag);
 
 ///The spatial dimension of the problem
 unsigned Dim;
 
 ///The index at which the velocity unknowns are stored at the nodes
 Vector<unsigned> U_index_axisym_poroelastic_fsi;
 
 /// Lagrange Id
 unsigned Id;
 
 /// Pointer to fluid Strouhal number
 double* St_pt;

 /// Pointer to inverse slip rate coefficient
 double* Inverse_slip_rate_coeff_pt;

};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////



//===========================================================================
/// Constructor, takes the pointer to the "bulk" element, and the
/// face index that identifies the face of the bulk element to which
/// this face element is to be attached.
/// The optional identifier can be used
/// to distinguish the additional nodal values created by
/// this element from thos created by other FaceElements.
//===========================================================================
 template <class FLUID_BULK_ELEMENT, class POROELASTICITY_BULK_ELEMENT>
  LinearisedAxisymPoroelasticBJS_FSIElement
  <FLUID_BULK_ELEMENT,POROELASTICITY_BULK_ELEMENT>::
  LinearisedAxisymPoroelasticBJS_FSIElement(
   FiniteElement* const &bulk_el_pt,
   const int &face_index,
   const unsigned &id) :
 FaceGeometry<FLUID_BULK_ELEMENT>(), FaceElement()
{
 // Set source element storage: one interaction with an external element
 // that provides the velocity of the adjacent linear elasticity
 // element
 this->set_ninteraction(1);
 
 //  Store the ID of the FaceElement -- this is used to distinguish
 // it from any others
 Id=id;

 // Initialise pointer to fluid Strouhal number. Defaults to 1
 St_pt=&LinearisedAxisymPoroelasticBJS_FSIHelper::Default_strouhal_number;

 // Initialise pointer to inverse slip rate coefficient. Defaults to 0 (no slip)
  Inverse_slip_rate_coeff_pt=&LinearisedAxisymPoroelasticBJS_FSIHelper::
   Default_inverse_slip_rate_coefficient;
  
 // Let the bulk element build the FaceElement, i.e. setup the pointers
 // to its nodes (by referring to the appropriate nodes in the bulk
 // element), etc.
 bulk_el_pt->build_face_element(face_index,this);
 
 // Extract the dimension of the problem from the dimension of
 // the first node
 Dim = this->node_pt(0)->ndim();
 
 // Upcast pointer to bulk element
 FLUID_BULK_ELEMENT* cast_bulk_el_pt=
  dynamic_cast<FLUID_BULK_ELEMENT*>(bulk_el_pt);
 
 // Read the index from the (cast) bulk element.
 U_index_axisym_poroelastic_fsi.resize(3);
 for(unsigned i=0;i<3;i++)
  {
   U_index_axisym_poroelastic_fsi[i] = cast_bulk_el_pt->u_index_axi_nst(i);
  }

 // The velocities in the bulk affect the shear stress acting
 // here so we must include them as external data
 unsigned n=cast_bulk_el_pt->nnode();
 for (unsigned j=0;j<n;j++)
  {
   Node* nod_pt=cast_bulk_el_pt->node_pt(j);
   bool do_it=true;
   unsigned nn=nnode();   
   for (unsigned jj=0;jj<nn;jj++)
    {
     if (nod_pt==node_pt(jj)) 
      {
       do_it=false; 
       break;
      }
    }
   if (do_it) add_external_data(cast_bulk_el_pt->node_pt(j));
  }
   
 // We need Dim+1 additional values for each FaceElement node to store the
 // Lagrange multipliers.
 Vector<unsigned> n_additional_values(nnode(), Dim+1);

 // Now add storage for Lagrange multipliers and set the map containing the
 // position of the first entry of this face element's additional values.
 add_additional_values(n_additional_values,id);

}

//===========================================================================
/// \short Helper function to compute the element's residual vector and
/// the Jacobian matrix.
//===========================================================================
template <class FLUID_BULK_ELEMENT, class POROELASTICITY_BULK_ELEMENT>
void LinearisedAxisymPoroelasticBJS_FSIElement
<FLUID_BULK_ELEMENT,POROELASTICITY_BULK_ELEMENT>::
  fill_in_generic_residual_contribution_axisym_poroelastic_fsi(
   Vector<double> &residuals, DenseMatrix<double> &jacobian,
   const unsigned& flag)
{
 // Find out how many nodes there are
 const unsigned n_node = nnode();

 // Set up memory for the shape and test functions
 Shape psif(n_node), testf(n_node);

 // Set the value of Nintpt
 const unsigned n_intpt = integral_pt()->nweight();

 // Set the Vector to hold local coordinates
 Vector<double> s(Dim-1);

 // Cache the Strouhal number
 const double local_st=st();

 // Cache the slip rate coefficient
 const double local_inverse_slip_rate_coeff=inverse_slip_rate_coefficient(); 

 // Integers to hold the local equation and unknown numbers
 int local_eqn=0;

 // Loop over the integration points
 // --------------------------------
 for(unsigned ipt=0;ipt<n_intpt;ipt++)
  {
   // Assign values of s
   for(unsigned i=0;i<(Dim-1);i++) {s[i] = integral_pt()->knot(ipt,i);}

   // Get the integral weight
   double w = integral_pt()->weight(ipt);

   // Find the shape and test functions and return the Jacobian
   // of the mapping
   double J = shape_and_test(s,psif,testf);

   // Calculate the coordinates
   double interpolated_r=0;

   // Premultiply the weights and the Jacobian
   double W = w*J;

   // Calculate the Lagrange multiplier and the fluid veloc
   Vector<double> lambda(Dim+1,0.0);
   Vector<double> fluid_veloc(Dim+1,0.0);

   // Loop over nodes
   for(unsigned j=0;j<n_node;j++)
    {
     Node* nod_pt=node_pt(j);

     // Cast to a boundary node
     BoundaryNodeBase *bnod_pt=dynamic_cast<BoundaryNodeBase*>(node_pt(j));

     // Get the index of the first nodal value associated with
     // this FaceElement
     unsigned first_index=
      bnod_pt->index_of_first_value_assigned_by_face_element(Id);

     // Work out radius
     interpolated_r+=nodal_position(j,0)*psif(j);

     // Assemble
     for(unsigned i=0;i<Dim+1;i++)
      {
       lambda[i]+=nod_pt->value(first_index+i)*psif(j);
       fluid_veloc[i]+=nod_pt->value(U_index_axisym_poroelastic_fsi[i])*psif(j);
      }
    }

   // Local coordinate in bulk element
   Vector<double> s_bulk(Dim);
   s_bulk=local_coordinate_in_bulk(s);
   
#ifdef PARANOID
   {
    // Get fluid velocity from bulk element
    Vector<double> fluid_veloc_from_bulk(Dim+1,0.0);
    dynamic_cast<FLUID_BULK_ELEMENT*>(bulk_element_pt())->
     interpolated_u_axi_nst(s_bulk,fluid_veloc_from_bulk);

    double error=0.0;
    for(unsigned i=0;i<Dim+1;i++)
     {
      error+=
       (fluid_veloc[i]-fluid_veloc_from_bulk[i])*
       (fluid_veloc[i]-fluid_veloc_from_bulk[i]);
     }
    error=sqrt(error);
    double tol=1.0e-15;
    if (error>tol)
     {
      std::stringstream junk;
      junk 
       << "Difference in Navier-Stokes velocities\n"
       << "is suspiciously large: "
       << error << "\nVeloc from bulk: " 
       << fluid_veloc_from_bulk[0] << " " << fluid_veloc_from_bulk[1] << "\n" 
       <<            "Veloc from face: " 
       << fluid_veloc[0] << " " << fluid_veloc[1] << "\n";
      OomphLibWarning(junk.str(),
                      OOMPH_CURRENT_FUNCTION,
                      OOMPH_EXCEPTION_LOCATION);
     }
   }
#endif

   // Get solid velocity from adjacent solid
   POROELASTICITY_BULK_ELEMENT* ext_el_pt=
    dynamic_cast<POROELASTICITY_BULK_ELEMENT*>(
      external_element_pt(0,ipt));
   Vector<double> s_ext(external_element_local_coord(0,ipt));
   Vector<double> du_dt(2), q(2);
   ext_el_pt->interpolated_du_dt(s_ext,du_dt);
   ext_el_pt->interpolated_q(s_ext,q);

   // Get the outer unit normal
   Vector<double> interpolated_normal(Dim);
   outer_unit_normal(ipt,interpolated_normal);

   // Calculate the unit tangent vector
   Vector<double> interpolated_tangent(Dim);
   interpolated_tangent[0]=-interpolated_normal[1];
   interpolated_tangent[1]= interpolated_normal[0];

   // Get permeability from the bulk poroelasticity element
   const double permeability=ext_el_pt->permeability();
   const double local_permeability_ratio=ext_el_pt->permeability_ratio();

   // We are given the normal and tangential components of the combined
   // poroelasticity "velocity" at the boundary from the BJS condition ---
   // calculate the vector in r-z coords from these
   double normal_component=0.0;
   double tangential_component=0.0;

   // Get the fluid traction from the NSt bulk element
   Vector<double> traction_nst(3);
   dynamic_cast<FLUID_BULK_ELEMENT*>(bulk_element_pt())->traction(
     s_bulk,interpolated_normal,traction_nst);

   // Calculate the normal and tangential components
   for(unsigned i=0;i<Dim;i++)
    {
     normal_component+=local_st*
      (du_dt[i]+permeability*q[i])*interpolated_normal[i];
     tangential_component+=
      (local_st*du_dt[i]-traction_nst[i]*
       sqrt(local_permeability_ratio)*local_inverse_slip_rate_coeff)
      *interpolated_tangent[i];
    }
   
   // Calculate the combined poroelasticity "velocity"
   Vector<double> poro_veloc(3,0.0);
   for(unsigned i=0;i<Dim;i++)
    {
     poro_veloc[i]+=
      normal_component*interpolated_normal[i]+
      tangential_component*interpolated_tangent[i];
    }

   // Now add to the appropriate equations

   // Loop over the test functions
   for(unsigned l=0;l<n_node;l++)
    {
     // Loop over directions
     for(unsigned i=0;i<Dim+1;i++)
      {
       // Add contribution to bulk Navier Stokes equations where
       // ------------------------------------------------------
       // the Lagrange multiplier acts as a traction
       // ------------------------------------------
       local_eqn=nodal_local_eqn(l,U_index_axisym_poroelastic_fsi[i]);

       /*IF it's not a boundary condition*/
       if(local_eqn>=0)
        {
         //Add the Lagrange multiplier "traction" to the bulk
         residuals[local_eqn]-=lambda[i]*testf[l]*interpolated_r*W;

         //Jacobian entries
         if(flag)
          {
           //Loop over the lagrange multiplier unknowns
           for(unsigned l2=0;l2<n_node;l2++)
            {
             // Cast to a boundary node
             BoundaryNodeBase *bnod_pt=
              dynamic_cast<BoundaryNodeBase*>(node_pt(l2));

             // Local unknown
             int local_unknown=nodal_local_eqn
              (l2,bnod_pt->
               index_of_first_value_assigned_by_face_element(Id)+i);

             // If it's not pinned
             if(local_unknown>=0)
              {
               jacobian(local_eqn,local_unknown)-=
                psif[l2]*testf[l]*interpolated_r*W;
              }
            }
          }
        }

       // Now do the Lagrange multiplier equations
       //-----------------------------------------
       // Cast to a boundary node
       BoundaryNodeBase *bnod_pt =
        dynamic_cast<BoundaryNodeBase*>(node_pt(l));

       // Local eqn number:
       int local_eqn=nodal_local_eqn
        (l,bnod_pt->index_of_first_value_assigned_by_face_element(Id)+i);

       // If it's not pinned
       if(local_eqn>=0)
        {

#ifdef PARANOID
         if (i==Dim)
          {
           std::stringstream junk;
           junk << "Elements have not been validated for nonzero swirl!\n";
           OomphLibWarning(junk.str(),
                           OOMPH_CURRENT_FUNCTION,
                           OOMPH_EXCEPTION_LOCATION);
          }
#endif

         residuals[local_eqn]+=
          (poro_veloc[i]-fluid_veloc[i])*testf(l)*interpolated_r*W;

         //Jacobian entries
         if(flag)
          {
           // Loop over the velocity unknowns [derivs w.r.t. to
           // wall velocity taken care of by fd-ing
           for(unsigned l2=0;l2<n_node;l2++)
            {
             int local_unknown =
              nodal_local_eqn(l2,U_index_axisym_poroelastic_fsi[i]);

             /*IF it's not a boundary condition*/
             if(local_unknown>=0)
              {
               jacobian(local_eqn,local_unknown)-=
                psif[l2]*testf[l]*interpolated_r*W;
              }
            }
          }

        }

      }
    }
  }
}

}



#endif
