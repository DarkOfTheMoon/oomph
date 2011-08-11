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
//This is the header file for the C++ wrapper functions for the
//mumps solver

//Include guards to prevent multiple inclusions of the header
#ifndef NEW_MUMPS_SOLVER_HEADER
#define NEW_MUMPS_SOLVER_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

#include "linear_solver.h"
#include "preconditioner.h"

#include <mumps_c_types.h>
#include <dmumps_c.h>

namespace oomph
{


//====================================================================
/// \short  Wrapper to Mumps solver
//====================================================================
class NewMumpsSolver : public LinearSolver
{
 
  public:
 
 /// \short Static flag that determines whether the warning about
 /// incorrect distribution of RHSs will be printed or not
 static bool Suppress_incorrect_rhs_distribution_warning_in_resolve;
 
 /// \short Constructor: Call setup
 NewMumpsSolver();
 
 /// Broken copy constructor
 NewMumpsSolver(const NewMumpsSolver& dummy) 
  { 
   BrokenCopy::broken_copy("NewMumpsSolver");
  } 
 
 /// Broken assignment operator
 void operator=(const NewMumpsSolver&) 
  {
   BrokenCopy::broken_assign("NewMumpsSolver");
  }

 ///Destructor: Cleanup
 ~NewMumpsSolver();
 
 /// Overload disable resolve so that it cleans up memory too
 void disable_resolve()
  {
   LinearSolver::disable_resolve();
   clean_up_memory();
  }


 /// Boolean to suppress warning about communicator not equal to MPI_COMM_WORLD
 bool& suppress_warning_about_MPI_COMM_WORLD()
  {
   return Suppress_warning_about_MPI_COMM_WORLD;
  }
 
 /// \short Solver: Takes pointer to problem and returns the results Vector
 /// which contains the solution of the linear system defined by
 /// the problem's fully assembled Jacobian and residual Vector.
 void solve(Problem* const &problem_pt, DoubleVector &result);

 /// \short Linear-algebra-type solver: Takes pointer to a matrix and rhs 
 /// vector and returns the solution of the linear system.
 /// The function returns the global result Vector.
 /// Note: if Delete_matrix_data is true the function 
 /// matrix_pt->clean_up_memory() will be used to wipe the matrix data.
 void solve(DoubleMatrixBase* const &matrix_pt,
            const DoubleVector &rhs,
            DoubleVector &result);
 
 /// \short Resolve the system defined by the last assembled Jacobian
 /// and the specified rhs vector if resolve has been enabled.
 /// Note: returns the global result Vector.
 void resolve(const DoubleVector &rhs, DoubleVector &result);

 /// Enable documentation of statistics
 void enable_doc_stats() {Doc_stats = true;}

 /// Disable documentation of statistics
 void disable_doc_stats() {Doc_stats = false;}

 /// \short Returns the time taken to assemble the Jacobian matrix and 
 /// residual vector
 double jacobian_setup_time()
  {
   return Jacobian_setup_time;
  }

 /// \short Return the time taken to solve the linear system (needs to be 
 /// overloaded for each linear solver)
 virtual double linear_solver_solution_time()
  {
   return Solution_time;
  }


 /// \short Return the flag that decides if we're actually solving the
 /// system or just assembling the Jacobian and the rhs.
 /// (Used only for timing runs, obviously)
 bool &suppress_solve() 
  {
   return Suppress_solve;
  }
 
 /// \short Returns Delete_matrix_data flag. NewMumpsSolver needs its own copy 
 /// of the input matrix, therefore a copy must be made if any matrix 
 /// used with this solver is to be preserved. If the input matrix can be 
 /// deleted the flag can be set to true to reduce the amount of memory 
 /// required, and the matrix data will be wiped using its clean_up_memory()
 /// function.  Default value is false.
 bool &delete_matrix_data()
  {
   return Delete_matrix_data;
  }
  
 /// \short Do the factorisation stage
 /// Note: if Delete_matrix_data is true the function 
 /// matrix_pt->clean_up_memory() will be used to wipe the matrix data.
 void factorise(DoubleMatrixBase* const &matrix_pt);
  
 /// \short Do the backsubstitution for mumps solver
 /// Note: returns the global result Vector.
 void backsub(const DoubleVector &rhs,
              DoubleVector &result);
 
 /// Clean up the memory allocated by the mumps solver
 void clean_up_memory(); 

 /// \short Default factor for workspace -- static so it can be overwritten
 /// globally.
 static int Default_workspace_scaling_factor;

  private:

 /// Initialise instance of mumps data structure
 void initialise_mumps();
 
 /// Shutdown mumps
 void shutdown_mumps();
 
 /// Jacobian setup time
 double Jacobian_setup_time;

