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
//Header functions for classes that define Qelements

//Include guards to prevent multiple inclusion of the header
#ifndef OOMPH_QSPECTRAL_ELEMENT_HEADER
#define OOMPH_QSPECTRAL_ELEMENT_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//oomph-lib headers
#include "Qelements.h"

namespace oomph
{

//=====================================================================
/// Class that returns the shape functions associated with legendre
//=====================================================================
class OneDLegendreShapeParam : public Shape
{
  public:
 static std::map<unsigned,Vector<double> > z;
 
 /// Static function used to populate the stored positions
 static inline void calculate_nodal_positions(const unsigned &order)
  {
   //If we haven't already calculated these
   if(z.find(order)==z.end()) 
    {
     Orthpoly::gll_nodes(order,z[order]);
    }
  }

 static inline double nodal_position(const unsigned &order, const unsigned &n)
  {return z[order][n];}

 /// Constructor
 OneDLegendreShapeParam(const unsigned &order,const double &s) : 
  Shape(order)
  {
   using namespace Orthpoly;
   
   unsigned p = order-1;
   //Now populate the shape function
   for(unsigned i=0;i<order;i++)
    {
     //If we're at one of the nodes, the value must be 1.0
     if(std::fabs(s-z[order][i]) < Orthpoly::eps)
      {
       (*this)[i] = 1.0;
      }
     //Otherwise use the lagrangian interpolant
     else
      {
       (*this)[i] = (1.0 - s*s)*dlegendre(p,s)/
        (p*(p+1)*legendre(p,z[order][i])*(z[order][i] - s));
      }
    }
  }
};


class OneDLegendreDShapeParam : public Shape
{
  public:
 // Constructor 
 OneDLegendreDShapeParam(const unsigned &order, const double &s) : 
  Shape(order)
  {
   unsigned p = order - 1;
   Vector <double> z = OneDLegendreShapeParam::z[order];
   
   bool root=false;
   for(unsigned i=0;i<order;i++)
    {
     unsigned rootnum=0;
     for(unsigned j=0;j<order;j++)
      {     // Loop over roots to check if
       if(std::fabs(s-z[j])<10.0*Orthpoly::eps)
        { // s happens to be a root.
         root=true;
         break;
        }
       rootnum+=1;
      }
     if(root==true)
      {
       if(i==rootnum && i==0)
        {
         (*this)[i]=-(1.0+p)*p/4.0;
        }
       else if(i==rootnum && i==p)
        {
         (*this)[i]=(1.0+p)*p/4.0;
        }
       else if(i==rootnum)
        {
         (*this)[i]=0.0;
        }
       else
        {
         (*this)[i]=Orthpoly::legendre(p,z[rootnum])
          /Orthpoly::legendre(p,z[i])/(z[rootnum]-z[i]);
        }
      }
     else
      {
       (*this)[i]=((1+s*(s-2*z[i]))/(s-z[i])* Orthpoly::dlegendre(p,s)
                   -(1-s*s)* Orthpoly::ddlegendre(p,s))
        /p/(p+1.0)/Orthpoly::legendre(p,z[i])/(s-z[i]);
      }
     root = false;
    }
   
   
  }

};



//========================================================================
// A Base class for Spectral elements
//========================================================================
class SpectralElement : public virtual FiniteElement
{
  protected:

 /// Additional Storage for shared spectral data
 Vector<Data*> *Spectral_data_pt;

 /// Vector that represents the spectral order in each dimension
 Vector<unsigned> Spectral_order;
 
 /// Vector that represents the nodal spectral order
 Vector<unsigned> Nodal_spectral_order;

 /// Local equation numbers for the spectral degrees of freedom
 DenseMatrix<int> Spectral_local_eqn;

  public:
 
 SpectralElement() : FiniteElement(), Spectral_data_pt(0) {}

 virtual ~SpectralElement()
  {
   if(Spectral_data_pt!=0) 
    {
     delete Spectral_data_pt;
     Spectral_data_pt=0;
    }
  }


 ///\short Return the i-th data object associated with the polynomials
 ///of order p. Note that i <= p.
 Data* spectral_data_pt(const unsigned &i) const
  {return (*Spectral_data_pt)[i];}

 /// \short Function to describe the local dofs of the element. The ostream 
 /// specifies the output stream to which the description 
 /// is written; the string stores the currently 
 /// assembled output that is ultimately written to the
 /// output stream by Data::describe_dofs(...); it is typically
 /// built up incrementally as we descend through the
 /// call hierarchy of this function when called from 
 /// Problem::describe_dofs(...)
 virtual void describe_local_dofs(std::ostream& out,
                                  const std::string& current_string) const
  {
   //Do the standard finite element stuff
   FiniteElement::describe_local_dofs(out,current_string);
   //Now get the number of spectral data.
   unsigned n_spectral = nspectral();
   
   //Now loop over the nodes again and assign local equation numbers
   for(unsigned n=0;n<n_spectral;n++)
    {
     //Can we upcast to a node
     Node* cast_node_pt = dynamic_cast<Node*>(spectral_data_pt(n));
     if(cast_node_pt)
      {
       std::stringstream conversion;
       conversion <<" of Node "<<n<<current_string;
       std::string in(conversion.str());
       cast_node_pt->describe_dofs(out,in);
      }
     // Otherwise it is data.
     else
      {
       Data* data_pt = spectral_data_pt(n);
       std::stringstream conversion;
       conversion <<" of Data "<<n<<current_string;
       std::string in(conversion.str());
       data_pt->describe_dofs(out,in);
      }
    }
  }

