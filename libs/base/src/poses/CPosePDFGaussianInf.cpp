/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2010  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include <mrpt/base.h>  // Precompiled headers


#include <mrpt/poses/CPosePDFGaussianInf.h>
#include <mrpt/poses/CPosePDFGaussian.h>
#include <mrpt/poses/CPose3DPDF.h>
#include <mrpt/math/CMatrix.h>

#include <mrpt/math/utils.h>
#include <mrpt/math/distributions.h>

#include <mrpt/random.h>

using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::poses;
using namespace mrpt::math;
using namespace mrpt::random;

using namespace std;

IMPLEMENTS_SERIALIZABLE( CPosePDFGaussianInf, CPosePDF, mrpt::poses )


/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPosePDFGaussianInf::CPosePDFGaussianInf() : mean(0,0,0), cov_inv()
{
}

/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPosePDFGaussianInf::CPosePDFGaussianInf(
	const CPose2D	&init_Mean,
	const CMatrixDouble33	&init_CovInv ) : mean(init_Mean), cov_inv(init_CovInv)
{
}

/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPosePDFGaussianInf::CPosePDFGaussianInf(const CPose2D  &init_Mean ) : mean(init_Mean), cov_inv()
{
}


/*---------------------------------------------------------------
						getMean
  Returns an estimate of the pose, (the mean, or mathematical expectation of the PDF)
 ---------------------------------------------------------------*/
void CPosePDFGaussianInf::getMean(CPose2D &p) const
{
	p=mean;
}

/*---------------------------------------------------------------
						getCovarianceAndMean
 ---------------------------------------------------------------*/
void CPosePDFGaussianInf::getCovarianceAndMean(CMatrixDouble33 &C,CPose2D &p) const
{
	p=mean;
	cov_inv.inv(C);
}

