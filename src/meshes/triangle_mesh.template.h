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
#ifndef OOMPH_TRIANGLE_MESH_HEADER
#define OOMPH_TRIANGLE_MESH_HEADER

//Standards
#include <iostream>
#include <fstream>
#include <string.h>

#include "../generic/triangle_scaffold_mesh.h" 
#include "../generic/triangle_mesh.h"

extern "C" {
 void triangulate(char *triswitches, struct oomph::triangulateio *in,
                  struct oomph::triangulateio *out, 
                  struct oomph::triangulateio *vorout);
}


namespace oomph
{

//============start_of_triangle_class===================================
/// Triangle mesh build with the help of the scaffold mesh coming  
/// from the triangle mesh generator Triangle.
/// http://www.cs.cmu.edu/~quake/triangle.html
//======================================================================
 template <class ELEMENT>
  class TriangleMesh : public virtual TriangleMeshBase
  {
   
    public:

   /// \short Constructor with the input files
   TriangleMesh(const std::string& node_file_name,
                const std::string& element_file_name,
                const std::string& poly_file_name,
                TimeStepper* time_stepper_pt=
                &Mesh::Default_TimeStepper,
                const bool &use_attributes=false)
              
    {
     // Using this constructor no Triangulateio object is built
     Triangulateio_exists=false;

     // Build scaffold
     Tmp_mesh_pt= new 
      TriangleScaffoldMesh(node_file_name,
                           element_file_name,
                           poly_file_name);
     
     // Convert mesh from scaffold to actual mesh
     build_from_scaffold(time_stepper_pt,use_attributes);
     
     // Kill the scaffold
     delete Tmp_mesh_pt;
     Tmp_mesh_pt=0;
     
     // Setup boundary coordinates for boundaries
     unsigned nb=nboundary();
     for (unsigned b=0;b<nb;b++)
      {
       this->setup_boundary_coordinates(b);
      }
    }
   
   /// \short Constructor with the triangulate object 
   TriangleMesh(triangulateio &triangulateio_data,
                TimeStepper* time_stepper_pt=
                &Mesh::Default_TimeStepper,
                const bool &use_attributes=false)
    
  
    {
     // Build scaffold
     Tmp_mesh_pt= new TriangleScaffoldMesh(triangulateio_data);
      
     // Initialize triangulateio structure
     initialize_triangulateio(Triangulateio);
     
     // Copy triangulateio_data into the Triangulateio private object
     // Appunto copy to check, if all fields have been filled in
     Triangulateio=triangulateio_data;
     
     // Set Triangulateio_exists to true
     Triangulateio_exists=true;
     
     // Convert mesh from scaffold to actual mesh
     build_from_scaffold(time_stepper_pt,use_attributes);
     
     // Kill the scaffold
     delete Tmp_mesh_pt;
     Tmp_mesh_pt=0;
     
     // Setup boundary coordinates for boundaries
     unsigned nb=nboundary();
     for (unsigned b=0;b<nb;b++)
      {
       this->setup_boundary_coordinates(b);
      }
    }
   
   /// \short Constructor with the oomph_lib MeshPolygon class object.
   /// Structure is defined using multiple,or not, segments defining a boundary
   /// which are called TriangleMeshPolyLine. Two closed structure are built.
   /// TriangleMeshPolygon, defining the outer boundary and  
   /// TriangleMeshHolePolygon, defining, if present, one or mulitple hole
   /// inside the structure.Triangulateio object is built
   /// using these objects and call the triangulate function with the
   /// triangle_input_string value, in order to build a new triangulateio 
   /// with the mesh parameter required.
   /// The oomph mesh_pt is built using the scaffold mesh constructor and
   /// the build_from_scaffold function
   TriangleMesh(TriangleMeshPolygon* &outer_boundary_pt,
                Vector<TriangleMeshHolePolygon*> &inner_hole_pt,
                const std::string &input_string,
                TimeStepper* time_stepper_pt=
                &Mesh::Default_TimeStepper,
                const bool &use_attributes=false)
  
