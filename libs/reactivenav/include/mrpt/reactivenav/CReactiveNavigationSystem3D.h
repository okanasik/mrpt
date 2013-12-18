/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2013, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2013, MAPIR group, University of Malaga                |
   | Copyright (c) 2012-2013, University of Almeria                            |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */
#ifndef CReactiveNavigationSystem3D_H
#define CReactiveNavigationSystem3D_H

#include <mrpt/maps.h>
#include <mrpt/poses.h>
#include <mrpt/math.h>
#include <mrpt/synch.h>
#include <mrpt/utils/CTimeLogger.h>


#include "CAbstractPTGBasedReactive.h"

namespace mrpt
{
  namespace reactivenav
  {
		/** A 3D robot shape stored as a "sliced" stack of 2D polygons, used for CReactiveNavigationSystem3D */
		struct TRobotShape {
				std::vector<math::CPolygon>	polygons;		// The polygonal bases
				std::vector<float>			heights;		// Heights of the prisms
		};

		/** Implements a 3D reactive navigation system based on TP-Space, with an arbitrary holonomic
		*  reactive method running on it, and any desired number of PTG for transforming the navigation space.
		*  Both, the holonomic method and the PTGs can be customized by the apropriate user derived classes.
		*
		*   How to use:
		*      - A class with callbacks must be defined by the user and provided to the constructor.
		*      - loadConfigFile() must be called to set up the bunch of parameters from a config file (could be a memory-based virtual config file).
		*      - navigationStep() must be called periodically in order to effectively run the navigation. This method will internally call the callbacks to gather sensor data and robot positioning data.
		*
		* - SEP/2012: First design.
		* - JUL/2013: Integrated into MRPT library.
		* - DEC/2013: Code refactoring between this class and CAbstractHolonomicReactiveMethod
		*
		*  \sa CAbstractReactiveNavigationSystem, CParameterizedTrajectoryGenerator, CAbstractHolonomicReactiveMethod
		*  \ingroup mrpt_reactivenav_grp
		*/
		class REACTIVENAV_IMPEXP CReactiveNavigationSystem3D : public CAbstractPTGBasedReactive
		{
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		public:
			/** See docs in ctor of base class */
			CReactiveNavigationSystem3D(
				CReactiveInterfaceImplementation &react_iterf_impl,
				bool enableConsoleOutput = true,
				bool enableLogFile = false);

			/** Destructor */
			virtual ~CReactiveNavigationSystem3D();

			/** Reload the configuration from a file */
			void loadConfigFile(const mrpt::utils::CConfigFileBase &ini);

			/** Change the robot shape, which is taken into account for collision grid building. */
			void changeRobotShape( TRobotShape robotShape );

			/** Returns the number of different PTGs that have been setup */
			virtual size_t getPTG_count() const { return m_ptgmultilevel.size(); } 

			/** Gets the i'th PTG */
			virtual CParameterizedTrajectoryGenerator* getPTG(size_t i) 
			{
				ASSERT_(i<m_ptgmultilevel.size() && !m_ptgmultilevel[i].PTGs.empty())
				return m_ptgmultilevel[i].PTGs[0];  // Return the 0'th because the PTG itself is the same, what changes is the collision grid.
			}

		private:
			// ------------------------------------------------------
			//					PRIVATE DEFINITIONS
			// ------------------------------------------------------

			/** A set of PTGs of the same type, one per "height level" */
			struct REACTIVENAV_IMPEXP TPTGmultilevel
			{
				std::vector <CParameterizedTrajectoryGenerator*> PTGs;
				mrpt::vector_double		TPObstacles;
				TPoint2D				TP_Target;
				THolonomicMovement		holonomicmov;

				TPTGmultilevel();
				~TPTGmultilevel();
			};

			// ------------------------------------------------------
			//					PRIVATE	VARIABLES
			// ------------------------------------------------------
			mrpt::slam::CSimplePointsMap              m_WS_Obstacles_unsorted;  //!< The unsorted set of obstacles from the sensors
			std::vector<mrpt::slam::CSimplePointsMap> m_WS_Obstacles_inlevels; //!< One point cloud per 2.5D robot-shape-slice


			/** The robot 3D shape model */
			TRobotShape		m_robotShape;

			/** The set of PTG-transformations to be used: */
			std::vector <TPTGmultilevel>	m_ptgmultilevel;


			// Steps for the reactive navigation sytem.
			// ----------------------------------------------------------------------------
			virtual void STEP1_CollisionGridsBuilder();
			
			// See docs in parent class
			virtual bool STEP2_SenseObstacles();

			// See docs in parent class
			virtual void STEP3_WSpaceToTPSpace(const size_t ptg_idx,mrpt::vector_double &out_TPObstacles);

			/** Generates a pointcloud of obstacles, and the robot shape, to be saved in the logging record for the current timestep */
			virtual void loggingGetWSObstaclesAndShape(CLogFileRecord &out_log);
				


		}; // end class
	}
}


#endif