/*---------------------------------------------------------------
						writeToStream
  ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::writeToStream(CStream &out,int *version) const
{
	if (version)
		*version = 0;
	else
	{
		out << mean.x() << mean.y() << mean.phi();
		out << cov_inv(0,0) << cov_inv(1,1) << cov_inv(2,2);
		out << cov_inv(0,1) << cov_inv(0,2) << cov_inv(1,2);
	}
}

/*---------------------------------------------------------------
						readFromStream
  ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::readFromStream(CStream &in,int version)
{
	switch(version)
	{
	case 0:
		{
			TPose2D p;
			in >> p.x >> p.y >> p.phi;
			mean = p;

			in >> cov_inv(0,0) >> cov_inv(1,1) >> cov_inv(2,2);
			in >> cov_inv(0,1) >> cov_inv(0,2) >> cov_inv(1,2);
		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
}


/*---------------------------------------------------------------
						copyFrom
  ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::copyFrom(const CPosePDF &o)
{
	if (this == &o) return;		// It may be used sometimes

	if (IS_CLASS(&o, CPosePDFGaussianInf))
	{	// It's my same class:
		const CPosePDFGaussianInf *ptr = static_cast<const CPosePDFGaussianInf*>(&o);
		mean    = ptr->mean;
		cov_inv = ptr->cov_inv;
	}
	else
	{	// Convert to gaussian pdf:
		o.getMean(mean);

		CMatrixDouble33 o_cov(UNINITIALIZED_MATRIX);
		o.getCovariance(o_cov);
		o_cov.inv_fast(this->cov_inv);
	}
}

/*---------------------------------------------------------------
						copyFrom 3D
  ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::copyFrom(const CPose3DPDF &o)
{
	// Convert to gaussian pdf:
	mean = CPose2D(o.getMeanVal());

	if (IS_CLASS(&o, CPose3DPDFGaussianInf))
	{	// Cov is already in information form:
		const CPose3DPDFGaussianInf *ptr = static_cast<const CPose3DPDFGaussianInf*>(&o);
		cov_inv(0,0)=ptr->cov_inv(0,0);
		cov_inv(1,1)=ptr->cov_inv(1,1);
		cov_inv(2,2)=ptr->cov_inv(3,3);
		cov_inv(0,1)=cov_inv(1,0)=ptr->cov_inv(0,1);
		cov_inv(0,2)=cov_inv(2,0)=ptr->cov_inv(0,3);
		cov_inv(1,2)=cov_inv(2,1)=ptr->cov_inv(1,3);
	}
	else
	{
		CMatrixDouble66 C(UNINITIALIZED_MATRIX);
		o.getCovariance(C);

		// Clip to 3x3:
		CMatrixDouble33 o_cov(UNINITIALIZED_MATRIX);
		o_cov(0,0)=C(0,0);
		o_cov(1,1)=C(1,1);
		o_cov(2,2)=C(3,3);
		o_cov(0,1)=o_cov(1,0)=C(0,1);
		o_cov(0,2)=o_cov(2,0)=C(0,3);
		o_cov(1,2)=o_cov(2,1)=C(1,3);

		o_cov.inv_fast(this->cov_inv);
	}
}


/*---------------------------------------------------------------

  ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::saveToTextFile(const std::string &file) const
{
	FILE	*f=os::fopen(file.c_str(),"wt");
	if (!f) return;

	os::fprintf(f,"%f %f %f\n", mean.x(), mean.y(), mean.phi() );

	for (unsigned int i=0;i<3;i++)
		os::fprintf(f,"%f %f %f\n", cov_inv(i,0),cov_inv(i,1),cov_inv(i,2) );

	os::fclose(f);
}

/*---------------------------------------------------------------
						changeCoordinatesReference
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::changeCoordinatesReference(const CPose3D &newReferenceBase_ )
{
	const CPose2D newReferenceBase = CPose2D(newReferenceBase_);

	// The mean:
	mean = CPose2D( newReferenceBase + mean );

	// The covariance:
	rotateCov( newReferenceBase.phi() );
}

/*---------------------------------------------------------------
						changeCoordinatesReference
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::changeCoordinatesReference(const CPose2D &newReferenceBase )
{
	// The mean:
	mean = newReferenceBase + mean;
	// The covariance:
	rotateCov( newReferenceBase.phi() );
}


/*---------------------------------------------------------------
						changeCoordinatesReference
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::rotateCov(const double ang)
{
	const double ccos = cos(ang);
	const double ssin = sin(ang);

	const double rot_vals[] = {
		ccos, -ssin, 0.,
		ssin, ccos,  0.,
		0.  ,   0.,  1. };

	const CMatrixFixedNumeric<double,3,3> rot(rot_vals);

	// NEW_COV = H C H^T
	// NEW_COV^(-1) = (H C H^T)^(-1) = (H^T)^(-1) C^(-1) H^(-1)
	// rot: Inverse of a rotation matrix is its trasposed.
	//      But we need H^t^-1 -> H !! so rot stays unchanged:
	cov_inv = (rot * cov_inv * rot.transpose()).eval();
}

/*---------------------------------------------------------------
					drawSingleSample
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::drawSingleSample( CPose2D &outPart ) const
{
	MRPT_START;

	CMatrixDouble33 cov(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(cov);

	vector_double	v;
	randomGenerator.drawGaussianMultivariate(v,cov);

	outPart.x(  mean.x() + v[0] );
	outPart.y(  mean.y() + v[1] );
	outPart.phi( mean.phi() + v[2] );

	// Range -pi,pi
	outPart.normalizePhi();

	MRPT_END_WITH_CLEAN_UP( \
        cov_inv.saveToTextFile("__DEBUG_EXC_DUMP_drawSingleSample_COV_INV.txt"); \
		);
}

/*---------------------------------------------------------------
					drawManySamples
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::drawManySamples(
	size_t						N,
	std::vector<vector_double>	&outSamples ) const
{
	MRPT_START;

	CMatrixDouble33 cov(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(cov);

	std::vector<vector_double>	rndSamples;

	randomGenerator.drawGaussianMultivariateMany(rndSamples,N,cov);
	outSamples.resize( N );
	for (unsigned int i=0;i<N;i++)
	{
		outSamples[i].resize(3);
		outSamples[i][0] = mean.x() + rndSamples[i][0] ;
		outSamples[i][1] = mean.y() + rndSamples[i][1] ;
		outSamples[i][2] = mean.phi() + rndSamples[i][2] ;

		wrapToPiInPlace( outSamples[i][2] );
	}

	MRPT_END;
}


/*---------------------------------------------------------------
					bayesianFusion
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::bayesianFusion(const  CPosePDF &p1_,const  CPosePDF &p2_,const double& minMahalanobisDistToDrop )
{
	MRPT_START;

	MRPT_UNUSED_PARAM(minMahalanobisDistToDrop); // Not used in this class!

	ASSERT_( p1_.GetRuntimeClass() == CLASS_ID( CPosePDFGaussianInf )  );
	ASSERT_( p2_.GetRuntimeClass() == CLASS_ID( CPosePDFGaussianInf )  );

	const CPosePDFGaussianInf	*p1 = static_cast<const CPosePDFGaussianInf*>( &p1_ );
	const CPosePDFGaussianInf	*p2 = static_cast<const CPosePDFGaussianInf*>( &p2_ );

	const CMatrixDouble33& C1_inv = p1->cov_inv;
	const CMatrixDouble33& C2_inv = p2->cov_inv;

	CMatrixDouble31	x1 = CMatrixDouble31(p1->mean);
	CMatrixDouble31	x2 = CMatrixDouble31(p2->mean);

	this->cov_inv = C1_inv + C2_inv;

	CMatrixDouble33 cov(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(cov);

	CMatrixDouble31	x = cov * ( C1_inv*x1 + C2_inv*x2 );

	this->mean.x( x(0,0) );
	this->mean.y( x(1,0) );
	this->mean.phi( x(2,0) );
	this->mean.normalizePhi();

	MRPT_END;

}

/*---------------------------------------------------------------
					inverse
 ---------------------------------------------------------------*/
