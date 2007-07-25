// Copyright (c) 2005  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://gankit@scm.gforge.inria.fr/svn/cgal/trunk/Principal_component_analysis/include/CGAL/linear_least_squares_fitting_triangles.h $
// $Id: linear_least_squares_fitting_2.h 37882 2007-04-03 15:15:30Z spion $
//
// Author(s) : Pierre Alliez and Sylvain Pion and Ankit Gupta

#ifndef CGAL_LINEAR_LEAST_SQUARES_FITTING_UTIL_H
#define CGAL_LINEAR_LEAST_SQUARES_FITTING_UTIL_H

#include <CGAL/basic.h>
#include <CGAL/Object.h>
#include <CGAL/Linear_algebraCd.h>
#include <CGAL/PCA_tags.h>

CGAL_BEGIN_NAMESPACE

namespace CGALi {

// Initialize a matrix in n dimension by an array or numbers
template<typename K>
typename CGAL::Linear_algebraCd<typename K::FT>::Matrix
init_Matrix(int n, typename K::FT entries[]) {
  CGAL_assertion(n>1); // the dimension > 1
  typedef typename CGAL::Linear_algebraCd<typename K::FT>::Matrix Matrix;
  Matrix ret(n);
  for(int i=0;i<n;i++) {
    for(int j=0;j<n;j++) {
      ret[i][j]=entries[i*n+j];
    }
  }
  return ret;
} // end initialization of matrix

// assemble covariance matrix from a point set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Point_3*,   // used for indirection
			     const CGAL::PCA_dimension_0_tag& tag)
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;
  typedef typename K::Vector_3 Vector;

  // Matrix numbering:
  // 0          
  // 1 2
  // 3 4 5          
  covariance[0] = covariance[1] = covariance[2] = 
  covariance[3] = covariance[4] = covariance[5] = (FT)0.0;
  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    const Point& p = *it;
    Vector d = p - c;
    covariance[0] += d.x() * d.x();
    covariance[1] += d.x() * d.y();
    covariance[2] += d.y() * d.y();
    covariance[3] += d.x() * d.z();
    covariance[4] += d.y() * d.z();
    covariance[5] += d.z() * d.z();
  }
}

