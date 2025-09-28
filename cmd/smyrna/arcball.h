/** KempoApi: The Turloc Toolkit *****************************/
/** *    *                                                  **/
/** **  **  Filename: ArcBall.h                             **/
/**   **    Version:  Common                                **/
/**   **                                                    **/
/**                                                         **/
/**  Arcball class for mouse manipulation.                  **/
/**                                                         **/
/**                                                         **/
/**                                                         **/
/**                                                         **/
/**                              (C) 1999-2003 Tatewake.com **/
/**   History:                                              **/
/**   08/17/2003 - (TJG) - Creation                         **/
/**   09/23/2003 - (TJG) - Bug fix and optimization         **/
/**   09/25/2003 - (TJG) - Version for NeHe Basecode users  **/
/**                                                         **/
/*************************************************************/

/*************************************************************************************/
/**                                                                                 **/
/** Copyright (c) 1999-2009 Tatewake.com                                            **/
/**                                                                                 **/
/** Permission is hereby granted, free of charge, to any person obtaining a copy    **/
/** of this software and associated documentation files (the "Software"), to deal   **/
/** in the Software without restriction, including without limitation the rights    **/
/** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       **/
/** copies of the Software, and to permit persons to whom the Software is           **/
/** furnished to do so, subject to the following conditions:                        **/
/**                                                                                 **/
/** The above copyright notice and this permission notice shall be included in      **/
/** all copies or substantial portions of the Software.                             **/
/**                                                                                 **/
/** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      **/
/** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        **/
/** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     **/
/** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          **/
/** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   **/
/** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN       **/
/** THE SOFTWARE.                                                                   **/
/**                                                                                 **/
/*************************************************************************************/

#pragma once

// 8<--Snip here if you have your own math types/funcs-->8 

# include <assert.h>

//Math types derived from the KempoApi tMath library
typedef struct {
    float X, Y;
} Tuple2fT;			//A generic 2-element tuple that is represented by single-precision floating point x,y coordinates. 

typedef struct {
    float X, Y, Z;
} Tuple3fT;			//A generic 3-element tuple that is represented by single precision-floating point x,y,z coordinates. 

typedef struct {
    float X, Y, Z, W;
} Tuple4fT;			//A 4-element tuple represented by single-precision floating point x,y,z,w coordinates. 

typedef union {
    float M[9];
    struct {
	//column major
	union {
	    float M00;
	    float XX;
	    float SX;
	};			//XAxis.X and Scale X
	union {
	    float M10;
	    float XY;
	};			//XAxis.Y
	union {
	    float M20;
	    float XZ;
	};			//XAxis.Z
	union {
	    float M01;
	    float YX;
	};			//YAxis.X
	union {
	    float M11;
	    float YY;
	    float SY;
	};			//YAxis.Y and Scale Y
	union {
	    float M21;
	    float YZ;
	};			//YAxis.Z
	union {
	    float M02;
	    float ZX;
	};			//ZAxis.X
	union {
	    float M12;
	    float ZY;
	};			//ZAxis.Y
	union {
	    float M22;
	    float ZZ;
	    float SZ;
	};			//ZAxis.Z and Scale Z
    } s;
} Matrix3fT;			//A single precision floating point 3 by 3 matrix. 

typedef union {
    float M[16];
    struct {
	float XX;
	float XY;
	float XZ;
	float XW;
	float YX;
	float YY;
	float YZ;
	float YW;
	float ZX;
	float ZY;
	float ZZ;
	float ZW;
    };
} Matrix4fT;			//A single precision floating point 4 by 4 matrix. 


//"Inherited" types
#define Point2fT    Tuple2fT	//A 2 element point that is represented by single precision floating point x,y coordinates.

#define Quat4fT     Tuple4fT	//A 4 element unit quaternion represented by single precision floating point x,y,z,w coordinates.

#define Vector3fT   Tuple3fT	//A 3-element vector that is represented by single-precision floating point x,y,z coordinates.

//Custom math, or speed overrides
#define FuncSqrt    sqrtf

//utility macros
//assuming IEEE-754(float), which i believe has max precision of 7 bits
# define Epsilon 1.0e-5

//Math functions

#ifdef ARCBALL_C
    /**
      * Returns a vector that is the vector cross product of vectors v1 and v2.
      * @param v1 the first vector
      * @param v2 the second vector
      */

static Vector3fT Vector3fCross(Vector3fT v1, Vector3fT v2) {
    return (Vector3fT){.X = v1.Y * v2.Z - v1.Z * v2.Y,
                       .Y = v1.Z * v2.X - v1.X * v2.Z,
                       .Z = v1.X * v2.Y - v1.Y * v2.X};
}

    /**
      * Computes the dot product of the this vector and vector v1.
      * @param  v1 the other vector
      */

static float Vector3fDot(Vector3fT NewObj, Vector3fT v1) {
    return NewObj.X * v1.X + NewObj.Y * v1.Y + NewObj.Z * v1.Z;
}

    /**
      * Returns the squared length of this vector.
      * @return the squared length of this vector
      */

