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
#ifndef OOMPH_TRILINOS_OPERATORS_HEADER
#define OOMPH_TRILINOS_OPERATORS_HEADER

// trilinos headers
#include "ml_include.h"
#include "ml_MultiLevelPreconditioner.h"
#include "Ifpack.h"

//oomph-lib headers
#include "trilinos_helpers.h"
#include "preconditioner.h"

namespace oomph
{
 
//=============================================================================
/// Base class for Trilinos preconditioners as oomph-lib preconditioner.\n
//=============================================================================
 class TrilinosPreconditionerBase : public Preconditioner
  {
    public:
   
   /// Constructor.
   TrilinosPreconditionerBase()
    {
     // Initialise pointers
     Epetra_preconditioner_pt=0;
     Epetra_matrix_pt=0;
     Epetra_map_pt=0;
     Epetra_comm_pt=0;
#ifdef OOMPH_HAS_MPI
     Epetra_global_rows=0;
#endif     
    }
   
   /// \short Destructor. 
   virtual ~TrilinosPreconditionerBase()
    {
     clean_up_memory();
    }
   
   /// deletes the preconditioner, matrices and maps
   void clean_up_memory()
    {
     // delete the Epetra preconditioner
     delete Epetra_preconditioner_pt;
     Epetra_preconditioner_pt = 0; 
     
     // delete the epetra matrix
     delete Epetra_matrix_pt;    
     Epetra_matrix_pt = 0;

     // delete the epetra matrix row map
     delete Epetra_map_pt;    
     Epetra_map_pt = 0;

     // delete the epetra matrix row map
     delete Epetra_comm_pt;    
     Epetra_comm_pt = 0;
     
#ifdef OOMPH_HAS_MPI
     // delete the vector of global rows
     delete[] Epetra_global_rows;
     Epetra_global_rows = 0;
   
     // preconditioner distribution is not setup
     Preconditioner_distribution.clear();
#endif
    }
   
   /// Broken copy constructor.
   TrilinosPreconditionerBase(const TrilinosPreconditionerBase&)
    {
     BrokenCopy::broken_copy("TrilinosPreconditionerBase");
    }


   /// Broken assignment operator.
   void operator=(const TrilinosPreconditionerBase&)
    {
     BrokenCopy::broken_assign("TrilinosPreconditionerBase");
    }

   /// \short Function to set up a preconditioner for the linear system 
   /// defined by matrix_pt. This function must be called before using 
   /// preconditioner_solve. \n
   /// \b NOTE 1. matrix_pt must point to an object of class CRDoubleMatrix or 
   /// DistributedCRDoubleMatrix\n
   /// This method should be called by oomph-lib solvers and preconditioners
   void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);

   /// \short Function to setup a preconditioner for the linear system defined
   /// by the oomph-lib oomph_matrix_pt and Epetra epetra_matrix_pt matrices.\n
   /// This method is called by Trilinos solvers.
   void setup(Problem* problem_pt, DoubleMatrixBase* oomph_matrix_pt, 
              Epetra_CrsMatrix* epetra_matrix_pt);

#ifdef OOMPH_HAS_MPI
   /// \short Function applies preconditioner to DistributedVector r, this 
   /// requires a call to setup(...) first.
   void preconditioner_solve(const DistributedVector<double>&r,
                             DistributedVector<double> &z);
#endif
   
   /// \short preconditioner_solve preconditions vector r taking serial 
   /// oomph-lib vectors (Vector<double>) as arguments. \n
   /// \b if MPI then the serial oomph-lib vectors are converted to
   /// distributed Epetra vector, then the preconditioner is applied in 
   /// parallel before the results are copied back to a serial oomph-lib 
   /// vectors  
   void preconditioner_solve(const Vector<double>&r,
                             Vector<double> &z);

   /// Access function to Epetra_preconditioner_pt.\n
   /// For use with \c TrilinosAztecOOSolver
   Epetra_Operator*& epetra_operator_pt()
    {
     return Epetra_preconditioner_pt;
    }
 
   /// Access function to Epetra_preconditioner_pt (const version) \n
   /// For use with \c TrilinosAztecOOSolver
   Epetra_Operator* epetra_operator_pt() const 
    {
     return Epetra_preconditioner_pt;
    }
 
    protected:

   /// \short Function to set up a specific Trilinos preconditioner.
   /// This is called by setup(...).
   virtual void setup_trilinos_preconditioner
    (Problem* problem_pt, DoubleMatrixBase* oomph_matrix_pt, 
     Epetra_CrsMatrix* epetra_matrix_pt)=0;

   /// \short The preconditioner which will be set up using function
   /// setup_trilinos_preconditioner(...)
   Epetra_Operator* Epetra_preconditioner_pt;  

   /// \short Pointer used to store the epetra matrix - only used when this 
   /// preconditioner is setup using the oomph-lib interface
   Epetra_CrsMatrix* Epetra_matrix_pt;

   /// \short Pointer to store the row map of the Epetra_matrix
   Epetra_Map* Epetra_map_pt;

#ifdef OOMPH_HAS_MPI
   /// \short Global rows of the Epetra_matrix_pt - only used when this 
   /// preconditioner is setup using the oomph-lib interface (and MPI)
   int* Epetra_global_rows;

