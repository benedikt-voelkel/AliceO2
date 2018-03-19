#include "TGeoManager.h"
#include "TCanvas.h"

static const int NCOLOURS = 10;
int colours[NCOLOURS] = { kPink-9, kRed-9, kRed-2, kCyan+3, kBlue+1, 
                    kGreen+2, kGreen-5, kMagenta+2, kViolet+7, kOrange+1 };


void draw_geometry()
{
  std::string fileName;
  std::cout << "Draw Cave..." << std::endl;
  //gGeoManager->SetTopVisible();
  TCanvas* c = new TCanvas( "CaveVol", "CaveVol" );
  c->cd();
  gGeoManager->GetTopNode()->GetVolume()->SetLineColor(kBlack);
  gGeoManager->GetTopNode()->GetVolume()->DrawOnly();
  TObjArray* nodes = gGeoManager->GetTopNode()->GetNodes();
  TIter next( nodes );
  TGeoNode* node = nullptr;
  int colour = 0;
  while( node = (TGeoNode*)next() )
  {
    node->GetVolume()->SetLineColor( colours[ colour++ % NCOLOURS ] );
    node->GetVolume()->DrawOnly("same");
    fileName += "_";
    fileName += node->GetName();
  }
  fileName = "cave" + fileName + ".png";
  c->SaveAs( fileName.c_str() );
}
