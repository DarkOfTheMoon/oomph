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
#ifndef QMESH3OOMPH_D_HEADER
#define QMESH3OOMPH_D_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif


// ooomph-lib includes
#include "brick_mesh.h"
#include "refineable_mesh.h"
#include "refineable_brick_element.h"

namespace oomph
{

//=======================================================================
/// Intermediate mesh class that implements the mesh adaptation functions
/// specified in the RefineableMesh class for meshes that contain the
/// refineable variant of QElement s [The class ELEMENT provided
/// as the template parameter must be of type 
/// RefineableQElement<3>].
/// 
/// Mesh adaptation/refinement is implemented by OcTree 
/// procedures and any concrete implementation of this class needs to
/// provide a OcTreeForest representation of the initial (coarse) mesh.
//=======================================================================
template <class ELEMENT>
class RefineableBrickMesh : public RefineableMesh<ELEMENT>, 
 public virtual BrickMeshBase
{

public:


 /// Constructor: Setup static octree data
 RefineableBrickMesh()
  {
   // OcTree static data needs to be setup before octree-based mesh 
   // refinement works
   OcTree::setup_static_data();
  }

 /// Broken copy constructor
 RefineableBrickMesh(const RefineableBrickMesh& dummy) 
  { 
   BrokenCopy::broken_copy("RefineableBrickMesh");
  } 
 
 /// Broken assignment operator
 void operator=(const RefineableBrickMesh&) 
  {
   BrokenCopy::broken_assign("RefineableBrickMesh");
  }

 /// Destructor:
 virtual ~RefineableBrickMesh() {}

 /// \short Set up the tree forest associated with the Mesh. 
 /// Forwards call to setup_octree_forest()
 virtual void setup_tree_forest()
  {
   setup_octree_forest();
  }

 /// Do what it says...
 void setup_octree_forest()
  {
   //Turn elements into individual octrees and plant in forest
   Vector<TreeRoot*> trees_pt;
   unsigned nel=nelement();
   for (unsigned iel=0;iel<nel;iel++)
    {
     // Get pointer to full element type 
     ELEMENT* el_pt=dynamic_cast<ELEMENT*>(element_pt(iel));
     
     // Build associated octree(root) -- pass pointer to corresponding
     // finite element and add the pointer to vector of octree (roots):
     OcTreeRoot* octree_root_pt=new OcTreeRoot(el_pt);
     trees_pt.push_back(octree_root_pt);
    }
   // Plant OcTreeRoots in OcTreeForest
   this->Forest_pt = new OcTreeForest(trees_pt);
  }

  protected:

};

}

#endif