 /// \short Assign the local equation numbers. If the boolean argument is
 /// true then store degrees of freedom at Dof_pt
 virtual void assign_all_generic_local_eqn_numbers(
  const bool &store_local_dof_pt)
  {
   //Do the standard finite element stuff
   FiniteElement::assign_all_generic_local_eqn_numbers(store_local_dof_pt);
  
   //Now need to loop over the spectral data 
   unsigned n_spectral = nspectral();
   if(n_spectral > 0)
    {
     //Find the number of local equations assigned so far
     unsigned local_eqn_number = ndof();
     
     //Find number of values stored at the first node
     unsigned max_n_value = spectral_data_pt(0)->nvalue();
     //Loop over the other nodes and find the maximum number of values stored
     for(unsigned n=1;n<n_spectral;n++)
      {
       unsigned n_value = spectral_data_pt(n)->nvalue();
       if(n_value > max_n_value) {max_n_value = n_value;}
      }
     
     //Resize the storage for the nodal local equation numbers
     //initially all local equations are unclassified
     Spectral_local_eqn.resize(n_spectral,max_n_value,Data::Is_unclassified);
     
     //Construct a set of pointers to the nodes
     std::set<Data*> set_of_node_pt;
     unsigned n_node = nnode();
     for(unsigned n=0;n<n_node;n++) {set_of_node_pt.insert(node_pt(n));}
     
    //A local queue to store the global equation numbers
    std::deque<unsigned long> global_eqn_number_queue;

     //Now loop over the nodes again and assign local equation numbers
     for(unsigned n=0;n<n_spectral;n++)
      {
       //Need to find whether the data is, in fact a node 
       //(and hence already assigned)
       
       //Can we upcast to a node
       Node* cast_node_pt = dynamic_cast<Node*>(spectral_data_pt(n));
       if((cast_node_pt) && 
          (set_of_node_pt.find(cast_node_pt)!=set_of_node_pt.end()))
        {
         //Find the number of values
         unsigned n_value = cast_node_pt->nvalue();
         //Copy them across
         for(unsigned j=0;j<n_value;j++)
          {
           Spectral_local_eqn(n,j) = 
            nodal_local_eqn(get_node_number(cast_node_pt),j);
          }
         //Remove from the set
         set_of_node_pt.erase(cast_node_pt);
        }
       //Otherwise it's just data
       else
        {
         Data* const data_pt = spectral_data_pt(n);
         unsigned n_value = data_pt->nvalue();
         //Loop over the number of values
         for(unsigned j=0;j<n_value;j++)
          {
           //Get the GLOBAL equation number
           long eqn_number = data_pt->eqn_number(j);
           //If the GLOBAL equation number is positive (a free variable)
           if(eqn_number >= 0)
            {
             //Add the GLOBAL equation number to the 
             //local-to-global translation
             //scheme
             global_eqn_number_queue.push_back(eqn_number);
            //Add pointer to the dof to the queue if required
             if(store_local_dof_pt)
              {
               GeneralisedElement::Dof_pt_deque.push_back(
                data_pt->value_pt(j));
              }
             //Add the local equation number to the local scheme
             Spectral_local_eqn(n,j) = local_eqn_number;
             //Increase the local number
             local_eqn_number++;
            }
           else
            {
             //Set the local scheme to be pinned
             Spectral_local_eqn(n,j) = Data::Is_pinned;
            }
          }
        } //End of case when it's not a nodal dof
      }

     //Now add our global equations numbers to the internal element storage
     add_global_eqn_numbers(global_eqn_number_queue,
                            GeneralisedElement::Dof_pt_deque);
     //Clear the memory used in the deque
     if(store_local_dof_pt)
      {std::deque<double*>().swap(GeneralisedElement::Dof_pt_deque);}
     
    } //End of case when there are spectral degrees of freedom
  }