 /// Solution time
 double Solution_time;

 /// Suppress solve?
 bool Suppress_solve;

 /// Set to true for NewMumpsSolver to output statistics (false by default).
 bool Doc_stats; 
  
 /// Boolean to suppress warning about communicator not equal to MPI_COMM_WORLD
 bool Suppress_warning_about_MPI_COMM_WORLD;

 /// Has mumps been initialised?
 bool Mumps_is_initialised;

 // Work space scaling factor
 unsigned Workspace_scaling_factor;

 /// \short Delete_matrix_data flag. NewMumpsSolver needs its own copy 
 /// of the input matrix, therefore a copy must be made if any matrix 
 /// used with this solver is to be preserved. If the input matrix can be 
 /// deleted the flag can be set to true to reduce the amount of memory 
 /// required, and the matrix data will be wiped using its clean_up_memory()
 /// function. Default value is false.
 bool Delete_matrix_data;

 /// Vector for row numbers
 Vector<int> Irn_loc;
 
 // Vector for column numbers
 Vector<int> Jcn_loc;
 
 // Vector for entries
 Vector<double> A_loc;
 
 /// Pointer to MUMPS struct that contains the solver data
 DMUMPS_STRUC_C* Mumps_struc_pt;
};
 



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



//====================================================================
/// An interface to allow Mumps to be used as an (exact) Preconditioner
//====================================================================
class NewMumpsPreconditioner : public Preconditioner
{
 public:

 /// Constructor.
 NewMumpsPreconditioner()
  {}
 
 /// Destructor.
 ~NewMumpsPreconditioner()
  {}
 
  /// Broken copy constructor.
  NewMumpsPreconditioner(const NewMumpsPreconditioner&)
  {
   BrokenCopy::broken_copy("NewMumpsPreconditioner");
  }


  /// Broken assignment operator.
  void operator=(const NewMumpsPreconditioner&)
  {
   BrokenCopy::broken_assign("NewMumpsPreconditioner");
  }
  
  /// \short Function to set up a preconditioner for the linear
  /// system defined by matrix_pt. This function must be called
  /// before using preconditioner_solve.
  /// Note: matrix_pt must point to an object of class
  /// CRDoubleMatrix or CCDoubleMatrix
  void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt)
  {
   oomph_info << "Setting up Mumps (exact) preconditioner" 
              << std::endl;
   
   DistributableLinearAlgebraObject* dist_matrix_pt=
    dynamic_cast<DistributableLinearAlgebraObject*>(matrix_pt);
   if (dist_matrix_pt!=0)
    {
     LinearAlgebraDistribution dist(dist_matrix_pt->distribution_pt());
     this->build_distribution(dist);
     Solver.factorise(matrix_pt);
    }
   else
    {
     std::ostringstream error_message_stream;                         
     error_message_stream                                        
      << "NewMumpsPreconditioner can only be applied to matrices derived \n"
      << "DistributableLinearAlgebraObject.\n";
     throw OomphLibError(error_message_stream.str(),     
                         "NewMumpsPreconditioner::setup()",             
                         OOMPH_EXCEPTION_LOCATION);        
    }
   
   
/*    // Wipe previously allocated memory */
/*    Solver.clean_up_memory(); */
   
/*    if (dynamic_cast<DistributableLinearAlgebraObject*>(matrix_pt) != 0) */
/*     { */
/*      this->distribution_pt() */
/*       ->build(dynamic_cast<DistributableLinearAlgebraObject*>(matrix_pt) */
/*               ->distribution_pt()); */
/*     } */
/*    else */
/*     { */
/*      this->distribution_pt()->build(problem_pt->communicator_pt(), */
/*                                     matrix_pt->nrow(),false); */
/*     } */
/*    Solver.doc_time() = false; */
// hierher do we need this?
/*    Solver.distribution_pt()->build(this->distribution_pt()); */
/*    Solver.factorise(matrix_pt); */
    
  }
  
  /// \short Function applies Mumps to vector r for (exact) 
  /// preconditioning, this requires a call to setup(...) first.
  void preconditioner_solve(const DoubleVector &r, DoubleVector &z)
  {
   Solver.resolve(r, z);
  }
  

  /// \short Clean up memory -- forward the call to the version in
  /// Mumps in its LinearSolver incarnation.
  void clean_up_memory()
  {
   Solver.clean_up_memory();
  }
  

  /// Enable documentation of timings
  void  enable_doc_time() {Solver.enable_doc_time();}
  
  /// Disable the documentation of timings
  void disable_doc_time() {Solver.disable_doc_time();}
  
  private:
  
  /// \short the Mumps solver emplyed by this preconditioner
  NewMumpsSolver Solver;

};

}

#endif

