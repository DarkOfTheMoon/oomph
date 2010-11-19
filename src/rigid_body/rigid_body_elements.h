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
//Header file for Rigid Body Elements
#ifndef OOMPH_RIGID_BODY_ELEMENTS_HEADER
#define OOMPH_RIGID_BODY_ELEMENTS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#include "../generic/elements.h"
#include "../generic/triangle_mesh.h"

namespace oomph
{
  
//=====================================================================
/// Class that solves the equations of motion for a general 
/// two-dimensional rigid body subject to a particular force and torque
/// distribution. The Data object whose three values
/// represent the x and y displacements of its centre of gravity and
/// the polygon's rotation about its centre of gravity.
/// If added to a mesh in the Problem (in its incarnation as a 
/// GeneralisedElement) the displacement/rotation of the polygon
/// is computed in response to (i) user-specifiable applied forces
/// and a torque and (ii) the net drag (and assocated torque) from
/// a mesh of elements that can exert a drag onto the polygon (typically
/// Navier-Stokes FaceElements that apply a viscous drag to an 
/// immersed body, represented by the polygon.)
//=====================================================================
 class RigidBodyElement : public GeneralisedElement, public GeomObject
 {
  
 public:
  
  /// \short Function pointer to function that specifies 
  /// external force
  typedef void (*ExternalForceFctPt)(const double& time, 
                                     Vector<double>& external_force);
  
  /// \short Function pointer to function that specifies 
  /// external torque
  typedef void (*ExternalTorqueFctPt)(const double& time, 
                                      double& external_torque);

  /// \short Default constructor that intialises everything to 
  /// zero
  RigidBodyElement(TimeStepper* const &time_stepper_pt,
                   Data* const &centre_displacement_data_pt=0)  : 
   Geom_object_pt(0),
   Initial_Phi(0.0), Mass(0.0), Moment_of_inertia(0.0),
   External_force_fct_pt(0), External_torque_fct_pt(0),
   Centre_displacement_data_pt(centre_displacement_data_pt),
   Drag_mesh_pt(0), G_pt(0), Scaled_mass_pt(0)
   {
    this->initialise(time_stepper_pt);
   }
  
  /// \short Constructor that takes an underlying geometric object:
  /// and timestepper
  RigidBodyElement(GeomObject* const &geom_object_pt,
                   TimeStepper* const &time_stepper_pt,
                   Data* const &centre_displacement_data_pt=0) :
   Geom_object_pt(geom_object_pt),
   Initial_Phi(0.0), Mass(0.0), Moment_of_inertia(0.0),
   External_force_fct_pt(0), External_torque_fct_pt(0),
   Centre_displacement_data_pt(centre_displacement_data_pt),
   Drag_mesh_pt(0), G_pt(0), Scaled_mass_pt(0)
   {
    this->initialise(time_stepper_pt);
   }

  
  ///Access function for the initial angle
  double &initial_phi() {return Initial_Phi;}

  ///Access function for the initial centre of mass
  double &initial_centre_of_mass(const unsigned &i) 
   {return Initial_centre_of_mass[i];}
  

  ///Access function for the initial centre of mass (const version)
  const double &initial_centre_of_mass(const unsigned &i) const
   {return Initial_centre_of_mass[i];}


  //Overload the position to apply the rotation and translation
  void position(const Vector<double> &xi, Vector<double> &r) const
   {
    Vector<double> initial_x(2);
    Geom_object_pt->position(xi,initial_x);
    this->apply_rigid_body_motion(0,initial_x,r);
   }

  //Include the time history of the motion of the object
  void position(const unsigned& t, const Vector<double>& xi, 
                Vector<double>& r) const
   {
    Vector<double> initial_x(2);
    Geom_object_pt->position(xi,initial_x);
    this->apply_rigid_body_motion(t,initial_x,r);
   }