    {
     
     // Create the data structures required to call the triangulate function
     triangulateio triangle_in;
     
     // Convert TriangleMeshPolyLine and TriangleMeshHolePolyLine
     // to a triangulateio object
     build_triangulateio(outer_boundary_pt,inner_hole_pt,
                         triangle_in);
     // Sub_boundary_id vector is generated using this constructor only
     // set the boolean to true
     Sub_boundary_id_exists=true;

     // Initialize triangulateio structure
     initialize_triangulateio(Triangulateio);

     // Convert the Input string in *char required by the triangulate function
     char * triswitches= const_cast<char*> (input_string.c_str() );

     // Build the mesh using triangulate function
     triangulate(triswitches, &triangle_in, &Triangulateio, 0);   
     
     // Set Triangulateio_exists to true
     Triangulateio_exists=true;
     
     // Build scaffold
     Tmp_mesh_pt= new TriangleScaffoldMesh(Triangulateio);
 
     // Convert mesh from scaffold to actual mesh
     build_from_scaffold(time_stepper_pt,use_attributes);
     
     // Kill the scaffold
     delete Tmp_mesh_pt;
     Tmp_mesh_pt=0;

     // Setup boundary coordinates for boundaries
     unsigned nb=nboundary();
     for (unsigned b=0;b<nb;b++)
      {
       this->setup_boundary_coordinates(b);
      }
    }

  /// \short Construct the mesh using a triangulateio object already built.
  /// Triangulateio object is updated according to the vector error_elem using
  /// triangulateio_refine function, in order to assign the maximum area value 
  /// required for each element. Calling the triangulate function using the 
  /// triangle_input_string a new triangulateio object(with a refined mesh)
  /// is built. The oomph mesh_pt is built using the scaffold mesh constructor 
  /// and the build_from_scaffold function
   TriangleMesh(Vector<double> &error_elem,
                const double &error_target,
                triangulateio &tmp_triangulateio,
                const std::string &input_string,
                TimeStepper* time_stepper_pt=
                &Mesh::Default_TimeStepper,
                const bool &use_attributes=false)  
    {

     // Create triangulateio object to refine
     triangulateio triangle_refine;
      
     // Initialize triangulateio structure
     initialize_triangulateio(Triangulateio);

     // Set max and min error ratio
     Max_error_ratio=10000;
     Min_error_ratio=0.0001;

     // Refine the mesh
     refine_triangulateio(tmp_triangulateio,
                          error_elem,
                          error_target,
                          triangle_refine);

     // Add the refinement string "-ra" to the previous one in order
     // to refine the mesh using the element area value build in
     // the triangulateio_refine function
     std::string refinement=input_string;
     refinement+="-ra";
     
     // Convert to a *char required by the triangulate function
     char * triswitches= const_cast<char*> (refinement.c_str() );

     // Build triangulateio refined object
     triangulate(triswitches, &triangle_refine, &Triangulateio, 0);       
     
     // Set Triangulateio_exists to true
     Triangulateio_exists=true;

     // Build scaffold
     Tmp_mesh_pt= new TriangleScaffoldMesh(Triangulateio);
       
     // Convert mesh from scaffold to actual mesh
     build_from_scaffold(time_stepper_pt,use_attributes);

     // Kill the scaffold
     delete Tmp_mesh_pt;
     Tmp_mesh_pt=0;

     // Setup boundary coordinates for boundaries
     unsigned nb=nboundary();
     for (unsigned b=0;b<nb;b++)
      {
       this->setup_boundary_coordinates(b);
      }       
    }

   /// \short Constructor with the .poly file 
   /// Build a triangulateio object using a .poly file.Triangle mesh input 
   /// file used in the first constructor 1.poly,1.ele and 1.node are no 
   /// longer needed.Calling the triangulate function using the 
   /// triangle_input_string a new triangulateio object(with a refined mesh)
   /// is built. The oomph mesh_pt is built using the scaffold mesh 
   /// constructor and the build_from_scaffold function
   TriangleMesh(const std::string& poly_file_name,
                std::string &input_string,
                TimeStepper* time_stepper_pt=
                &Mesh::Default_TimeStepper,
                const bool &use_attributes=false)
  
