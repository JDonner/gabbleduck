#ifndef BETA_POINT_H
#define BETA_POINT_H

#include <vector>


typedef itk::Point<double, 3> Point;

struct PointPos
{
   // offset from parent
   Point offset;
   // absolute
   Point pos;
};


class BetaNode
{
public:
   void Explore();
private:
   void set_up_pipeline();
   PointPos pos;

#if NOT_YET
   // (or triangle list)
   Polygon polygon;
#endif

   ImageType::ConstPointer m_source;
   std::vector<BetaNode*> children;

#if NOT_YET
   static PointSet s_all_beta_points;
#endif
};

#endif // BETA_POINT_H