  //Work out the position derivative
  void dposition_dt(const Vector<double> &zeta, const unsigned &j,
                    Vector<double> &drdt)
   {
    switch(j)
     {
     case 0:
      return position(zeta,drdt);
      break;

     case 1:
     {
      //Get the initial position of the underlying geometric object
      Vector<double> initial_x(2);
      Geom_object_pt->position(zeta,initial_x);
      //Scale relative to the centre of mass
      double X = initial_x[0] - Initial_centre_of_mass[0];
      double Y = initial_x[1] - Initial_centre_of_mass[1];
      
      //Now calculate the original angle and radius
      double phi_orig=atan2(Y,X);
      double r_orig=sqrt(X*X+Y*Y);
      
      // Get first time derivatives of all displacement data
      Vector<double> veloc(3);
      // Get the velocity of the data
      this->Centre_displacement_data_pt->time_stepper_pt()
       ->time_derivative(1,this->Centre_displacement_data_pt,
                         veloc);
 
     //Now use the chain rule to specify the boundary velocities
      drdt[0] = veloc[0] 
       - r_orig*sin(phi_orig + 
                    this->Centre_displacement_data_pt->value(2))*veloc[2];
      drdt[1] = veloc[1] 
       + r_orig*cos(phi_orig + this->Centre_displacement_data_pt->value(2))
       *veloc[2];
     }
      //Done
      return;
      break;

     default:
      std::ostringstream warning_stream;
      warning_stream    
       << "Using default (static) assignment " << j 
       << "-th time derivative in GeomObject::dposition_dt(...) is zero\n" 
       << "Overload for your specific geometric object if this is not \n" 
       << "appropriate. \n";
      OomphLibWarning(warning_stream.str(),
                      "GeomObject::dposition_dt()",
                      OOMPH_EXCEPTION_LOCATION);
      
      unsigned n=drdt.size();
      for (unsigned i=0;i<n;i++) {drdt[i]=0.0;}
      break;
     }
   }
  
 /// \short Destuctor: Cleanup if required
 ~RigidBodyElement() 
  { 
   if(Displacement_data_is_internal)
    {
     delete Centre_displacement_data_pt;
     //Null out the pointer stored as internal data
     this->internal_data_pt(0) = 0;
    }
  }
  
  /// Access to "mass" (area) of polygon
  double &mass() {return Mass;}
  
  /// Access to polar moment of inertia of polygon
  double &moment_of_inertia() {return Moment_of_inertia;}
  
  // hierher provide lambda squared and use it
   
 /// \short Pointer to Data for centre of gravity displacement. 
 /// Values: 0: x-displ; 1: y-displ; 2: rotation angle.
 Data*& centre_displacement_data_pt()
  {
   return Centre_displacement_data_pt;
  }
 
 /// x-displacement of centre of mass
 double& centre_x_displacement()
  {
   return *(Centre_displacement_data_pt->value_pt(0));
  }
   
 /// y-displacement of centre of mass
 double& centre_y_displacement()
  {
   return *(Centre_displacement_data_pt->value_pt(1));
  }
   
 /// rotation of centre of mass
 double& centre_rotation_angle()
  {
   return *(Centre_displacement_data_pt->value_pt(2));
  }
   

  /// Get current centre of gravity
  Vector<double> centre_of_gravity()
   {
    Vector<double> cog(2);
    for(unsigned i=0;i<2;i++)
     {
      cog[i] = Initial_centre_of_mass[i] + 
       this->Centre_displacement_data_pt->value(i);
     }
    return cog;
   }
  
 /// Output position velocity and acceleration of centre of gravity
 void output_centre_of_gravity(std::ostream& outfile)
  {
   // Get timestepper
   TimeStepper* time_stepper_pt=
    this->Centre_displacement_data_pt->time_stepper_pt();
   
   // Get first time derivatives of all displacement data
   Vector<double> veloc(3);
   time_stepper_pt->time_derivative(1,this->Centre_displacement_data_pt,
                                    veloc);
   
   // Get second time derivatives of all displacement data
   Vector<double> accel(3);
   time_stepper_pt->time_derivative(2,this->Centre_displacement_data_pt,
                                    accel);
   
   outfile << time_stepper_pt->time() << " "
           << Initial_centre_of_mass[0] + 
    this->Centre_displacement_data_pt->value(0) << " "
           << Initial_centre_of_mass[1] + 
    this->Centre_displacement_data_pt->value(1) << " "
           << Initial_Phi + this->Centre_displacement_data_pt->value(2) << " "
           << veloc[0] << " " << veloc[1] << " " << veloc[2] << " "
           << accel[0] << " " << accel[1] << " " << accel[2] << std::endl;
  }
 

 /// Get residuals
 void fill_in_contribution_to_residuals(Vector<double>& residuals)
  {
   // Dummy
   DenseMatrix<double> jacobian;
   bool flag=false;

   // Get generic function
   get_residuals_hole_polygon_generic(residuals,jacobian,flag);
  }