    {
     
     // Create the data structures required to call the triangulate function
     triangulateio triangle_in,triangle_out;
     
     // Initialize triangulateio structure
     initialize_triangulateio(triangle_out);
          
     // Convert to a *char required by the triangulate function
     char * triswitches= const_cast<char*> (input_string.c_str() );

     // Build the input triangulateio object from the .poly file
     build_triangulateio(poly_file_name, triangle_in);

     // Build the triangulateio out object
     triangulate(triswitches, &triangle_in, &Triangulateio, 0);
    
     // Set Triangulateio_exists to true
     Triangulateio_exists=true;
     
     // Build scaffold
     Tmp_mesh_pt= new TriangleScaffoldMesh(Triangulateio);
 
     // Convert mesh from scaffold to actual mesh
     build_from_scaffold(time_stepper_pt,use_attributes);

     // Kill the scaffold
     delete Tmp_mesh_pt;
     Tmp_mesh_pt=0;
     
     // Setup boundary coordinates for boundaries
     unsigned nb=nboundary();
     for (unsigned b=0;b<nb;b++)
      {
       this->setup_boundary_coordinates(b);
      }
    }

   /// Broken copy constructor
   TriangleMesh(const TriangleMesh& dummy) 
    { 
     BrokenCopy::broken_copy("TriangleMesh");
    } 
 
   /// Broken assignment operator
   void operator=(const TriangleMesh&) 
    {
     BrokenCopy::broken_assign("TriangleMesh");
    }


   /// Empty destructor 
   ~TriangleMesh() 
    {
     clear_triangulateio();
    }
 
   /// \short Setup boundary coordinate on boundary b.
   /// Boundary coordinate increases continously along
   /// polygonal boundary. It's zero at the lowest left
   /// smallest node on the boundary.
   void setup_boundary_coordinates(const unsigned& b)
    {
     // Dummy file
     std::ofstream some_file;
     setup_boundary_coordinates(b,some_file);
    }
 
   /// \short Setup boundary coordinate on boundary b. Doc Faces
   /// in outfile.
   /// Boundary coordinate increases continously along
   /// polygonal boundary. It's zero at the lowest left
   /// node on the boundary.
   void setup_boundary_coordinates(const unsigned& b,
                                   std::ofstream& outfile);

   /// Return the number of regions specified by attributes
   unsigned nregion() {return Region_element_pt.size();}

   /// Return the number of elements in region i
   unsigned nregion_element(const unsigned &i) 
    {return Region_element_pt[i].size();}
   
   /// Return the oomph_vertex_nodes_id vector storing the vector enumeration
   /// for the oomph_lib mesh, allowing the conversion to update the
   /// Triangulateio object used in the triangulateio_refinement()
   Vector<unsigned> oomph_vertex_nodes_id(){return Oomph_vertex_nodes_id;}
  
   /// Access to the triangulateio representation of the mesh
   triangulateio& triangulateio_representation()
    {
     // Check if the triangulateio object exists or not
     if(Triangulateio_exists)
      {
       return Triangulateio;
      }
     else
      // Error message. Call build_triangulateio function
      {
       std::ostringstream error_stream;
       error_stream<<"Function triangulateio_representation()\n"
                   <<"cannot be called if no triangulateio object\n"
                   <<"has been built. Check if a wrong constructor\n"
                   <<"has been being used or if the Triangulateio\n"
                   <<"has been alredy deleted"<<std::endl;
       throw OomphLibError(error_stream.str(),
                           "TriangleMesh<ELEMENT>::"
                           "triangulateio_representation()",
                           OOMPH_EXCEPTION_LOCATION); 
      }
    }
   
   /// Return the attribute associated with region i
   double region_attribute(const unsigned &i)
    {return Region_attribute[i];}

   /// Return the e-th element in the i-th region
   FiniteElement* region_element_pt(const unsigned &i,
                                    const unsigned &e)
    {return Region_element_pt[i][e];}

   /// \short Helper function.Write a Triangulate_object file with all the 
   /// triangulateio fields. String s is add to assign a different value for
   /// the input and/or output structure
   void write_triangulateio(struct triangulateio &triangle_out,std::string& s);
   
