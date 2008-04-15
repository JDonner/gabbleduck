#include "local-maxima.h"

#include <itkNeighborhoodIterator.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConstantBoundaryCondition.h>

#include <queue>
#include <set>
#include <iostream>

#include <assert.h>


typedef itk::BinaryThresholdImageFilter< InputImage, BeatenImage > ThresholdFilter;

typedef itk::ConstantBoundaryCondition<BeatenImage> BeatenConstantBoundaryCondition;
typedef itk::NeighborhoodIterator<BeatenImage, BeatenConstantBoundaryCondition> BeatenNit;

typedef itk::ConstantBoundaryCondition<InputImage> InputConstantBoundaryCondition;
// The boundary condition here shouldn't matter (image becomes 0
// toward the edges) but if it did, we'd want the beyond pixels to
// be 0).
typedef itk::ConstNeighborhoodIterator<
   InputImage, InputConstantBoundaryCondition> InputNit;

typedef BeatenImage UncollectedImage;
typedef BeatenNit   UncollectedNit;

typedef std::queue<BeatenNit::IndexType> PosQueue;


using namespace std;

extern void fully_explore(InputNit itIn,
                          BeatenNit itBt,
                          unsigned nNbrs,
                          PosQueue& newly_beaten_queue);

unsigned g_n_explored = 0;
unsigned g_n_components = 0;
unsigned g_n_snuffed = 0;

// 2008-02-05 - three passes:
// * threshold phase,
// full-exploration phase -
//   clear-beaten, enqueue new losers (but after enqueuing, mark them
//   as pixmap-beaten) if I'm beaten, nothing special, no need to
//   enqueue me, I'm done. Just finish nbrs.  Ah but there's that 2nd
//   condition which may change, now I can deal out 'tie' deaths.
// - if they're pix-mapped 'beaten', then they're already enqueued or
// dealt with.  propagating beating phase.
// -- the queue should indeed be a queue, traversing all the pixels at
// most, once.  So, ok.

BeatenImage::Pointer
   threshhold_phase(InputImage::Pointer input, double threshhold)
{
   ThresholdFilter::Pointer thresh = ThresholdFilter::New();
   thresh->SetInput(input);
cout << "threshhold: " << threshhold << endl;

   thresh->SetLowerThreshold(threshhold);
   thresh->SetUpperThreshold(1.0e38);
   thresh->SetInsideValue(true);
   thresh->SetOutsideValue(false);

   thresh->Update();
   // go!
   return thresh->GetOutput();
}


void exploration_phase(InputImage::Pointer inputImage,
                       BeatenImage::Pointer beatenImage)
{
   assert(inputImage->GetBufferPointer());

   //////////////////////////////////////////////
   // Legit preparation

   // define the neighborhoods - simple, complete cube
   BeatenNit::RadiusType beatenRadius;
   beatenRadius.Fill(1);
   BeatenConstantBoundaryCondition beatenBoundaryCondition;
   beatenBoundaryCondition.SetConstant(false);
   BeatenNit itBt(beatenRadius, beatenImage, beatenImage->GetLargestPossibleRegion());
   itBt.SetBoundaryCondition(beatenBoundaryCondition);


   InputNit::RadiusType inputRadius;
   inputRadius.Fill(1);
   // The boundary condition here shouldn't matter (image becomes 0
   // toward the edges) but if it did, we'd want the beyond pixels to
   // be 0).
   InputConstantBoundaryCondition inputBoundaryCondition;
   inputBoundaryCondition.SetConstant(0.0);
   InputNit itIn(inputRadius, inputImage, inputImage->GetLargestPossibleRegion());
   itIn.SetBoundaryCondition(inputBoundaryCondition);

   InputNit itNIn(inputRadius, inputImage, inputImage->GetLargestPossibleRegion());
   BeatenNit itNBt(beatenRadius, beatenImage, beatenImage->GetLargestPossibleRegion());


   //////////////////////////////////////////////
   // Algorithm proper
   //////////////////////////////////////////////

   unsigned nNbrs = itIn.Size();
   PosQueue newly_beaten_queue;

unsigned top_level = 0, n_unbeaten = 0;
   for (itBt.GoToBegin(); !itBt.IsAtEnd(); ++itBt) {
++top_level;
      if (itBt.GetCenterPixel()) {
++n_unbeaten;
         BeatenImage::IndexType idx = itBt.GetIndex();
         // &&& may be type-incompatible...
         itIn.SetLocation(idx);
         fully_explore(itIn, itBt,
                       nNbrs, newly_beaten_queue);
         while (!newly_beaten_queue.empty()) {
            idx = newly_beaten_queue.front();
            newly_beaten_queue.pop();

            itNIn.SetLocation(idx);
            itNBt.SetLocation(idx);
            fully_explore(itNIn, itNBt,
                          nNbrs, newly_beaten_queue);
         }
      }
   }
cout << "top level: " << top_level << endl
     << "unbeat: " << n_unbeaten << endl;
}