   unsigned nspectral() const
    {
     if(Spectral_data_pt==0) {return 0;}
     else
      {
       return Spectral_data_pt->size();
      }
    }

};



//=======================================================================
///General QLegendreElement class
///
/// Empty, just establishes the template parameters
//=======================================================================
template<unsigned DIM, unsigned NNODE_1D> 
                            class QSpectralElement 
{
};


//=======================================================================
///General QSpectralElement class specialised to one spatial dimension
//=======================================================================
template<unsigned NNODE_1D>
class QSpectralElement<1,NNODE_1D> : public virtual SpectralElement,
 public virtual LineElementBase
{
  private:
 
 /// \short Default integration rule: Gaussian integration of same 'order' 
 /// as the element
 //This is sort of optimal, because it means that the integration is exact
 //for the shape functions. Can overwrite this in specific element defintion.
 static GaussLobattoLegendre<1,NNODE_1D> integral;

public:

  /// Constructor
  QSpectralElement() 
  {
   //There are NNODE_1D nodes in this element
   this->set_n_node(NNODE_1D);
   Spectral_order.resize(1,NNODE_1D);
   Nodal_spectral_order.resize(1,NNODE_1D);
   //Set the elemental and nodal dimensions
   this->set_dimension(1);
   //Assign integral point
   this->set_integration_scheme(&integral);
   //Calculate the nodal positions for the shape functions
   OneDimensionalLegendreShape<NNODE_1D>::calculate_nodal_positions();
   //OneDLegendreShapeParam::calculate_nodal_positions(NNODE_1D);
  }

 /// Min. value of local coordinate
 double s_min() const {return -1.0;}

 /// Max. value of local coordinate
 double s_max() const {return 1.0;}

 /// Number of vertex nodes in the element
 unsigned nvertex_node() const
  { return 2; }

 /// Pointer to the j-th vertex node in the element
 Node* vertex_node_pt(const unsigned &j) const
  {
   unsigned n_node_1d = nnode_1d();
   Node* nod_pt;
   switch(j)
    {
    case 0:
     nod_pt = this->node_pt(0);
     break;
    case 1:
     nod_pt = this->node_pt(n_node_1d-1);
     break;
    default:
     std::ostringstream error_message;
     error_message << "Vertex node number is " << j << 
      " but must be from 0 to 1\n";

     throw OomphLibError(error_message.str(),
                         OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
   return nod_pt;
  }

 /// Get local coordinates of node j in the element; vector sets its own size
 void local_coordinate_of_node(const unsigned& n, Vector<double>& s)
  {
   s.resize(1);
   s[0] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n);
  }

 /// Get the local fractino of node j in the element
 void local_fraction_of_node(const unsigned &n, Vector<double> &s_fraction)
  {
   this->local_coordinate_of_node(n,s_fraction);
   //Resize
   s_fraction[0] = 0.5*(s_fraction[0] + 1.0);
  }

 /// The local one-d fraction is the same
 double local_one_d_fraction_of_node(const unsigned &n1d, const unsigned &i)
  {
   return 
    0.5*(OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n1d) + 1.0);
  }

 /// Calculate the geometric shape functions at local coordinate s
 inline void shape(const Vector<double> &s, Shape &psi) const;

 /// \short Compute the  geometric shape functions and 
 /// derivatives w.r.t. local coordinates at local coordinate s
 inline void dshape_local(const Vector<double> &s, Shape &psi, DShape &dpsids) 
  const;

 /// \short Compute the geometric shape functions, derivatives and
 /// second derivatives w.r.t. local coordinates at local coordinate s 
 /// d2psids(i,0) = \f$ d^2 \psi_j / d s^2 \f$
 inline void d2shape_local(const Vector<double> &s, Shape &psi, DShape &dpsids,
                           DShape &d2psids) const;

 /// \short Overload the template-free interface for the calculation of
 /// inverse jacobian matrix. This is a one-dimensional element, so
 /// use the 1D version.
 double invert_jacobian_mapping(const DenseMatrix<double> &jacobian,
                                DenseMatrix<double> &inverse_jacobian) const
  {return FiniteElement::invert_jacobian<1>(jacobian,inverse_jacobian);}
 
 
 /// Number of nodes along each element edge
 unsigned nnode_1d() const {return NNODE_1D;}

 /// C-style output 
 void output(FILE* file_pt)
  {FiniteElement::output(file_pt);}

 /// C_style output at n_plot points
 void output(FILE* file_pt, const unsigned &n_plot)
  {FiniteElement::output(file_pt,n_plot);}

 /// Output 
 void output(std::ostream &outfile);

 /// Output at n_plot points
 void output(std::ostream &outfile, const unsigned &n_plot);
 
/// \short  Get cector of local coordinates of plot point i (when plotting 
 /// nplot points in each "coordinate direction).
 void get_s_plot(const unsigned& i, const unsigned& nplot,
                 Vector<double>& s) const
  {
   if (nplot>1)
    {
     s[0]=-1.0+2.0*double(i)/double(nplot-1);
    }
   else
    {
     s[0]=0.0;
    }
  }
 
 /// \short Return string for tecplot zone header (when plotting 
 /// nplot points in each "coordinate direction)
 std::string tecplot_zone_string(const unsigned& nplot) const
  {
   std::ostringstream header;
   header << "ZONE I=" << nplot << "\n";
   return header.str();
  }
 
 /// \short Return total number of plot points (when plotting 
 /// nplot points in each "coordinate direction)
 unsigned nplot_points(const unsigned& nplot) const
  {
   unsigned DIM=1;
   unsigned np=1;
   for (unsigned i=0;i<DIM;i++) {np*=nplot;}
   return np;
  }

 /// \short Build the lower-dimensional FaceElement (an element of type
 /// QSpectralElement<0,NNODE_1D>). The face index takes two values
 /// corresponding to the two possible faces:
 /// -1 (Left)  s[0] = -1.0
 /// +1 (Right) s[0] =  1.0
 void build_face_element(const int &face_index, 
                         FaceElement* face_element_pt);
 
};


//=======================================================================
///Shape function for specific QSpectralElement<1,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<1,NNODE_1D>::shape(const Vector<double> &s, Shape &psi) 
 const
{
 //Call the OneDimensional Shape functions
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]);
 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]);

 //Now let's loop over the nodal points in the element
 //and copy the values back in  
 for(unsigned i=0;i<NNODE_1D;i++) {psi(i) = psi1[i];}
}