void	 CPosePDFGaussianInf::inverse(CPosePDF &o) const
{
	ASSERT_(o.GetRuntimeClass() == CLASS_ID(CPosePDFGaussianInf));
	CPosePDFGaussianInf	*out = static_cast<CPosePDFGaussianInf*>( &o );

	// The mean:
	out->mean = CPose2D(0,0,0) - mean;

	// The covariance:
	const double ccos = ::cos(mean.phi());
	const double ssin = ::sin(mean.phi());

	// jacobian:
	const double H_values[] = {
		-ccos, -ssin,  mean.x()*ssin-mean.y()*ccos,
		 ssin, -ccos,  mean.x()*ccos+mean.y()*ssin,
		 0   ,     0,  -1
		};
	const CMatrixFixedNumeric<double,3,3> H(H_values);

	out->cov_inv.noalias() = (H * cov_inv * H.transpose()).eval();  // o.cov = H * cov * Ht. It's the same with inverse covariances.
}


/*---------------------------------------------------------------
							+=
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::operator += ( const CPose2D &Ap)
{
	mean = mean + Ap;
	rotateCov( Ap.phi() );
}

/*---------------------------------------------------------------
						evaluatePDF
 ---------------------------------------------------------------*/
double  CPosePDFGaussianInf::evaluatePDF( const CPose2D &x ) const
{
	CMatrixDouble31	X = CMatrixDouble31(x);
	CMatrixDouble31	MU = CMatrixDouble31(mean);

	return math::normalPDF( X, MU, cov_inv.inverse() );
}

/*---------------------------------------------------------------
						evaluateNormalizedPDF
 ---------------------------------------------------------------*/
