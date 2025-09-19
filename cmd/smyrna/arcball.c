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

#include <glcomp/glcompdefs.h>
#define ARCBALL_C
#include "smyrnadefs.h"
#include "arcball.h"

static void setBounds(ArcBall_t *a, float NewWidth, float NewHeight) {
    assert((NewWidth > 1.0f) && (NewHeight > 1.0f));
    //Set adjustment factor for width/height
    a->AdjustWidth = 1.0f / ((NewWidth - 1.0f) * 0.5f);
    a->AdjustHeight = 1.0f / ((NewHeight - 1.0f) * 0.5f);
}

static void mapToSphere(ArcBall_t * a, const Point2fT * NewPt,
			Vector3fT * NewVec)
{
    Point2fT TempPt;

    //Copy paramter into temp point
    TempPt = *NewPt;

    //Adjust point coords and scale down to range of [-1 ... 1]
    TempPt.s.X = (TempPt.s.X * a->AdjustWidth) - 1.0f;
    TempPt.s.Y = 1.0f - (TempPt.s.Y * a->AdjustHeight);

    //Compute the square of the length of the vector to the point from the center
    float length = TempPt.s.X * TempPt.s.X + TempPt.s.Y * TempPt.s.Y;

    //If the point is mapped outside of the sphere... (length > radius squared)
    if (length > 1.0f) {

	//Compute a normalizing factor (radius / sqrt(length))
	float norm = 1.0f / FuncSqrt(length);

	//Return the "normalized" vector, a point on the sphere
	NewVec->X = TempPt.s.X * norm;
	NewVec->Y = TempPt.s.Y * norm;
	NewVec->Z = 0.0f;
    } else			//Else it's on the inside
    {
	//Return a vector to a point mapped inside the sphere sqrt(radius squared - length)
	NewVec->X = TempPt.s.X;
	NewVec->Y = TempPt.s.Y;
	NewVec->Z = FuncSqrt(1.0f - length);
    }
}

static Matrix4fT Transform = { {1.0f, 0.0f, 0.0f, 0.0f,	// NEW: Final Transform
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f}
};

static Matrix3fT LastRot = { {1.0f, 0.0f, 0.0f,	// NEW: Last Rotation
			      0.0f, 1.0f, 0.0f,
			      0.0f, 0.0f, 1.0f}
};

static Matrix3fT ThisRot = { {1.0f, 0.0f, 0.0f,	// NEW: This Rotation
			      0.0f, 1.0f, 0.0f,
			      0.0f, 0.0f, 1.0f}
};

//Create/Destroy
ArcBall_t init_arcBall(float NewWidth, float NewHeight) {
    ArcBall_t a = {.Transform = Transform, .LastRot = LastRot,
                   .ThisRot = ThisRot};

    //Set initial bounds
    setBounds(&a, NewWidth, NewHeight);
    return a;
}

//Mouse down
static void click(ArcBall_t * a, const Point2fT * NewPt)
{
    //Map the point to the sphere
    mapToSphere(a, NewPt, &a->StVec);
}

//Mouse drag, calculate rotation
static Quat4fT drag(ArcBall_t *a, const Point2fT *NewPt) {
    //Map the point to the sphere
    mapToSphere(a, NewPt, &a->EnVec);

    //Return the quaternion equivalent to the rotation
    // compute the vector perpendicular to the begin and end vectors
    const Vector3fT Perp = Vector3fCross(&a->StVec, &a->EnVec);
    
    // compute the length of the perpendicular vector
    if (Vector3fLength(&Perp) > Epsilon) { // if it’s non-zero
        // we're OK, so return the perpendicular vector as the transform after
        // all
        return (Quat4fT){.X = Perp.X,
                         .Y = Perp.Y,
                         .Z = Perp.Z,
        // in the quaternion values, W is cosine (theta / 2), where theta is
        // rotation angle
                         .W = Vector3fDot(&a->StVec, &a->EnVec)};
    } else { // if it’s zero
        // the begin and end vectors coincide, so return an identity transform
        return (Quat4fT){0};
    }
}

void arcmouseClick(void)
{
    view->arcball->isDragging = 1;	// Prepare For Dragging
    view->arcball->LastRot = view->arcball->ThisRot;	// Set Last Static Rotation To Last Dynamic One
    click(view->arcball, &view->arcball->MousePt);

}

void arcmouseDrag(void)
{
    Quat4fT ThisQuat = drag(view->arcball, &view->arcball->MousePt);
    Matrix3fSetRotationFromQuat4f(&view->arcball->ThisRot, &ThisQuat);	// Convert Quaternion Into Matrix3fT
    Matrix3fMulMatrix3f(&view->arcball->ThisRot, &view->arcball->LastRot);	// Accumulate Last Rotation Into This One
    Matrix4fSetRotationFromMatrix3f(&view->arcball->Transform, &view->arcball->ThisRot);	// Set Our Final Transform's Rotation From This One

}
