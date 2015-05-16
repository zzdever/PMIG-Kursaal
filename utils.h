#ifndef UTILS_H
#define UTILS_H

#include <QPoint>

#include "params.h"

#include "p2dengine/general/p2dmath.h"

namespace CoordinateInterface {

P2DVec2 MapToEngine(QPointF pIn);
QPointF MapToScene(P2DVec2 pIn);

double RadToDeg(double Rad);
double DegToRad(double Deg);
}


#endif // UTILS_H
