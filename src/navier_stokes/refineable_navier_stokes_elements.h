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
//Header file for refineable 2D quad Navier Stokes elements

#ifndef OOMPH_REFINEABLE_NAVIER_STOKES_ELEMENTS_HEADER
#define OOMPH_REFINEABLE_NAVIER_STOKES_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

//Oomph-lib headers
#include "../generic/refineable_quad_element.h"
#include "../generic/refineable_brick_element.h"
#include "../generic/error_estimator.h"
#include "navier_stokes_elements.h"

namespace oomph
{


//======================================================================
/// Refineable version of the Navier--Stokes equations
///
///
//======================================================================
template<unsigned DIM>
class RefineableNavierStokesEquations : 
public virtual NavierStokesEquations<DIM>,
public virtual RefineableElement,
public virtual ElementWithZ2ErrorEstimator
{
  protected:
 
 /// \short Pointer to n_p-th pressure node (Default: NULL, 
 /// indicating that pressure is not based on nodal interpolation).
 virtual Node* pressure_node_pt(const unsigned& n_p) {return NULL;}

 /// \short Unpin all pressure dofs in the element 
 virtual void unpin_elemental_pressure_dofs()=0;

 /// \short Pin unused nodal pressure dofs (empty by default, because
 /// by default pressure dofs are not associated with nodes)
 virtual void pin_elemental_redundant_nodal_pressure_dofs(){}
   
  public:
 
 /// \short Constructor
 RefineableNavierStokesEquations() : 
  NavierStokesEquations<DIM>(),
  RefineableElement(),
  ElementWithZ2ErrorEstimator() {}

 
 /// \short  Loop over all elements in Vector (which typically contains
 /// all the elements in a fluid mesh) and pin the nodal pressure degrees
 /// of freedom that are not being used. Function uses 
 /// the member function
 /// - \c RefineableNavierStokesEquations::
 ///      pin_elemental_redundant_nodal_pressure_dofs()
 /// .
 /// which is empty by default and should be implemented for
 /// elements with nodal pressure degrees of freedom  
 /// (e.g. for refineable Taylor-Hood.)
 static void pin_redundant_nodal_pressures(const Vector<GeneralisedElement*>&
                                           element_pt)
  {
   // Loop over all elements and call the function that pins their
   // unused nodal pressure data
   unsigned n_element = element_pt.size();
   for(unsigned e=0;e<n_element;e++)
    {
     dynamic_cast<RefineableNavierStokesEquations<DIM>*>(element_pt[e])->
      pin_elemental_redundant_nodal_pressure_dofs();
    }
  }

 /// \short Unpin all pressure dofs in elements listed in vector.
 static void unpin_all_pressure_dofs(const Vector<GeneralisedElement*>&
                                     element_pt)
  {
   // Loop over all elements
   unsigned n_element = element_pt.size();
   for(unsigned e=0;e<n_element;e++)
    {
     dynamic_cast<RefineableNavierStokesEquations<DIM>*>(element_pt[e])->
      unpin_elemental_pressure_dofs();
    }
  }

 
 /// Number of 'flux' terms for Z2 error estimation 
 unsigned num_Z2_flux_terms()
  {
   // DIM diagonal strain rates, DIM(DIM -1) /2 off diagonal rates
   return DIM + (DIM*(DIM-1))/2;
  }

 /// \short Get 'flux' for Z2 error recovery:   Upper triangular entries
 /// in strain rate tensor.
 void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
  {
#ifdef PARANOID
   unsigned num_entries=DIM+(DIM*(DIM-1))/2;
   if (flux.size()!=num_entries)
    {
     std::ostringstream error_message;
     error_message << "The flux vector has the wrong number of entries, " 
                   << flux.size() << ", whereas it should be " 
                   << num_entries << std::endl;
     throw OomphLibError(error_message.str(),
                         "RefineableNavierStokesEquations::get_Z2_flux()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   
   // Get strain rate matrix
   DenseMatrix<double> strainrate(DIM);
   this->strain_rate(s,strainrate);
   
   // Pack into flux Vector
   unsigned icount=0;
   
   // Start with diagonal terms
   for(unsigned i=0;i<DIM;i++)
    {
     flux[icount]=strainrate(i,i);
     icount++;
    }
   
   //Off diagonals row by row
   for(unsigned i=0;i<DIM;i++)
    {
     for(unsigned j=i+1;j<DIM;j++)
      {
       flux[icount]=strainrate(i,j);
       icount++;
      }
    }
  }

 ///  Further build, pass the pointers down to the sons
 void further_build()
  {
   //Find the father element
   RefineableNavierStokesEquations<DIM>* cast_father_element_pt
    = dynamic_cast<RefineableNavierStokesEquations<DIM>*>
    (this->father_element_pt());
   
   //Set the viscosity ratio pointer
   this->Viscosity_Ratio_pt = cast_father_element_pt->viscosity_ratio_pt(); 
   //Set the density ratio pointer
   this->Density_Ratio_pt = cast_father_element_pt->density_ratio_pt();
   //Set pointer to global Reynolds number
   this->Re_pt = cast_father_element_pt->re_pt();
   //Set pointer to global Reynolds number x Strouhal number (=Womersley)
   this->ReSt_pt = cast_father_element_pt->re_st_pt();
   //Set pointer to global Reynolds number x inverse Froude number
   this->ReInvFr_pt = cast_father_element_pt->re_invfr_pt();
   //Set pointer to global gravity Vector
   this->G_pt = cast_father_element_pt->g_pt();
   
   //Set pointer to body force function
   this->Body_force_fct_pt = cast_father_element_pt->body_force_fct_pt();
 
   //Set pointer to volumetric source function
   this->Source_fct_pt = cast_father_element_pt->source_fct_pt();

   //Set the ALE flag
   this->ALE_is_disabled = cast_father_element_pt->ALE_is_disabled;
  }

  protected:


/// \short Add element's contribution to elemental residual vector and/or 
/// Jacobian matrix 
/// flag=1: compute both
/// flag=0: compute only residual vector
 void fill_in_generic_residual_contribution_nst(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian, 
  DenseMatrix<double> &mass_matrix,
  unsigned flag); 

 /// \short Compute derivatives of elemental residual vector with respect
 /// to nodal coordinates. Overwrites default implementation in 
 /// FiniteElement base class.
 /// dresidual_dnodal_coordinates(l,i,j) = d res(l) / dX_{ij}
 virtual void get_dresidual_dnodal_coordinates(RankThreeTensor<double>&
                                               dresidual_dnodal_coordinates);
  
};


//======================================================================
/// Refineable version of Taylor Hood elements. These classes
/// can be written in total generality.
//======================================================================
template<unsigned DIM>
class RefineableQTaylorHoodElement : 
public QTaylorHoodElement<DIM>,
public virtual RefineableNavierStokesEquations<DIM>,
public virtual RefineableQElement<DIM>
{
  private:
  
 /// \short Pointer to n_p-th pressure node
 Node* pressure_node_pt(const unsigned &n_p)
  {return this->node_pt(this->Pconv[n_p]);}

 /// Unpin all pressure dofs
 void unpin_elemental_pressure_dofs()
  {
   //find the index at which the pressure is stored
   int p_index = this->p_nodal_index_nst();
   unsigned n_node = this->nnode();
   // loop over nodes
   for(unsigned n=0;n<n_node;n++) 
    {this->node_pt(n)->unpin(p_index);}
  }
 
 ///  Pin all nodal pressure dofs that are not required
 void pin_elemental_redundant_nodal_pressure_dofs()
  {
   //Find the pressure index
   int p_index = this->p_nodal_index_nst();
   //Loop over all nodes
   unsigned n_node = this->nnode();
   // loop over all nodes and pin all  the nodal pressures
   for(unsigned n=0;n<n_node;n++) {this->node_pt(n)->pin(p_index);}
   
   // Loop over all actual pressure nodes and unpin if they're not hanging
   unsigned n_pres = this->npres_nst();
   for(unsigned l=0;l<n_pres;l++)
    {
     Node* nod_pt = this->pressure_node_pt(l);
     if (!nod_pt->is_hanging(p_index)) {nod_pt->unpin(p_index);}
    }
  }
 
  public:
 
 /// \short Constructor
 RefineableQTaylorHoodElement() : 
  RefineableElement(),
  RefineableNavierStokesEquations<DIM>(),
  RefineableQElement<DIM>(), 
  QTaylorHoodElement<DIM>() {}
 
 /// \short Number of values required at local node n. In order to simplify
 /// matters, we allocate storage for pressure variables at all the nodes
 /// and then pin those that are not used.
 unsigned required_nvalue(const unsigned &n) const {return DIM+1;}

 /// Number of continuously interpolated values: (DIM velocities + 1 pressure)
 unsigned ncont_interpolated_values() const {return DIM+1;}

 /// Rebuild from sons: empty
 void rebuild_from_sons(Mesh* &mesh_pt) {}

 /// \short Order of recovery shape functions for Z2 error estimation:
 /// Same order as shape functions.
 unsigned nrecovery_order() {return 2;}

 /// \short Number of vertex nodes in the element
 unsigned nvertex_node() const
  {return QTaylorHoodElement<DIM>::nvertex_node();}

 /// \short Pointer to the j-th vertex node in the element
 Node* vertex_node_pt(const unsigned& j) const
  {return QTaylorHoodElement<DIM>::vertex_node_pt(j);}

/// \short Get the function value u in Vector.
/// Note: Given the generality of the interface (this function
/// is usually called from black-box documentation or interpolation routines),
/// the values Vector sets its own size in here.
 void get_interpolated_values(const Vector<double>&s,  Vector<double>& values)
  {
   // Set size of Vector: u,v,p and initialise to zero
   values.resize(DIM+1,0.0);
   
   //Calculate velocities: values[0],...
   for(unsigned i=0;i<DIM;i++) {values[i] = this->interpolated_u_nst(s,i);}
   
   //Calculate pressure: values[DIM]
   values[DIM] = this->interpolated_p_nst(s);
  }
 
/// \short Get the function value u in Vector.
/// Note: Given the generality of the interface (this function
/// is usually called from black-box documentation or interpolation routines),
/// the values Vector sets its own size in here.
 void get_interpolated_values(const unsigned& t, const Vector<double>&s, 
                              Vector<double>& values)
  {
   // Set size of Vector: u,v,p
   values.resize(DIM+1);
   
   // Initialise
   for(unsigned i=0;i<DIM+1;i++) {values[i]=0.0;}
   
   //Find out how many nodes there are
   unsigned n_node = this->nnode();
   
   // Shape functions
   Shape psif(n_node);
   this->shape(s,psif);
   
   //Calculate velocities: values[0],...
   for(unsigned i=0;i<DIM;i++) 
    {
     //Get the index at which the i-th velocity is stored
     unsigned u_nodal_index = this->u_index_nst(i);
     for(unsigned l=0;l<n_node;l++) 
      {
       values[i] += this->nodal_value(t,l,u_nodal_index)*psif[l];
      } 
    }
   
   //Calculate pressure: values[DIM] 
   //(no history is carried in the pressure)
   values[DIM] = this->interpolated_p_nst(s);
  }
  
 ///  \short Perform additional hanging node procedures for variables
 /// that are not interpolated by all nodes. The pressures are stored 
 /// at the p_nodal_index_nst-th location in each node
 void further_setup_hanging_nodes()
  {
   this->setup_hang_for_value(this->p_nodal_index_nst());
  }

 /// \short The velocities are isoparametric and so the "nodes" interpolating
 /// the velocities are the geometric nodes. The pressure "nodes" are a 
 /// subset of the nodes, so when value_id==DIM, the n-th pressure
 /// node is returned.
 Node* interpolating_node_pt(const unsigned &n,
                             const int &value_id) 

  {
   //The only different nodes are the pressure nodes
   if(value_id==DIM) {return this->pressure_node_pt(n);}
   //The other variables are interpolated via the usual nodes
   else {return this->node_pt(n);}
  }

 /// \short The pressure nodes are the corner nodes, so when n_value==DIM,
 /// the fraction is the same as the 1d node number, 0 or 1.
 double local_one_d_fraction_of_interpolating_node(const unsigned &n1d,
                                                   const unsigned &i, 
                                                   const int &value_id)
  {
   if(value_id==DIM) 
    {
     //The pressure nodes are just located on the boundaries at 0 or 1
     return double(n1d); 
    }
   //Otherwise the velocity nodes are the same as the geometric ones
   else
    {
     return this->local_one_d_fraction_of_node(n1d,i);
    }
  }
 
 /// \short The velocity nodes are the same as the geometric nodes. The
 /// pressure nodes must be calculated by using the same methods as
 /// the geometric nodes, but by recalling that there are only two pressure
 /// nodes per edge.
 Node* get_interpolating_node_at_local_coordinate(const Vector<double> &s,   
                                                  const int &value_id)
  {
   //If we are calculating pressure nodes
   if(value_id==DIM)
    {
     //Storage for the index of the pressure node
     unsigned total_index=0;
     //The number of nodes along each 1d edge is 2.
     unsigned NNODE_1D = 2;
     //Storage for the index along each boundary
     Vector<int> index(DIM);
     //Loop over the coordinates
     for(unsigned i=0;i<DIM;i++)
      {
       //If we are at the lower limit, the index is zero
       if(s[i]==-1.0)
        {
         index[i] = 0;
        }
       //If we are at the upper limit, the index is the number of nodes minus 1
       else if(s[i] == 1.0)
        {
         index[i] = NNODE_1D-1;
        }
       //Otherwise, we have to calculate the index in general
       else
        {
         //For uniformly spaced nodes the 0th node number would be
         double float_index = 0.5*(1.0 + s[i])*(NNODE_1D-1);
         index[i] = int(float_index);
         //What is the excess. This should be safe because the
         //taking the integer part rounds down
         double excess = float_index - index[i];
         //If the excess is bigger than our tolerance there is no node,
         //return null
         if((excess > FiniteElement::Node_location_tolerance) && (
             (1.0 - excess) > FiniteElement::Node_location_tolerance))
          {
           return 0;
          }
        }
       ///Construct the general pressure index from the components.
       total_index += index[i]*
        static_cast<unsigned>(pow(static_cast<float>(NNODE_1D),
                                  static_cast<int>(i)));
      }
     //If we've got here we have a node, so let's return a pointer to it
     return this->pressure_node_pt(total_index);
    }
   //Otherwise velocity nodes are the same as pressure nodes
   else
    {
     return this->get_node_at_local_coordinate(s);
    }
  }


 /// \short The number of 1d pressure nodes is 2, the number of 1d velocity
 /// nodes is the same as the number of 1d geometric nodes.
 unsigned ninterpolating_node_1d(const int &value_id)
  {
   if(value_id==DIM) {return 2;}
   else {return this->nnode_1d();}
  }

 /// \short The number of pressure nodes is 2^DIM. The number of 
 /// velocity nodes is the same as the number of geometric nodes.
 unsigned ninterpolating_node(const int &value_id)
  {
   if(value_id==DIM) 
    {return static_cast<unsigned>(pow(2.0,static_cast<int>(DIM)));}
   else {return this->nnode();}
  }
 
 /// \short The basis interpolating the pressure is given by pshape().
 //// The basis interpolating the velocity is shape().
 void interpolating_basis(const Vector<double> &s,
                          Shape &psi,
                          const int &value_id) const
  {
   if(value_id==DIM) {return this->pshape_nst(s,psi);}
   else {return this->shape(s,psi);}
  }


 /// \short Add to the set \c paired_load_data pairs containing
 /// - the pointer to a Data object
 /// and
 /// - the index of the value in that Data object
 /// .
 /// for all values (pressures, velocities) that affect the
 /// load computed in the \c get_load(...) function.
 /// (Overloads non-refineable version and takes hanging nodes
 /// into account)
 void identify_load_data(
  std::set<std::pair<Data*,unsigned> > &paired_load_data)
  {
   //Get the nodal indices at which the velocities are stored
   unsigned u_index[DIM];
   for(unsigned i=0;i<DIM;i++) {u_index[i] = this->u_index_nst(i);}

   //Loop over the nodes
   unsigned n_node = this->nnode();
   for(unsigned n=0;n<n_node;n++)
    {
     // Pointer to current node
     Node* nod_pt=this->node_pt(n);
     
     // Check if it's hanging:
     if (nod_pt->is_hanging())
      {
       // It's hanging -- get number of master nodes
       unsigned nmaster=nod_pt->hanging_pt()->nmaster();
       
       // Loop over masters
       for (unsigned j=0;j<nmaster;j++)
        {
         Node* master_nod_pt=nod_pt->hanging_pt()->master_node_pt(j);
         
         //Loop over the velocity components and add pointer to their data
         //and indices to the vectors
         for(unsigned i=0;i<DIM;i++)
          {
           paired_load_data.insert(std::make_pair(master_nod_pt,u_index[i]));
          }
        }
      }
     //Not hanging
     else
      {
       //Loop over the velocity components and add pointer to their data
       //and indices to the vectors
       for(unsigned i=0;i<DIM;i++)
        {
         paired_load_data.insert(std::make_pair(this->node_pt(n),u_index[i]));
        }
      }
    }
   
   //Get the nodal index at which the pressure is stored
   int p_index = this->p_nodal_index_nst();
   
   //Loop over the pressure data
   unsigned n_pres= this->npres_nst();
   for(unsigned l=0;l<n_pres;l++)
    {
     //Get the pointer to the nodal pressure
     Node* pres_node_pt = this->pressure_node_pt(l);
     // Check if the pressure dof is hanging
     if(pres_node_pt->is_hanging(p_index))
      {
       //Get the pointer to the hang info object
       // (pressure is stored as p_index--th nodal dof).
       HangInfo* hang_info_pt = pres_node_pt->hanging_pt(p_index);

       // Get number of pressure master nodes (pressure is stored  
       unsigned nmaster = hang_info_pt->nmaster();
       
       // Loop over pressure master nodes
       for(unsigned m=0;m<nmaster;m++)
        {
         //The p_index-th entry in each nodal data is the pressure, which
         //affects the traction
         paired_load_data.insert(
          std::make_pair(hang_info_pt->master_node_pt(m),p_index));
        }
      }
     // It's not hanging
     else
      {
       //The p_index-th entry in each nodal data is the pressure, which
       //affects the traction
       paired_load_data.insert(std::make_pair(pres_node_pt,p_index));
      }
    }
   
  }


};


//=======================================================================
/// \short Face geometry of the RefineableQTaylorHoodElements is the
/// same as the Face geometry of the QTaylorHoodElements.
//=======================================================================
template<unsigned DIM>
class FaceGeometry<RefineableQTaylorHoodElement<DIM> >: 
public virtual FaceGeometry<QTaylorHoodElement<DIM> >
{
  public:
 FaceGeometry() : FaceGeometry<QTaylorHoodElement<DIM> >() {}
};


//=======================================================================
/// \short Face geometry of the face geometry of 
/// the RefineableQTaylorHoodElements is the
/// same as the Face geometry of the Face geometry of QTaylorHoodElements.
//=======================================================================
template<unsigned DIM>
class FaceGeometry<FaceGeometry<RefineableQTaylorHoodElement<DIM> > >: 
public virtual FaceGeometry<FaceGeometry<QTaylorHoodElement<DIM> > >
{
  public:
 FaceGeometry() : FaceGeometry<FaceGeometry<QTaylorHoodElement<DIM> > >() 
  {}
};


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//======================================================================
/// Refineable version of Crouzeix Raviart elements. Generic class definitions
//======================================================================
template<unsigned DIM>
class RefineableQCrouzeixRaviartElement :
public QCrouzeixRaviartElement<DIM>, 
public virtual RefineableNavierStokesEquations<DIM>,
public virtual RefineableQElement<DIM> 
{
  private:
 
 /// Unpin all internal pressure dofs
 void unpin_elemental_pressure_dofs()
  {
   unsigned n_pres = this->npres_nst();
   // loop over pressure dofs and unpin them
   for(unsigned l=0;l<n_pres;l++) 
    {this->internal_data_pt(this->P_nst_internal_index)->unpin(l);}
  }

  public:
 
 /// \short Constructor
 RefineableQCrouzeixRaviartElement() : 
  RefineableElement(),
  RefineableNavierStokesEquations<DIM>(),
  RefineableQElement<DIM>(),  
  QCrouzeixRaviartElement<DIM>() {}
 
 /// Number of continuously interpolated values: DIM (velocities)
 unsigned ncont_interpolated_values() const {return DIM;}
 
 /// \short Rebuild from sons: Reconstruct pressure from the (merged) sons
 /// This must be specialised for each dimension.
 inline void rebuild_from_sons(Mesh* &mesh_pt);

 /// \short Order of recovery shape functions for Z2 error estimation:
 /// Same order as shape functions.
 unsigned nrecovery_order() {return 2;}

 /// \short Number of vertex nodes in the element
 unsigned nvertex_node() const
  {return QCrouzeixRaviartElement<DIM>::nvertex_node();}

 /// \short Pointer to the j-th vertex node in the element
 Node* vertex_node_pt(const unsigned& j) const
  {
   return QCrouzeixRaviartElement<DIM>::vertex_node_pt(j);
  }

/// \short Get the function value u in Vector.
/// Note: Given the generality of the interface (this function
/// is usually called from black-box documentation or interpolation routines),
/// the values Vector sets its own size in here.
void get_interpolated_values(const Vector<double>&s,  Vector<double>& values)
 {
  // Set size of Vector: u,v,p and initialise to zero
  values.resize(DIM,0.0);
  
  //Calculate velocities: values[0],...
  for(unsigned i=0;i<DIM;i++) {values[i] = this->interpolated_u_nst(s,i);}
 }

 /// \short Get all function values [u,v..,p] at previous timestep t
 /// (t=0: present; t>0: previous timestep). 
 /// \n 
 /// Note: Given the generality of the interface (this function
 /// is usually called from black-box documentation or interpolation routines),
 /// the values Vector sets its own size in here.
 /// \n
 /// Note: No pressure history is kept, so pressure is always
 /// the current value.
 void get_interpolated_values(const unsigned& t, const Vector<double>&s, 
                              Vector<double>& values)
  {
   // Set size of Vector: u,v,p
   values.resize(DIM);

   // Initialise
   for(unsigned i=0;i<DIM;i++) {values[i]=0.0;}
   
   //Find out how many nodes there are
   unsigned n_node = this->nnode();
   
   // Shape functions
   Shape psif(n_node);
   this->shape(s,psif);
   
   //Calculate velocities: values[0],...
   for(unsigned i=0;i<DIM;i++) 
    {
     //Get the nodal index at which the i-th velocity component is stored
     unsigned u_nodal_index = this->u_index_nst(i);
     for(unsigned l=0;l<n_node;l++) 
      {
       values[i] += this->nodal_value(t,l,u_nodal_index)*psif[l];
      } 
    }
  }
 
 ///  \short Perform additional hanging node procedures for variables
 /// that are not interpolated by all nodes. Empty
 void further_setup_hanging_nodes() {}

 /// Further build for Crouzeix_Raviart interpolates the internal 
 /// pressure dofs from father element: Make sure pressure values and 
 /// dp/ds agree between fathers and sons at the midpoints of the son 
 /// elements. This must be specialised for each dimension.
 inline void further_build();



 /// \short Add to the set \c paired_load_data pairs containing
 /// - the pointer to a Data object
 /// and
 /// - the index of the value in that Data object
 /// .
 /// for all values (pressures, velocities) that affect the
 /// load computed in the \c get_load(...) function.
 /// (Overloads non-refineable version and takes hanging nodes
 /// into account)
 void identify_load_data(
  std::set<std::pair<Data*,unsigned> > &paired_load_data)
  {
   //Get the nodal indices at which the velocities are stored
   unsigned u_index[DIM];
   for(unsigned i=0;i<DIM;i++) {u_index[i] = this->u_index_nst(i);}

   //Loop over the nodes
   unsigned n_node = this->nnode();
   for(unsigned n=0;n<n_node;n++)
    {
     // Pointer to current node
     Node* nod_pt=this->node_pt(n);
     
     // Check if it's hanging:
     if (nod_pt->is_hanging())
      {
       // It's hanging -- get number of master nodes
       unsigned nmaster=nod_pt->hanging_pt()->nmaster();
       
       // Loop over masters
       for (unsigned j=0;j<nmaster;j++)
        {
         Node* master_nod_pt=nod_pt->hanging_pt()->master_node_pt(j);
         
         //Loop over the velocity components and add pointer to their data
         //and indices to the vectors
         for(unsigned i=0;i<DIM;i++)
          {
           paired_load_data.insert(std::make_pair(master_nod_pt,u_index[i]));
          }
        }
      }
     //Not hanging
     else
      {
       //Loop over the velocity components and add pointer to their data
       //and indices to the vectors
       for(unsigned i=0;i<DIM;i++)
        {
         paired_load_data.insert(std::make_pair(this->node_pt(n),u_index[i]));
        }
      }
    }
   

   //Loop over the pressure data (can't be hanging!)
   unsigned n_pres = this->npres_nst();
   for(unsigned l=0;l<n_pres;l++)
    {
     //The entries in the internal data at P_nst_internal_index
     //are the pressures, which affect the traction
     paired_load_data.insert(
      std::make_pair(this->internal_data_pt(this->P_nst_internal_index),l));
    }
  }


};


//=======================================================================
/// Face geometry of the RefineableQuadQCrouzeixRaviartElements
//=======================================================================
template<unsigned DIM>
class FaceGeometry<RefineableQCrouzeixRaviartElement<DIM> >: 
public virtual FaceGeometry<QCrouzeixRaviartElement<DIM> >
{
  public:
 FaceGeometry() : FaceGeometry<QCrouzeixRaviartElement<DIM> >() {}
};

//======================================================================
/// \short Face geometry of the face geometry of 
/// the RefineableQCrouzeixRaviartElements is the
/// same as the Face geometry of the Face geometry of 
/// QCrouzeixRaviartElements.
//=======================================================================
template<unsigned DIM>
class FaceGeometry<FaceGeometry<RefineableQCrouzeixRaviartElement<DIM> > >: 
public virtual FaceGeometry<FaceGeometry<QCrouzeixRaviartElement<DIM> > >
{
  public:
 FaceGeometry() : FaceGeometry<FaceGeometry<QCrouzeixRaviartElement<DIM> > >() 
  {}
};


//=====================================================================
/// 2D Rebuild from sons: Reconstruct pressure from the (merged) sons
//=====================================================================
template<>
inline void RefineableQCrouzeixRaviartElement<2>::
rebuild_from_sons(Mesh* &mesh_pt)
{
 using namespace QuadTreeNames;
 
 //Central pressure value: 
 //-----------------------
 
 // Use average of the sons central pressure values
 // Other options: Take average of the four (discontinuous)
 // pressure values at the father's midpoint]
 
 double av_press=0.0;
 
 // Loop over the sons
 for (unsigned ison=0;ison<4;ison++)
  {
   // Add the sons midnode pressure
   // Note that we can assume that the pressure is stored at the same
   // location because these are EXACTLY the same type of elements
   av_press += quadtree_pt()->son_pt(ison)->object_pt()->
    internal_data_pt(this->P_nst_internal_index)->value(0);
  }
 
 // Use the average
 internal_data_pt(this->P_nst_internal_index)->set_value(0,0.25*av_press);
 
 
 //Slope in s_0 direction
 //----------------------
 
 // Use average of the 2 FD approximations based on the 
 // elements central pressure values
 // [Other options: Take average of the four 
 // pressure derivatives]
 
 double slope1= 
  quadtree_pt()->son_pt(SE)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  quadtree_pt()->son_pt(SW)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 double slope2 = 
  quadtree_pt()->son_pt(NE)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  quadtree_pt()->son_pt(NW)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 
 // Use the average
 internal_data_pt(this->P_nst_internal_index)->
  set_value(1,0.5*(slope1+slope2));
 
 

 //Slope in s_1 direction
 //----------------------
 
   // Use average of the 2 FD approximations based on the 
   // elements central pressure values
   // [Other options: Take average of the four 
   // pressure derivatives]
 
 slope1 = 
  quadtree_pt()->son_pt(NE)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  quadtree_pt()->son_pt(SE)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);
 
 slope2=
  quadtree_pt()->son_pt(NW)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  quadtree_pt()->son_pt(SW)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);


 // Use the average
 internal_data_pt(this->P_nst_internal_index)->
  set_value(2,0.5*(slope1+slope2));
}


 
//=================================================================
/// 3D Rebuild from sons: Reconstruct pressure from the (merged) sons
//=================================================================
template<>
inline void RefineableQCrouzeixRaviartElement<3>::
rebuild_from_sons(Mesh* &mesh_pt)
{
 using namespace OcTreeNames;
 
 //Central pressure value: 
 //-----------------------
 
 // Use average of the sons central pressure values
 // Other options: Take average of the four (discontinuous)
 // pressure values at the father's midpoint]
 
 double av_press=0.0;
 
 // Loop over the sons
 for (unsigned ison=0;ison<8;ison++)
  {
   // Add the sons midnode pressure
   av_press += octree_pt()->son_pt(ison)->object_pt()->
    internal_data_pt(this->P_nst_internal_index)->value(0);
  }
 
 // Use the average
 internal_data_pt(this->P_nst_internal_index)->
  set_value(0,0.125*av_press);
 
 
 //Slope in s_0 direction
 //----------------------
 
 // Use average of the 4 FD approximations based on the 
 // elements central pressure values
 // [Other options: Take average of the four 
 // pressure derivatives]
 
 double slope1 = 
  octree_pt()->son_pt(RDF)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LDF)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 double slope2 =
  octree_pt()->son_pt(RUF)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LUF)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 double slope3 =
  octree_pt()->son_pt(RDB)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LDB)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 double slope4 = 
  octree_pt()->son_pt(RUB)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LUB)->object_pt()->
  internal_data_pt(this->P_nst_internal_index)->value(0);
 
 
 // Use the average
 internal_data_pt(this->P_nst_internal_index)->
  set_value(1,0.25*(slope1+slope2+slope3+slope4));
 
 
   //Slope in s_1 direction
   //----------------------
 
   // Use average of the 4 FD approximations based on the 
   // elements central pressure values
   // [Other options: Take average of the four 
   // pressure derivatives]
 
 slope1 = 
  octree_pt()->son_pt(LUB)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LDB)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);
 
 slope2 = 
  octree_pt()->son_pt(RUB)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(RDB)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);
   
 slope3 = 
  octree_pt()->son_pt(LUF)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(LDF)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);

 slope4 = 
  octree_pt()->son_pt(RUF)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0) -
  octree_pt()->son_pt(RDF)->object_pt()
  ->internal_data_pt(this->P_nst_internal_index)->value(0);


   // Use the average
 internal_data_pt(this->P_nst_internal_index)->
  set_value(2,0.25*(slope1+slope2+slope3+slope4));
   

   //Slope in s_2 direction
   //----------------------

   // Use average of the 4 FD approximations based on the 
   // elements central pressure values
   // [Other options: Take average of the four 
   // pressure derivatives]

   slope1 =
    octree_pt()->son_pt(LUF)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0) -
    octree_pt()->son_pt(LUB)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0);

   slope2 =
    octree_pt()->son_pt(RUF)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0) -
    octree_pt()->son_pt(RUB)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0);
   
   slope3 =
    octree_pt()->son_pt(LDF)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0) -
    octree_pt()->son_pt(LDB)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0);
   
   slope4 =
    octree_pt()->son_pt(RDF)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0) -
    octree_pt()->son_pt(RDB)->object_pt()
    ->internal_data_pt(this->P_nst_internal_index)->value(0);

   // Use the average
   internal_data_pt(this->P_nst_internal_index)->
    set_value(3,0.25*(slope1+slope2+slope3+slope4));

}