//=======================================================================
///Derivatives of shape functions for specific  QSpectralElement<1,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<1,NNODE_1D>::dshape_local(const Vector<double> &s, 
						Shape &psi, 
						DShape &dpsids) const
{
 //Call the shape functions and derivatives
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]);
 OneDimensionalLegendreDShape<NNODE_1D> dpsi1ds(s[0]);

 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]);
 //OneDLegendreDShapeParam dpsi1ds(NNODE_1D,s[0]);

 //Loop over shape functions in element and assign to psi
 for(unsigned l=0;l<NNODE_1D;l++) 
  {
   psi(l) = psi1[l];
   dpsids(l,0) = dpsi1ds[l];
  }
}

//=======================================================================
/// Second derivatives of shape functions for specific QSpectralElement<1,..>
///  d2psids(i,0) = \f$ d^2 \psi_j / d s^2 \f$
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<1,NNODE_1D>::d2shape_local(const Vector<double> &s, Shape &psi, 
                                         DShape &dpsids, DShape &d2psids) const
{
 std::ostringstream error_message;
 error_message <<"\nd2shpe_local currently not implemented for this element\n";
 throw OomphLibError(error_message.str(),
                     OOMPH_CURRENT_FUNCTION,
                     OOMPH_EXCEPTION_LOCATION);

/*  //Call the shape functions and derivatives */
/*  OneDimensionalShape<NNODE_1D> psi1(s[0]); */
/*  OneDimensionalDShape<NNODE_1D> dpsi1ds(s[0]); */
/*  OneDimensionalD2Shape<NNODE_1D> d2psi1ds(s[0]); */

/*  //Loop over shape functions in element and assign to psi */
/*  for(unsigned l=0;l<NNODE_1D;l++)  */
/*   { */
/*    psi[l] = psi1[l]; */
/*    dpsids[l][0] = dpsi1ds[l]; */
/*    d2psids[l][0] = d2psi1ds[l]; */
/*   } */
}