void fully_explore(InputNit itIn,
                   BeatenNit itBt,
                   unsigned nNbrs,
                   PosQueue& newly_beaten_queue)
{
++g_n_explored;
   unsigned ctr = nNbrs / 2;
   InputImage::PixelType ctrVal = itIn.GetCenterPixel();
   bool need_tie_pass = false;
   bool killing_ties = false;

   for (unsigned i = 0; i < nNbrs; ++i) {
      if (i == ctr)
         continue;
      // neighbor is not beaten
      if (itBt.GetPixel(i)) {
         InputImage::PixelType nbrVal = itIn.GetPixel(i);
         if (ctrVal < nbrVal) {
            // pixel is beaten, ie no longer candidate for local maximum
            itBt.SetCenterPixel(false);
++g_n_snuffed;
            need_tie_pass = true;
            // if we're beaten, every neighbor to which we're equal
            // is beaten, too.
            killing_ties = true;
         }
         else if (ctrVal > nbrVal or killing_ties) {
            itBt.SetPixel(i, false);
++g_n_snuffed;
            InputImage::IndexType nbrIdx = itIn.GetIndex(i);
            newly_beaten_queue.push(nbrIdx);
         }
         else {
            assert(ctrVal == nbrVal and !killing_ties);
         }
      }
   }
}

// Gather up all points connected to <itFrom> includes corner-connected
void collect_connected(UncollectedNit itFrom,
//                       UncollectedImage::Pointer uncollected,
                       PointSet* outSeedRegion)
{
++g_n_components;
   unsigned nNbrs = itFrom.Size();
   unsigned ctr = nNbrs / 2;
   PosQueue unexplored;

   unexplored.push(itFrom.GetIndex(ctr));
   while (!unexplored.empty()) {
      // pick off
      UncollectedImage::IndexType idx = unexplored.front();
      unexplored.pop();
      // set up
      UncollectedNit it = itFrom;
      it.SetLocation(idx);
      // take action
      it.SetCenterPixel(false);
      outSeedRegion->insert(it.GetIndex(ctr));
      // explore
      for (unsigned i = 0; i < nNbrs; ++i) {
         if (i == ctr)
            continue;
         if (it.GetPixel(i)) {
            unexplored.push(it.GetIndex(i));
         }
      }
   }
}


// We collect 'regions' instead of leaving the points separate,
// because we'd like to eventually choose one point to represent
// the whole region, following the paper (not sure how difference
// that would make, though. The most important thing is whether a
// point is beta-like, and whether we find it as a seed or not
// doesn't seem like it should make much difference).
void collect_seed_regions(BeatenImage::Pointer beatenImage,
                          SeedRegionSet& outSeedRegions)
{
   BeatenNit::RadiusType beatenRadius;
   beatenRadius.Fill(1);
   BeatenConstantBoundaryCondition beatenBoundaryCondition;
   beatenBoundaryCondition.SetConstant(false);
   BeatenNit itBt(beatenRadius, beatenImage, beatenImage->GetLargestPossibleRegion());
   itBt.SetBoundaryCondition(beatenBoundaryCondition);

   // hum, I guess I can destroy the bitmap while collecting the seeds..
   for (itBt.GoToBegin(); !itBt.IsAtEnd(); ++itBt) {
      if (itBt.GetCenterPixel()) {
         // &&& Hey there, needs memory management
         PointSet* region = new PointSet;
         collect_connected(itBt, region);
         outSeedRegions.insert(region);
      }
   }
}


void individual_seeds_from_regions(SeedRegionSet const& seedRegions, Seeds& outSeeds)
{
   for (SeedRegionSet::const_iterator it = seedRegions.begin(), end = seedRegions.end();
        it != end; ++it) {
      PointSet const* pset = *it;
      for (PointSet::const_iterator it = pset->begin(), end = pset->end();
           it != end; ++it) {
         outSeeds.push_back(*it);
      }
   }
}


void find_seeds(InputImage::Pointer image,
                double threshhold,
                Seeds& outSeeds)
{
   BeatenImage::Pointer beaten_bitmap = threshhold_phase(image, threshhold);
   exploration_phase(image, beaten_bitmap);

   SeedRegionSet seedRegions;
   collect_seed_regions(beaten_bitmap, seedRegions);

   individual_seeds_from_regions(seedRegions, outSeeds);
}


#if defined(TESTING_LOCAL_MAXIMA)

void make_seeds_image()
{
   medianFilter->Update();
   ImageType::Pointer image = medianFilter->GetOutput();
   typedef ImageDuplicator< ImageType > DuplicatorType;
   DuplicatorType::Pointer duplicator = DuplicatorType::New();
   duplicator->SetInput();
   duplicator->Update();
   // also has: duplicator->CreateAnother()
   ImageType::Pointer clonedImage = duplicator->GetOutput();
   CastImageFilter<InputImageType, OutputImageType> cast_filter;
}

int main(int argc, char** argv)
{
   --argc, ++argv;
   assert(1 <= argc);

   double threshhold = 0.3;
   if (2 <= argc) {
      threshhold = atof(argv[1]);
   }

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(argv[0]);
   reader->Update();

   InputImage::Pointer inputImage = reader->GetOutput();
   SeedRegionSet seedRegions;
   find_seeds(inputImage, seedRegions, threshhold);
cout << seedRegions.size() << " seed regions" << endl;

cout << "number explored: " << g_n_explored << endl;
cout << "snuffed: " << g_n_explored << endl;
cout << "components: " << g_n_components << endl;
}

#endif // TESTING_LOCAL_MAXIMA