double  CPosePDFGaussianInf::evaluateNormalizedPDF( const CPose2D &x ) const
{
	CMatrixDouble31	X = CMatrixDouble31(x);
	CMatrixDouble31	MU = CMatrixDouble31(mean);

	CMatrixDouble33 cov(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(cov);

	return math::normalPDF( X, MU, cov ) / math::normalPDF( MU, MU, cov );
}

/*---------------------------------------------------------------
						assureSymmetry
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::assureSymmetry()
{
	// Differences, when they exist, appear in the ~15'th significant
	//  digit, so... just take one of them arbitrarily!
	cov_inv(0,1) = cov_inv(1,0);
	cov_inv(0,2) = cov_inv(2,0);
	cov_inv(1,2) = cov_inv(2,1);
}

/*---------------------------------------------------------------
						mahalanobisDistanceTo
 ---------------------------------------------------------------*/
double  CPosePDFGaussianInf::mahalanobisDistanceTo( const CPosePDFGaussianInf& theOther )
{
	MRPT_START;

	CArrayDouble<3> MU=CArrayDouble<3>(mean);
	MU-=CArrayDouble<3>(theOther.mean);

	wrapToPiInPlace(MU[2]);

	if (MU[0]==0 && MU[1]==0 && MU[2]==0)
		return 0; // This is the ONLY case where we know the result, whatever COVs are.

	CMatrixDouble33	COV_(UNINITIALIZED_MATRIX), cov2(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(COV_);
	theOther.cov_inv.inv(cov2);

	COV_+=cov2; // COV_ = cov1+cov2

	CMatrixDouble33 COV_inv(UNINITIALIZED_MATRIX);
	COV_.inv_fast(COV_inv);

	// (~MU) * (!COV_) * MU
	return std::sqrt( mrpt::math::multiply_HCHt_scalar(MU,COV_inv) );

	MRPT_END;
}

/*---------------------------------------------------------------
						operator <<
 ---------------------------------------------------------------*/
std::ostream &   mrpt::poses::operator << (
	std::ostream		&out,
	const CPosePDFGaussianInf	&obj )
{
	out << "Mean: " << obj.mean << "\n";
	out << "Inverse cov:\n" << obj.cov_inv << "\n";

	return out;
}

/*---------------------------------------------------------------
						operator +
 ---------------------------------------------------------------*/
poses::CPosePDFGaussianInf	 operator + ( const mrpt::poses::CPose2D &A, const mrpt::poses::CPosePDFGaussianInf &B  )
{
	poses::CPosePDFGaussianInf	ret(B);
	ret.changeCoordinatesReference(A);
	return ret;
}

/*---------------------------------------------------------------
						inverseComposition
  Set 'this' = 'x' - 'ref', computing the mean using the "-"
    operator and the covariances through the corresponding Jacobians.
 ---------------------------------------------------------------*/
void CPosePDFGaussianInf::inverseComposition(
	const CPosePDFGaussianInf &xv,
	const CPosePDFGaussianInf &xi  )
{
	// Use implementation in CPosePDFGaussian:
	CMatrixDouble33 xv_cov(UNINITIALIZED_MATRIX), xi_cov(UNINITIALIZED_MATRIX);
	xv.cov_inv.inv(xv_cov);
	xi.cov_inv.inv(xi_cov);

	const CPosePDFGaussian  xv_(xv.mean,xv_cov);
	const CPosePDFGaussian  xi_(xi.mean,xi_cov);

	CPosePDFGaussian  RET;
	RET.inverseComposition(xv_,xi_);

	// Copy result to "this":
	this->mean = RET.mean;
	RET.cov.inv(this->cov_inv);
}

/*---------------------------------------------------------------
						inverseComposition
  Set \f$ this = x1 \ominus x0 \f$ , computing the mean using
   the "-" operator and the covariances through the corresponding
    Jacobians (Given the 3x3 cross-covariance matrix of variables x0 and x0).
 ---------------------------------------------------------------*/
void CPosePDFGaussianInf::inverseComposition(
	const CPosePDFGaussianInf &x1,
	const CPosePDFGaussianInf &x0,
	const CMatrixDouble33  &COV_01 )
{
	// Use implementation in CPosePDFGaussian:
	CMatrixDouble33 x1_cov(UNINITIALIZED_MATRIX), x0_cov(UNINITIALIZED_MATRIX);
	x1.cov_inv.inv(x1_cov);
	x0.cov_inv.inv(x0_cov);

	const CPosePDFGaussian  x1_(x1.mean,x1_cov);
	const CPosePDFGaussian  x0_(x0.mean,x0_cov);

	CPosePDFGaussian  RET;
	RET.inverseComposition(x1_,x0_,COV_01);

	// Copy result to "this":
	this->mean = RET.mean;
	RET.cov.inv(this->cov_inv);
}


/*---------------------------------------------------------------
					jacobiansPoseComposition
 ---------------------------------------------------------------*/
void CPosePDFGaussianInf::jacobiansPoseComposition(
	const CPosePDFGaussianInf &x,
	const CPosePDFGaussianInf &u,
	CMatrixDouble33			 &df_dx,
	CMatrixDouble33			 &df_du)
{
/*
	df_dx =
	[ 1, 0, -sin(phi_x)*x_u-cos(phi_x)*y_u ]
	[ 0, 1,  cos(phi_x)*x_u-sin(phi_x)*y_u ]
	[ 0, 0,                              1 ]
*/
	df_dx.unit(3,1.0);

	const double   xu = u.mean.x();
	const double   yu = u.mean.y();
	const double   spx = sin(x.mean.phi());
	const double   cpx = cos(x.mean.phi());

	df_dx.get_unsafe(0,2) = -spx*xu-cpx*yu;
	df_dx.get_unsafe(1,2) =  cpx*xu-spx*yu;

/*
	df_du =
	[ cos(phi_x) , -sin(phi_x) ,  0  ]
	[ sin(phi_x) ,  cos(phi_x) ,  0  ]
	[         0  ,          0  ,  1  ]
*/
	// This is the homogeneous matrix of "x":
	df_du.get_unsafe(0,2) =
	df_du.get_unsafe(1,2) =
	df_du.get_unsafe(2,0) =
	df_du.get_unsafe(2,1) = 0;
	df_du.get_unsafe(2,2) = 1;

	df_du.get_unsafe(0,0) =  cpx;
	df_du.get_unsafe(0,1) = -spx;
	df_du.get_unsafe(1,0) =  spx;
	df_du.get_unsafe(1,1) =  cpx;
}

/*---------------------------------------------------------------
							+=
 ---------------------------------------------------------------*/
void  CPosePDFGaussianInf::operator += ( const CPosePDFGaussianInf &Ap)
{
	// COV:
	CMatrixDouble33  OLD_COV(UNINITIALIZED_MATRIX);
	this->cov_inv.inv(OLD_COV);

	CMatrixDouble33  df_dx, df_du;

	CPosePDFGaussianInf::jacobiansPoseComposition(
		*this,  // x
		Ap,     // u
		df_dx,
		df_du );

	// this->cov = H1*this->cov*~H1 + H2*Ap.cov*~H2;
	CMatrixDouble33 cov;

	CMatrixDouble33 Ap_cov(UNINITIALIZED_MATRIX);
	Ap.cov_inv.inv(Ap_cov);

	df_dx.multiply_HCHt( OLD_COV, cov );
	df_du.multiply_HCHt( Ap_cov,  cov, true); // Accumulate result

	cov.inv_fast(this->cov_inv);

	// MEAN:
	this->mean = this->mean + Ap.mean;
}

bool mrpt::poses::operator==(const CPosePDFGaussianInf &p1,const CPosePDFGaussianInf &p2)
{
	return p1.mean==p1.mean && p1.cov_inv==p2.cov_inv;
}