//=======================================================================
///General QSpectralElement class specialised to two spatial dimensions
//=======================================================================
template<unsigned NNODE_1D>
class QSpectralElement<2,NNODE_1D> : public virtual SpectralElement,
 public virtual QuadElementBase
{
  private:
 
 /// \short Default integration rule: Gaussian integration of same 'order' 
 /// as the element
 //This is sort of optimal, because it means that the integration is exact
 //for the shape functions. Can overwrite this in specific element defintion.
 static GaussLobattoLegendre<2,NNODE_1D> integral;

public:

  /// Constructor
  QSpectralElement() 
  {
   //There are NNODE_1D^2 nodes in this element
   this->set_n_node(NNODE_1D*NNODE_1D);
   Spectral_order.resize(2,NNODE_1D);
   Nodal_spectral_order.resize(2,NNODE_1D);
   //Set the elemental and nodal dimensions
   this->set_dimension(2);
   //Assign integral pointer
   this->set_integration_scheme(&integral);
   //Calculate the nodal positions for the shape functions
   OneDimensionalLegendreShape<NNODE_1D>::calculate_nodal_positions();
   //OneDLegendreShapeParam::calculate_nodal_positions(NNODE_1D);
  }

 /// Min. value of local coordinate
 double s_min() const {return -1.0;}

 /// Max. value of local coordinate
 double s_max() const {return 1.0;}

 /// Number of vertex nodes in the element
 unsigned nvertex_node() const
  { return 4;}

 /// Pointer to the j-th vertex node in the element
 Node* vertex_node_pt(const unsigned &j) const
  {
   unsigned n_node_1d = nnode_1d();
    Node* nod_pt;
   switch(j)
    {
    case 0:
     nod_pt = this->node_pt(0);
     break;
    case 1:
     nod_pt = this->node_pt(n_node_1d-1);
     break;
    case 2:
     nod_pt = this->node_pt(n_node_1d*(n_node_1d-1));
     break;
    case 3:
     nod_pt = this->node_pt(n_node_1d*n_node_1d-1);
     break;
    default:
     std::ostringstream error_message;
     error_message << "Vertex node number is " << j << 
      " but must be from 0 to 3\n";

     throw OomphLibError(error_message.str(),
OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
   return nod_pt;
  }
 

 /// Get local coordinates of node j in the element; vector sets its own size
 void local_coordinate_of_node(const unsigned& n, Vector<double>& s)
  {
   s.resize(2);
   unsigned n0 = n%NNODE_1D;
   unsigned n1 = unsigned(double(n)/double(NNODE_1D));
   s[0] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n0);
   s[1] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n1);
  }

 /// Get the local fractino of node j in the element
 void local_fraction_of_node(const unsigned &n, Vector<double> &s_fraction)
  {
   this->local_coordinate_of_node(n,s_fraction);
   //Resize
   s_fraction[0] = 0.5*(s_fraction[0] + 1.0);
   s_fraction[1] = 0.5*(s_fraction[1] + 1.0);
  }

 /// The local one-d fraction is the same
 double local_one_d_fraction_of_node(const unsigned &n1d, const unsigned &i)
  {
   return 
    0.5*(OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n1d) + 1.0);
  }

 /// Calculate the geometric shape functions at local coordinate s
 inline void shape(const Vector<double> &s, Shape &psi) const;

 /// \short Compute the  geometric shape functions and 
 /// derivatives w.r.t. local coordinates at local coordinate s
 inline void dshape_local(const Vector<double> &s, Shape &psi, DShape &dpsids) 
  const;

 /// \short Compute the geometric shape functions, derivatives and
 /// second derivatives w.r.t. local coordinates at local coordinate s 
 /// d2psids(i,0) = \f$ \partial ^2 \psi_j / \partial s_0^2 \f$ 
 /// d2psids(i,1) = \f$ \partial ^2 \psi_j / \partial s_1^2 \f$ 
 /// d2psids(i,2) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_1 \f$ 
 inline void d2shape_local(const Vector<double> &s, Shape &psi, DShape &dpsids,
                           DShape &d2psids) const;

 /// \short Overload the template-free interface for the calculation of
 /// inverse jacobian matrix. This is a one-dimensional element, so
 /// use the 1D version.
 double invert_jacobian_mapping(const DenseMatrix<double> &jacobian,
                                DenseMatrix<double> &inverse_jacobian) const
  {return FiniteElement::invert_jacobian<2>(jacobian,inverse_jacobian);}
 
 
 /// Number of nodes along each element edge
 unsigned nnode_1d() const {return NNODE_1D;}
 
 /// C-style output 
 void output(FILE* file_pt)
  {FiniteElement::output(file_pt);}

 /// C_style output at n_plot points
 void output(FILE* file_pt, const unsigned &n_plot)
  {FiniteElement::output(file_pt,n_plot);}

 /// Output 
 void output(std::ostream &outfile);

 /// Output at n_plot points
 void output(std::ostream &outfile, const unsigned &n_plot);
 
/// \short  Get cector of local coordinates of plot point i (when plotting 
 /// nplot points in each "coordinate direction).
 void get_s_plot(const unsigned& i, const unsigned& nplot,
                 Vector<double>& s) const
  {
   if (nplot>1)
    {
     unsigned i0=i%nplot;
     unsigned i1=(i-i0)/nplot;
     
     s[0]=-1.0+2.0*double(i0)/double(nplot-1);
     s[1]=-1.0+2.0*double(i1)/double(nplot-1);
    }
   else
    {
     s[0]=0.0;
     s[1]=0.0;
    }
  }

 
 /// \short Return string for tecplot zone header (when plotting 
 /// nplot points in each "coordinate direction)
 std::string tecplot_zone_string(const unsigned& nplot) const
  {
   std::ostringstream header;
   header << "ZONE I=" << nplot << ", J=" << nplot << "\n";
   return header.str();
  }
 
 /// \short Return total number of plot points (when plotting 
 /// nplot points in each "coordinate direction)
 unsigned nplot_points(const unsigned& nplot) const
  {
   unsigned DIM=2;
   unsigned np=1;
   for (unsigned i=0;i<DIM;i++) {np*=nplot;}
   return np;
  }

 /// \short Build the lower-dimensional FaceElement (an element of type
 /// QSpectralElement<1,NNODE_1D>). The face index takes four values
 /// corresponding to the four possible faces:
 /// -1 (West)  s[0] = -1.0
 /// +1 (East)  s[0] =  1.0
 /// -2 (South) s[1] = -1.0
 /// +2 (North) s[1] =  1.0
 void build_face_element(const int &face_index,
                         FaceElement* face_element_pt);
};


//=======================================================================
///Shape function for specific QSpectralElement<2,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<2,NNODE_1D>::shape(const Vector<double> &s, Shape &psi) 
 const
{
 //Call the OneDimensional Shape functions
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]), psi2(s[1]);
 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]), psi2(NNODE_1D,s[1]);

 //Now let's loop over the nodal points in the element
 //and copy the values back in  
 for(unsigned i=0;i<NNODE_1D;i++) 
  {
   for(unsigned j=0;j<NNODE_1D;j++)
    {
     psi(NNODE_1D*i + j) = psi2[i]*psi1[j];
    }
  }
}

