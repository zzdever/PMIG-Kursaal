#ifndef UTILS_H
#define UTILS_H

#include <QPoint>

#include "params.h"

#include "p2dengine/general/p2dmath.h"

namespace CoordinateInterface {

P2DVec2 MapToEngine(QPointF pIn);
QPointF MapToScene(P2DVec2 pIn);

}


#endif // UTILS_H