static float Vector3fLengthSquared(Vector3fT NewObj) {
    return NewObj.X * NewObj.X + NewObj.Y * NewObj.Y + NewObj.Z * NewObj.Z;
}

    /**
      * Returns the length of this vector.
      * @return the length of this vector
      */

static float Vector3fLength(Vector3fT NewObj) {
    return FuncSqrt(Vector3fLengthSquared(NewObj));
}

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * quaternion argument. 
      * @param q1 the quaternion to be converted 
      */
    //$hack this can be optimized some(if s == 0)

static Matrix3fT Matrix3fSetRotationFromQuat4f(const Quat4fT *q1) {
    float n, s;
    float xs, ys, zs;
    float wx, wy, wz;
    float xx, xy, xz;
    float yy, yz, zz;
    Matrix3fT NewObj = {0};

    assert(q1);

    n = q1->X * q1->X + q1->Y * q1->Y + q1->Z * q1->Z + q1->W * q1->W;
    s = (n > 0.0f) ? (2.0f / n) : 0.0f;

    xs = q1->X * s;
    ys = q1->Y * s;
    zs = q1->Z * s;
    wx = q1->W * xs;
    wy = q1->W * ys;
    wz = q1->W * zs;
    xx = q1->X * xs;
    xy = q1->X * ys;
    xz = q1->X * zs;
    yy = q1->Y * ys;
    yz = q1->Y * zs;
    zz = q1->Z * zs;

    NewObj.s.XX = 1.0f - (yy + zz);
    NewObj.s.YX = xy - wz;
    NewObj.s.ZX = xz + wy;
    NewObj.s.XY = xy + wz;
    NewObj.s.YY = 1.0f - (xx + zz);
    NewObj.s.ZY = yz - wx;
    NewObj.s.XZ = xz - wy;
    NewObj.s.YZ = yz + wx;
    NewObj.s.ZZ = 1.0f - (xx + yy);
    return NewObj;
}

    /**
     * Sets the value of this matrix to the result of multiplying itself
     * with matrix m1. 
     * @param m1 the other matrix 
     */

static void Matrix3fMulMatrix3f(Matrix3fT * NewObj, const Matrix3fT * m1)
{
    Matrix3fT Result;		//safe not to initialize

    assert(NewObj && m1);

    // alias-safe way.
    Result.s.M00 =
	(NewObj->s.M00 * m1->s.M00) + (NewObj->s.M01 * m1->s.M10) +
	(NewObj->s.M02 * m1->s.M20);
    Result.s.M01 =
	(NewObj->s.M00 * m1->s.M01) + (NewObj->s.M01 * m1->s.M11) +
	(NewObj->s.M02 * m1->s.M21);
    Result.s.M02 =
	(NewObj->s.M00 * m1->s.M02) + (NewObj->s.M01 * m1->s.M12) +
	(NewObj->s.M02 * m1->s.M22);

    Result.s.M10 =
	(NewObj->s.M10 * m1->s.M00) + (NewObj->s.M11 * m1->s.M10) +
	(NewObj->s.M12 * m1->s.M20);
    Result.s.M11 =
	(NewObj->s.M10 * m1->s.M01) + (NewObj->s.M11 * m1->s.M11) +
	(NewObj->s.M12 * m1->s.M21);
    Result.s.M12 =
	(NewObj->s.M10 * m1->s.M02) + (NewObj->s.M11 * m1->s.M12) +
	(NewObj->s.M12 * m1->s.M22);

    Result.s.M20 =
	(NewObj->s.M20 * m1->s.M00) + (NewObj->s.M21 * m1->s.M10) +
	(NewObj->s.M22 * m1->s.M20);
    Result.s.M21 =
	(NewObj->s.M20 * m1->s.M01) + (NewObj->s.M21 * m1->s.M11) +
	(NewObj->s.M22 * m1->s.M21);
    Result.s.M22 =
	(NewObj->s.M20 * m1->s.M02) + (NewObj->s.M21 * m1->s.M12) +
	(NewObj->s.M22 * m1->s.M22);

    //copy result back to this
    *NewObj = Result;
}

    /**
      * Performs SVD on this matrix and gets scale and rotation.
      * Rotation is placed into rot3, and rot4.
      * @param rot3 the rotation factor(Matrix3d). if null, ignored
      * @param rot4 the rotation factor(Matrix4) only upper 3x3 elements are changed. if null, ignored
      * @return scale factor
      */