 /// Get residuals
  void fill_in_contribution_to_jacobian(Vector<double>& residuals,
                                       DenseMatrix<double>& jacobian)
   {
    // Get generic function
    //bool flag=true;
    bool flag = false;
    get_residuals_hole_polygon_generic(residuals,jacobian,flag);
    this->fill_in_jacobian_from_internal_by_fd(residuals,jacobian);
    //Get the effect of the fluid loading on the rigid body
    this->fill_in_jacobian_from_external_by_fd(residuals,jacobian);
    }


  /// \short Update the positions of the nodes in fluid elements 
  /// adjacent to the rigid body, defined as being elements in the
  /// drag mesh.
  void node_update_adjacent_fluid_elements()
   {
    if (Drag_mesh_pt==0)
     {
      return;
     }
    else
     {
      unsigned nel=Drag_mesh_pt->nelement();
      for (unsigned e=0;e<nel;e++)
       {       
        dynamic_cast<FaceElement*>(Drag_mesh_pt->element_pt(e))->
         bulk_element_pt()->node_update();
       }
     }
   }
  
  
 /// \short After an external data change, update the nodal positions
 inline void update_in_external_fd(const unsigned &i) 
  {
   node_update_adjacent_fluid_elements();
  }

 ///\short Do nothing to reset within finite-differencing of  external data
 inline void reset_in_external_fd(const unsigned &i) { } 

 ///\short After all external data finite-differencing, update nodal positions
 inline void reset_after_external_fd() 
  { 
    node_update_adjacent_fluid_elements();
   }

 ///\short After an internal data change, update the nodal positions
 inline void update_in_internal_fd(const unsigned &i) 
  {
   node_update_adjacent_fluid_elements();
  }
 
 ///\short Do nothing to reset within finite-differencing of internal data
 inline void reset_in_internal_fd(const unsigned &i) { } 

 ///\short After all internal data finite-differencing, update nodal positions
 inline void reset_after_internal_fd() 
  { 
   node_update_adjacent_fluid_elements();
  }
 
 /// \short Get force and torque from specified fct pointers and
 /// drag mesh
 void get_force_and_torque(const double& time,
                           Vector<double>& force,
                           double& torque)
  {
   // Get external force
   if (External_force_fct_pt==0)
    {
     force[0]=0.0;
     force[1]=0.0;
    }
   else
    {
     External_force_fct_pt(time,force);
    }
   
   // Get external torque
   if (External_torque_fct_pt==0)
    {
     torque=0.0;
    }
   else
    {
     External_torque_fct_pt(time,torque);
    }
   
   // Add drag from any (fluid) mesh attached to surface of polygon     
   Vector<double> element_drag_force(2);
   Vector<double> element_drag_torque(1);
   if (Drag_mesh_pt==0)
    {
     return;
    }
   else
    {
     unsigned nel=Drag_mesh_pt->nelement();

     for (unsigned e=0;e<nel;e++)
      {       
       dynamic_cast<ElementWithDragFunction*>(Drag_mesh_pt->element_pt(e))->
        get_drag_and_torque(element_drag_force,
                            element_drag_torque);       
       force[0]+=element_drag_force[0];
       force[1]+=element_drag_force[1];
       torque+=element_drag_torque[0];
      }
    }   
  }

 /// \short Access to function pointer to function that specifies 
 /// external force
 ExternalForceFctPt& external_force_fct_pt()
  {
   return External_force_fct_pt;
  }


 /// \short Access to function pointer to function that specifies 
 /// external torque
 ExternalTorqueFctPt& external_torque_fct_pt()
  {
   return External_torque_fct_pt;
  }


 /// \short Access fct to mesh containing face elements that allow 
 /// the computation of the drag on the polygon
 Mesh*& drag_mesh_pt()
  {
   return Drag_mesh_pt;
  }
  
 
 /// The position of the object depends on one data item
 unsigned ngeom_data() const {return 1;}
 
 /// \short Return pointer to the j-th (only) Data item that the object's 
 /// shape depends on.
 Data* geom_data_pt(const unsigned& j) 
  {return this->Centre_displacement_data_pt;}
 
 /// \short Access function to the direction of gravity
 Vector<double>* &g_pt() {return G_pt;}
 
 /// \short Access function to the scaled mass
 double* &scaled_mass() {return Scaled_mass_pt;}
 
   protected: 
 