   /// Update the triangulateio object to the current mesh nodes position
   /// and the centre hole coordinates 
   void update_triangulateio(Vector<Vector<double> >&hole_centre)
    {
     
     // Move the hole center
     // Get number of holes
     unsigned nhole=Triangulateio.numberofholes;
     unsigned count_coord=0;
     for(unsigned ihole=0;ihole<nhole;ihole++)
      {
       Triangulateio.holelist[count_coord]+=hole_centre[ihole][0];  
       Triangulateio.holelist[count_coord+1]+=hole_centre[ihole][1]; 
       
       // Increment counter
       count_coord+=2;
      }

     // Call the update_triangulateio
     update_triangulateio();
    }
   
   /// Update the triangulateio object to the current mesh nodes position
   /// without a hole in the mesh
   void update_triangulateio()
    {   
   
     // Get number of points
     unsigned nnode = Triangulateio.numberofpoints;
     double new_x=0;
     double new_y=0;
   
     // Loop over the points
     for(unsigned inod=0;inod<nnode;inod++)
      {      
       // Get the node Id to be updated
       unsigned count=Oomph_vertex_nodes_id[inod];
     
       // Update vertices using the vertex_node_id giving for the triangulateio
       // vertex enumeration the corresponding oomphlib mesh enumeration
       Node* mesh_node_pt=this->node_pt(inod);
       new_x=mesh_node_pt->x(0); 
       new_y=mesh_node_pt->x(1); 
       Triangulateio.pointlist[count*2] = new_x;  
       Triangulateio.pointlist[(count*2)+1] = new_y;       
      }
    }

   /// Get the map of vector of sub_bound_id containing
   /// the Polyline boundary id and the oomph mesh boudary id (defined like 
   /// sub boundaries, assigned one for each segment) for each Polyline
   /// boundary in the second vector
   std::map<unsigned,Vector<unsigned> >& sub_boundary_id()
    {
     // Check if the triangulateio object exists or not
     if(Sub_boundary_id_exists)
      {
       return Sub_boundary_id;
      }
     else
      // Error message. Call build_triangulateio function
      {
       std::ostringstream error_stream;
       error_stream<<"Function sub_boundary_id()"
                   <<"cannot be called if\n no sub_boundary_id vector"
                   <<"has been built. Please call a different constructor\n "
                   <<std::endl;
       throw OomphLibError(error_stream.str(),
                           "TriangleMesh<ELEMENT>::"
                           "triangulateio_representation()",
                           OOMPH_EXCEPTION_LOCATION); 
      }
    }
   
   // Boolean defining if Triangulateio object has been built or not
   bool sub_boundary_id_exists(){return Sub_boundary_id_exists;}

   // Boolean defining if Triangulateio object has been built or not
   bool triangulateio_exists(){return Triangulateio_exists;}
   
    private:

   // Max error ratio used in the triangulateio_refine() 
   double Max_error_ratio;
   
   // Max error ratio used in the triangulateio_refine() 
   double Min_error_ratio;
   
   // Boolean defining if Triangulateio object has been built or not
   bool Triangulateio_exists;

   // Boolean defining if Triangulateio object has been built or not
   bool Sub_boundary_id_exists;
   
   /// Map of Vector of sub_bound_id containing 
   /// the Polyline boundary id and the oomph mesh boudary id (defined like
   /// sub boundaries, assigned one for each segment) for each Polyline
   /// boundary in the second vector
   std::map<unsigned,Vector<unsigned> > Sub_boundary_id;

   /// Temporary scaffold mesh
   TriangleScaffoldMesh* Tmp_mesh_pt;
   
   /// Pointer to [hierher change] triangulateio representation of the mesh
   triangulateio Triangulateio;
   
   /// Store the id number of corresponding vertex node in oomph-lib mesh
   /// allowing the conversion to update the Triangulateio object before 
   /// the mesh refinement
   Vector<unsigned> Oomph_vertex_nodes_id;
   
   /// Initialize triangulateio object
   void initialize_triangulateio(struct triangulateio &triangle_out);
   