//======================================================================
/// 2D Further build for Crouzeix_Raviart interpolates the internal 
/// pressure dofs from father element: Make sure pressure values and 
/// dp/ds agree between fathers and sons at the midpoints of the son 
/// elements.
//======================================================================
template<>
inline void RefineableQCrouzeixRaviartElement<2>::further_build()
{ 
 //Call the generic further build
 RefineableNavierStokesEquations<2>::further_build();
 
 using namespace QuadTreeNames;
 
 // What type of son am I? Ask my quadtree representation...
 int son_type=quadtree_pt()->son_type();
 
 // Pointer to my father (in element impersonation)
 RefineableElement* father_el_pt= quadtree_pt()->father_pt()->object_pt();
 
 Vector<double> s_father(2);
 
 // Son midpoint is located at the following coordinates in father element:
 
 // South west son
 if (son_type==SW)
  {
   s_father[0]=-0.5;
   s_father[1]=-0.5;
  }
 // South east son
 else if (son_type==SE)
  {
   s_father[0]= 0.5;
   s_father[1]=-0.5;
  }
 // North east son
 else if (son_type==NE)
  {
   s_father[0]=0.5;
   s_father[1]=0.5;
  }
 
 // North west son
 else if (son_type==NW)
  {
   s_father[0]=-0.5;
   s_father[1]= 0.5;
  }
 
 // Pressure value in father element
 RefineableQCrouzeixRaviartElement<2>* cast_father_element_pt=
  dynamic_cast<RefineableQCrouzeixRaviartElement<2>*>(father_el_pt);
 
 double press=cast_father_element_pt->interpolated_p_nst(s_father);
 
 // Pressure value gets copied straight into internal dof:
 internal_data_pt(this->P_nst_internal_index)->set_value(0,press);
  
 // The slopes get copied from father
 for(unsigned i=1;i<3;i++)
  {
   double half_father_slope = 0.5*cast_father_element_pt->
    internal_data_pt(this->P_nst_internal_index)->value(i);
   //Set the value in the son
   internal_data_pt(this->P_nst_internal_index)->
    set_value(i,half_father_slope);
  }
}