// assemble covariance matrix from a triangle set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Triangle_3*,// used for indirection
			     const CGAL::PCA_dimension_2_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Triangle_3  Triangle;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all triangles and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {1.0/12.0, 1.0/24.0, 1.0/24.0,
		1.0/24.0, 1.0/12.0, 1.0/24.0,
                1.0/24.0, 1.0/24.0, 1.0/12.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each triangle, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Triangle& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT delta[9] = {t[0].x(), t[1].x(), t[2].x(), 
		   t[0].y(), t[1].y(), t[2].y(),
                   t[0].z(), t[1].z(), t[2].z()};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT area = std::sqrt(t.squared_area());
    CGAL_assertion(area != 0.0);

    // Find the 2nd order moment for the triangle wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = 2 * area * transformation * moment * LA::transpose(transformation);
    
    // and add to covariance matrix
    covariance[0] += transformation[0][0];
    covariance[1] += transformation[1][0];
    covariance[2] += transformation[1][1];
    covariance[3] += transformation[2][0];
    covariance[4] += transformation[2][1];
    covariance[5] += transformation[2][2];

    mass += area;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

// assemble covariance matrix from a cuboid set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Iso_cuboid_3*,// used for indirection
			     const CGAL::PCA_dimension_3_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Iso_cuboid_3    Iso_cuboid;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all cuboids and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {1.0/3.0, 1.0/4.0, 1.0/4.0,
		1.0/4.0, 1.0/3.0, 1.0/4.0,
                1.0/4.0, 1.0/4.0, 1.0/3.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each cuboid, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Iso_cuboid& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT x0 = t[0].x();
    FT y0 = t[0].y();
    FT z0 = t[0].z();
    FT delta[9] = {t[1].x()-x0, t[3].x()-x0, t[5].x()-x0, 
		   t[1].y()-y0, t[3].y()-y0, t[5].y()-y0,
                   t[1].z()-z0, t[3].z()-z0, t[5].z()-z0};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT volume = t.volume();
    CGAL_assertion(volume != 0.0);

    // Find the 2nd order moment for the cuboid wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = volume * transformation * moment * LA::transpose(transformation);
    
    // Translate the 2nd order moment to the minimum corner (x0,y0,z0) of the cuboid.
    FT xav0 = (delta[0] + delta[1] + delta[2])/4.0;
    FT yav0 = (delta[3] + delta[4] + delta[5])/4.0;
    FT zav0 = (delta[6] + delta[7] + delta[8])/4.0;

    // and add to covariance matrix
    covariance[0] += transformation[0][0] + volume * (2*x0*xav0 + x0*x0);
    covariance[1] += transformation[1][0] + volume * (xav0*y0 + yav0*x0 + x0*y0);
    covariance[2] += transformation[1][1] + volume * (2*y0*yav0 + y0*y0);
    covariance[3] += transformation[2][0] + volume * (x0*zav0 + xav0*z0 + x0*z0);
    covariance[4] += transformation[2][1] + volume * (yav0*z0 + y0*zav0 + z0*y0);
    covariance[5] += transformation[2][2] + volume * (2*zav0*z0 + z0*z0);

    mass += volume;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

// assemble covariance matrix from a cuboid set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Iso_cuboid_3*,// used for indirection
			     const CGAL::PCA_dimension_2_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Iso_cuboid_3    Iso_cuboid;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all cuboids and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {7.0/3.0, 1.5, 1.5,
		1.5, 7.0/3.0, 1.5,
                1.5, 1.5, 7.0/3.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each cuboid, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Iso_cuboid& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT x0 = t[0].x();
    FT y0 = t[0].y();
    FT z0 = t[0].z();
    FT delta[9] = {t[1].x()-x0, t[3].x()-x0, t[5].x()-x0, 
		   t[1].y()-y0, t[3].y()-y0, t[5].y()-y0,
                   t[1].z()-z0, t[3].z()-z0, t[5].z()-z0};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT area = pow(delta[0]*delta[0] + delta[3]*delta[3] + delta[6]*delta[6],1/3.0)*pow(delta[1]*delta[1] + delta[4]*delta[4] + delta[7]*delta[7],1/3.0)*2 + pow(delta[0]*delta[0] + delta[3]*delta[3] + delta[6]*delta[6],1/3.0)*pow(delta[2]*delta[2] + delta[5]*delta[5] + delta[8]*delta[8],1/3.0)*2 + pow(delta[1]*delta[1] + delta[4]*delta[4] + delta[7]*delta[7],1/3.0)*pow(delta[2]*delta[2] + delta[5]*delta[5] + delta[8]*delta[8],1/3.0)*2;
    CGAL_assertion(area != 0.0);

    // Find the 2nd order moment for the cuboid wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = area * transformation * moment * LA::transpose(transformation);
    
    // Translate the 2nd order moment to the minimum corner (x0,y0,z0) of the cuboid.
    FT xav0 = (delta[0] + delta[1] + delta[2])/4.0;
    FT yav0 = (delta[3] + delta[4] + delta[5])/4.0;
    FT zav0 = (delta[6] + delta[7] + delta[8])/4.0;

    // and add to covariance matrix
    covariance[0] += transformation[0][0] + area * (2*x0*xav0 + x0*x0);
    covariance[1] += transformation[1][0] + area * (xav0*y0 + yav0*x0 + x0*y0);
    covariance[2] += transformation[1][1] + area * (2*y0*yav0 + y0*y0);
    covariance[3] += transformation[2][0] + area * (x0*zav0 + xav0*z0 + x0*z0);
    covariance[4] += transformation[2][1] + area * (yav0*z0 + y0*zav0 + z0*y0);
    covariance[5] += transformation[2][2] + area * (2*zav0*z0 + z0*z0);

    mass += area;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

// assemble covariance matrix from a sphere set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Sphere_3*,// used for indirection
			     const CGAL::PCA_dimension_3_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Sphere_3  Sphere;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all spheres and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {4.0/15.0, 0.0,     0.0,
		0.0,     4.0/15.0, 0.0,
                0.0,     0.0,     4.0/15.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each sphere, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Sphere& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT radius = std::sqrt(t.squared_radius());
    FT delta[9] = {radius, 0.0, 0.0, 
		   0.0, radius, 0.0,
                   0.0, 0.0, radius};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT volume = 4/3.0 * radius*t.squared_radius();
    CGAL_assertion(volume != 0.0);

    // Find the 2nd order moment for the sphere wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = (3.0/4.0) * volume * transformation * moment * LA::transpose(transformation);
    
    // Translate the 2nd order moment to the center of the sphere.
    FT x0 = t.center().x();
    FT y0 = t.center().y();
    FT z0 = t.center().z();

    // and add to covariance matrix
    covariance[0] += transformation[0][0] + volume * x0*x0;
    covariance[1] += transformation[1][0] + volume * x0*y0;
    covariance[2] += transformation[1][1] + volume * y0*y0;
    covariance[3] += transformation[2][0] + volume * x0*z0;
    covariance[4] += transformation[2][1] + volume * z0*y0;
    covariance[5] += transformation[2][2] + volume * z0*z0;

    mass += volume;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}
// assemble covariance matrix from a sphere set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Sphere_3*,// used for indirection
			     const CGAL::PCA_dimension_2_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Sphere_3  Sphere;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all spheres and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {4.0/3.0, 0.0,     0.0,
		0.0,     4.0/3.0, 0.0,
                0.0,     0.0,     4.0/3.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each sphere, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Sphere& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT radius = std::sqrt(t.squared_radius());
    FT delta[9] = {radius, 0.0, 0.0, 
		   0.0, radius, 0.0,
                   0.0, 0.0, radius};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT area = 4 * t.squared_radius();
    CGAL_assertion(area != 0.0);

    // Find the 2nd order moment for the sphere wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = (1.0/4.0) * area * transformation * moment * LA::transpose(transformation);
    
    // Translate the 2nd order moment to the center of the sphere.
    FT x0 = t.center().x();
    FT y0 = t.center().y();
    FT z0 = t.center().z();

    // and add to covariance matrix
    covariance[0] += transformation[0][0] + area * x0*x0;
    covariance[1] += transformation[1][0] + area * x0*y0;
    covariance[2] += transformation[1][1] + area * y0*y0;
    covariance[3] += transformation[2][0] + area * x0*z0;
    covariance[4] += transformation[2][1] + area * z0*y0;
    covariance[5] += transformation[2][2] + area * z0*z0;

    mass += area;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

// assemble covariance matrix from a tetrahedron set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Tetrahedron_3*,// used for indirection
			     const CGAL::PCA_dimension_3_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Tetrahedron_3  Tetrahedron;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all tetrahedrons and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {1.0/60.0,  1.0/120.0, 1.0/120.0,
		1.0/120.0, 1.0/60.0,  1.0/120.0,
		1.0/120.0, 1.0/120.0, 1.0/60.0};
  Matrix moment = init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each tetrahedron, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Tetrahedron& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT x0 = t[0].x();
    FT y0 = t[0].y();
    FT z0 = t[0].z();

    FT delta[9] = {t[1].x()-x0, t[2].x()-x0, t[3].x()-x0, 
		   t[1].y()-y0, t[2].y()-y0, t[3].y()-y0,
                   t[1].z()-z0, t[2].z()-z0, t[3].z()-z0};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT volume = t.volume();
    CGAL_assertion(volume != 0.0);

    // Find the 2nd order moment for the tetrahedron wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = 6 * volume * transformation * moment * LA::transpose(transformation);
    
    // Translate the 2nd order moment to the center of the tetrahedron.
    FT xav0 = (delta[0]+delta[1]+delta[2])/4.0;
    FT yav0 = (delta[3]+delta[4]+delta[5])/4.0;
    FT zav0 = (delta[6]+delta[7]+delta[8])/4.0;

    // and add to covariance matrix
    covariance[0] += transformation[0][0] + volume * (2*x0*xav0 + x0*x0);
    covariance[1] += transformation[1][0] + volume * (xav0*y0 + yav0*x0 + x0*y0);
    covariance[2] += transformation[1][1] + volume * (2*y0*yav0 + y0*y0);
    covariance[3] += transformation[2][0] + volume * (x0*zav0 + xav0*z0 + x0*z0);
    covariance[4] += transformation[2][1] + volume * (yav0*z0 + y0*zav0 + z0*y0);
    covariance[5] += transformation[2][2] + volume * (2*zav0*z0 + z0*z0);

    mass += volume;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

// assemble covariance matrix from a segment set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Segment_3*,// used for indirection
			     const CGAL::PCA_dimension_1_tag& tag)
{
  typedef typename K::FT          FT;
  typedef typename K::Point_3     Point;
  typedef typename K::Vector_3    Vector;
  typedef typename K::Segment_3  Segment;
  typedef typename CGAL::Linear_algebraCd<FT> LA;
  typedef typename LA::Matrix Matrix;

  // assemble covariance matrix as a semi-definite matrix. 
  // Matrix numbering:
  // 0
  // 1 2 
  // 3 4 5
  //Final combined covariance matrix for all segments and their combined mass
  FT mass = 0.0;

  // assemble 2nd order moment about the origin.  
  FT temp[9] = {1.0, 0.5, 0.0,
		0.5, 1.0, 0.0,
                0.0, 0.0, 0.0};
  Matrix moment = 1.0/3.0 * init_Matrix<K>(3,temp);

  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    // Now for each segment, construct the 2nd order moment about the origin.
    // assemble the transformation matrix.
    const Segment& t = *it;

    // defined for convenience.
    // FT example = CGAL::to_double(t[0].x());
    FT delta[9] = {t[0].x(), t[1].x(), 0.0, 
		   t[0].y(), t[1].y(), 0.0,
                   t[0].z(), t[1].z(), 1.0};
    Matrix transformation = init_Matrix<K>(3,delta);
    FT length = std::sqrt(t.squared_length());
    CGAL_assertion(length != 0.0);

    // Find the 2nd order moment for the segment wrt to the origin by an affine transformation.
    
    // Transform the standard 2nd order moment using the transformation matrix
    transformation = length * transformation * moment * LA::transpose(transformation);

    // and add to covariance matrix
    covariance[0] += transformation[0][0];
    covariance[1] += transformation[1][0];
    covariance[2] += transformation[1][1];
    covariance[3] += transformation[2][0];
    covariance[4] += transformation[2][1];
    covariance[5] += transformation[2][2];

    mass += length;
  }

  // Translate the 2nd order moment calculated about the origin to
  // the center of mass to get the covariance.
  covariance[0] += mass * (-1.0 * c.x() * c.x());
  covariance[1] += mass * (-1.0 * c.x() * c.y());
  covariance[2] += mass * (-1.0 * c.y() * c.y());
  covariance[3] += mass * (-1.0 * c.z() * c.x());
  covariance[4] += mass * (-1.0 * c.z() * c.y());
  covariance[5] += mass * (-1.0 * c.z() * c.z());

}

/*
// assemble covariance matrix from a triangle set 
template < typename InputIterator,
           typename K >
void
assemble_covariance_matrix_3_pierre(InputIterator first,
                             InputIterator beyond, 
                             typename K::FT covariance[6], // covariance matrix
                             const typename K::Point_3& c, // centroid
                             const K& ,                    // kernel
                             const typename K::Triangle_3*)// used for indirection
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;
  typedef typename K::Vector_3 Vector;
  typedef typename K::Triangle_3 Triangle;

  // Matrix numbering:
  // 0          
  // 1 2
  // 3 4 5          
  covariance[0] = covariance[1] = covariance[2] = 
  covariance[3] = covariance[4] = covariance[5] = (FT)0.0;
  FT sum_areas = 0.0;
  for(InputIterator it = first;
      it != beyond;
      it++)
  {
    const Triangle& triangle = *it;
    FT area = std::sqrt(triangle.squared_area());
    Point c_t = K().construct_centroid_3_object()(triangle[0],triangle[1],triangle[2]);
    sum_areas += area;

    // e1 = ab, e2 = ac
    Vector e1 = Vector(triangle[0],triangle[1]);
    Vector e2 = Vector(triangle[0],triangle[2]);

    FT c1 = (FT)(2.0 * area * 10.0 / 72.0);
    FT c2 = (FT)(2.0 * area *  7.0 / 72.0);
        
    covariance[0] += c1*(e1[0]*e1[0] + e2[0]*e2[0]) + (FT)2.0*c2*e1[0]*e2[0];
    covariance[1] += c1*(e1[1]*e1[0] + e2[1]*e2[0]) + c2*(e1[1]*e2[0] + e1[0]*e2[1]);
    covariance[2] += c1*(e1[1]*e1[1] + e2[1]*e2[1]) + (FT)2.0*c2*e1[1]*e2[1];
    covariance[3] += c1*(e1[2]*e1[0] + e2[2]*e2[0]) + c2*(e1[2]*e2[0] + e1[0]*e2[2]);
    covariance[4] += c1*(e1[2]*e1[1] + e2[2]*e2[1]) + c2*(e1[2]*e2[1] + e1[1]*e2[2]);
    covariance[5] += c1*(e1[2]*e1[2] + e2[2]*e2[2]) + (FT)2.0*c2*e1[2]*e2[2];
    
    // add area(t) c(t) * transpose(c(t))
    covariance[0] += area * c_t.x() * c_t.x();
    covariance[1] += area * c_t.y() * c_t.x();
    covariance[2] += area * c_t.y() * c_t.y();
    covariance[3] += area * c_t.z() * c_t.x();
    covariance[4] += area * c_t.z() * c_t.y();
    covariance[5] += area * c_t.z() * c_t.z();
  }

  // remove sum(area) * (c * transpose(c))
  covariance[0] -= sum_areas * c.x() * c.x();
  covariance[1] -= sum_areas * c.y() * c.x();
  covariance[2] -= sum_areas * c.y() * c.y();
  covariance[3] -= sum_areas * c.z() * c.x();
  covariance[4] -= sum_areas * c.z() * c.y();
  covariance[5] -= sum_areas * c.z() * c.z();
}
*/
// compute the eigen values and vectors of the covariance 
// matrix and deduces the best linear fitting plane.
// returns fitting quality
template < typename K >
typename K::FT
fitting_plane_3(const typename K::FT covariance[6], // covariance matrix
                const typename K::Point_3& c,       // centroid
                typename K::Plane_3& plane,         // best fit plane
                const K& )                          // kernel
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;
  typedef typename K::Plane_3  Plane;
  typedef typename K::Vector_3 Vector;

  // solve for eigenvalues and eigenvectors.
  // eigen values are sorted in descending order, 
  // eigen vectors are sorted in accordance.
  FT eigen_values[3];
  FT eigen_vectors[9];
  eigen_symmetric<FT>(covariance,3,eigen_vectors,eigen_values);

  // check unicity and build fitting line accordingly
  if(eigen_values[0] != eigen_values[1] &&
     eigen_values[0] != eigen_values[2])
  {
    // regular case
    Vector normal(eigen_vectors[6],
                  eigen_vectors[7],
                  eigen_vectors[8]);
    plane = Plane(c,normal);
    return (FT)1.0 - eigen_values[2] / eigen_values[0];
  } // end regular case
  else
  {
    // isotropic case (infinite number of directions)
    // by default: assemble a horizontal plane that goes
    // through the centroid.
    plane = Plane(c,Vector(0.0,0.0,1.0));
    return (FT)0.0;
  } 
}

// compute the eigen values and vectors of the covariance 
// matrix and deduces the best linear fitting line
// (this is an internal function)
// returns fitting quality
template < typename K >
typename K::FT
fitting_line_3(const typename K::FT covariance[6], // covariance matrix
               const typename K::Point_3& c,       // centroid
               typename K::Line_3& line,           // best fit line
               const K&  )                         // kernel
{
  typedef typename K::FT       FT;
  typedef typename K::Point_3  Point;
  typedef typename K::Line_3   Line;
  typedef typename K::Vector_3 Vector;

  // solve for eigenvalues and eigenvectors.
  // eigen values are sorted in descending order, 
  // eigen vectors are sorted in accordance.
  FT eigen_values[3];
  FT eigen_vectors[9];
  eigen_symmetric<FT>(covariance,3,eigen_vectors,eigen_values);

  // check unicity and build fitting line accordingly
  if(eigen_values[0] != eigen_values[1])
  {
    // regular case
    Vector direction(eigen_vectors[0],eigen_vectors[1],eigen_vectors[2]);
    line = Line(c,direction);
    return (FT)1.0 - eigen_values[1] / eigen_values[0];
  } // end regular case
  else
  {
    // isotropic case (infinite number of directions)
    // by default: assemble a horizontal plane that goes
    // through the centroid.
    line = Line(c,Vector(1.0,0.0,0.0));
    return (FT)0.0;
  } 
}

} // end namespace CGALi

CGAL_END_NAMESPACE

#endif // CGAL_LINEAR_LEAST_SQUARES_FITTING_UTIL_H