   /// Build mesh from scaffold
   void build_from_scaffold(TimeStepper* time_stepper_pt,
                            const bool &use_attributes);

   /// \short Convert TriangleMeshPolyLine and TriangleMeshHolePolyLine
   /// to triangulateio object
   void build_triangulateio(TriangleMeshPolygon* &outer_boundary_pt,
                            Vector<TriangleMeshHolePolygon*> &inner_hole,
                            triangulateio &triangle_in);

   /// \short Helper function to create triangulateio object (return in
   /// triangle_data) from the .poly file 
   void build_triangulateio(const std::string& poly_file_name,
                                   triangulateio &triangle_data);
  
   /// Build a new triangulateio object, copying the previous triangulateio
   /// and updating the maximum area for each element, driven by the
   /// estimate computed
   void refine_triangulateio(triangulateio &triangle_out, 
                             Vector<double> &error_elem, 
                             const double &error_target,
                             triangulateio &triangle_refine);
   
   /// Clear the Triangulateio object
   void clear_triangulateio();

   /// Vectors of elements in each region differentiated by attribute
   Vector<Vector<FiniteElement* > > Region_element_pt;

   /// Vector of attributes associated with the elements in each region
   Vector<double> Region_attribute;
  };

//=========================================================================
// Unstructured Triangle Mesh upgraded to pseudo-elasticity
//=========================================================================
 template<class ELEMENT>
  class SolidTriangleMesh : public virtual TriangleMesh<ELEMENT>,
  public virtual SolidMesh 
  {
   
 public:


  /// \short Construct the mesh using TriangleMeshPolygon and 
  /// TriangleMeshHolePolygon objects. Build a triangulateio object
  /// using these objects and call the triangulate function with the
  /// triangle_input_string value, in order to build a new triangulateio 
  /// with the mesh parameter required.
  /// The oomph mesh_pt is built using the scaffold mesh constructor and
  /// the build_from_scaffold function
  SolidTriangleMesh(TriangleMeshPolygon* &outer_boundary_polyline_pt,
         Vector<TriangleMeshHolePolygon*> &inner_hole_pt,
         std::string &triangle_input_string,
         TimeStepper* time_stepper_pt=
         &Mesh::Default_TimeStepper,
         const bool &use_attributes=false):
   TriangleMesh<ELEMENT>(outer_boundary_polyline_pt, 
                         inner_hole_pt, 
                         triangle_input_string,
                         time_stepper_pt,
                         use_attributes)
   {
    //Assign the Lagrangian coordinates
    set_lagrangian_nodal_coordinates();
  
    // Setup boundary coordinates for boundaries
    unsigned nb=nboundary();
    for (unsigned b=0;b<nb;b++)
     {
      this->setup_boundary_coordinates(b);
     }
   }

  /// \short Construct the mesh using a triangulateio object already built.
  /// Triangulateio object is updated according to the vector error_elem using
  /// triangulateio_refine function, in order to assign the maximum area value 
  /// required for each element. Calling the triangulateio function using the 
  /// triangle_input_string a new triangulateio object(with a refined mesh)
  /// is built. The oomph mesh_pt is built using the scaffold mesh constructor 
  /// and the build_from_scaffold function
  SolidTriangleMesh(Vector<double> &error_elem,
                    const double &error_target,
                    triangulateio &tmp_triangulateio,
                    std::string &triangle_input_string, 
                    TimeStepper* time_stepper_pt=
                    &Mesh::Default_TimeStepper,
                    const bool &use_attributes=false):
   TriangleMesh<ELEMENT>(error_elem,
                         error_target,
                         tmp_triangulateio,
                         triangle_input_string,
                         time_stepper_pt,
                         use_attributes)
   {
    //Assign the Lagrangian coordinates
    set_lagrangian_nodal_coordinates();
   
    // Setup boundary coordinates for boundaries
    unsigned nb=nboundary();
    for (unsigned b=0;b<nb;b++)
     {
      this->setup_boundary_coordinates(b);
     }
   }
 
  /// Empty Destructor
  virtual ~SolidTriangleMesh() { }
 };

}

#endif