   /// \short Epetra communicator object (MPI version)
   Epetra_MpiComm* Epetra_comm_pt;
#else
   /// \short Epetra communicator object (serial version)
   Epetra_SerialComm* Epetra_comm_pt;
#endif
  };


 //============================================================================
 /// \short An interface to the Trilinos ML class - provides a function
 /// to construct a serial ML object, and functions to modify some
 /// of the ML paramaters.
 //============================================================================
 class TrilinosMLPreconditioner : public TrilinosPreconditionerBase
  {
    public:

   /// Constructor.
   TrilinosMLPreconditioner()
    {  
     // set default values
     Max_levels = 10;
     N_cycles = 1;
     Smoother_damping = 0.67;
     Smoother_sweeps = 2;
     Smoother_type = "symmetric Gauss-Seidel";
     Output=0;
    }

   /// Destructor empty -- clean up is done in base class
   virtual ~TrilinosMLPreconditioner() {}

   /// Broken copy constructor.
   TrilinosMLPreconditioner(const TrilinosMLPreconditioner&)
    {
     BrokenCopy::broken_copy("TrilinosMLPreconditioner");
    }

   /// Broken assignment operator.
   void operator=(const TrilinosMLPreconditioner&)
    {
     BrokenCopy::broken_assign("TrilinosMLPreconditioner");
    }

   /// Access function to Max_levels
   int& max_levels() {return Max_levels;}

   /// Access function to N_cycles
   int& n_cycles() {return N_cycles;}

   /// Access function to Smoother_damping
   double& smoother_damping() {return Smoother_damping;}

   /// Access function to Smoother_sweeps
   int& smoother_sweeps() {return Smoother_sweeps;}

   /// Function to set Smoother_type to "Jacobi"
   void set_smoother_jacobi() {Smoother_type="Jacobi";}

   /// Function to set Smoother_type to "symmetric Gauss-Seidel"
   void set_smoother_gauss_seidel() {Smoother_type="symmetric Gauss-Seidel";}

   /// Access function to Output
   int& output() {return Output;}

    protected:

   /// \short Function to set up the ML preconditioner. It is assumed
   /// Trilinos_matrix_pt points to a suitable matrix.
   void setup_trilinos_preconditioner
    (Problem* problem_pt, DoubleMatrixBase* oomph_matrix_pt, 
     Epetra_CrsMatrix* epetra_matrix_pt);

   /// Maximum number of levels used
   int Max_levels;

   /// Number of cycles used
   int N_cycles;

   /// Smoother damping paramater
   double Smoother_damping;

   /// Type of smoothing
   string Smoother_type;

   /// Number of smoother sweeps
   int Smoother_sweeps;

   /// Flag controls level of information output by ML
   int Output;
  };


 //============================================================================
 /// \short An interface to the Trilinos IFPACK class- provides a function
 /// to construct an IFPACK object, and functions to modify some
 /// of the IFPACK paramaters.
 //============================================================================
 class TrilinosIFPACKPreconditioner : public TrilinosPreconditionerBase
  {
    public:
   
   /// Constructor.
   TrilinosIFPACKPreconditioner()
    {
     // set default values
     Preconditioner_type = "ILU";
     ILU_fill_level = 0;
     ILUT_fill_level = 1.0;
     Overlap = 0;
    }

   /// Destructor -- empty, cleanup is done in base class
   virtual ~TrilinosIFPACKPreconditioner() {}

   /// Broken copy constructor.
   TrilinosIFPACKPreconditioner(const TrilinosIFPACKPreconditioner&)
    {
     BrokenCopy::broken_copy("TrilinosIFPACKPreconditioner");
    }


   /// Broken assignment operator.
   void operator=(const TrilinosIFPACKPreconditioner&)
    {
     BrokenCopy::broken_assign("TrilinosIFPACKPreconditioner");
    }

   /// Function to set Preconditioner_type to "ILU"
   void set_preconditioner_ILU()
    {
     Preconditioner_type="ILU";
    }

   /// Function to set Preconditioner_type to "ILUT"
   void set_preconditioner_ILUT()
    {
     Preconditioner_type="ILUT";
    }

   /// Access function for ILU_fill_level
   int& ilu_fill_level() {return ILU_fill_level;}

   /// Access function for ILUT_fill_level
   double& ilut_fill_level() {return ILUT_fill_level;}

    protected:

   /// \short Function to set up an IFPACK preconditioner. It is assumed
   /// Trilinos_matrix_pt points to a suitable matrix.
   void setup_trilinos_preconditioner
       (Problem* problem_pt, DoubleMatrixBase* oomph_matrix_pt, 
        Epetra_CrsMatrix* epetra_matrix_pt);

   /// Type of ILU preconditioner
   string Preconditioner_type;

   /// Level of fill for "ILU"
   int ILU_fill_level;

   /// Level of fill for "ILUT"
   double ILUT_fill_level;

   /// Value of overlap level - used in parallel ILU
   int Overlap;

  };
}
#endif