 /// \short Helper function to adjust the position in 
 /// response to changes in position and angle of the solid
 /// about the centre of mass
 void apply_rigid_body_motion(const unsigned &t,
                              const Vector<double> &initial_x, 
                              Vector<double> &r) const
  {
   //Scale relative to the centre of mass
   double X = initial_x[0] - Initial_centre_of_mass[0];
   double Y = initial_x[1] - Initial_centre_of_mass[1];
   
   //Find the original angle and radius
   double phi_orig=atan2(Y,X);
   double r_orig=sqrt(X*X+Y*Y);
   
   // Updated position vector
   r[0] = Initial_centre_of_mass[0] + 
    this->Centre_displacement_data_pt->value(t,0)+
    r_orig*cos(phi_orig + this->Centre_displacement_data_pt->value(t,2));
   
   r[1] = Initial_centre_of_mass[1] + 
    this->Centre_displacement_data_pt->value(t,1)+
     r_orig*sin(phi_orig + this->Centre_displacement_data_pt->value(t,2));
  }
 
 
   private:
 
 /// \short Return the equation number associated with the i-th
 /// centre of gravity displacment 
 /// 0: x-displ; 1: y-displ; 2: rotation angle.
 inline int centre_displacement_local_eqn(const unsigned &i)
  {
   if(Displacement_data_is_internal)
    {
     return this->internal_local_eqn(Index_for_centre_displacement,i);
    }
   else
    {
     return this->external_local_eqn(Index_for_centre_displacement,i);
    }
  }
 
 /// \short Initialisation function
 void initialise(TimeStepper* const &time_stepper_pt)
  {
   //This could be calculated by an integral around the boundary
   Initial_centre_of_mass.resize(2,0.0);
   
   //Temporary hack
   if(time_stepper_pt==0) {return;}
   
   // Provide Data for centre-of-mass displacement internally
   if (Centre_displacement_data_pt==0)
    {
     Centre_displacement_data_pt=new Data(time_stepper_pt,3);
     
     // I've created it so I have to tidy up too!
     Displacement_data_is_internal=true;
     
     // Centre displacement is internal Data for this element
     Index_for_centre_displacement =
      add_internal_data(Centre_displacement_data_pt);
    }
   // Data created externally, so somebody else will clean up
   else
    {
     Displacement_data_is_internal=false;
     
     // Centre displacement is external Data for this element
     Index_for_centre_displacement =
      add_external_data(Centre_displacement_data_pt);
    }
  }
 
 
 
 /// Get residuals and/or Jacobian
 void get_residuals_hole_polygon_generic(Vector<double> &residuals,
                                         DenseMatrix<double> &jacobian,
                                         const bool& flag)
  {   
   // Get timestepper and time
   TimeStepper* timestepper_pt=
    this->Centre_displacement_data_pt->time_stepper_pt();
   double time=timestepper_pt->time();
   
   // Get second time derivatives of all displacement data
   Vector<double> accel(3);
   timestepper_pt->time_derivative(2,this->Centre_displacement_data_pt,
                                   accel);
   
   // Get force and torque
   Vector<double> external_force(2);
    double external_torque;
    get_force_and_torque(time,external_force,external_torque);

    // Newton's law
    int local_eqn=0;
    local_eqn = this->centre_displacement_local_eqn(0);
    if(local_eqn >= 0)
     {
      residuals[local_eqn]=Mass*accel[0]-external_force[0] - Mass*(*G_pt)[0];

      // Get Jacobian too?
      if (flag)
       {
        jacobian(local_eqn,local_eqn)=Mass*timestepper_pt->weight(2,0);
       }
     }

    local_eqn = this->centre_displacement_local_eqn(1);
    if(local_eqn >= 0)
     {
      residuals[local_eqn]=Mass*accel[1]-external_force[1] - Mass*(*G_pt)[1];
      // Get Jacobian too?
      if (flag)
       {
        jacobian(local_eqn,local_eqn)=Mass*timestepper_pt->weight(2,0);
       }
     }

    local_eqn = this->centre_displacement_local_eqn(2);
    if(local_eqn >= 0)
     {
      residuals[local_eqn]=Moment_of_inertia*accel[2]-external_torque;
      // Get Jacobian too?
      if (flag)
       {
        jacobian(local_eqn,local_eqn)=
         Moment_of_inertia*timestepper_pt->weight(2,0);
       }
     }
   }


  /// Underlying geometric object
  GeomObject* Geom_object_pt;

   protected: 

  /// X-coordinate of initial centre of gravity
  Vector<double> Initial_centre_of_mass; 
 
  /// Original rotation angle 
  double Initial_Phi;

  // Mass of polygon
  double Mass;
  
  /// Polar moment of inertia of polygon
  double Moment_of_inertia;
  