//=======================================================================
///Derivatives of shape functions for specific  QSpectralElement<2,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<2,NNODE_1D>::dshape_local(const Vector<double> &s, 
						Shape &psi, 
						DShape &dpsids) const
{
 //Call the shape functions and derivatives
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]), psi2(s[1]);
 OneDimensionalLegendreDShape<NNODE_1D> dpsi1ds(s[0]), dpsi2ds(s[1]);

 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]), psi2(NNODE_1D,s[1]);
 //OneDLegendreDShapeParam dpsi1ds(NNODE_1D,s[0]), dpsi2ds(NNODE_1D,s[1]);

 //Index for the shape functions
 unsigned index=0;
 //Loop over shape functions in element
 for(unsigned i=0;i<NNODE_1D;i++)
  {
   for(unsigned j=0;j<NNODE_1D;j++)
    {
     //Assign the values
     dpsids(index,0) = psi2[i]*dpsi1ds[j];
     dpsids(index,1) = dpsi2ds[i]*psi1[j];
     psi[index]      = psi2[i]*psi1[j];
     //Increment the index
     ++index;
    }
  }
}


//=======================================================================
///Second derivatives of shape functions for specific  QSpectralElement<2,..> 
/// d2psids(i,0) = \f$ \partial ^2 \psi_j / \partial s_0^2 \f$ 
/// d2psids(i,1) = \f$ \partial ^2 \psi_j / \partial s_1^2 \f$ 
/// d2psids(i,2) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_1 \f$ 
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<2,NNODE_1D>::d2shape_local(const Vector<double> &s, Shape &psi, 
                                         DShape &dpsids, DShape &d2psids) const
{
 std::ostringstream error_message;
 error_message <<"\nd2shpe_local currently not implemented for this element\n";
 throw OomphLibError(error_message.str(),
                     OOMPH_CURRENT_FUNCTION,
                     OOMPH_EXCEPTION_LOCATION);

/*  //Call the shape functions and derivatives */
/*  OneDimensionalShape<NNODE_1D> psi1(s[0]); */
/*  OneDimensionalDShape<NNODE_1D> dpsi1ds(s[0]); */
/*  OneDimensionalD2Shape<NNODE_1D> d2psi1ds(s[0]); */

/*  //Loop over shape functions in element and assign to psi */
/*  for(unsigned l=0;l<NNODE_1D;l++)  */
/*   { */
/*    psi[l] = psi1[l]; */
/*    dpsids[l][0] = dpsi1ds[l]; */
/*    d2psids[l][0] = d2psi1ds[l]; */
/*   } */
}


