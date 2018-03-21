#if !defined(__CLING__) || defined(__ROOTCLING__)
#include "TGeoManager.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGeoMatrix.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"
#include "TGeoPhysicalNode.h"

#endif

static const int NCOLOURS = 10;
int colours[NCOLOURS] = { kPink-9, kRed-9, kRed-2, kCyan+3, kBlue+1, 
                    kGreen+2, kGreen-5, kMagenta+2, kViolet+7, kOrange+1 };


//void draw_geometry(const char* geomROOTFile )
void draw_geometry()
{
  //geom = new TGeoManager( "world", "ALICE Detector" );
  //gGeoManager->Import( geomROOTFile );
  std::string fileName;
  std::cout << "Draw Cave..." << std::endl;
  //gGeoManager->SetTopVisible();
  TCanvas* c = new TCanvas( "ALICE", "ALICE" );
  TPad* pad = new TPad();
  pad->Draw();
  pad->cd();
  gGeoManager->GetTopNode()->GetVolume()->SetLineColor(kBlack);
  TGeoVolume::CreateDummyMedium();

/*
  TGeoRotation* someMatRot = new TGeoRotation( "someMatRot", 70., 30., 0. );
  TGeoTranslation* someMatTrans = new TGeoTranslation( "someMatTrans", 400., -300., -1000. );
  TGeoCombiTrans* someMatCombi = new TGeoCombiTrans();
  //someMatCombi->SetTranslation( *someMatTrans );
  //someMatCombi->SetRotation( *someMatRot );
  //TGeoTube* someTube = new TGeoTube( 0., 500., 500. );
  //TGeoVolume* someTubeVol =  new TGeoVolume( "someTube", someTube, TGeoVolume::DummyMedium() );
  //someTube->SetTransform( someMatCombi );

  //someTubeVol->Voxelize("");
  //gGeoManager->GetTopNode()->GetVolume()->AddNode( someTubeVol, 1, someMatCombi );
  gGeoManager->CloseGeometry();
*/
  TObjArray* nodes = gGeoManager->GetTopNode()->GetNodes();
  TIter next( nodes );
  TGeoNode* node = nullptr;
  int colour = 0;
  gGeoManager->SetVisOption(0);
  gGeoManager->SetVisLevel(2);
  gGeoManager->SetTopVisible();
  while( node = (TGeoNode*)next() )
  {
    if(!node)
    {
      continue;
    }
    //node->SetLineColor( colours[ colour++ % NCOLOURS ] );
    node->GetVolume()->SetLineColor( colours[ colour++ % NCOLOURS ] );

    //node->GetVolume()->Draw("same");
    //node->GetVolume()->SetVisOnly();
    /// Gives segfault...
    //node->Draw("same");
    //fileName += "_";
    //fileName += node->GetName();
  }
  gGeoManager->GetTopNode()->GetVolume()->Draw();
  fileName = "cave";
  if( getenv("CAVEOPT") )
  {
    fileName = fileName + "_optimized";
  }
  TView* view = pad->GetView();
  //view->RotateView( 0., 90.);
  view->ShowAxis();
  pad->Update();
  //fileName = "cave" + fileName + ".png";
  fileName += ".png";
  c->SaveAs( fileName.c_str() );
}