   private:
  /// \short Function pointer to function that specifies 
  /// external force
  ExternalForceFctPt External_force_fct_pt;
  
  /// \short Function pointer to function that specifies 
  /// external torque
  ExternalTorqueFctPt External_torque_fct_pt;
  
   protected:
  /// \short Data for centre of gravity displacement. 
  /// Values: 0: x-displ; 1: y-displ; 2: rotation angle.
  Data* Centre_displacement_data_pt;
  
   private:

  /// \short Mesh containing face elements that allow the computation of
  /// the drag on the polygon
  Mesh* Drag_mesh_pt;

  /// The direction of gravity
  Vector<double> *G_pt;

  /// The scaled mass
  double *Scaled_mass_pt;

  /// \short Index for the data (internal or external) that contains the 
  /// centre-of-gravity displacement
  unsigned Index_for_centre_displacement;

  /// Boolean flag to indicate whether data is internal
  bool Displacement_data_is_internal;

};
 


//=====================================================================
/// Class upgrading a TriangleMeshPolygon to a "hole" for use during 
/// triangle mesh generation. For mesh generation purposes, the main (and only)
/// addition to the base class is the provision of the coordinates
/// of a hole inside the polygon. To faciliate the movement of the "hole"
/// through the domain we also provide a Data object whose three values
/// represent the x and y displacements of its centre of gravity and
/// the polygon's rotation about its centre of gravity.
/// If added to a mesh in the Problem (in its incarnation as a 
/// GeneralisedElement) the displacement/rotation of the polygon
/// is computed in response to (i) user-specifiable applied forces
/// and a torque and (ii) the net drag (and assocated torque) from
/// a mesh of elements that can exert a drag onto the polygon (typically
/// Navier-Stokes FaceElements that apply a viscous drag to an 
/// immersed body, represented by the polygon.)
//=====================================================================
class RigidBodyTriangleMeshHolePolygon : public TriangleMeshHolePolygon, 
 public RigidBodyElement
{
 
public:

 
 /// \short Constructor: Specify coordinates of a point inside the hole
 /// and a vector of pointers to TriangleMeshPolyLines
 /// that define the boundary segments of the polygon.
 /// Each TriangleMeshPolyLine has its own boundary ID and can contain
 /// multiple (straight-line) segments. The optional final argument
 /// is a pointer to a Data object whose three values represent 
 /// the two displacements of and the rotation angle about the polygon's 
 /// centre of mass.
 RigidBodyTriangleMeshHolePolygon(const Vector<double>& hole_center,
                                  const Vector<TriangleMeshPolyLine*>& 
                                  boundary_polyline_pt,
                                  TimeStepper* const &time_stepper_pt,
                                  Data* const &centre_displacement_data_pt=0);
 
 /// \short Empty Destuctor
 ~RigidBodyTriangleMeshHolePolygon() 
  {

  }

 // hierher provide lambda squared and use it
 
  ///Overload (again) the position to apply the rotation and translation
  void position(const Vector<double> &xi, Vector<double> &r) const
   {
    Vector<double> initial_x(2);
    this->get_initial_position(xi,initial_x);
    this->apply_rigid_body_motion(0,initial_x,r);
   }


  ///Overload (again) the position to apply the rotation and translation
  void position(const unsigned &t,
                const Vector<double> &xi, Vector<double> &r) const
   {
    Vector<double> initial_x(2);
    this->get_initial_position(xi,initial_x);
    this->apply_rigid_body_motion(t,initial_x,r);
   }



 /// \short Update the reference configuration by re-setting the original
 /// position of the vertices to their current ones, re-set the 
 /// original position of the centre of mass, and the displacements 
 /// and rotations relative to it
 void reset_reference_configuration();

private:

 /// \short Get the initial position of the polygon
 void get_initial_position(const Vector<double> &xi, Vector<double> &r) const
   {
    //Find the number of polylines (boundaries)
    unsigned n_poly = this->npolyline();

    //The boundary coordinate will be contiguous from polyline to 
    //polyline and each polyline will have the scaled arclength coordinate
    //in the range 0->1.

    //Find the maximum coordinate
    double zeta_max = Zeta_vertex[n_poly-1].back();
    
    //If we are above the maximum complain
    if(xi[0] > zeta_max)
     {
      std::ostringstream error_message;
      error_message << "Value of intrinsic coordinate " << xi[0] << 
       "greater than maximum " << zeta_max << "\n";
      throw 
       OomphLibError(error_message.str(),
                     "TriangleMeshHolePolygon::position()",
                     OOMPH_EXCEPTION_LOCATION);
     }

    //If equal to the maximum, return the final vertex
    if(xi[0] == zeta_max)
     {
      unsigned n_vertex = this->polyline_pt(n_poly-1)->nvertex();
      r = this->polyline_pt(n_poly-1)->vertex_coordinate(n_vertex-1);
      return;
     }
      
    //Otherwise

    //Find out which polyline we are in
    //If we've got here this should be less than n_poly
    unsigned p = static_cast<unsigned> (floor(xi[0]));
    
    //If we are "above" the last polyline then throw an error
    //This should have been caught by the trap above
    if(p>=n_poly)
     {
      std::ostringstream error_message;
      error_message 
       << "Something has gone wrong.\n"
       << "The integer part of the input intrinsic coordinate is " 
       << p << 
       "\nwhich is equal to or greater than the number of polylines, "
       << n_poly << ".\n"
       << "This should have triggered an earlier error\n";
      

      throw OomphLibError(error_message.str(),
                          "TriangleMeshHolePolygon::position()",
                          OOMPH_EXCEPTION_LOCATION);
     }

    //Cache the appropriate polyline
    TriangleMeshPolyLine* const line_pt = this->polyline_pt(p);

    //If we are at the first vertex in the polyline, return it
    if(xi[0] == Zeta_vertex[p][0])
     {
      r = line_pt->vertex_coordinate(0);
      return;
     }

    //Otherwise loop over the other points to find the appropriate
    //segment

    //Find the number of vertices in the chosen polyline
    unsigned n_vertex = line_pt->nvertex();
    //Now start from the first node in the appropriate polyline and loop up
    for(unsigned v=1;v<n_vertex;v++)
     {
      //First time that zeta falls below the vertex coordinate 
      //we have something
      if(xi[0] < Zeta_vertex[p][v])
       {
        double fraction = (xi[0] - Zeta_vertex[p][v-1])/
         (Zeta_vertex[p][v] - Zeta_vertex[p][v-1]);
        Vector<double> first = line_pt->vertex_coordinate(v-1);
        Vector<double> last = line_pt->vertex_coordinate(v);
        r.resize(2);
        for(unsigned i=0;i<2;i++)
         {
          r[i] = first[i] + fraction*(last[i] - first[i]);
         }
        return;
       }
     }
   }


 /// Helper function to assign the values of the (scaled) arc-length
 /// to each node of each polyline. The direction will be the natural
 /// order of the vertices within the polyline.
 void assign_zeta()
  {
   //Find the number of polylines
   unsigned n_poly = this->npolyline();
   
   //Allocate appropriate storage for the zeta values
   Zeta_vertex.resize(n_poly);
   
   //Temporary storage for the vertex coordinates
   Vector<double> vertex_coord_first;
   Vector<double> vertex_coord_next;
   
   //Set the initial value of zeta
   double zeta_offset = 0.0;

   //Loop over the polylines
   for(unsigned p=0;p<n_poly;++p)
    {
     //Cache the pointer to the polyline
     TriangleMeshPolyLine* const line_pt = this->polyline_pt(p);

     //Find the number of vertices in the polyline
     unsigned n_vertex = line_pt->nvertex();
     
     //Allocate storage and set initial value
     Zeta_vertex[p].resize(n_vertex);
     Zeta_vertex[p][0] = 0.0;
 
     //Loop over the vertices in the polyline and calculate the length
     //between each for use as the intrinsic coordinate
     vertex_coord_first = line_pt->vertex_coordinate(0);
     for(unsigned v=1;v<n_vertex;v++)
      {
       vertex_coord_next = line_pt->vertex_coordinate(v);
       double length = 
        sqrt(pow(vertex_coord_next[0] - vertex_coord_first[0],2.0)
             + pow(vertex_coord_next[1] - vertex_coord_first[1],2.0));
       Zeta_vertex[p][v] = Zeta_vertex[p][v-1] + length;
       vertex_coord_first = vertex_coord_next;
      }
   
     //Now scale the length to unity and add the offset
     Zeta_vertex[p][0] += zeta_offset;
     for(unsigned v=1;v<n_vertex;v++)
      {
       Zeta_vertex[p][v] /= Zeta_vertex[p][n_vertex-1];
       Zeta_vertex[p][v] += zeta_offset;
      }
     zeta_offset += 1.0;
    } //End of loop over polylines
  }

 /// \short Vector of intrisic coordinate values at the nodes
 Vector<Vector<double> > Zeta_vertex;

};

}
#endif
