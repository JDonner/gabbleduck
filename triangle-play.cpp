polygonPoints = vtk.vtkPoints()
polygonPoints.SetNumberOfPoints(4)
polygonPoints.InsertPoint(0, 0, 0, 0)
polygonPoints.InsertPoint(1, 1, 0, 0)
polygonPoints.InsertPoint(2, 1, 1, 0)
polygonPoints.InsertPoint(3, 0, 1, 0)
aPolygon = vtk.vtkPolygon()
aPolygon.GetPointIds().SetNumberOfIds(4)
aPolygon.GetPointIds().SetId(0, 0)
aPolygon.GetPointIds().SetId(1, 1)
aPolygon.GetPointIds().SetId(2, 2)
aPolygon.GetPointIds().SetId(3, 3)
aPolygonGrid = vtk.vtkUnstructuredGrid()
aPolygonGrid.Allocate(1, 1)
aPolygonGrid.InsertNextCell(aPolygon.GetCellType(), aPolygon.GetPointIds())
aPolygonGrid.SetPoints(polygonPoints)
aPolygonMapper = vtk.vtkDataSetMapper()
aPolygonMapper.SetInput(aPolygonGrid)
aPolygonActor = vtk.vtkActor()
aPolygonActor.SetMapper(aPolygonMapper)
aPolygonActor.AddPosition(6, 0, 2)
aPolygonActor.GetProperty().SetDiffuseColor(1, .4, .5)

GetNumberOfIds
//Polygon
  vtkPolygon *polygon = vtkPolygon::New();
  double polygonCoords[4][2];

  polygon->GetPointIds()->SetNumberOfIds(4);
  polygon->GetPointIds()->SetId(0,0);
  polygon->GetPointIds()->SetId(1,1);
  polygon->GetPointIds()->SetId(2,2);
  polygon->GetPointIds()->SetId(3,3);

  polygon->GetPoints()->SetNumberOfPoints(4);
  polygon->GetPoints()->SetPoint(0, 0.0, 0.0, 0.0);
  polygon->GetPoints()->SetPoint(1, 1.0, 0.0, 0.0);
  polygon->GetPoints()->SetPoint(2, 1.0, 1.0, 0.0);
  polygon->GetPoints()->SetPoint(3, 0.0, 1.0, 0.0);

void MakePolygon(VectorType const& /*normal*/,
                 Points const& planar_points,
                 Polygon& outPolygon)
{
   // We don't use the normal, but we might, for
   // find centroid -- needn't be centroid, just center-of-bounds
   // is ok, no? -- at least, I believe under the circumstances that
   // we'll be ok with it.

   PointType lo, hi;
   for (unsigned i = 0; i < Dimension; ++i) {
      lo[i] =  1e38, hi[i] = -1e38;
   }
   for (unsigned pt = 0; pt < planar_points.size(); ++pt) {
      for (unsigned dim = 0; dim < Dimension; ++dim) {
         if (planar_points[pt][dim] < lo[dim]) {
            lo[dim] = planar_points[pt][dim];
         }
         if (hi[dim] < planar_points[pt][dim]) {
            hi[dim] = planar_points[pt][dim];
         }
      }
   }

   // &&& Not sure this is on the plane! The bounds would define
   // a rectangle no (if they're all truly co-planar) and the ctr
   // would be on it, surely. Yah, should be ok.
   PointType ctr;
   for (unsigned i = 0; i < Dimension; ++i) {
      ctr[i] = (hi[i] + lo[i]) / 2.0;
   }

   typedef std::set<std::pair<InternalPrecisionType, unsigned> > VertexSet;
   // use for insertion sort
   VertexSet vertices;

   // We arbitrarily pick the first point as the axis against which
   // to measure the other pts' angles.
   PointType first = planar_points[0];
   VectorType v0 = planar_points[0] - ctr;
   InternalPrecisionType v0Norm = v0.GetNorm();

   // 0 angle with itself
   vertices.insert(std::make_pair(0.0, 0));

   // VTK setup
   polygon->GetPointIds()->SetNumberOfIds(planar_points.size());
   for (unsigned i = 0, size = planar_points.size(); i < size; ++i) {
      polygon->GetPointIds()->SetId(i,i);
   }


   for (unsigned iPt = 1; iPt < planar_points.size(); ++iPt) {
      VectorType vi = planar_points[iPt] - ctr;
      // dot product
      InternalPrecisionType cos_alpha = v0 * vi;
      // cross product
      VectorType crossed = CrossProduct(v0, vi);
      double sin_alpha = crossed.GetNorm() / (vi.GetNorm() * v0Norm);

      InternalPrecisionType angle_with_first = atan2(cos_alpha, sin_alpha);
      vertices.insert(std::make_pair(angle_with_first, iPt));
   }

//   copy(vertices.begin(), vertices.end(), back_insert_iterator<Polygon>(outPolygon));
   for (VertexSet::const_iterator it = vertices.begin(), end = vertices.end();
        it != end; ++it) {
      outPolygon.push_back(planar_points[it->second]);
   }

   polygon->GetPoints()->SetNumberOfPoints(planar_points.size());
   for (unsigned i = 0, size = planar_points.size(); i < size; ++i) {
      polygon->GetPoints()->SetPoint(i, planar_points[it->second]);
   }
}