//=======================================================================
///General QSpectralElement class specialised to three spatial dimensions
//=======================================================================
template<unsigned NNODE_1D>
class QSpectralElement<3,NNODE_1D> : public virtual SpectralElement,
 public virtual BrickElementBase
{
  private:
 
 /// \short Default integration rule: Gaussian integration of same 'order' 
 /// as the element
 //This is sort of optimal, because it means that the integration is exact
 //for the shape functions. Can overwrite this in specific element defintion.
 static GaussLobattoLegendre<3,NNODE_1D> integral;

public:

  /// Constructor
  QSpectralElement() 
  {
   //There are NNODE_1D^3 nodes in this element
   this->set_n_node(NNODE_1D*NNODE_1D*NNODE_1D);
   Spectral_order.resize(3,NNODE_1D);
   Nodal_spectral_order.resize(3,NNODE_1D);
   //Set the elemental and nodal dimensions
   this->set_dimension(3);
   //Assign integral point
   this->set_integration_scheme(&integral);
   //Calculate the nodal positions for the shape functions
   OneDimensionalLegendreShape<NNODE_1D>::calculate_nodal_positions();
   //OneDLegendreShapeParam::calculate_nodal_positions(NNODE_1D);
  }

 /// Min. value of local coordinate
 double s_min() const {return -1.0;}

 /// Max. value of local coordinate
 double s_max() const {return 1.0;}

 /// Number of vertex nodes in the element
 unsigned nvertex_node() const
  { return 8;}

 /// Pointer to the j-th vertex node in the element
 Node* vertex_node_pt(const unsigned &j) const
  {
   unsigned n_node_1d = nnode_1d();
    Node* nod_pt;
   switch(j)
    {
    case 0:
     nod_pt = this->node_pt(0);
     break;
    case 1:
     nod_pt=this->node_pt(n_node_1d-1);
     break;
    case 2:
     nod_pt=this->node_pt(n_node_1d*(n_node_1d-1));
     break;
    case 3:
     nod_pt=this->node_pt(n_node_1d*n_node_1d-1);
     break;
    case 4:
     nod_pt=this->node_pt(n_node_1d*n_node_1d*(n_node_1d-1));
     break;
    case 5:
     nod_pt=this->node_pt(n_node_1d*n_node_1d*(n_node_1d-1)+(n_node_1d-1));
     break;
    case 6:
     nod_pt=this->node_pt(n_node_1d*n_node_1d*n_node_1d-n_node_1d);
     break;
    case 7:
     nod_pt=this->node_pt(n_node_1d*n_node_1d*n_node_1d-1);
     break;
    default:
     std::ostringstream error_message;
     error_message << "Vertex node number is " << j << 
      " but must be from 0 to 7\n";
     
     throw OomphLibError(error_message.str(),
OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
   return nod_pt;
  }
 

 /// Get local coordinates of node j in the element; vector sets its own size
 void local_coordinate_of_node(const unsigned& n, Vector<double>& s)
  {
   s.resize(3);
   unsigned n0 = n%NNODE_1D;
   unsigned n1 = unsigned(double(n)/double(NNODE_1D))%NNODE_1D;
   unsigned n2 = unsigned(double(n)/double(NNODE_1D*NNODE_1D));
   s[0] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n0);
   s[1] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n1);
   s[2] = OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n2);
  }

 /// Get the local fractino of node j in the element
 void local_fraction_of_node(const unsigned &n, Vector<double> &s_fraction)
  {
   this->local_coordinate_of_node(n,s_fraction);
   //Resize
   s_fraction[0] = 0.5*(s_fraction[0] + 1.0);
   s_fraction[1] = 0.5*(s_fraction[1] + 1.0);
   s_fraction[2] = 0.5*(s_fraction[2] + 1.0);
  }

 /// The local one-d fraction is the same
 double local_one_d_fraction_of_node(const unsigned &n1d, const unsigned &i)
  {
   return 
    0.5*(OneDimensionalLegendreShape<NNODE_1D>::nodal_position(n1d) + 1.0);
  }

 /// Calculate the geometric shape functions at local coordinate s
 inline void shape(const Vector<double> &s, Shape &psi) const;

 /// \short Compute the  geometric shape functions and 
 /// derivatives w.r.t. local coordinates at local coordinate s
 inline void dshape_local(const Vector<double> &s, Shape &psi, DShape &dpsids) 
  const;

 /// \short Compute the geometric shape functions, derivatives and
 /// second derivatives w.r.t. local coordinates at local coordinate s 
 /// d2psids(i,0) = \f$ \partial ^2 \psi_j / \partial s_0^2 \f$ 
 /// d2psids(i,1) = \f$ \partial ^2 \psi_j / \partial s_1^2 \f$ 
 /// d2psids(i,2) = \f$ \partial ^2 \psi_j / \partial s_2^2 \f$ 
 /// d2psids(i,3) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_1 \f$ 
 /// d2psids(i,4) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_2 \f$ 
 /// d2psids(i,5) = \f$ \partial ^2 \psi_j / \partial s_1 \partial s_2 \f$ 
 inline void d2shape_local(const Vector<double> &s, Shape &psi, DShape &dpsids,
                           DShape &d2psids) const;


 /// \short Overload the template-free interface for the calculation of
 /// inverse jacobian matrix. This is a one-dimensional element, so
 /// use the 3D version.
 double invert_jacobian_mapping(const DenseMatrix<double> &jacobian,
                                DenseMatrix<double> &inverse_jacobian) const
  {return FiniteElement::invert_jacobian<3>(jacobian,inverse_jacobian);}
 
 /// Number of nodes along each element edge
 unsigned nnode_1d() const {return NNODE_1D;}
 
 /// C-style output 
 void output(FILE* file_pt)
  {FiniteElement::output(file_pt);}

 /// C_style output at n_plot points
 void output(FILE* file_pt, const unsigned &n_plot)
  {FiniteElement::output(file_pt,n_plot);}

 /// Output 
 void output(std::ostream &outfile);

 /// Output at nplot points
 void output(std::ostream &outfile, const unsigned& nplot);
 
/// \short  Get cector of local coordinates of plot point i (when plotting 
 /// nplot points in each "coordinate direction).
 void get_s_plot(const unsigned& i, const unsigned& nplot,
                 Vector<double>& s) const
 {
  if (nplot>1)
   {
    unsigned i01=i%(nplot*nplot);
    unsigned i0=i01%nplot;
    unsigned i1=(i01-i0)/nplot;
    unsigned i2=(i-i01)/(nplot*nplot);
    
    s[0]=-1.0+2.0*double(i0)/double(nplot-1);
    s[1]=-1.0+2.0*double(i1)/double(nplot-1);
    s[2]=-1.0+2.0*double(i2)/double(nplot-1);
   }
  else
   {
    s[0]=0.0;
    s[1]=0.0;
    s[2]=0.0;
   }
 }

 
 /// \short Return string for tecplot zone header (when plotting 
 /// nplot points in each "coordinate direction)
 std::string tecplot_zone_string(const unsigned& nplot) const
  {
   std::ostringstream header;
   header << "ZONE I=" << nplot << ", J=" << nplot << ", K=" 
          << nplot << "\n";
   return header.str();
  }
 
 /// \short Return total number of plot points (when plotting 
 /// nplot points in each "coordinate direction)
 unsigned nplot_points(const unsigned& nplot) const
  {
   unsigned DIM=3;
   unsigned np=1;
   for (unsigned i=0;i<DIM;i++) {np*=nplot;}
   return np;
  }

 /// \short Build the lower-dimensional FaceElement (an element of type
 /// QSpectralElement<2,NNODE_1D>). The face index takes six values
 /// corresponding to the six possible faces:
 /// -1 (Left)   s[0] = -1.0
 /// +1 (Right)  s[0] =  1.0
 /// -2 (Down)   s[1] = -1.0
 /// +2 (Up)     s[1] =  1.0
 /// -3 (Back)   s[2] = -1.0
 /// +3 (Front)  s[2] =  1.0
 void build_face_element(const int &face_index,
                         FaceElement* face_element_pt);
};