//=======================================================================
/// 3D Further build for Crouzeix_Raviart interpolates the internal 
/// pressure dofs from father element: Make sure pressure values and 
/// dp/ds agree between fathers and sons at the midpoints of the son 
/// elements.
//=======================================================================
template<>
inline void RefineableQCrouzeixRaviartElement<3>::further_build()
{ 
 RefineableNavierStokesEquations<3>::further_build();
 
 using namespace OcTreeNames;
 
 // What type of son am I? Ask my octree representation...
 int son_type=octree_pt()->son_type();
 
 // Pointer to my father (in element impersonation)
 RefineableQElement<3>* father_el_pt=
  dynamic_cast<RefineableQElement<3>*>(
   octree_pt()->father_pt()->object_pt());
 
 Vector<double> s_father(3);
 
 // Son midpoint is located at the following coordinates in father element:
 for(unsigned i=0;i<3;i++)
  {
   s_father[i]=0.5*OcTree::Direction_to_vector[son_type][i];
  }
 
 // Pressure value in father element
 RefineableQCrouzeixRaviartElement<3>* cast_father_element_pt=
  dynamic_cast<RefineableQCrouzeixRaviartElement<3>*>(father_el_pt);
 
 double press=cast_father_element_pt->interpolated_p_nst(s_father);
 
 // Pressure value gets copied straight into internal dof:
 internal_data_pt(this->P_nst_internal_index)->set_value(0,press);
 
 // The slopes get copied from father
 for(unsigned i=1;i<4;i++)
  {
   double half_father_slope = 0.5*cast_father_element_pt
    ->internal_data_pt(this->P_nst_internal_index)->value(i);
   //Set the value
   internal_data_pt(this->P_nst_internal_index)
    ->set_value(i,half_father_slope);
  }
}

}

#endif