static float Matrix4fSVD(const Matrix4fT * NewObj, Matrix3fT * rot3,
			   Matrix4fT * rot4)
{
    float s, n;

    assert(NewObj);

    // this is a simple svd.
    // Not complete but fast and reasonable.
    // See comment in Matrix3d.

    s = FuncSqrt((NewObj->XX * NewObj->XX +
		  NewObj->XY * NewObj->XY +
		  NewObj->XZ * NewObj->XZ +
		  NewObj->YX * NewObj->YX +
		  NewObj->YY * NewObj->YY +
		  NewObj->YZ * NewObj->YZ +
		  NewObj->ZX * NewObj->ZX +
		  NewObj->ZY * NewObj->ZY +
		  NewObj->ZZ * NewObj->ZZ) / 3.0f);

    if (rot3)			//if pointer not null
    {
	rot3->s.XX = NewObj->XX;
	rot3->s.XY = NewObj->XY;
	rot3->s.XZ = NewObj->XZ;
	rot3->s.YX = NewObj->YX;
	rot3->s.YY = NewObj->YY;
	rot3->s.YZ = NewObj->YZ;
	rot3->s.ZX = NewObj->ZX;
	rot3->s.ZY = NewObj->ZY;
	rot3->s.ZZ = NewObj->ZZ;

	// zero-div may occur.

	n = 1.0f / FuncSqrt(NewObj->XX * NewObj->XX +
			    NewObj->XY * NewObj->XY +
			    NewObj->XZ * NewObj->XZ);
	rot3->s.XX *= n;
	rot3->s.XY *= n;
	rot3->s.XZ *= n;

	n = 1.0f / FuncSqrt(NewObj->YX * NewObj->YX +
			    NewObj->YY * NewObj->YY +
			    NewObj->YZ * NewObj->YZ);
	rot3->s.YX *= n;
	rot3->s.YY *= n;
	rot3->s.YZ *= n;

	n = 1.0f / FuncSqrt(NewObj->ZX * NewObj->ZX +
			    NewObj->ZY * NewObj->ZY +
			    NewObj->ZZ * NewObj->ZZ);
	rot3->s.ZX *= n;
	rot3->s.ZY *= n;
	rot3->s.ZZ *= n;
    }

    if (rot4)			//if pointer not null
    {
	*rot4 = *NewObj;
	// zero-div may occur.

	n = 1.0f / FuncSqrt(NewObj->XX * NewObj->XX +
			    NewObj->XY * NewObj->XY +
			    NewObj->XZ * NewObj->XZ);
	rot4->XX *= n;
	rot4->XY *= n;
	rot4->XZ *= n;

	n = 1.0f / FuncSqrt(NewObj->YX * NewObj->YX +
			    NewObj->YY * NewObj->YY +
			    NewObj->YZ * NewObj->YZ);
	rot4->YX *= n;
	rot4->YY *= n;
	rot4->YZ *= n;

	n = 1.0f / FuncSqrt(NewObj->ZX * NewObj->ZX +
			    NewObj->ZY * NewObj->ZY +
			    NewObj->ZZ * NewObj->ZZ);
	rot4->ZX *= n;
	rot4->ZY *= n;
	rot4->ZZ *= n;
    }

    return s;
}


static void Matrix4fSetRotationScaleFromMatrix3f(Matrix4fT * NewObj,
						 const Matrix3fT * m1)
{
    assert(NewObj && m1);

    NewObj->XX = m1->s.XX;
    NewObj->YX = m1->s.YX;
    NewObj->ZX = m1->s.ZX;
    NewObj->XY = m1->s.XY;
    NewObj->YY = m1->s.YY;
    NewObj->ZY = m1->s.ZY;
    NewObj->XZ = m1->s.XZ;
    NewObj->YZ = m1->s.YZ;
    NewObj->ZZ = m1->s.ZZ;
}


static void Matrix4fMulRotationScale(Matrix4fT * NewObj, float scale)
{
    assert(NewObj);

    NewObj->XX *= scale;
    NewObj->YX *= scale;
    NewObj->ZX *= scale;
    NewObj->XY *= scale;
    NewObj->YY *= scale;
    NewObj->ZY *= scale;
    NewObj->XZ *= scale;
    NewObj->YZ *= scale;
    NewObj->ZZ *= scale;
}

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * values in the T precision Matrix3d argument; the other elements of
      * this matrix are unchanged; a singular value decomposition is performed
      * on this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the passed rotation
      * components, and then the scale is reapplied to the rotational
      * components.
      * @param m1 T precision 3x3 matrix
      */

static void Matrix4fSetRotationFromMatrix3f(Matrix4fT * NewObj,
					    const Matrix3fT * m1)
{
    float scale;

    assert(NewObj && m1);

    scale = Matrix4fSVD(NewObj, NULL, NULL);

    Matrix4fSetRotationScaleFromMatrix3f(NewObj, m1);
    Matrix4fMulRotationScale(NewObj, scale);
}
#endif

// 8<--Snip here if you have your own math types/funcs-->8 
struct _ArcBall_t {
    Vector3fT StVec;
    Vector3fT EnVec;
    float AdjustWidth;
    float AdjustHeight;
    Matrix4fT Transform;
    Matrix3fT LastRot;
    Matrix3fT ThisRot;
    Point2fT MousePt;
    int isDragging;
};

ArcBall_t init_arcBall(float NewWidth, float NewHeight);
void arcmouseClick(void);
void arcmouseDrag(void);