//=======================================================================
///Shape function for specific QSpectralElement<3,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<3,NNODE_1D>::shape(const Vector<double> &s, Shape &psi) 
 const
{
 //Call the OneDimensional Shape functions
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]), psi2(s[1]), psi3(s[2]);
 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]), psi2(NNODE_1D,s[1]), 
 // psi3(NNODE_1D,s[2]);

 //Now let's loop over the nodal points in the element
 //and copy the values back in  
 for(unsigned i=0;i<NNODE_1D;i++) 
  {
   for(unsigned j=0;j<NNODE_1D;j++)
    {
     for(unsigned k=0;k<NNODE_1D;k++)
      {
       psi(NNODE_1D*NNODE_1D*i + NNODE_1D*j + k) = psi3[i]*psi2[j]*psi1[k];
      }
    }
  }
}

//=======================================================================
///Derivatives of shape functions for specific  QSpectralElement<3,..>
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<3,NNODE_1D>::dshape_local(const Vector<double> &s, 
						Shape &psi, 
						DShape &dpsids) const
{
 //Call the shape functions and derivatives
 //OneDLegendreShapeParam psi1(NNODE_1D,s[0]), psi2(NNODE_1D,s[1]),
 // psi3(NNODE_1D,s[2]);
 //OneDLegendreDShapeParam dpsi1ds(NNODE_1D,s[0]), dpsi2ds(NNODE_1D,s[1]),
 // dpsi3ds(NNODE_1D,s[2]);
 OneDimensionalLegendreShape<NNODE_1D> psi1(s[0]), psi2(s[1]), psi3(s[2]);
 OneDimensionalLegendreDShape<NNODE_1D> dpsi1ds(s[0]), dpsi2ds(s[1]),
  dpsi3ds(s[2]);


 //Index for the shape functions
 unsigned index=0;
 
 //Loop over shape functions in element
 for(unsigned i=0;i<NNODE_1D;i++)
  {
   for(unsigned j=0;j<NNODE_1D;j++)
    {
     for(unsigned k=0;k<NNODE_1D;k++)
      {
       //Assign the values
       dpsids(index,0) = psi3[i]*psi2[j]*dpsi1ds[k];
       dpsids(index,1) = psi3[i]*dpsi2ds[j]*psi1[k];
       dpsids(index,2) = dpsi3ds[i]*psi2[j]*psi1[k];
       psi[index]      = psi3[i]*psi2[j]*psi1[k];
       //Increment the index
       ++index;
      }
    }
  }
}


//=======================================================================
///Second derivatives of shape functions for specific QSpectralElement<3,..> 
/// d2psids(i,0) = \f$ \partial ^2 \psi_j / \partial s_0^2 \f$ 
/// d2psids(i,1) = \f$ \partial ^2 \psi_j / \partial s_1^2 \f$ 
/// d2psids(i,2) = \f$ \partial ^2 \psi_j / \partial s_2^2 \f$ 
/// d2psids(i,3) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_1 \f$ 
/// d2psids(i,4) = \f$ \partial ^2 \psi_j / \partial s_0 \partial s_2 \f$ 
/// d2psids(i,5) = \f$ \partial ^2 \psi_j / \partial s_1 \partial s_2 \f$ 
//=======================================================================
template <unsigned NNODE_1D>
void QSpectralElement<3,NNODE_1D>::d2shape_local(const Vector<double> &s, Shape &psi, 
                                         DShape &dpsids, DShape &d2psids) const
{
 std::ostringstream error_message;
 error_message <<"\nd2shpe_local currently not implemented for this element\n";
 throw OomphLibError(error_message.str(),
                     OOMPH_CURRENT_FUNCTION,
                     OOMPH_EXCEPTION_LOCATION);

/*  //Call the shape functions and derivatives */
/*  OneDimensionalShape<NNODE_1D> psi1(s[0]); */
/*  OneDimensionalDShape<NNODE_1D> dpsi1ds(s[0]); */
/*  OneDimensionalD2Shape<NNODE_1D> d2psi1ds(s[0]); */

/*  //Loop over shape functions in element and assign to psi */
/*  for(unsigned l=0;l<NNODE_1D;l++)  */
/*   { */
/*    psi[l] = psi1[l]; */
/*    dpsids[l][0] = dpsi1ds[l]; */
/*    d2psids[l][0] = d2psi1ds[l]; */
/*   } */
}

//==============================================================
/// A class that is used to template the refineable Q spectral elements
/// by dimension. It's really nothing more than a policy class
//=============================================================
template<unsigned DIM>
class RefineableQSpectralElement
{
  public:

 /// Empty constuctor
 RefineableQSpectralElement() {}
};

}

#endif
